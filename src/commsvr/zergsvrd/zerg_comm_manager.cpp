
#include "zerg_predefine.h"
#include "zerg_tcp_ctrl_handler.h"
#include "zerg_udp_ctrl_handler.h"
#include "zerg_accept_handler.h"
#include "zerg_app_timer.h"
#include "zerg_comm_manager.h"


//ʵ��
Zerg_Comm_Manager *Zerg_Comm_Manager::instance_ = NULL;

Zerg_Comm_Manager::Zerg_Comm_Manager():
    error_try_num_(3),
    monitor_size_(0),
    zerg_mmap_pipe_(NULL),
    zbuffer_storage_(NULL),
    server_status_(NULL),
    send_frame_count_(0),
    zerg_config_(NULL)

{
    zerg_mmap_pipe_ = Soar_MMAP_BusPipe::instance();
    zbuffer_storage_ = ZBuffer_Storage::instance();
    server_status_ = Soar_Stat_Monitor::instance();
    count_start_time_ = static_cast<unsigned int>(Zerg_App_Timer_Handler::now_time_.sec());

    memset(monitor_cmd_, 0, sizeof(monitor_cmd_));
}

Zerg_Comm_Manager::~Zerg_Comm_Manager()
{
    //�ȹر�ACCEPET
    for (size_t i = 0; i < zerg_acceptor_.size(); ++i)
    {
        zerg_acceptor_[i]->handle_close();
    }

    //�ȹر�UDP
    for (size_t i = 0; i < zerg_updsvc_.size(); ++i)
    {
        zerg_updsvc_[i]->handle_close();
    }
}

//��ʼ��
int Zerg_Comm_Manager::get_config(const Zerg_Server_Config *config)
{

    zerg_config_ = config;

    //����������
    monitor_size_ = 0;
    memset(monitor_cmd_, 0, sizeof(monitor_cmd_));

    //���������ݳ��Է��ʹ���
    error_try_num_ = config->zerg_cfg_data_.retry_error_;


    //���������ݳ��Է��ʹ���
    monitor_size_ = config->zerg_cfg_data_.monitor_cmd_count_;

    ZCE_LOG(RS_INFO, "[zergsvr] Zerg_Comm_Manager::get_config monitor_size_ = %u", monitor_size_);

    //��ȡ��ص������ص���������������٣���õ���==0���ȽϺ�ʱ������ԶԵ������м�ء�
    //������ò�Ҫ�����еĻ������м�أ�
    for (size_t i = 0; i < monitor_size_; ++i)
    {
        monitor_cmd_[i] = config->zerg_cfg_data_.monitor_cmd_list_[i];
    }


    return 0;
}

int Zerg_Comm_Manager::init_allpeer()
{
    int ret = 0;
    //��ʼ�����еļ����˿�
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
int Zerg_Comm_Manager::init_socketpeer(const SERVICES_ID &init_svcid)
{
    int ret = 0;


    SERVICES_INFO svc_info;
    ret = zerg_config_->get_svcinfo_by_svcid(init_svcid, svc_info);
    if (0 != ret)
    {
        return ret;
    }

    //���һ�¶˿�
    ret = check_safeport(svc_info.ip_address_);
    if (ret != 0)
    {
        return ret;
    }

    //�����TCP�Ľӿ�
    if (init_svcid.services_type_ < SVC_UDP_SERVER_BEGIN )
    {
        //����Bind��ַ
        TCP_Accept_Handler *ptr_acceptor = new TCP_Accept_Handler(init_svcid,
                                                                  svc_info.ip_address_);
        //����ͬ���ķ�ʽ����LISTER PEER
        ret = ptr_acceptor->create_listen();

        if (ret != 0 )
        {
            ZCE_LOG(RS_ERROR, "[zergsvr] Init tcp accept scoket fail ret = %d.error[%u|%s]",
                    ret,
                    zce::last_error(),
                    strerror(zce::last_error())
                   );
            return  SOAR_RET::ERR_ZERG_INIT_ACCEPT_SOCKET_FAIL;
        }

        zerg_acceptor_.push_back(ptr_acceptor);
    }
    //�����UDP�Ľӿ�
    else
    {
        //
        UDP_Svc_Handler *tmp_udphdl =  new UDP_Svc_Handler(init_svcid,
                                                           svc_info.ip_address_);

        //��ʼ��UDP�Ķ˿�
        ret = tmp_udphdl->init_udp_services();

        if (ret != 0 )
        {
            ZCE_LOG(RS_ERROR, "[zergsvr] Init udp scoket fail ret = %d.", ret);
            return  SOAR_RET::ERR_ZERG_INIT_UPD_PORT_FAIL;
        }

        zerg_updsvc_.push_back(tmp_udphdl);
    }

    return 0;
}

//���һ���˿��Ƿ�ȫ
int Zerg_Comm_Manager::check_safeport(const ZCE_Sockaddr_In  &inetadd)
{
    //��Σ�˿ڼ�鳣��
    const unsigned short UNSAFE_PORT1 = 1024;
    const unsigned short UNSAFE_PORT2 = 3306;
    const unsigned short UNSAFE_PORT3 = 36000;
    const unsigned short UNSAFE_PORT4 = 56000;
    const unsigned short SAFE_PORT1 = 80;

    //������˱��ռ��,������õĶ˿�
    if ((inetadd.get_port_number() <= UNSAFE_PORT1 && inetadd.get_port_number() != SAFE_PORT1)  ||
        inetadd.get_port_number() == UNSAFE_PORT2  ||
        inetadd.get_port_number() == UNSAFE_PORT3 ||
        inetadd.get_port_number() == UNSAFE_PORT4 )
    {
        //���ʹ�ñ��մ�(TRUE)
        if (zerg_config_->zerg_cfg_data_.zerg_insurance_)
        {
            ZCE_LOG(RS_ERROR, "[zergsvr] Unsafe port %u,if you need to open this port,please close insurance. ",
                    inetadd.get_port_number());
            return SOAR_RET::ERR_ZERG_UNSAFE_PORT_WARN;
        }
        //�����ʹ�ñ���(FALSE)
        else
        {
            //��������
            ZCE_LOG(RS_INFO, "[zergsvr] Warn!Warn! Unsafe port %u.Please notice! ",
                    inetadd.get_port_number());
        }
    }

    //
    return 0;
}


//ȡ�÷������ݽ��з���
int Zerg_Comm_Manager::popall_sendpipe_write(const size_t want_send_frame, size_t &num_send_frame)
{
    num_send_frame = 0;
    int ret = 0;

    while (zerg_mmap_pipe_->is_empty_bus(Soar_MMAP_BusPipe::SEND_PIPE_ID) == false && num_send_frame < want_send_frame )
    {

        Zerg_Buffer *tmpbuf = zbuffer_storage_->allocate_buffer();
        Zerg_App_Frame *proc_frame = reinterpret_cast<Zerg_App_Frame *>( tmpbuf->buffer_data_);

        //ע��ѹ������ݲ�Ҫ����APPFRAME�������󳤶�,���������Ȩ��ѡ��Ч��
        zerg_mmap_pipe_->pop_front_bus(Soar_MMAP_BusPipe::SEND_PIPE_ID,
                                       reinterpret_cast< zce::lockfree::dequechunk_node *&>(
                                           proc_frame));

        tmpbuf->size_of_use_ = proc_frame->frame_length_;

        //�����Ҫ���ٵ�����
        if (proc_frame->frame_option_ & Zerg_App_Frame::DESC_MONITOR_TRACK)
        {
            proc_frame->dumpoutput_framehead("[TRACK MONITOR][SEND]opt", RS_INFO);
        }
        else
        {
            for (size_t i = 0; i < monitor_size_; ++i)
            {
                if (monitor_cmd_[i] == proc_frame->frame_command_)
                {
                    proc_frame->frame_option_ |= Zerg_App_Frame::DESC_MONITOR_TRACK;
                    proc_frame->dumpoutput_framehead("[TRACK MONITOR][SEND]cmd", RS_INFO);
                }
            }
        }

        //����UDP������
        if (proc_frame->frame_option_ & Zerg_App_Frame::DESC_UDP_FRAME)
        {
            //���ʹ�����־��send_all_to_udp�����ڲ���������������ظ���¼
            UDP_Svc_Handler::send_all_to_udp(proc_frame);
            //UDP�İ��黹����
            zbuffer_storage_->free_byte_buffer(tmpbuf);
        }
        else
        {
            //// ����ǹ㲥�Ļ�������ӦSvr_type������Svr��
            //if (proc_frame->recv_service_.services_id_ == SERVICES_ID::BROADCAST_SERVICES_ID)
            //{
            //    std::vector<SERVICES_ID> vec;
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
            //            reinterpret_cast<Zerg_App_Frame *>(tmpbuf->buffer_data_)->recv_service_.services_id_ = vec[i].services_id_;
            //            // last Buf���ñ����tmpbuf��������΢�ӿ��ٶ�
            //            ret = send_single_buf(tmpbuf);
            //        }
            //        else
            //        {
            //            Zerg_Buffer *send_buf = zbuffer_storage_->allocate_buffer();
            //            memcpy(send_buf->buffer_data_, tmpbuf->buffer_data_, tmpbuf->size_of_use_);
            //            reinterpret_cast<Zerg_App_Frame *>(send_buf->buffer_data_)->recv_service_.services_id_ = vec[i].services_id_;
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
                    // û�з��ɹ�������������ͳ��
                    continue;
                }

                ++send_frame_count_;
            }

        }

        ++num_send_frame;
    }

    return 0;
}

//��鷢��Ƶ��
void Zerg_Comm_Manager::check_freamcount(unsigned int now)
{
    if (now <= count_start_time_)
    {
        ZCE_LOG(RS_ERROR, "[zergsvr] Zerg_Comm_Manager::check_freamcount time err.now:%u, count_start_time_:%u",
                now,
                count_start_time_);
        return;
    }

    unsigned int interval = now - count_start_time_;
    unsigned int frame_per_sec = send_frame_count_ / interval;

    if (frame_per_sec > SEND_FRAME_ALERT_VALUE)
    {
        ZCE_LOG(RS_ALERT, "[zergsvr] Zerg_Comm_Manager::check_freamcount ALERT frame_per_sec:%u, send_frame_count_:%u, interval:%u.",
                frame_per_sec,
                send_frame_count_,
                interval);
    }
    else
    {
        ZCE_LOG(RS_INFO, "[zergsvr] Zerg_Comm_Manager::check_freamcount frame_per_sec:%u, send_frame_count_:%u, interval:%u.",
                frame_per_sec,
                send_frame_count_,
                interval);
    }

    send_frame_count_ = 0;
    count_start_time_ = now;
}


//����ʵ������
Zerg_Comm_Manager *Zerg_Comm_Manager::instance()
{
    //���û�г�ʼ��
    if (instance_ == NULL)
    {
        instance_ = new Zerg_Comm_Manager();
    }

    return instance_;
}

//����ʵ��
void Zerg_Comm_Manager::clean_instance()
{
    if (instance_)
    {
        delete instance_;
        instance_ = NULL;
    }
}



int Zerg_Comm_Manager::send_single_buf( Zerg_Buffer *tmpbuf )
{
    //���ʹ�����־��process_send_data�����ڲ���������������ظ���¼
    int ret = TCP_Svc_Handler::process_send_data(tmpbuf);

    Zerg_App_Frame *proc_frame = reinterpret_cast<Zerg_App_Frame *>(tmpbuf->buffer_data_);

    //���ʧ�ܹ黹���棬����ɹ�������£�����뷢�Ͷ��У����뷢�Ͷ��еĹ黹�������һ��
    if (ret != 0)
    {
        //��¼��������
        if (proc_frame->frame_option_ & Zerg_App_Frame::DESC_SEND_FAIL_RECORD )
        {
            ZCE_LOG(RS_ERROR, "[zergsvr] A Frame frame len[%u] cmd[%u] uid[%u] recv_service[%u|%u] proxy_service[%u|%u] send_service[%u|%u] option [%u],ret =%d Discard!",
                    proc_frame->frame_length_,
                    proc_frame->frame_command_,
                    proc_frame->frame_command_,
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
        server_status_->increase_once(ZERG_SEND_FAIL_COUNTER,
                                      proc_frame->app_id_,
                                      0);
        if (proc_frame->recv_service_.services_type_ == 0)
        {
            // ��Ӧ�ó���0��services_type
            server_status_->increase_once(ZERG_SEND_FAIL_COUNTER_BY_SVR_TYPE,
                                          proc_frame->app_id_, proc_frame->recv_service_.services_type_);
        }
        //
        zbuffer_storage_->free_byte_buffer(tmpbuf);

        return ret;
    }

    return 0;
}


//
void Zerg_Comm_Manager::pushback_recvpipe(Zerg_App_Frame *recv_frame)
{
    // �����ͨ�ŷ�����������,�������κδ���
    if (true == recv_frame->is_zerg_processcmd())
    {
        return;
    }

    //Ϊ�����Ч�ʣ��ȼ���־λ��
    if (recv_frame->frame_option_ & Zerg_App_Frame::DESC_MONITOR_TRACK)
    {
        recv_frame->dumpoutput_framehead("[TRACK MONITOR][RECV]opt", RS_INFO);
    }
    else
    {
        //��δ����ڷ��ͽ���ʱ��Ҫ��飬�����п���©��
        //�����Ҫ���ٵ�����
        for (size_t i = 0; i < monitor_size_; ++i)
        {
            if (monitor_cmd_[i] == recv_frame->frame_command_)
            {
                recv_frame->frame_option_ |= Zerg_App_Frame::DESC_MONITOR_TRACK;
                recv_frame->dumpoutput_framehead("[TRACK MONITOR][RECV]cmd", RS_INFO);
            }
        }
    }

    int ret = zerg_mmap_pipe_->push_back_bus(Soar_MMAP_BusPipe::RECV_PIPE_ID,
                                             reinterpret_cast<const zce::lockfree::dequechunk_node *>(recv_frame));

    if (ret != 0)
    {
        server_status_->increase_once(ZERG_RECV_PIPE_FULL_COUNTER,
                                      recv_frame->app_id_,
                                      0);
    }
    else
    {
        server_status_->increase_once(ZERG_RECV_FRAME_COUNTER,
                                      recv_frame->app_id_,
                                      0);
        server_status_->increase_once(ZERG_RECV_FRAME_COUNTER_BY_CMD,
                                      recv_frame->app_id_,
                                      recv_frame->frame_command_);
        server_status_->increase_once(ZERG_RECV_FRAME_COUNTER_BY_SVR_TYPE,
                                      recv_frame->app_id_,
                                      recv_frame->send_service_.services_type_);
    }

}


