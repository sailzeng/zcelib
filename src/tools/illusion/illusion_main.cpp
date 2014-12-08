
// illusion.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "illusion_excel_file.h"
#include "illusion_read_config.h"
#include "illusion_main.h"



int main(int argc, char *argv[])
{
    int ret = 0;
    std::string config_path = "";

    std::string excel_cfg_path = config_path;
    ZCE_LIB::path_string_cat(excel_cfg_path, "excelcfg");
    if (ZCE_LIB::is_directory(excel_cfg_path.c_str()))
    {
        return -1;
    }

    std::string proto_path = config_path;
    ZCE_LIB::path_string_cat(proto_path, "protofile");
    if (ZCE_LIB::is_directory(proto_path.c_str()))
    {
        return -1;
    }

    std::string db3_path = config_path;
    ZCE_LIB::path_string_cat(db3_path, "db3");
    if (ZCE_LIB::is_directory(db3_path.c_str()))
    {
        
    }

    //读取.xls , .xlsx 文件
    std::vector<std::string > execl_cfg_fileary;
    ret = ZCE_LIB::readdir_fileary(excel_cfg_path.c_str(),
        NULL,
        ".xls",
        execl_cfg_fileary);
    if (ret != 0)
    {
        return ret;
    }
    ret = ZCE_LIB::readdir_fileary(excel_cfg_path.c_str(),
        NULL,
        ".xlsx",
        execl_cfg_fileary);
    if (ret != 0)
    {
        return ret;
    }

    if (execl_cfg_fileary.size() > 0)
    {
        return 0;
    }

    //读取.proto文件
    ret = ZCE_LIB::readdir_fileary(proto_path.c_str(),
        NULL,
        ".proto",
        execl_cfg_fileary);
    if (ret != 0)
    {
        return ret;
    }

    //
    if (Illusion_Read_Config::instance()->initialize())
    {

    }
    Illusion_Read_Config::instance()->read_excelconfig();

    Illusion_Read_Config::instance()->finalize();

    ZCE_Protobuf_Reflect proto_reflect;
    proto_reflect.map_path(proto_path);


    return 0;
}
