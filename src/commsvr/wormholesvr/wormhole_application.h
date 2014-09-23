#ifndef ARBITER_APPLICATION_H_
#define ARBITER_APPLICATION_H_

#include "arbiter_proxyprocess.h"

/****************************************************************************************************
class  Arbiter_Appliction
****************************************************************************************************/
class ArbiterAppliction : public Comm_SvrdApp_NonCtrl
{
protected:

    // 定时器处罚时间间隔, 不能太小, 这里设置为200毫秒
    static const unsigned int TIMER_INTERVAL = 200000;

protected:
    // 自己的单子偷偷藏着，
    ArbiterAppliction();
    ~ArbiterAppliction();

protected:
    // 处理的PROXY的接口
    InterfaceProxyProcess     *interface_proxy_;

    // 处理的FRAME的总数
    unsigned int                 process_frame_count_;

    // 配置文件
    conf_proxysvr::LPCONFIG      conf_;

protected:
    static ArbiterAppliction   *instance_;

public:
    // 处理收到的APPFRAME,不使用const的原因是因为为了加快速度，很多地方是直接将recv_frame修改
    virtual int process_recv_appframe(Comm_App_Frame *recv_frame);

    // 加载配置
    virtual int load_app_conf();
    virtual int merge_app_cfg_file();

    virtual int init();

    virtual void exit();

    // 重新加载路由表
    virtual int reload();

public:
    // 单子实例函数
    static ArbiterAppliction *instance();

    // 清理单子实例
    static void clean_instance();

    const conf_proxysvr::LPCONFIG get_config()
    {
        return conf_;
    }

};

#endif //ARBITER_APPLICATION_H_

