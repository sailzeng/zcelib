
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
    ret = ZCE_LIB::readdir_nameary(excel_path_.c_str(),
        NULL,
        ".xls",
        false,
        true,
        execl_cfg_fileary_);
    if (ret != 0)
    {
        return ret;
    }
    ret = ZCE_LIB::readdir_nameary(excel_path_.c_str(),
        NULL,
        ".xlsx",
        false,
        true,
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
    ret = ZCE_LIB::readdir_nameary(proto_path_.c_str(),
        NULL,
        ".proto",
        false,
        true,
        proto_cfg_fileary_);
    if (ret != 0)
    {
        return ret;
    }
    if (proto_cfg_fileary_.size() <= 0)
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
    std::string mbcs_name;
    for (size_t i = 0; i < execl_cfg_fileary_.size(); ++i)
    {
        
        //
        mbcs_name = excel_path_;
        //临时文件跳过
        if (execl_cfg_fileary_[i].length() > 0 && execl_cfg_fileary_[i][0] == '~')
        {
            continue;
        }
        ZCE_LIB::path_string_cat(mbcs_name, execl_cfg_fileary_[i]);
        //ZCE LIB的库是按照MBCS编码考虑的，
        ret = Illusion_Read_Config::instance()->read_excelfile_mbcs(
            mbcs_name);
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
    Illusion_Read_Config::instance()->finalize();

    ::CoUninitialize();

    return 0;
}



//==========================================================================================

int main(int argc,const char *argv[])
{
    Illusion_Application the_app;
    try
    {
        int ret = 0;
        ret = the_app.on_start(argc, argv);
        if (ret != 0)
        {
            the_app.on_exit();
            return ret;
        }

        the_app.on_run();
    }
    // For the Try...Catch error message.
    catch (COleException *e)
    {
        TCHAR buf[1024];     
        _sntprintf(buf, 
            1024 -1 , 
            _T("COleException. SCODE: %08lx."), 
            (long)e->m_sc);
        ::MessageBox(NULL, 
            buf, 
            _T("COleException"), 
            MB_SETFOREGROUND | MB_OK);
    }
    catch (COleDispatchException *e)
    {
        TCHAR buf[1024];     
        _sntprintf(buf, 
            1024-1,
            _T("COleDispatchException. SCODE: %08lx, Description: \"%s\"."),
            (long)e->m_wCode, 
            e->m_strDescription.GetBuffer(512));
        ::MessageBox(NULL, 
            buf, 
            _T("COleDispatchException"),
            MB_SETFOREGROUND | MB_OK);
    }
    catch (...)
    {
        ::MessageBox(NULL, _T("General Exception caught."),
            _T("Catch-All"),
            MB_SETFOREGROUND | MB_OK);
    }

    the_app.on_exit();

    return 0;
}
