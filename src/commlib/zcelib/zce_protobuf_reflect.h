
#ifndef _ZCE_LIB_PROTOBUF_REFLECT_H_
#define _ZCE_LIB_PROTOBUF_REFLECT_H_


#if defined ZCE_USE_PROTOBUF && ZCE_USE_PROTOBUF == 1

//
struct ZCE_PROTO_ERROR
{
    std::string file_name_;
    int line_;
    int column_;
    std::string message_;
};

typedef std::vector<ZCE_PROTO_ERROR> PROTO_ERROR_ARRAY;


//¥ÌŒÛ ’ºØ
class ZCE_Error_Collector :public google::protobuf::compiler::MultiFileErrorCollector
{

    ZCE_Error_Collector();
    virtual ~ZCE_Error_Collector();

protected:

    virtual void AddError(const std::string & filename, 
        int line, 
        int column, 
        const std::string & message);

protected:
    //
    PROTO_ERROR_ARRAY error_array_;
};


//
class ZCE_Protobuf_Reflect
{
public:

    ZCE_Protobuf_Reflect();
    ~ZCE_Protobuf_Reflect();


public:

    int map_path(const std::string &path);


    int import_file(const std::string &file_name);


    void error_info(PROTO_ERROR_ARRAY &error_ary);

    //
    google::protobuf::Message *new_mesage(const std::string &type_name);

    //
    int set_msg_field(google::protobuf::Message *msg,
        const std::string &field_name,
        const std::string &set_data);

    //
    void delete_message(google::protobuf::Message *msg);

    //


protected:

    //
    google::protobuf::compiler::DiskSourceTree source_tree_;

    //


};

#endif 

#endif //# _ZCE_LIB_PROTOBUF_REFLECT_H_


