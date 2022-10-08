#include "ogre/predefine.h"
#include "ogre/comm_manager.h"

#include "ogre/svc_udp.h"
#include "ogre/svc_accept.h"
#include "ogre/svc_tcp.h"
#include "ogre/buf_storage.h"
#include "ogre/configure.h"
#include "ogre/ip_restrict.h"
namespace ogre
{
comm_manager* comm_manager::instance_ = NULL;

comm_manager::comm_manager() :
    ogre_config_(NULL)
{
}

comm_manager::~comm_manager()
{
}

//检查一个端口是否安全.然后根据配置进行处理
int comm_manager::check_safe_port(zce::skt::addr_in& inetadd)
{
    const size_t IP_ADDR_LEN = 31;
    char ip_addr_str[IP_ADDR_LEN + 1];
    size_t use_len = 0;

    //如果打开了保险检查,检查配置的端口
    if (false == inetadd.check_safeport())
    {
        //如果使用保险打开(TRUE)
        if (ogre_config_->ogre_cfg_data_.ogre_insurance_)
        {
            ZCE_LOG(RS_ERROR, "Unsafe port [%s],if you need to open this port,please close insurance. \n",
                    inetadd.to_string(ip_addr_str, IP_ADDR_LEN, use_len));
            return SOAR_RET::ERR_OGRE_UNSAFE_PORT_WARN;
        }
        //如果不使用保险(FALSE)
        else
        {
            //给出警告
            ZCE_LOG(RS_INFO, "Warn!Warn! Unsafe port [%s] listen.Please notice! \n",
                    inetadd.to_string(ip_addr_str, IP_ADDR_LEN, use_len));
        }
    }
    //
    return 0;
}

//得到配置
int comm_manager::get_config(const configure* config)
{
    int ret = 0;

    //设置处理的帧的最大长度
    soar::ogre4a_frame::set_max_framedata_len(
        config->ogre_cfg_data_.max_data_len_);

    //IP限制,
    ret = ip_restrict::instance()->get_config(config);

    if (0 != ret)
    {
        return ret;
    }

    //TCP 读取配置
    ret = svc_tcp::get_config(config);
    if (ret != 0)
    {
        return ret;
    }
    ogre_config_ = config;
    return 0;
}

//将所有的队列中的数据发送，从SEND管道找到所有的数据去发送,
//想了想，还是加了一个最多发送的帧的限额
int comm_manager::get_all_senddata_to_write(size_t& procframe)
{
    int ret = 0;

    //
    for (procframe = 0;
         soar::svrd_buspipe::instance()->is_empty_sendbus() == false &&
         procframe < MAX_ONCE_SEND_FRAME; ++procframe)
    {
        soar::ogre4a_frame* send_frame =
            buffer_storage::instance()->allocate_byte_buffer();

        //
        ret = soar::svrd_buspipe::instance()->pop_front_sendbus(
            reinterpret_cast<soar::zerg_frame *>(send_frame));

        if (ret != 0)
        {
            //归还缓存
            buffer_storage::instance()->free_byte_buffer(send_frame);
            continue;
        }

        //如果FRAME的长度
        if (send_frame->ogre_frame_len_ > soar::ogre4a_frame::MAX_OF_OGRE_FRAME_LEN)
        {
            ZCE_LOG(RS_ALERT, "comm_manager::get_all_senddata_to_write len %u\n",
                    send_frame->ogre_frame_len_);
            DEBUGDUMP_OGRE_HEAD(send_frame, "comm_manager::get_all_senddata_to_write", RS_ALERT);
            ZCE_ASSERT(false);
            return SOAR_RET::ERR_OGRE_SEND_FRAME_TOO_LEN;
        }

        //如果是TCP
        if (send_frame->ogre_frame_option_ & soar::ogre4a_frame::OGREDESC_PEER_TCP)
        {
            ret = svc_tcp::process_send_data(send_frame);

            if (ret != 0)
            {
                //归还缓存
                buffer_storage::instance()->free_byte_buffer(send_frame);
                continue;
            }
        }

        //如果是UDP
        else if (send_frame->ogre_frame_option_ & soar::ogre4a_frame::OGREDESC_PEER_UDP)
        {
            //不检查错误
            svc_udp::send_alldata_to_udp(send_frame);
            buffer_storage::instance()->free_byte_buffer(send_frame);
        }
        //你都不填写，我如何发送？
        else
        {
            ZCE_LOG(RS_ERROR, "Ogre frame have not send option,Please Check you code.\n");
            buffer_storage::instance()->free_byte_buffer(send_frame);
        }
    }

    return 0;
}

//初始化通讯管理器
int comm_manager::init_comm_manger()
{
    int ret = 0;

    //初始化所有的监听端口

    for (unsigned int i = 0; i < ogre_config_->ogre_cfg_data_.accept_peer_num_; ++i)
    {
        svc_accept* accpet_hd = new svc_accept(
            ogre_config_->ogre_cfg_data_.accept_peer_ary_[i]);

        ret = accpet_hd->create_listenpeer();
        if (0 != ret)
        {
            return ret;
        }
    }

    for (unsigned int i = 0; i <= ogre_config_->ogre_cfg_data_.udp_peer_num_; ++i)
    {
        svc_udp* udp_hd = new svc_udp(
            ogre_config_->ogre_cfg_data_.udp_peer_ary_[i]);

        ret = udp_hd->init_udp_peer();
        if (0 != ret)
        {
            return ret;
        }
    }

    //初始化静态数据
    ret = svc_tcp::init_all_static_data();

    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

//注销通讯管理器
int comm_manager::uninit_comm_manger()
{
    //
    svc_tcp::unInit_all_static_data();
    //
    ip_restrict::clear_inst();

    return 0;
}

//得到单子的实例
comm_manager* comm_manager::instance()
{
    if (instance_ == NULL)
    {
        instance_ = new comm_manager();
    }

    return instance_;
}

//清理单子的实例
void comm_manager::clear_inst()
{
    if (instance_)
    {
        delete instance_;
        instance_ = NULL;
    }
}
}