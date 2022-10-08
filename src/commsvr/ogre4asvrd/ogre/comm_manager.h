#pragma once

class ogre4a_frame;

namespace ogre
{
class svc_accept;
class svc_udp;
class auto_connect;
class configure;
//通讯集中管理器
class comm_manager
{
public:

    comm_manager();
    ~comm_manager();

    ///读取配置文件
    int get_config(const configure* config);

    //从SEND管道找到所有的数据去发送,
    int get_all_senddata_to_write(size_t& procframe);

    //检查安全端口
    int check_safe_port(zce::skt::addr_in& inetadd);

    //初始化通讯管理器
    int init_comm_manger();
    //注销通讯管理器
    int uninit_comm_manger();

public:
    //得到单子的实例
    static comm_manager* instance();
    //清理单子的实例
    static void clear_inst();

protected:

    //一次最多发送1024帧
    static const unsigned int MAX_ONCE_SEND_FRAME = 1024;

protected:
    //
    static comm_manager* instance_;

protected:

    //配置实例指针
    const configure* ogre_config_;
};
}//namespace ogre
