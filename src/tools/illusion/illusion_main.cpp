
// illusion.cpp : Defines the class behaviors for the application.
//



#include "stdafx.h"
#include "illusion_excel_file.h"
#include "illusion_read_config.h"
#include "illusion_main.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

Illusion_Application::Illusion_Application()
{

}


Illusion_Application::~Illusion_Application()
{

}


/// app的开始运行
int Illusion_Application::on_start(int /*argc*/, const char * /*argv*/[])
{
    int ret = 0;
    //使用UTF8代码也输出，测试发现还是有很多问题，因为自己的库内部使用的是Not set的字符集。
    //bret = ::SetConsoleOutputCP(CP_UTF8);
    //if (bret == FALSE)
    //{
    //    return -1;
    //}

    //HANDLE handle_out = ::GetStdHandle(STD_OUTPUT_HANDLE);
    //if (handle_out == INVALID_HANDLE_VALUE)
    //{
    //    return -1;
    //}
    ////设置屏幕缓冲区和输出屏幕大小
    //COORD coord = { 161, 481 };
    //bret = ::SetConsoleScreenBufferSize(handle_out, coord);
    //if (bret == FALSE)
    //{
    //    return -1;
    //}
    //SMALL_RECT rect = { 0, 0, 160, 48 };
    //bret = ::SetConsoleWindowInfo(handle_out, TRUE, &rect);
    //if (bret == FALSE)
    //{
    //    return -1;
    //}

    ::SetConsoleTitle(_T("illusion -- EXCEL config to SQlite3 DB Tools!"));

    config_path_ = "E:\\Courage\\zcelib.git\\src\\tools\\illusion\\template";

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

    logdir_path_ = config_path_;
    ZCE_LIB::path_string_cat(logdir_path_, "log");
    if (false == ZCE_LIB::is_directory(logdir_path_.c_str()))
    {
        ZCE_LIB::mkdir_recurse(logdir_path_.c_str());
    }
    ZCE_LIB::path_string_cat(logdir_path_, "illusion_operation");

    ZCE_Trace_LogMsg::instance()->init_time_log(LOGDEVIDE_BY_DAY,
        logdir_path_.c_str());

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

    bool std_bret = Illusion_Read_Config::instance()->initialize(true, config_path_);
    if (!std_bret)
    {
        return -1;
    }

    return 0;
}

/// app 运行
int Illusion_Application::on_run()
{
    int ret = 0;

    Illusion_Read_Config::instance()->map_proto_path(proto_path_);
    for (size_t i = 0; i < proto_cfg_fileary_.size(); ++i)
    {

        ret = Illusion_Read_Config::instance()->read_proto(proto_cfg_fileary_[i]);
        if (ret != 0)
        {
            return ret;
        }
    }

    //
    std::string mbcs_xls_name;
    for (size_t i = 0; i < execl_cfg_fileary_.size(); ++i)
    {

        //
        mbcs_xls_name = excel_path_;
        //临时文件跳过
        if (execl_cfg_fileary_[i].length() > 0 && execl_cfg_fileary_[i][0] == '~')
        {
            continue;
        }
        ZCE_LIB::path_string_cat(mbcs_xls_name, execl_cfg_fileary_[i]);
        //ZCE LIB的库是按照MBCS编码考虑的，
        ret = Illusion_Read_Config::instance()->read_excel(
                  mbcs_xls_name);
        if (ret != 0)
        {
            return ret;
        }
    }

    ret = Illusion_Read_Config::instance()->read_db3_conftable(
              "game_config.db3",
              "GAME_CFG_STRUCT_2",
              2);
    if (ret != 0)
    {
        return ret;
    }
    return 0;
}

/// app的退出
int Illusion_Application::on_exit()
{
    Illusion_Read_Config::instance()->finalize();

    Illusion_Read_Config::clean_instance();

    return 0;
}



//==========================================================================================

int main(int argc, const char *argv[])
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
        the_app.on_exit();
        system("PAUSE");
        return 0;
    }
    // For the Try...Catch error message.
    catch (COleException *e)
    {
        TCHAR buf[1024];
        _sntprintf(buf,
                   1024 - 1 ,
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
                   1024 - 1,
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
