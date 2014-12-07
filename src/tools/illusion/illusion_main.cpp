
// illusion.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "illusion_excel_file.h"
#include "illusion_read_config.h"
#include "illusion_main.h"



int main(int argc, char *argv[])
{
    std::string config_path = "";

    std::string excel_cfg_path = config_path;
    ZCE_LIB::path_string_cat(excel_cfg_path, "excelcfg");

    std::string proto_path = config_path;
    ZCE_LIB::path_string_cat(excel_cfg_path, "protofile");

    std::string db3_path = config_path;
    ZCE_LIB::path_string_cat(excel_cfg_path, "db3");

    ZCE_Protobuf_Reflect proto_reflect;

    proto_reflect.map_path(proto_path);

    return 0;
}
