
#ifndef _ZCE_LIB_PROTOBUF_REFLECT_H_
#define _ZCE_LIB_PROTOBUF_REFLECT_H_


#if defined ZCE_USE_PROTOBUF && ZCE_USE_PROTOBUF == 1



class ZCE_Error_Collector :public MultiFileErrorCollector
{

};

class ZCE_Protobuf_Reflect
{
    ZCE_Protobuf_Reflect();
    ~ZCE_Protobuf_Reflect();



    int map_path(const std::string &path);


    int import_file(const std::string &file_name);


    int error_info();

};

#endif 

#endif //# _ZCE_LIB_PROTOBUF_REFLECT_H_


