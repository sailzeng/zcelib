
#ifndef ZERG_SERVER_APPLICATION_H_
#define ZERG_SERVER_APPLICATION_H_

#include "zerg_tcp_ctrl_handler.h"


class Zerg_Comm_Manager;



/*!
* @brief
*
* @note
*/
class Zerg_Service_App : public Comm_Svrd_Appliction
{


public:
    //我又要偷偷藏着
    Zerg_Service_App();
    ~Zerg_Service_App();

public:

    ///初始化,根据启动参数启动
    virtual int on_start(int argc, const char *argv[]);
    ///处理退出的清理工作
    virtual int on_exit();

    ///运行,运行函数,不到万不得已,不会退出,为了加快发送的速度，对多种请求做了不同的微调。
    virtual int on_run();

    ///加载配置
    virtual int load_config();


    bool if_proxy();

private:


    

public:

    //单子实例函数
    static Zerg_Service_App *instance();
    //清理单子实例
    static void clean_instance();

public:


    Zerg_Comm_Manager       *zerg_comm_mgr_;

    //OP KEY文件的时间戳
    time_t                   conf_timestamp_;

protected:
    //单子的static实例
    static Zerg_Service_App *instance_;


};

//

#endif //_ZERG_SERVER_APPLICATION_H_

