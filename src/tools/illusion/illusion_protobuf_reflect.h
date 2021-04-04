/*!
* @copyright  2004-2014  Apache License, Version 2.0 FULLSAIL
* @filename   zce_protobuf_reflect.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       Tuesday, November 25, 2014
* @brief
*
*
* @details
*
*
*
* @note
* ������֣�ݵļ��䡷 ��־
* ����֣����֪���Ĳ��࣬Ϊ�˰�������ȥ������
* ���ٴ��ڻ���·������У�һ�������ĵ�������
* ��˵��ϲ��֣�ݶ�������⣬������Ʈ��ú¯��ζ��
* ��������������Ĳ��ӣ�ֱ�����춼û��ɢȥ
* ����֣�������ȫ���㣬������ȥ������ں�ί��
* ����֣���Ұ���ȫ���㣬������ȥ�����װ�������
* ����֣��ֻ��ż����������ζ�����ڻ�����
* ÿ�κ�����˵���ȥ�����У��Ҳ���˵����ȥ������
* ����֣�������ȫ���㣬���������޷���ʹ�������
* ����֣���Ұ���ȫ���㣬����������Ƕ���·��ȥ
* ���Ƕ��ǻ������¿�η�������������������޴�
* ʱ��ı��˺ܶ���ʲô��û��
* �����ٴ�ӵ���㣬֣��
*/

#ifndef _ILLUSION_PROTOBUF_REFLECT_H_
#define _ILLUSION_PROTOBUF_REFLECT_H_


//======================================================================================

//
struct ZCE_PROTO_ERROR
{
    std::string file_name_;
    int line_;
    int column_;
    std::string message_;
};

typedef std::vector<ZCE_PROTO_ERROR> PROTO_ERROR_ARRAY;


//�����ռ�
class ZCE_Error_Collector : public google::protobuf::compiler::MultiFileErrorCollector
{
public:
    ZCE_Error_Collector();
    virtual ~ZCE_Error_Collector();

public:

    virtual void AddError(const std::string &filename,
                          int line,
                          int column,
                          const std::string &message);

    void clear_error();

public:
    //
    PROTO_ERROR_ARRAY error_array_;
};


//======================================================================================

/*!
* @brief
*
* @note
*/
class Illusion_Protobuf_Reflect
{
public:
    //
    Illusion_Protobuf_Reflect();
    //
    ~Illusion_Protobuf_Reflect();

public:

    ///ӳ��һ��·��
    void map_path(const std::string &path);

    ///����һ��proto �ļ�
    int import_file(const std::string &file_name);

    //���ش�����Ϣ
    void error_info(PROTO_ERROR_ARRAY &error_ary);

    //�������ƴ���Message,new ��message �ᱣ��������ǰ�����message����
    int new_mesage(const std::string &type_name,
                   google::protobuf::Message *&new_msg);

    //
    void del_message(google::protobuf::Message *&del_msg);

    //
    int set_proc_msg_field(const std::string &field_name,
                           const std::string &set_data);

    //
public:

    ///����fullname��Ҳ���� phone_book.number ����һ��Message��field
    static int set_field(google::protobuf::Message *msg,
                         const std::string &full_name,
                         const std::string &set_data,
                         bool message_add);

    ///����fullname,�õ�ĳ���ֶε�������Ϣ
    static int get_fielddesc(google::protobuf::Message *msg,
                             const std::string &full_name,
                             bool message_add,
                             google::protobuf::Message *&field_msg,
                             const google::protobuf::FieldDescriptor *&field_desc);

    ///����һ��Message��field
    static int set_fielddata(google::protobuf::Message *msg,
                             const google::protobuf::FieldDescriptor *field,
                             const std::string &set_data);


    ///��λһ���ӽṹ
    static int locate_sub_msg(google::protobuf::Message *msg,
                              const std::string &submsg_field_name,
                              bool message_add,
                              google::protobuf::Message *&sub_msg);

    ///��ӡ���һ��Message����Ϣ��ostream���棬
    static void protobuf_output(const google::protobuf::Message *msg,
                                std::ostream *out);

    ///��message�������е��ֶ�����ΪĬ��ֵ��
    ///ע�������ֱ����Clear������ԭ���ǣ����ڶ�ȡ���õĹ����Ѿ���Message�ṹ�������
    ///��Ӧ��Field��ָ��Ҳ�����ˣ����Clear��һ�ж�ʧЧ�ˣ���repeated����������Mesage����
    static void message_set_default(google::protobuf::Message *msg);

protected:

    ///
    google::protobuf::compiler::Importer *protobuf_importer_;

    ///
    google::protobuf::compiler::DiskSourceTree *source_tree_;

    //
    google::protobuf::DynamicMessageFactory *msg_factory_;

    //
    ZCE_Error_Collector error_collector_;

};



#endif //# _ILLUSION_PROTOBUF_REFLECT_H_


