#pragma once

//本来用宏实现了，但是写完了，稍微想改几行，就发现剧烈痛苦。还是用模版把。

namespace soar
{
template <class app_class, class  config_class, class timer_class >
int svrd_main(int argc, const char* argv[])
{
    //不处理异常，因为处理了不好调试,特别是在Win32下调试。

    app_class::set_instance(new app_class());

    app_class::instance()->initialize(new config_class(),
                                      new timer_class);

    //开始日志是无法输出，
    //ZCE_LOG(RS_INFO,"[framework] App init_instance start");

    //初始化
    int ret = app_class::instance()->app_start(argc, argv);

    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR, "[framework] App name [%s] class [%s] init_instance fail:%d|%s",
                app_class::instance()->get_app_runname(),
                typeid(*app_class::instance()).name(),
                ret,
                SOAR_RET::error_string(ret));
        return ret;
    }

    ZCE_LOG(RS_INFO, "[framework] App name [%s] class [%s] init_instance sucess.",
            app_class::instance()->get_app_runname(),
            typeid(*app_class::instance()).name());

    ZCE_LOG(RS_INFO, "[framework] App name [%s] class [%s] run_instance start.",
            app_class::instance()->get_app_runname(),
            typeid(*app_class::instance()).name());
    //运行
    ret = app_class::instance()->app_run();

    //标示运行失败
    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR, "[framework] App name [%s] class [%s] run_instance fail:%d|%s .",
                app_class::instance()->get_app_runname(),
                typeid(*app_class::instance()).name(),
                ret,
                SOAR_RET::error_string(ret));
        //这儿是退出,还是进行exit_instance,这是一个问题，哈哈
        //return ret;
    }
    else
    {
        ZCE_LOG(RS_ERROR, "[framework] App name [%s] class [%s] run_instance sucess.",
                app_class::instance()->get_app_runname(),
                typeid(*app_class::instance()).name());
    }

    ZCE_LOG(RS_INFO, "[framework] App name [%s] class [%s] exit_instance start.",
            app_class::instance()->get_app_runname(),
            typeid(*app_class::instance()).name());
    //退出处理
    app_class::instance()->app_exit();

    ZCE_LOG(RS_INFO, "[framework] App name [%s] class [%s] exit_instance start.",
            app_class::instance()->get_app_runname(),
            typeid(*app_class::instance()).name());

    //
    app_class::clear_inst();
    //关闭日志
    zce::log_msg::clear_inst();

    return 0;
}
};
