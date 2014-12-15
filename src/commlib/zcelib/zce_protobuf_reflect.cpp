#include "zce_predefine.h"
#include "zce_os_adapt_string.h"
#include "zce_protobuf_reflect.h"


#if defined ZCE_USE_PROTOBUF && ZCE_USE_PROTOBUF == 1

//======================================================================================
//分析的错误信息收集装置，
ZCE_Error_Collector::ZCE_Error_Collector()
{
}


ZCE_Error_Collector::~ZCE_Error_Collector()
{
}

void ZCE_Error_Collector::AddError(const std::string &file_name,
                                   int line,
                                   int column,
                                   const std::string &message)
{
    ZCE_PROTO_ERROR proto_err;
    proto_err.file_name_ = file_name;
    proto_err.line_ = line;
    proto_err.column_ = column;
    proto_err.message_ = message;

    error_array_.push_back(proto_err);
}

void ZCE_Error_Collector::clear_error()
{
    error_array_.clear();
}
//======================================================================================

//构造
ZCE_Protobuf_Reflect::ZCE_Protobuf_Reflect():
    protobuf_importer_(NULL)
{

}

ZCE_Protobuf_Reflect::~ZCE_Protobuf_Reflect()
{
    if (protobuf_importer_)
    {
        delete protobuf_importer_;
        protobuf_importer_ = NULL;
    }
}

//映射路径
void ZCE_Protobuf_Reflect::map_path(const std::string &path)
{
    source_tree_.MapPath("", path);
    protobuf_importer_ = new google::protobuf::compiler::Importer(&source_tree_, &error_collector_);
}

//
int ZCE_Protobuf_Reflect::import_file(const std::string &file_name)
{

    error_collector_.clear_error();

    const google::protobuf::FileDescriptor *file_desc =
        protobuf_importer_->Import(file_name);
    if (file_desc)
    {
        return -1;
    }

    return 0;
}

//
void ZCE_Protobuf_Reflect::error_info(PROTO_ERROR_ARRAY &error_ary)
{
    error_ary = error_collector_.error_array_;
}

//
int ZCE_Protobuf_Reflect::new_mesage(const std::string &type_name,
                                     google::protobuf::Message *&new_msg)
{

    //根据名称得到结构描述
    const google::protobuf::Descriptor *proc_msg_desc =
        protobuf_importer_->pool()->FindMessageTypeByName(type_name);
    if (!proc_msg_desc)
    {
        return -1;
    }

    // build a dynamic message by "proc_msg_desc" proto
    const google::protobuf::Message *message = msg_factory_.GetPrototype(proc_msg_desc);
    if (!message)
    {
        return -1;
    }

    new_msg = message->New();

    return 0;
}

//
void ZCE_Protobuf_Reflect::del_message(google::protobuf::Message *del_msg)
{
    delete del_msg;
    del_msg = NULL;
}


int ZCE_Protobuf_Reflect::set_field(google::protobuf::Message *msg,
                                    const std::string &field_name,
                                    const std::string &set_data)
{
    const google::protobuf::Reflection *reflection = msg->GetReflection();
    const google::protobuf::Descriptor *msg_desc = msg->GetDescriptor();

    const google::protobuf::FieldDescriptor *field = NULL;

    field = msg_desc->FindFieldByName(field_name);

    //没有找到对应的字段描述
    if (!field)
    {
        return -1;
    }

    //如果没有设置数据，看看是否是必要字段，而且是否有默认值
    if (set_data.length() == 0)
    {
        if (field->label() == google::protobuf::FieldDescriptor::Label::LABEL_REQUIRED)
        {
            return -1;
        }
        else
        {
            return 0;
        }
    }

    if (field->label() == google::protobuf::FieldDescriptor::Label::LABEL_REQUIRED ||
        field->label() == google::protobuf::FieldDescriptor::Label::LABEL_OPTIONAL)
    {
        // double, exactly eight bytes on the wire.
        if (field->type() == google::protobuf::FieldDescriptor::Type::TYPE_DOUBLE)
        {
            reflection->SetDouble(msg, field, ZCE_LIB::string_to_value<double>(set_data));
        }
        // float, exactly four bytes on the wire.
        else if (field->type() == google::protobuf::FieldDescriptor::Type::TYPE_FLOAT)
        {
            reflection->SetFloat(msg, field, ZCE_LIB::string_to_value<float>(set_data));
        }
        // int64, varint on the wire.  Negative numbers take 10 bytes.  Use TYPE_SINT32 if negative values are likely.
        else if (field->type() == google::protobuf::FieldDescriptor::Type::TYPE_INT64)
        {
            reflection->SetInt64(msg, field, ZCE_LIB::string_to_value<int64_t>(set_data));
        }
        // uint64, exactly eight bytes on the wire.
        else if (field->type() == google::protobuf::FieldDescriptor::Type::TYPE_UINT64)
        {
            reflection->SetUInt64(msg, field, ZCE_LIB::string_to_value<uint64_t>(set_data));
        }
        //int32, varint on the wire.Negative numbers take 10 bytes.  Use TYPE_SINT32 if negative values are likely.
        else if (field->type() == google::protobuf::FieldDescriptor::Type::TYPE_INT32)
        {
            reflection->SetInt32(msg, field, ZCE_LIB::string_to_value<int32_t>(set_data));
        }
        //
        else if (field->type() == google::protobuf::FieldDescriptor::Type::TYPE_FIXED64)
        {
            reflection->SetUInt64(msg, field, ZCE_LIB::string_to_value<uint64_t>(set_data));
        }
        // uint32, exactly four bytes on the wire.
        else if (field->type() == google::protobuf::FieldDescriptor::Type::TYPE_FIXED32)
        {
            reflection->SetUInt32(msg, field, ZCE_LIB::string_to_value<uint32_t>(set_data));
        }
        // bool, varint on the wire.
        else if (field->type() == google::protobuf::FieldDescriptor::Type::TYPE_BOOL)
        {
            reflection->SetBool(msg, field, ZCE_LIB::string_to_value<bool>(set_data));
        }
        // UTF-8 text.
        else if (field->type() == google::protobuf::FieldDescriptor::Type::TYPE_STRING)
        {
            reflection->SetString(msg, field, set_data);
        }
        // Arbitrary byte array.
        else if (field->type() == google::protobuf::FieldDescriptor::Type::TYPE_BYTES)
        {
            reflection->SetString(msg, field, set_data);
        }
        // uint32, varint on the wire
        else if (field->type() == google::protobuf::FieldDescriptor::Type::TYPE_UINT32)
        {
            reflection->SetUInt32(msg, field, 0);
        }
        // Enum, varint on the wire
        else if (field->type() == google::protobuf::FieldDescriptor::Type::TYPE_ENUM)
        {
            const google::protobuf::EnumDescriptor *enum_desc =
                field->enum_type();
            const google::protobuf::EnumValueDescriptor *evalue_desc = enum_desc->FindValueByName(set_data);
            if (evalue_desc)
            {
                reflection->SetEnum(msg, field, evalue_desc);
            }
            else
            {
                return -1;
            }

        }
        // int32, exactly four bytes on the wire
        else if (field->type() == google::protobuf::FieldDescriptor::Type::TYPE_SFIXED32)
        {
            reflection->SetInt32(msg, field, ZCE_LIB::string_to_value<int32_t>(set_data));
        }
        // int64, exactly eight bytes on the wire
        else if (field->type() == google::protobuf::FieldDescriptor::Type::TYPE_SFIXED64)
        {
            reflection->SetInt64(msg, field, ZCE_LIB::string_to_value<int64_t>(set_data));
        }
        // int32, ZigZag-encoded varint on the wire
        else if (field->type() == google::protobuf::FieldDescriptor::Type::TYPE_SINT32)
        {
            reflection->SetInt32(msg, field, ZCE_LIB::string_to_value<int32_t>(set_data));
        }
        // int64, ZigZag-encoded varint on the wire
        else if (field->type() == google::protobuf::FieldDescriptor::Type::TYPE_SINT64)
        {
            reflection->SetInt64(msg, field, ZCE_LIB::string_to_value<int64_t>(set_data));
        }
        else
        {
            assert(false);
            return -1;
        }
    }
    //数组
    else if (field->label() == google::protobuf::FieldDescriptor::Label::LABEL_REPEATED)
    {
        // double, exactly eight bytes on the wire.
        if (field->type() == google::protobuf::FieldDescriptor::Type::TYPE_DOUBLE)
        {
            reflection->AddDouble(msg, field, ZCE_LIB::string_to_value<double>(set_data));
        }
        // float, exactly four bytes on the wire.
        else if (field->type() == google::protobuf::FieldDescriptor::Type::TYPE_FLOAT)
        {
            reflection->AddFloat(msg, field, ZCE_LIB::string_to_value<float>(set_data));
        }
        // int64, varint on the wire.  Negative numbers take 10 bytes.  Use TYPE_SINT32 if negative values are likely.
        else if (field->type() == google::protobuf::FieldDescriptor::Type::TYPE_INT64)
        {
            reflection->AddInt64(msg, field, ZCE_LIB::string_to_value<int64_t>(set_data));
        }
        // uint64, exactly eight bytes on the wire.
        else if (field->type() == google::protobuf::FieldDescriptor::Type::TYPE_UINT64)
        {
            reflection->AddUInt64(msg, field, ZCE_LIB::string_to_value<uint64_t>(set_data));
        }
        //int32, varint on the wire.Negative numbers take 10 bytes.  Use TYPE_SINT32 if negative values are likely.
        else if (field->type() == google::protobuf::FieldDescriptor::Type::TYPE_INT32)
        {
            reflection->AddInt32(msg, field, ZCE_LIB::string_to_value<int32_t>(set_data));
        }
        //
        else if (field->type() == google::protobuf::FieldDescriptor::Type::TYPE_FIXED64)
        {
            reflection->AddUInt64(msg, field, ZCE_LIB::string_to_value<uint64_t>(set_data));
        }
        // uint32, exactly four bytes on the wire.
        else if (field->type() == google::protobuf::FieldDescriptor::Type::TYPE_FIXED32)
        {
            reflection->AddUInt32(msg, field, ZCE_LIB::string_to_value<uint32_t>(set_data));
        }
        // bool, varint on the wire.
        else if (field->type() == google::protobuf::FieldDescriptor::Type::TYPE_BOOL)
        {
            reflection->AddBool(msg, field, ZCE_LIB::string_to_value<bool>(set_data));
        }
        // UTF-8 text.
        else if (field->type() == google::protobuf::FieldDescriptor::Type::TYPE_STRING)
        {
            reflection->AddString(msg, field, set_data);
        }
        // Length-delimited message.
        else if (field->type() == google::protobuf::FieldDescriptor::Type::TYPE_MESSAGE)
        {
            //reflection->SetInt32(msg, field, 0);
        }
        // Arbitrary byte array.
        else if (field->type() == google::protobuf::FieldDescriptor::Type::TYPE_BYTES)
        {
            reflection->AddString(msg, field, set_data);
        }
        // uint32, varint on the wire
        else if (field->type() == google::protobuf::FieldDescriptor::Type::TYPE_UINT32)
        {
            reflection->AddUInt32(msg, field, 0);
        }
        // Enum, varint on the wire
        else if (field->type() == google::protobuf::FieldDescriptor::Type::TYPE_ENUM)
        {
            const google::protobuf::EnumDescriptor *enum_desc =
                field->enum_type();
            const google::protobuf::EnumValueDescriptor *evalue_desc = enum_desc->FindValueByName(set_data);
            if (evalue_desc)
            {
                reflection->AddEnum(msg, field, evalue_desc);
            }
            else
            {
                return -1;
            }

        }
        // int32, exactly four bytes on the wire
        else if (field->type() == google::protobuf::FieldDescriptor::Type::TYPE_SFIXED32)
        {
            reflection->AddInt32(msg, field, ZCE_LIB::string_to_value<int32_t>(set_data));
        }
        // int64, exactly eight bytes on the wire
        else if (field->type() == google::protobuf::FieldDescriptor::Type::TYPE_SFIXED64)
        {
            reflection->AddInt64(msg, field, ZCE_LIB::string_to_value<int64_t>(set_data));
        }
        // int32, ZigZag-encoded varint on the wire
        else if (field->type() == google::protobuf::FieldDescriptor::Type::TYPE_SINT32)
        {
            reflection->AddInt32(msg, field, ZCE_LIB::string_to_value<int32_t>(set_data));
        }
        // int64, ZigZag-encoded varint on the wire
        else if (field->type() == google::protobuf::FieldDescriptor::Type::TYPE_SINT64)
        {
            reflection->AddInt64(msg, field, ZCE_LIB::string_to_value<int64_t>(set_data));
        }
        else
        {
            assert(false);
            return -1;
        }
    }
    else
    {
        assert(false);
        return -1;
    }

    return 0;
}

//定位一个子结构
int ZCE_Protobuf_Reflect::locate_sub_msg(google::protobuf::Message *msg,
                                         const std::string &submsg_field_name,
                                         bool repeated_add,
                                         google::protobuf::Message *&sub_msg)
{
    //得到结构的描述和反射
    const google::protobuf::Reflection *reflection = msg->GetReflection();
    const google::protobuf::Descriptor *msg_desc = msg->GetDescriptor();

    const google::protobuf::FieldDescriptor *field = NULL;
    field = msg_desc->FindFieldByName(submsg_field_name);

    //没有找到对应的字段描述
    if (!field)
    {
        return -1;
    }

    if (field->label() == google::protobuf::FieldDescriptor::Label::LABEL_REQUIRED ||
        field->label() == google::protobuf::FieldDescriptor::Label::LABEL_OPTIONAL)
    {
        //如果是一个子结构
        if (field->type() == google::protobuf::FieldDescriptor::Type::TYPE_MESSAGE)
        {
            sub_msg = reflection->MutableMessage(msg, field, 0);
            if (!sub_msg)
            {
                return -1;
            }
        }
    }
    else if (field->label() == google::protobuf::FieldDescriptor::Label::LABEL_REPEATED)
    {
        // Length-delimited message.
        if (field->type() == google::protobuf::FieldDescriptor::Type::TYPE_MESSAGE)
        {
            //如果要增加一个repeated的数据
            if (repeated_add)
            {
                sub_msg = reflection->AddMessage(msg, field, NULL);
                if (!sub_msg)
                {
                    return -1;
                }
            }
            //如果不用增加，使用repeated 数组的最后一个数据，
            else
            {
                int ary_size = reflection->FieldSize(*msg, field);
                if (ary_size == 0)
                {
                    return -1;
                }
                sub_msg = reflection->MutableRepeatedMessage(msg, field, ary_size - 1);
                if (!sub_msg)
                {
                    return -1;
                }
            }
        }
    }
    else
    {
        assert(false);
        return -1;
    }


    return 0;
}

//根据fullname，也就是 phone_book.number 设置一个Message的field
int ZCE_Protobuf_Reflect::set_field_by_fullname(google::protobuf::Message *msg,
                                                const std::string &full_name,
                                                const std::string &set_data,
                                                bool repeated_add)
{
    int ret = 0;
    google::protobuf::Message *src_msg = msg;
    google::protobuf::Message *sub_msg = NULL;
    const char *FIELD_SEPARATOR = ".";
    std::vector<std::string> v;
    ZCE_LIB::string_split(full_name, FIELD_SEPARATOR, v);

    size_t level_num = v.size();
    if (level_num > 1)
    {
        for (size_t i = 0; i < level_num - 1; ++i)
        {
            ret = ZCE_Protobuf_Reflect::locate_sub_msg(src_msg,
                                                       v[i],
                                                       repeated_add,
                                                       sub_msg);
            if (0 != ret)
            {
                return ret;
            }
            src_msg = sub_msg;
        }
    }
    ret = ZCE_Protobuf_Reflect::set_field(src_msg,
                                          v[level_num - 1],
                                          set_data);
    if (0 != ret)
    {
        return ret;
    }

    return 0;
}


#endif

