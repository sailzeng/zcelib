
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

    //初始化
    virtual int on_start(int argc,const char *argv[]);
    //退出实例化
    virtual int on_exit();
    //运行
    virtual int on_run();

    bool if_proxy();

private:
   

    int reload_daynamic_config();

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

