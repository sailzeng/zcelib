
// illusion.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "illusion_excel_file.h"
#include "illusion_read_config.h"
#include "illusion_main.h"


Illusion_Application::Illusion_Application()
{

}


Illusion_Application::~Illusion_Application()
{

}


/// app的开始运行
int Illusion_Application::on_start(int argc, const char *argv[])
{
    int ret = 0;

    if (::CoInitialize(NULL) != 0)
    {
        ::AfxMessageBox(_T("初始化COM支持库失败!"));
        return -1;
    }

    config_path_ = "E:\\Config.Test";

    excel_path_ = config_path_;
    ZCE_LIB::path_string_cat(excel_path_, "excelcfg");
    if (false == ZCE_LIB::is_directory(excel_path_.c_str()))
    {
        return -1;
    }

    proto_path_ = config_path_;
    ZCE_LIB::path_string_cat(proto_path_, "protofile");
    if (false == ZCE_LIB::is_directory(proto_path_.c_str()))
    {
        return -1;
    }

    //db3的路径没有可以创建
    db3_path_ = config_path_;
    ZCE_LIB::path_string_cat(db3_path_, "db3");
    if (false == ZCE_LIB::is_directory(db3_path_.c_str()))
    {
        ZCE_LIB::mkdir_recurse(db3_path_.c_str());
    }

    //读取.xls , .xlsx 文件
    ret = ZCE_LIB::readdir_fileary(excel_path_.c_str(),
        NULL,
        ".xls",
        execl_cfg_fileary_);
    if (ret != 0)
    {
        return ret;
    }
    ret = ZCE_LIB::readdir_fileary(excel_path_.c_str(),
        NULL,
        ".xlsx",
        execl_cfg_fileary_);
    if (ret != 0)
    {
        return ret;
    }

    if (execl_cfg_fileary_.size() <= 0)
    {
        return 0;
    }

    //读取.proto文件
    ret = ZCE_LIB::readdir_fileary(proto_path_.c_str(),
        NULL,
        ".proto",
        proto_cfg_fileary_);
    if (ret != 0)
    {
        return ret;
    }
    if (proto_cfg_fileary_.size() > 0)
    {
        return 0;
    }

    BOOL bret = Illusion_Read_Config::instance()->initialize();
    if (!bret)
    {
        return -1;
    }

    return 0;
}

/// app 运行
int Illusion_Application::on_run()
{
    int ret = 0;

    //
    for (size_t i = 0; i < execl_cfg_fileary_.size(); ++i)
    {
        //ZCE LIB的库是按照MBCS编码考虑的，
        ret = Illusion_Read_Config::instance()->read_excelfile_mbcs(execl_cfg_fileary_[i]);
        if (ret != 0)
        {
            return ret;
        }
    }
   

    ZCE_Protobuf_Reflect proto_reflect;
    proto_reflect.map_path(proto_path_);

    for (size_t i = 0; i < proto_cfg_fileary_.size(); ++i)
    {
        ret = proto_reflect.import_file(proto_cfg_fileary_[i]);
        if (ret != 0)
        {
            return ret;
        }
    }
    

    return 0;
}

/// app的退出
int Illusion_Application::on_exit()
{
    ::CoUninitialize();

    Illusion_Read_Config::instance()->finalize();

    return 0;
}



//==========================================================================================

int main(int argc,const char *argv[])
{
    int ret = 0;
    Illusion_Application the_app;
    ret = the_app.on_start(argc, argv);
    if (ret != 0)
    {
        the_app.on_exit();
        return ret;
    }

    the_app.on_run();

    the_app.on_exit();

    return 0;
}
