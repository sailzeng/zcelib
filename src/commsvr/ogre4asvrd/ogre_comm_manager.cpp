#include "ogre_predefine.h"
#include "ogre_comm_manager.h"

#include "ogre_udp_ctrl_handler.h"
#include "ogre_accept_handler.h"
#include "ogre_tcp_ctrl_handler.h"
#include "ogre_buf_storage.h"
#include "ogre_configure.h"
#include "ogre_ip_restrict.h"

Ogre_Comm_Manger *Ogre_Comm_Manger::instance_ = NULL;

Ogre_Comm_Manger::Ogre_Comm_Manger()
{
}

Ogre_Comm_Manger::~Ogre_Comm_Manger()
{

}

//检查一个端口是否安全.然后根据配置进行处理
int Ogre_Comm_Manger::check_safe_port(ZCE_Sockaddr_In &inetadd)
{

    //如果打开了保险检查,检查配置的端口
    if ( false == inetadd.check_safeport() )
    {
        //如果使用保险打开(TRUE)
        if (ogre_config_->ogre_cfg_data_.ogre_insurance_)
        {
            ZLOG_ERROR( "Unsafe port [%s|%u],if you need to open this port,please close insurance. \n",
                        inetadd.get_host_addr(),
                        inetadd.get_port_number());
            return SOAR_RET::ERR_OGRE_UNSAFE_PORT_WARN;
        }
        //如果不使用保险(FALSE)
        else
        {
            //给出警告
            ZLOG_INFO( "Warn!Warn! Unsafe port [%s|%u] listen.Please notice! \n",
                       inetadd.get_host_addr(),
                       inetadd.get_port_number());
        }
    }
    //
    return 0;
}

//得到配置
int Ogre_Comm_Manger::get_config(const Ogre_Server_Config *config)
{
    int ret = 0;

    //IP限制,
    ret = Ogre_IPRestrict_Mgr::instance()->get_config(config);

    if (0 != ret )
    {
        return ret;
    }

    //TCP 读取配置
    ret = Ogre_TCP_Svc_Handler::get_config(config);
    if (ret != 0)
    {
        return ret;
    }

    for (unsigned int i = 1; i <= accept_num; ++i)
    {
        OgreTCPAcceptHandler *accpet_hd = new OgreTCPAcceptHandler(accept_bind_addr, tmp_value);
        accept_handler_ary_.push_back(accpet_hd);
    }


    for (unsigned int i = 1; i <= accept_num; ++i)
    {
        OgreUDPSvcHandler *udp_hd = new OgreUDPSvcHandler(udp_bind_addr);
        udp_handler_ary_.push_back(udp_hd);
    }
    return 0;
}


//将所有的队列中的数据发送，从SEND管道找到所有的数据去发送,
//想了想，还是加了一个最多发送的帧的限额
int Ogre_Comm_Manger::get_all_senddata_to_write(size_t &procframe)
{
    int ret = 0;

    //
    for (procframe = 0;
         Soar_MMAP_BusPipe::instance()->is_empty_bus(Soar_MMAP_BusPipe::SEND_PIPE_ID) == false &&
         procframe < MAX_ONCE_SEND_FRAME; ++procframe)
    {
        Ogre4a_App_Frame *send_frame = Ogre_Buffer_Storage::instance()->allocate_byte_buffer();

        //
        ret = Soar_MMAP_BusPipe::instance()->pop_front_bus(Soar_MMAP_BusPipe::SEND_PIPE_ID,
                                                           reinterpret_cast< ZCE_LIB::dequechunk_node*&>(send_frame));

        if (ret != 0)
        {
            //归还缓存
            Ogre_Buffer_Storage::instance()->free_byte_buffer(send_frame);
            continue;
        }

        //如果FRAME的长度
        if (send_frame->ogre_frame_len_ > Ogre4a_App_Frame::MAX_OF_OGRE_FRAME_LEN)
        {
            ZLOG_ALERT("Ogre_Comm_Manger::get_all_senddata_to_write len %u\n",
                       send_frame->ogre_frame_len_);
            DEBUGDUMP_OGRE_HEAD(send_frame, "Ogre_Comm_Manger::get_all_senddata_to_write", RS_ALERT);
            ZCE_ASSERT(false);
            return SOAR_RET::ERR_OGRE_SEND_FRAME_TOO_LEN;
        }

        //如果是TCP
        if (send_frame->ogre_frame_option_ & Ogre4a_App_Frame::OGREDESC_PEER_TCP )
        {
            ret = Ogre_TCP_Svc_Handler::process_send_data(send_frame);

            if (ret != 0)
            {
                //归还缓存
                Ogre_Buffer_Storage::instance()->free_byte_buffer(send_frame);
                continue;
            }
        }

        //如果是UDP
        else if (send_frame->ogre_frame_option_ & Ogre4a_App_Frame::OGREDESC_PEER_UDP )
        {
            //不检查错误
            OgreUDPSvcHandler::SendAllDataToUDP(send_frame);
            Ogre_Buffer_Storage::instance()->free_byte_buffer(send_frame);
        }
        //你都不填写，我如何发送？
        else
        {
            ZLOG_ERROR( "Ogre frame have not send option,Please Check you code.\n");
            Ogre_Buffer_Storage::instance()->free_byte_buffer(send_frame);
        }
    }

    return 0;
}

//初始化通讯管理器
int Ogre_Comm_Manger::init_comm_manger()
{
    int ret = 0;

    //初始化所有的监听端口
    for (size_t i = 0; i < accept_handler_ary_.size(); ++i)
    {
        ret = accept_handler_ary_[i]->create_listenpeer();

        if (ret != 0)
        {
            return ret;
        }
    }

    ret = OgreUDPSvcHandler::OpenUDPSendPeer();
    if (ret != 0)
    {
        return ret;
    }

    //初始化所有的UDP端口
    for (size_t i = 0; i < udp_handler_ary_.size(); ++i)
    {
        ret = udp_handler_ary_[i]->InitUDPServices();

        if (ret != 0)
        {
            return ret;
        }
    }

    //初始化静态数据
    ret = Ogre_TCP_Svc_Handler::init_all_static_data();

    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

//注销通讯管理器
int Ogre_Comm_Manger::uninit_comm_manger()
{
    int ret = 0;

    //初始化所有的监听端口
    for (size_t i = 0; i < accept_handler_ary_.size(); ++i)
    {
        ret = accept_handler_ary_[i]->handle_close();

        if (ret != 0)
        {
            return ret;
        }
    }

    //初始化所有的UDP端口
    for (size_t i = 0; i < udp_handler_ary_.size(); ++i)
    {
        ret = udp_handler_ary_[i]->handle_close();

        if (ret != 0)
        {
            return ret;
        }
    }

    //
    Ogre_TCP_Svc_Handler::unInit_all_static_data();
    //
    Ogre_IPRestrict_Mgr::clean_instance();

    return 0;
}

//得到单子的实例
Ogre_Comm_Manger *Ogre_Comm_Manger::instance()
{
    if (instance_ == NULL)
    {
        instance_ = new Ogre_Comm_Manger();
    }

    return instance_;
}

//清理单子的实例
void Ogre_Comm_Manger::clean_instance()
{
    if (instance_)
    {
        delete instance_;
        instance_ = NULL;
    }
}

