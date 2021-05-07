#include "zerg/predefine.h"
#include "zerg/tcp_ctrl_handler.h"
#include "zerg/udp_ctrl_handler.h"
#include "zerg/accept_handler.h"
#include "zerg/app_timer.h"
#include "zerg/comm_manager.h"

namespace zerg
{
//实例
Comm_Manager *Comm_Manager::instance_ = NULL;

Comm_Manager::Comm_Manager():
    error_try_num_(3),
    monitor_size_(0),
    zerg_mmap_pipe_(NULL),
    zbuffer_storage_(NULL),
    server_status_(NULL),
    send_frame_count_(0),
    zerg_config_(NULL)

{
    zerg_mmap_pipe_ = soar::App_BusPipe::instance();
    zbuffer_storage_ = zerg::Buffer_Storage::instance();
    server_status_ = soar::Stat_Monitor::instance();
    count_start_time_ = static_cast<unsigned int>(zerg::App_Timer::now_time_.sec());

    memset(monitor_cmd_,0,sizeof(monitor_cmd_));
}

Comm_Manager::~Comm_Manager()
{
    //先关闭ACCEPET
    for (size_t i = 0; i < zerg_acceptor_.size(); ++i)
    {
        zerg_acceptor_[i]->handle_close();
    }

    //先关闭UDP
    for (size_t i = 0; i < zerg_updsvc_.size(); ++i)
    {
        zerg_updsvc_[i]->handle_close();
    }
}

//初始化
int Comm_Manager::get_config(const Zerg_Config *config)
{
    zerg_config_ = config;

    //清理监控命令
    monitor_size_ = 0;
    memset(monitor_cmd_,0,sizeof(monitor_cmd_));

    //错误发送数据尝试发送次数
    error_try_num_ = config->zerg_cfg_data_.retry_error_;

    //错误发送数据尝试发送次数
    monitor_size_ = config->zerg_cfg_data_.monitor_cmd_count_;

    ZCE_LOG(RS_INFO,"[zergsvr] Comm_Manager::get_config monitor_size_ = %u",monitor_size_);

    //读取监控的命令，监控的命令数量必须很少，最好等于==0，比较耗时。你可以对单机进行监控。
    //但是最好不要对所有的机器进行监控，
    for (size_t i = 0; i < monitor_size_; ++i)
    {
        monitor_cmd_[i] = config->zerg_cfg_data_.monitor_cmd_list_[i];
    }

    return 0;
}

int Comm_Manager::init_allpeer()
{
    int ret = 0;
    //初始化所有的监听端口
    for (unsigned int i = 0; i < zerg_config_->zerg_cfg_data_.bind_svcid_num_; ++i)
    {
        ret = init_socketpeer(zerg_config_->zerg_cfg_data_.bind_svcid_ary_[i]);
        if (ret != 0)
        {
            return ret;
        }
    }

    return 0;
}

//
int Comm_Manager::init_socketpeer(const soar::SERVICES_ID &init_svcid)
{
    int ret = 0;

    soar::SERVICES_INFO svc_info;
    ret = zerg_config_->get_svcinfo_by_svcid(init_svcid,svc_info);
    if (0 != ret)
    {
        return ret;
    }

    //检查一下端口
    ret = check_safeport(svc_info.ip_address_);
    if (ret != 0)
    {
        return ret;
    }

    //如果是TCP的接口
    if (init_svcid.services_type_ < SVC_UDP_SERVER_BEGIN)
    {
        //设置Bind地址
        TCP_Accept_Handler *ptr_acceptor = new TCP_Accept_Handler(init_svcid,
                                                                  svc_info.ip_address_);
        //采用同步的方式创建LISTER PEER
        ret = ptr_acceptor->create_listen();

        if (ret != 0)
        {
            ZCE_LOG(RS_ERROR,"[zergsvr] Init tcp accept scoket fail ret = %d.error[%u|%s]",
                    ret,
                    zce::last_error(),
                    strerror(zce::last_error())
            );
            return  SOAR_RET::ERR_ZERG_INIT_ACCEPT_SOCKET_FAIL;
        }

        zerg_acceptor_.push_back(ptr_acceptor);
    }
    //如果是UDP的接口
    else
    {
        //
        UDP_Svc_Handler *tmp_udphdl = new UDP_Svc_Handler(init_svcid,
                                                          svc_info.ip_address_);

        //初始化UDP的端口
        ret = tmp_udphdl->init_udp_services();

        if (ret != 0)
        {
            ZCE_LOG(RS_ERROR,"[zergsvr] Init udp scoket fail ret = %d.",ret);
            return  SOAR_RET::ERR_ZERG_INIT_UPD_PORT_FAIL;
        }

        zerg_updsvc_.push_back(tmp_udphdl);
    }

    return 0;
}

//检查一个端口是否安全
int Comm_Manager::check_safeport(const zce::Sockaddr_In &inetadd)
{
    //高危端口检查常量
    const unsigned short UNSAFE_PORT1 = 1024;
    const unsigned short UNSAFE_PORT2 = 3306;
    const unsigned short UNSAFE_PORT3 = 36000;
    const unsigned short UNSAFE_PORT4 = 56000;
    const unsigned short SAFE_PORT1 = 80;

    //如果打开了保险检查,检查配置的端口
    if ((inetadd.get_port_number() <= UNSAFE_PORT1 && inetadd.get_port_number() != SAFE_PORT1) ||
        inetadd.get_port_number() == UNSAFE_PORT2 ||
        inetadd.get_port_number() == UNSAFE_PORT3 ||
        inetadd.get_port_number() == UNSAFE_PORT4)
    {
        //如果使用保险打开(TRUE)
        if (zerg_config_->zerg_cfg_data_.zerg_insurance_)
        {
            ZCE_LOG(RS_ERROR,"[zergsvr] Unsafe port %u,if you need to open this port,please close insurance. ",
                    inetadd.get_port_number());
            return SOAR_RET::ERR_ZERG_UNSAFE_PORT_WARN;
        }
        //如果不使用保险(FALSE)
        else
        {
            //给出警告
            ZCE_LOG(RS_INFO,"[zergsvr] Warn!Warn! Unsafe port %u.Please notice! ",
                    inetadd.get_port_number());
        }
    }

    //
    return 0;
}

//取得发送数据进行发送
int Comm_Manager::popall_sendpipe_write(const size_t want_send_frame,size_t &num_send_frame)
{
    num_send_frame = 0;
    int ret = 0;

    while (zerg_mmap_pipe_->is_empty_sendbus() == false
           && num_send_frame < want_send_frame)
    {
        zerg::Buffer *tmpbuf = zbuffer_storage_->allocate_buffer();
        soar::Zerg_Frame *proc_frame = reinterpret_cast<soar::Zerg_Frame *>(tmpbuf->buffer_data_);

        //注意压入的数据不要大于APPFRAME允许的最大长度,对于这儿我权衡选择效率
        zerg_mmap_pipe_->pop_front_sendbus(proc_frame);

        tmpbuf->size_of_use_ = proc_frame->length_;

        //如果是要跟踪的命令
        if (proc_frame->u32_option_ & soar::Zerg_Frame::DESC_TRACK_MONITOR)
        {
            DUMP_ZERG_FRAME_HEAD(RS_INFO,"[TRACK MONITOR][SEND]opt",proc_frame);
        }
        else
        {
            for (size_t i = 0; i < monitor_size_; ++i)
            {
                if (monitor_cmd_[i] == proc_frame->command_)
                {
                    DUMP_ZERG_FRAME_HEAD(RS_INFO,"[TRACK MONITOR][SEND]cmd",proc_frame);
                }
            }
        }

        //发送UDP的数据
        if (proc_frame->frame_option_.protocol_ == soar::Zerg_Frame::DESC_UDP_FRAME)
        {
            //发送错误日志在send_all_to_udp函数内部处理，这儿不增加重复记录
            UDP_Svc_Handler::send_all_to_udp(proc_frame);
            //UDP的包归还缓存
            zbuffer_storage_->free_byte_buffer(tmpbuf);
        }
        else
        {
            //// 如果是广播的话，给对应Svr_type的所有Svr发
            //if (proc_frame->recv_service_.services_id_ == soar::SERVICES_ID::BROADCAST_SERVICES_ID)
            //{
            //    std::vector<soar::SERVICES_ID> vec;
            //    ret = TCP_Svc_Handler::get_zerg_auto_connect().get_all_conn_server(proc_frame->recv_service_.services_type_, vec);
            //    if (ret != 0)
            //    {
            //        ZCE_LOG(RS_ERROR,"[%s] fetch broadcast pkg error, recv svrinfo:[%u|%u]",
            //            __ZCE_FUNCTION__,
            //            proc_frame->recv_service_.services_type_,
            //            proc_frame->recv_service_.services_id_);

            //        continue;
            //    }

            //    uint32_t size = (uint32_t)vec.size();
            //    for (uint32_t i = 0; i < size; i ++)
            //    {
            //        if (i == size -1)
            //        {
            //            reinterpret_cast<soar::Zerg_Frame *>(tmpbuf->buffer_data_)->recv_service_.services_id_ = vec[i].services_id_;
            //            // last Buf就用本身的tmpbuf来发，稍微加快速度
            //            ret = send_single_buf(tmpbuf);
            //        }
            //        else
            //        {
            //            zerg::Buffer *send_buf = zbuffer_storage_->allocate_buffer();
            //            memcpy(send_buf->buffer_data_, tmpbuf->buffer_data_, tmpbuf->size_of_use_);
            //            reinterpret_cast<soar::Zerg_Frame *>(send_buf->buffer_data_)->recv_service_.services_id_ = vec[i].services_id_;
            //            send_buf->size_of_use_ = tmpbuf->size_of_use_;
            //            ret = send_single_buf(send_buf);
            //        }

            //        if (ret != 0)
            //        {
            //            continue;
            //        }

            //        ++send_frame_count_;
            //    }
            //}
            //else
            {
                ret = send_single_buf(tmpbuf);
                if (ret != 0)
                {
                    // 没有发成功，不加入后面的统计
                    continue;
                }

                ++send_frame_count_;
            }
        }

        ++num_send_frame;
    }

    return 0;
}

//检查发包频率
void Comm_Manager::check_freamcount(unsigned int now)
{
    if (now <= count_start_time_)
    {
        ZCE_LOG(RS_ERROR,"[zergsvr] Comm_Manager::check_freamcount time err.now:%u, count_start_time_:%u",
                now,
                count_start_time_);
        return;
    }

    unsigned int interval = now - count_start_time_;
    unsigned int frame_per_sec = send_frame_count_ / interval;

    if (frame_per_sec > SEND_FRAME_ALERT_VALUE)
    {
        ZCE_LOG(RS_ALERT,"[zergsvr] Comm_Manager::check_freamcount ALERT frame_per_sec:%u, send_frame_count_:%u, interval:%u.",
                frame_per_sec,
                send_frame_count_,
                interval);
    }
    else
    {
        ZCE_LOG(RS_INFO,"[zergsvr] Comm_Manager::check_freamcount frame_per_sec:%u, send_frame_count_:%u, interval:%u.",
                frame_per_sec,
                send_frame_count_,
                interval);
    }

    send_frame_count_ = 0;
    count_start_time_ = now;
}

//单子实例函数
Comm_Manager *Comm_Manager::instance()
{
    //如果没有初始化
    if (instance_ == NULL)
    {
        instance_ = new Comm_Manager();
    }

    return instance_;
}

//清理实例
void Comm_Manager::clean_instance()
{
    if (instance_)
    {
        delete instance_;
        instance_ = NULL;
    }
}

int Comm_Manager::send_single_buf(zerg::Buffer *tmpbuf)
{
    //发送错误日志在process_send_data函数内部处理，这儿不增加重复记录
    int ret = TCP_Svc_Handler::process_send_data(tmpbuf);

    soar::Zerg_Frame *proc_frame = reinterpret_cast<soar::Zerg_Frame *>(tmpbuf->buffer_data_);

    //如果失败归还缓存，如果成功的情况下，会放入发送队列，放入发送队列的归还和这个不一样
    if (ret != 0)
    {
        //记录下来处理
        if (proc_frame->u32_option_ & soar::Zerg_Frame::DESC_SEND_FAIL_RECORD)
        {
            ZCE_LOG(RS_ERROR,"[zergsvr] A Frame frame len[%u] cmd[%u] uid[%u] recv_service[%u|%u] proxy_service[%u|%u] send_service[%u|%u] option [%u],ret =%d Discard!",
                    proc_frame->length_,
                    proc_frame->command_,
                    proc_frame->command_,
                    proc_frame->recv_service_.services_type_,
                    proc_frame->recv_service_.services_id_,
                    proc_frame->proxy_service_.services_type_,
                    proc_frame->proxy_service_.services_id_,
                    proc_frame->send_service_.services_type_,
                    proc_frame->send_service_.services_id_,
                    proc_frame->frame_option_,
                    ret
            );
        }

        //
        server_status_->add_one(ZERG_SEND_FAIL_COUNTER,
                                proc_frame->business_id_,
                                0);
        if (proc_frame->recv_service_.services_type_ == 0)
        {
            // 不应该出现0的services_type
            server_status_->add_one(ZERG_SEND_FAIL_COUNTER_BY_SVR_TYPE,
                                    proc_frame->business_id_,proc_frame->recv_service_.services_type_);
        }
        //
        zbuffer_storage_->free_byte_buffer(tmpbuf);
        return ret;
    }

    return 0;
}

//
void Comm_Manager::pushback_recvpipe(soar::Zerg_Frame *recv_frame)
{
    // 如果是通信服务器的命令,不进行任何处理
    if (true == recv_frame->is_zerg_processcmd())
    {
        return;
    }

    //为了提高效率，先检查标志位，
    if (recv_frame->u32_option_ & soar::Zerg_Frame::DESC_TRACK_MONITOR)
    {
        DUMP_ZERG_FRAME_HEAD(RS_INFO,"[TRACK MONITOR][RECV]opt",recv_frame);
    }
    else
    {
        //这段代码在发送接收时都要检查，否则都有可能漏掉
        //如果是要跟踪的命令
        for (size_t i = 0; i < monitor_size_; ++i)
        {
            if (monitor_cmd_[i] == recv_frame->command_)
            {
                DUMP_ZERG_FRAME_HEAD(RS_INFO,"[TRACK MONITOR][RECV]cmd",recv_frame);
            }
        }
    }

    int ret = zerg_mmap_pipe_->push_back_recvbus(recv_frame);
    if (ret != 0)
    {
        server_status_->add_one(ZERG_RECV_PIPE_FULL_COUNTER,
                                recv_frame->business_id_,
                                0);
    }
    else
    {
        server_status_->add_one(ZERG_RECV_FRAME_COUNTER,
                                recv_frame->business_id_,
                                0);
        server_status_->add_one(ZERG_RECV_FRAME_COUNTER_BY_CMD,
                                recv_frame->business_id_,
                                recv_frame->command_);
        server_status_->add_one(ZERG_RECV_FRAME_COUNTER_BY_SVR_TYPE,
                                recv_frame->business_id_,
                                recv_frame->send_service_.services_type_);
    }
}
}