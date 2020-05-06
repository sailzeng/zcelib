锘塊敇?// illusion_main.h : main header file for the illusion application
//
#pragma once


class Illusion_Application
{

public:

    Illusion_Application();

    virtual ~Illusion_Application();


<<<<<<< HEAD
    /// app闀勫嫬绱戞慨瀣
=======
    /// app的开始运行
    virtual int app_start(int argc, const char *argv[]);

    /// app 运行
    virtual int on_run();

    /// app的退出
    virtual int on_exit();

protected:
    ///
    std::string config_path_;

    ///
    std::string excel_path_;
    ///
    std::string proto_path_;
    ///
    std::string logdir_path_;

    ///
    std::vector<std::string > execl_cfg_fileary_;
    ///
    std::vector<std::string > proto_cfg_fileary_;

};



>>>>>>> ecb76a1a4aa8381667ced3cb31202915f48ca78b
