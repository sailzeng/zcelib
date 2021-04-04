
// illusion_main.h : main header file for the illusion application
//
#pragma once


class Illusion_Application
{

public:

    Illusion_Application();

    virtual ~Illusion_Application();


    /// app�Ŀ�ʼ����
    virtual int app_start(int argc, const char *argv[]);

    /// app ����
    virtual int on_run();

    /// app���˳�
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



