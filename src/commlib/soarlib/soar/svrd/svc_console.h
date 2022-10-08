#pragma once

//

namespace zce
{
class event_handler;
class reactor;
}

namespace soar
{
class svc_console : public zce::event_handler
{
public:
    explicit svc_console(zce::reactor *reactor_inst);

protected:
    virtual ~svc_console();

protected:
    //
    virtual int process_mml_command();

    //处理的命令组

    //查询ZERG服务器的状态
    int cmd_get_zergstats();
    //查询联接的Peer状态,不一定能全部返回,而且比较耗时
    int cmd_get_peer_stats();
    //关闭服务器
    int cmd_close_services();
    //关闭相应的端口
    int cmd_close_socketpeer();
    //修改日志的输出级别
    int cmd_modify_logpriority();

protected:
    ///
    std::string  req_string_;
    ///
    std::string  rsp_string_;
    ///
    zce::mml_cmd mml_process_;
};
}