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

/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2007年4月12日
Function        : Ogre_Comm_Manger::check_safe_port
Return          : int
Parameter List  :
Param1: ZEN_Sockaddr_In& ipaddr
Description     : 检查一个端口是否安全，
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
int Ogre_Comm_Manger::check_safe_port(ZEN_Sockaddr_In &inetadd)
{
    //高危端口检查常量
    const unsigned short UNSAFE_PORT1 = 1024;
    const unsigned short UNSAFE_PORT2 = 3306;
    const unsigned short UNSAFE_PORT3 = 36000;
    const unsigned short UNSAFE_PORT4 = 56000;
    const unsigned short SAFE_PORT1 = 80;

    //如果打开了保险检查,检查配置的端口
    if ((inetadd.get_port_number() <= UNSAFE_PORT1 && inetadd.get_port_number() != SAFE_PORT1)  ||
        inetadd.get_port_number() == UNSAFE_PORT2  ||
        inetadd.get_port_number() == UNSAFE_PORT3 ||
        inetadd.get_port_number() == UNSAFE_PORT4 )
    {
        //如果使用保险打开(TRUE)
        if (Ogre_Svr_Config::instance()->zerg_insurance_)
        {
            ZLOG_ERROR( "Unsafe port [%s|%u],if you need to open this port,please close insurance. \n",
                        inetadd.get_host_addr(),
                        inetadd.get_port_number());
            return TSS_RET::ERR_OGRE_UNSAFE_PORT_WARN;
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
    return TSS_RET::TSS_RET_SUCC;
}

//
int Ogre_Comm_Manger::init_accept_by_conf(Zen_INI_PropertyTree &cfg_file)
{
    int ret = 0;
    unsigned int tmp_uint = 0;

    const size_t TMP_BUFFER_LEN = 256;
    char tmp_key[TMP_BUFFER_LEN + 1];
    char tmp_value[TMP_BUFFER_LEN + 1];
    char err_outbuf[TMP_BUFFER_LEN + 1];

    ret = cfg_file.get_uint32_value("TCPACCEPT", "OPENNUMBER", tmp_uint);
    snprintf(err_outbuf, TMP_BUFFER_LEN, "TCPACCEPT|OPENNUMBER key .");
    TESTCONFIG((ret == 0), err_outbuf);
    unsigned int accept_num = tmp_uint;

    //如果没有配置打开，不用读取后面的配置
    if (0 == accept_num)
    {
        return 0;
    }

    for (unsigned int i = 1; i <= accept_num; ++i)
    {

        //读取bind的IP端口
        snprintf(tmp_key, TMP_BUFFER_LEN, "ACCEPTIP%u", i);
        ret = cfg_file.get_string_value("TCPACCEPT", tmp_key, tmp_value, TMP_BUFFER_LEN);
        snprintf(err_outbuf, TMP_BUFFER_LEN, "TCPACCEPT|%s key error.", tmp_key);
        TESTCONFIG((ret == 0), err_outbuf);

        snprintf(tmp_key, TMP_BUFFER_LEN, "ACCEPTPORT%u", i);
        ret  = cfg_file.get_uint32_value("TCPACCEPT", tmp_key, tmp_uint);
        snprintf(err_outbuf, TMP_BUFFER_LEN, "TCPACCEPT|%s key error.", tmp_key);
        TESTCONFIG((ret == 0), err_outbuf);

        ZEN_Sockaddr_In accept_bind_addr;
        ret = accept_bind_addr.set(tmp_value, static_cast<unsigned short>(tmp_uint));
        TESTCONFIG((ret == 0), err_outbuf);

        ret = check_safe_port(accept_bind_addr);

        if (ret != TSS_RET::TSS_RET_SUCC)
        {
            return ret;
        }

        //TCP收取数据的模块
        snprintf(tmp_key, TMP_BUFFER_LEN, "ACCEPTMODULE%u", i);
        ret = cfg_file.get_string_value("TCPACCEPT", tmp_key, tmp_value, TMP_BUFFER_LEN);
        snprintf(err_outbuf, TMP_BUFFER_LEN, "TCPACCEPT|%s key error.", tmp_key);
        TESTCONFIG((ret == 0), err_outbuf);

        OgreTCPAcceptHandler *accpet_hd = new OgreTCPAcceptHandler(accept_bind_addr, tmp_value);
        accept_handler_ary_.push_back(accpet_hd);
    }

    return TSS_RET::TSS_RET_SUCC;
}

//
int Ogre_Comm_Manger::init_udp_by_conf(Zen_INI_PropertyTree &cfg_file)
{
    int ret = 0;
    unsigned int tmp_uint = 0;

    const size_t TMP_BUFFER_LEN = 256;
    char tmp_key[TMP_BUFFER_LEN + 1];
    char tmp_value[TMP_BUFFER_LEN + 1];
    char err_outbuf[TMP_BUFFER_LEN + 1];

    ret = cfg_file.get_uint32_value("UDPCOMM", "OPENNUMBER", tmp_uint);
    snprintf(err_outbuf, TMP_BUFFER_LEN, "UDPCOMM|OPENNUMBER key .");
    TESTCONFIG((ret == 0), err_outbuf);
    unsigned int udp_num = tmp_uint;

    //如果没有配置打开，不用读取后面的配置
    if (0 == udp_num)
    {
        return 0;
    }

    for (unsigned int i = 1; i <= udp_num; ++i)
    {

        //读取bind的IP端口
        snprintf(tmp_key, TMP_BUFFER_LEN, "UDPIP%u", i);
        ret = cfg_file.get_string_value("UDPCOMM", tmp_key, tmp_value, TMP_BUFFER_LEN);
        snprintf(err_outbuf, TMP_BUFFER_LEN, "UDPCOMM|%s key error.", tmp_key);
        TESTCONFIG((ret == 0), err_outbuf);

        snprintf(tmp_key, TMP_BUFFER_LEN, "UDPPORT%u", i);
        ret  = cfg_file.get_uint32_value("UDPCOMM", tmp_key, tmp_uint);
        snprintf(err_outbuf, TMP_BUFFER_LEN, "UDPCOMM|%s key error.", tmp_key);
        TESTCONFIG((ret == 0), err_outbuf);

        ZEN_Sockaddr_In udp_bind_addr;
        ret = udp_bind_addr.set(tmp_value, static_cast<unsigned short>(tmp_uint));
        TESTCONFIG((ret == 0), err_outbuf);

        ret = check_safe_port(udp_bind_addr);

        if (ret != TSS_RET::TSS_RET_SUCC)
        {
            return ret;
        }

        OgreUDPSvcHandler *udp_hd = new OgreUDPSvcHandler(udp_bind_addr);
        udp_handler_ary_.push_back(udp_hd);
    }

    return TSS_RET::TSS_RET_SUCC;

}

//得到配置
int Ogre_Comm_Manger::get_configure(Zen_INI_PropertyTree &cfg_file)
{
    int ret = 0;

    //IP限制,
    ret = Ogre4aIPRestrictMgr::instance()->get_ip_restrict_conf(cfg_file);

    if (TSS_RET::TSS_RET_SUCC != ret )
    {
        return ret;
    }

    //TCP 读取配置
    ret = Ogre_TCP_Svc_Handler::get_configure(cfg_file);

    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2006年3月20日
Function        : Ogre_TCP_Svc_Handler::get_all_senddata_to_write
Return          : int
Parameter List  :
Param1: size_t& procframe
Description     : 将所有的队列中的数据发送，从SEND管道找到所有的数据去发送,
Calls           :
Called By       :
Other           : 想了想，还是加了一个最多发送的帧的限额
Modify Record   :
******************************************************************************************/
int Ogre_Comm_Manger::get_all_senddata_to_write(size_t &procframe)
{
    int ret = 0;

    //
    for (procframe = 0;
         Zerg_MMAP_BusPipe::instance()->is_empty_bus(Zerg_MMAP_BusPipe::SEND_PIPE_ID) == false &&
         procframe < MAX_ONCE_SEND_FRAME; ++procframe)
    {
        Ogre4a_AppFrame *send_frame = Ogre_Buffer_Storage::instance()->allocate_byte_buffer();

        //
        ret = Zerg_MMAP_BusPipe::instance()->pop_front_bus(Zerg_MMAP_BusPipe::SEND_PIPE_ID,
                                                           reinterpret_cast< ZEN_LIB::dequechunk_node*&>(send_frame));

        if (ret != 0)
        {
            //归还缓存
            Ogre_Buffer_Storage::instance()->free_byte_buffer(send_frame);
            continue;
        }

        //如果FRAME的长度
        if (send_frame->ogre_frame_len_ > Ogre4a_AppFrame::MAX_OF_OGRE_FRAME_LEN)
        {
            ZLOG_ALERT("Ogre_Comm_Manger::get_all_senddata_to_write len %u\n",
                       send_frame->ogre_frame_len_);
            DEBUGDUMP_OGRE_HEAD(send_frame, "Ogre_Comm_Manger::get_all_senddata_to_write", RS_ALERT);
            ZEN_ASSERT(false);
            return TSS_RET::ERR_OGRE_SEND_FRAME_TOO_LEN;
        }

        //如果是TCP
        if (send_frame->ogre_frame_option_ & Ogre4a_AppFrame::OGREDESC_PEER_TCP )
        {
            ret = Ogre_TCP_Svc_Handler::process_send_data(send_frame);

            if (ret != TSS_RET::TSS_RET_SUCC)
            {
                //归还缓存
                Ogre_Buffer_Storage::instance()->free_byte_buffer(send_frame);
                continue;
            }
        }

        //如果是UDP
        else if (send_frame->ogre_frame_option_ & Ogre4a_AppFrame::OGREDESC_PEER_UDP )
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

    return TSS_RET::TSS_RET_SUCC;
}

//初始化通讯管理器
int Ogre_Comm_Manger::init_comm_manger(Zen_INI_PropertyTree &cfg_file)
{
    int ret = 0;

    ret = Zerg_MMAP_BusPipe::instance()->getpara_from_zergcfg(cfg_file);

    if (ret != 0)
    {
        return ret;
    }

    //ACCEPT Handler读取配置
    ret = init_accept_by_conf(cfg_file);

    if (ret != 0)
    {
        return ret;
    }

    //UDP Handler读取配置
    ret =  init_udp_by_conf(cfg_file);

    if (ret != 0)
    {
        return ret;
    }

    ZLOG_INFO( "Restore MMAP Pipe %s \n", Ogre_Svr_Config::instance()->if_restore_pipe_ == true ? "TRUE" : "FALSE");
    //初始化内存管道
    ret = Zerg_MMAP_BusPipe::instance()->initialize(Ogre_Svr_Config::instance()->self_svr_info_,
                                                    0,
                                                    0,
                                                    Ogre4a_AppFrame::MAX_OF_OGRE_FRAME_LEN,
                                                    Ogre_Svr_Config::instance()->if_restore_pipe_,
                                                    false);

    if (ret != TSS_RET::TSS_RET_SUCC)
    {
        return ret;
    }

    //初始化所有的监听端口
    for (size_t i = 0; i < accept_handler_ary_.size(); ++i)
    {
        ret = accept_handler_ary_[i]->create_listenpeer();

        if (ret != TSS_RET::TSS_RET_SUCC)
        {
            return ret;
        }
    }

    ret = OgreUDPSvcHandler::OpenUDPSendPeer();

    if (ret != TSS_RET::TSS_RET_SUCC)
    {
        return ret;
    }

    //初始化所有的UDP端口
    for (size_t i = 0; i < udp_handler_ary_.size(); ++i)
    {
        ret = udp_handler_ary_[i]->InitUDPServices();

        if (ret != TSS_RET::TSS_RET_SUCC)
        {
            return ret;
        }
    }

    //初始化静态数据
    ret = Ogre_TCP_Svc_Handler::init_all_static_data();

    if (ret != TSS_RET::TSS_RET_SUCC)
    {
        return ret;
    }

    return TSS_RET::TSS_RET_SUCC;
}

//注销通讯管理器
int Ogre_Comm_Manger::uninit_comm_manger()
{
    int ret = 0;

    //初始化所有的监听端口
    for (size_t i = 0; i < accept_handler_ary_.size(); ++i)
    {
        ret = accept_handler_ary_[i]->handle_close();

        if (ret != TSS_RET::TSS_RET_SUCC)
        {
            return ret;
        }
    }

    //初始化所有的UDP端口
    for (size_t i = 0; i < udp_handler_ary_.size(); ++i)
    {
        ret = udp_handler_ary_[i]->handle_close();

        if (ret != TSS_RET::TSS_RET_SUCC)
        {
            return ret;
        }
    }

    //
    Ogre_TCP_Svc_Handler::unInit_all_static_data();
    //
    Ogre4aIPRestrictMgr::clean_instance();

    return TSS_RET::TSS_RET_SUCC;
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

