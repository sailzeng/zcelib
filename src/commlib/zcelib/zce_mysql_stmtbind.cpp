
#include "zce_predefine.h"
#include "zce_mysql_stmtbind.h"

//这些函数都是4.1.2后的版本功能
#if MYSQL_VERSION_ID >= 40100

//构造函数
ZCE_Mysql_STMT_Bind::ZCE_Mysql_STMT_Bind(size_t numbind):
    num_bind_(numbind),
    current_bind_(0),
    stmt_bind_(NULL)
{
    stmt_bind_ = new MYSQL_BIND [numbind];
    memset(stmt_bind_, 0, sizeof(MYSQL_BIND )* numbind);

}

//析构函数
ZCE_Mysql_STMT_Bind::~ZCE_Mysql_STMT_Bind()
{
    if (stmt_bind_)
    {
        delete [] stmt_bind_;
    }

}

//绑定一个参数
int ZCE_Mysql_STMT_Bind::bind_one_param(size_t paramno,
                                        ::enum_field_types paramtype,
                                        my_bool *is_null,
                                        void *paramdata,
                                        unsigned long szparam)
{
    ZCE_ASSERT(paramno < num_bind_);
    if (paramno >= num_bind_)
    {
        return -1;
    }

    stmt_bind_[paramno].buffer_type = paramtype;
    stmt_bind_[paramno].buffer = paramdata;

    stmt_bind_[paramno].is_null = is_null;
    stmt_bind_[paramno].length = NULL;
    stmt_bind_[paramno].buffer_length  = szparam;
    return 0;
}

int ZCE_Mysql_STMT_Bind::bind_one_result(size_t paramno,
                                         ::enum_field_types paramtype,
                                         void *paramdata,
                                         unsigned long *szparam )
{
    ZCE_ASSERT(paramno < num_bind_);
    if (paramno >= num_bind_)
    {
        return -1;
    }

    stmt_bind_[paramno].buffer_type = paramtype;
    stmt_bind_[paramno].buffer = paramdata;

    stmt_bind_[paramno].buffer_length = *szparam;
    stmt_bind_[paramno].length = szparam;
    return 0;
}



//绑定一个char
ZCE_Mysql_STMT_Bind &ZCE_Mysql_STMT_Bind::operator << (char &val)
{
    stmt_bind_[current_bind_].buffer_type = MYSQL_TYPE_TINY;
    stmt_bind_[current_bind_].buffer = (void *)(&val);
    stmt_bind_[current_bind_].buffer_length = sizeof(char);

    //增加当前绑定变量序号
    ++current_bind_;

    return *this;
}
//
ZCE_Mysql_STMT_Bind &ZCE_Mysql_STMT_Bind::operator << (short &val)
{
    stmt_bind_[current_bind_].buffer_type = MYSQL_TYPE_SHORT;
    stmt_bind_[current_bind_].buffer = (void *)(&val);
    stmt_bind_[current_bind_].buffer_length = sizeof(short);

    //增加当前绑定变量序号
    ++current_bind_;

    return *this;
}
//
ZCE_Mysql_STMT_Bind &ZCE_Mysql_STMT_Bind::operator << (int &val)
{
    stmt_bind_[current_bind_].buffer_type = MYSQL_TYPE_LONG;
    stmt_bind_[current_bind_].buffer = (void *)(&val);

    //增加当前绑定变量序号
    ++current_bind_;

    return *this;
}
//
ZCE_Mysql_STMT_Bind &ZCE_Mysql_STMT_Bind::operator << (long &val)
{
    stmt_bind_[current_bind_].buffer_type = MYSQL_TYPE_LONG;
    stmt_bind_[current_bind_].buffer = (void *)(&val);

    //增加当前绑定变量序号
    ++current_bind_;

    return *this;
}
//
ZCE_Mysql_STMT_Bind &ZCE_Mysql_STMT_Bind::operator << (long long &val)
{
    stmt_bind_[current_bind_].buffer_type = MYSQL_TYPE_LONGLONG;
    stmt_bind_[current_bind_].buffer = (void *)(&val);
    //增加当前绑定变量序号
    ++current_bind_;

    return *this;
}
//
ZCE_Mysql_STMT_Bind &ZCE_Mysql_STMT_Bind::operator << (unsigned char &val)
{
    stmt_bind_[current_bind_].buffer_type = MYSQL_TYPE_TINY;
    stmt_bind_[current_bind_].buffer = (void *)(&val);

    //无符号,绑定结果时应该不用
    stmt_bind_[current_bind_].is_unsigned = 1;
    //增加当前绑定变量序号
    ++current_bind_;
    return *this;
}
//
ZCE_Mysql_STMT_Bind &ZCE_Mysql_STMT_Bind::operator << (unsigned short &val)
{
    stmt_bind_[current_bind_].buffer_type = MYSQL_TYPE_SHORT;
    stmt_bind_[current_bind_].buffer = (void *)(&val);

    //无符号,绑定结果时应该不用
    stmt_bind_[current_bind_].is_unsigned = 1;
    //增加当前绑定变量序号
    ++current_bind_;
    return *this;
}
//
ZCE_Mysql_STMT_Bind &ZCE_Mysql_STMT_Bind::operator << (unsigned int &val)
{
    stmt_bind_[current_bind_].buffer_type = MYSQL_TYPE_LONG;
    stmt_bind_[current_bind_].buffer = (void *)(&val);

    //无符号,绑定结果时应该不用
    stmt_bind_[current_bind_].is_unsigned = 1;
    //增加当前绑定变量序号
    ++current_bind_;
    return *this;
}
//
ZCE_Mysql_STMT_Bind &ZCE_Mysql_STMT_Bind::operator << (unsigned long &val)
{
    stmt_bind_[current_bind_].buffer_type = MYSQL_TYPE_LONG;
    stmt_bind_[current_bind_].buffer = (void *)(&val);
    stmt_bind_[current_bind_].buffer_length  = sizeof(unsigned long );
    //无符号,绑定结果时应该不用
    stmt_bind_[current_bind_].is_unsigned = 1;

    //增加当前绑定变量序号
    ++current_bind_;

    return *this;
}
//
ZCE_Mysql_STMT_Bind &ZCE_Mysql_STMT_Bind::operator << (unsigned long long &val)
{
    stmt_bind_[current_bind_].buffer_type = MYSQL_TYPE_LONGLONG;
    stmt_bind_[current_bind_].buffer = reinterpret_cast<void *>(&val);
    stmt_bind_[current_bind_].buffer_length  = sizeof(unsigned long long);
    //无符号,绑定结果时应该不用
    stmt_bind_[current_bind_].is_unsigned = 1;
    //增加当前绑定变量序号
    ++current_bind_;

    return *this;
}
//
ZCE_Mysql_STMT_Bind &ZCE_Mysql_STMT_Bind::operator << (float &val)
{
    stmt_bind_[current_bind_].buffer_type = MYSQL_TYPE_FLOAT;
    stmt_bind_[current_bind_].buffer = reinterpret_cast<void *>(&val);

    //循环当前绑定变量序号
    current_bind_  = (current_bind_ < num_bind_ - 1) ? current_bind_ + 1 : current_bind_;

    return *this;
}

//
ZCE_Mysql_STMT_Bind &ZCE_Mysql_STMT_Bind::operator << (double &val)
{
    stmt_bind_[current_bind_].buffer_type = MYSQL_TYPE_DOUBLE;
    stmt_bind_[current_bind_].buffer = reinterpret_cast<void *>(&val);

    //增加当前绑定变量序号
    ++current_bind_;
    return *this;
}

//
ZCE_Mysql_STMT_Bind &ZCE_Mysql_STMT_Bind::operator << (ZCE_Mysql_STMT_Bind::BinData_Param &bin_data)
{
    stmt_bind_[current_bind_].buffer_type = bin_data.stmt_data_type_;
    stmt_bind_[current_bind_].buffer = bin_data.stmt_pdata_;

    //这个可能既是绑定参数,也是绑定结果
    stmt_bind_[current_bind_].buffer_length  = bin_data.stmt_data_length_;
    stmt_bind_[current_bind_].length = NULL;

    //增加当前绑定变量序号
    ++current_bind_;
    return *this;
}

//绑定二进制结果的适配器
ZCE_Mysql_STMT_Bind &ZCE_Mysql_STMT_Bind::operator << (ZCE_Mysql_STMT_Bind::BinData_Result &bin_data)
{
    stmt_bind_[current_bind_].buffer_type = bin_data.stmt_data_type_;
    stmt_bind_[current_bind_].buffer = bin_data.stmt_pdata_;

    //这个可能既是绑定参数,也是绑定结果
    stmt_bind_[current_bind_].buffer_length = *bin_data.stmt_data_length_;
    stmt_bind_[current_bind_].length = bin_data.stmt_data_length_;

    //增加当前绑定变量序号
    ++current_bind_;
    return *this;
}

//
ZCE_Mysql_STMT_Bind &ZCE_Mysql_STMT_Bind::operator << (ZCE_Mysql_STMT_Bind::TimeData &timeadaptor)
{
    stmt_bind_[current_bind_].buffer_type = timeadaptor.stmt_timetype_;
    stmt_bind_[current_bind_].buffer = reinterpret_cast<void *>(timeadaptor.stmt_ptime_);

    stmt_bind_[current_bind_].buffer_length  = sizeof(MYSQL_TIME);
    stmt_bind_[current_bind_].length = NULL;

    //增加当前绑定变量序号
    ++current_bind_;
    return *this;
}


//绑定一个空参数
ZCE_Mysql_STMT_Bind &ZCE_Mysql_STMT_Bind::operator << (ZCE_Mysql_STMT_Bind::NULL_Param & bind_null)
{
    stmt_bind_[current_bind_].buffer_type = MYSQL_TYPE_NULL;
    
    stmt_bind_[current_bind_].is_null = bind_null.is_null_;

    //增加当前绑定变量序号
    ++current_bind_;
    return *this;
}



#endif //MYSQL_VERSION_ID >= 40100

