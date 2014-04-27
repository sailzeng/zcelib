
#include "zce_predefine.h"
#include "zce_mysql_stmtbind.h"

//这些函数都是4.1.2后的版本功能
#if MYSQL_VERSION_ID >= 40100

//构造函数
ZCE_Mysql_STMT_Bind::ZCE_Mysql_STMT_Bind(size_t numbind):
    num_bind_(numbind),
    current_bind_(0),
    stmt_bind_(NULL),
    is_bind_null_(NULL),
    bind_length_(NULL)
{
    stmt_bind_ = new MYSQL_BIND [numbind];
    memset(stmt_bind_, 0, sizeof(MYSQL_BIND )* numbind);

    //默认所有的绑定都是非NULL
    is_bind_null_ = new my_bool [numbind];
    memset(is_bind_null_, 0, sizeof(my_bool )* numbind);

    bind_length_ = new unsigned long [numbind];
    memset(bind_length_, 0, sizeof(unsigned long )* numbind);

    //自动帮你绑定
    for (size_t i = 0 ; i < num_bind_ ; ++i)
    {
        stmt_bind_[i].is_null = &(is_bind_null_[i]);
        stmt_bind_[i].length   = &(bind_length_[i]);
    }
}

//析构函数
ZCE_Mysql_STMT_Bind::~ZCE_Mysql_STMT_Bind()
{
    if (stmt_bind_)
    {
        delete [] stmt_bind_;
    }

    if (is_bind_null_)
    {
        delete [] is_bind_null_;
    }

    if (bind_length_)
    {
        delete [] bind_length_;
    }
}

//绑定一个参数
int ZCE_Mysql_STMT_Bind::bind_one_param(size_t paramno,
                                        ::enum_field_types paramtype,
                                        bool bisnull,
                                        void *paramdata ,
                                        unsigned long szparam)
{
    ZCE_ASSERT(paramno < num_bind_);
    if (paramno >= num_bind_)
    {
        return -1;
    }

    stmt_bind_[paramno].buffer_type = paramtype;
    stmt_bind_[paramno].buffer = paramdata;

    if (bisnull)
    {
        *(stmt_bind_[paramno].is_null) = 1;
    }
    else
    {
        *(stmt_bind_[paramno].is_null) = 0;
    }

    if (stmt_bind_[paramno].length )
    {
        *(stmt_bind_[paramno].length) = szparam;
    }

    stmt_bind_[paramno].buffer_length  = szparam;
    return 0;
}

inline MYSQL_BIND *ZCE_Mysql_STMT_Bind::operator[](unsigned int paramno) const
{
    return &stmt_bind_[paramno];
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

    current_bind_  = (current_bind_ < num_bind_ - 1) ? current_bind_ + 1 : current_bind_;

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
    //循环当前绑定变量序号
    current_bind_  = (current_bind_ < num_bind_ - 1) ? current_bind_ + 1 : current_bind_;

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

    //循环当前绑定变量序号
    current_bind_  = (current_bind_ < num_bind_ - 1) ? current_bind_ + 1 : current_bind_;
    return *this;
}
//
ZCE_Mysql_STMT_Bind &ZCE_Mysql_STMT_Bind::operator << (STMT_BindData_Adaptor &dataadaptor)
{
    stmt_bind_[current_bind_].buffer_type = dataadaptor.stmt_data_type_;
    stmt_bind_[current_bind_].buffer = dataadaptor.stmt_pdata_;

    //这个可能既是绑定参数,也是绑定结果
    stmt_bind_[current_bind_].buffer_length  = dataadaptor.stmt_data_length_;
    *(stmt_bind_[current_bind_].length) =  dataadaptor.stmt_data_length_;

    current_bind_  = (current_bind_ < num_bind_ - 1) ? current_bind_ + 1 : current_bind_;
    return *this;
}
//
ZCE_Mysql_STMT_Bind &ZCE_Mysql_STMT_Bind::operator << (STMT_BindTime_Adaptor &timeadaptor)
{
    stmt_bind_[current_bind_].buffer_type = timeadaptor.stmt_timetype_;
    stmt_bind_[current_bind_].buffer = reinterpret_cast<void *>(timeadaptor.stmt_ptime_);

    stmt_bind_[current_bind_].buffer_length  = sizeof(MYSQL_TIME);
    *(stmt_bind_[current_bind_].length) =  sizeof(MYSQL_TIME);

    //增加当前绑定变量序号
    ++current_bind_;
    return *this;
}

//绑定一个空参数
ZCE_Mysql_STMT_Bind &ZCE_Mysql_STMT_Bind::operator << (STMT_BindNULL_Adaptor &bindisnull)
{
    is_bind_null_[current_bind_] = 1;
    //增加当前绑定变量序号
    ++current_bind_;
    return *this;
}

//
bool ZCE_Mysql_STMT_Bind::get_bind_is_null(const size_t paramno) const
{
    if (is_bind_null_[paramno] != 0)
    {
        return true;
    }

    return false;
}
//
void ZCE_Mysql_STMT_Bind::set_bind_is_null(const size_t paramno, const bool isnull)
{
    if (isnull)
    {
        is_bind_null_[paramno] = 1;
    }
    else
    {
        is_bind_null_[paramno] = 0;
    }

    return;
}

//得到第几个绑定的参数Buf的长度
unsigned long ZCE_Mysql_STMT_Bind::get_bind_buf_length(const size_t paramno) const
{
    return stmt_bind_[paramno].buffer_length;
}

//设置第几个绑定的参数Buf的长度
void ZCE_Mysql_STMT_Bind::set_bind_buf_length(const size_t paramno, unsigned long buflen)
{
    stmt_bind_[paramno].buffer_length  = buflen;
    return;
}

//得到返回数据的实际长度
unsigned long ZCE_Mysql_STMT_Bind::get_bind_data_length(const size_t paramno) const
{
    return (bind_length_[paramno]);
}

//得到返回数据的实际长度
void ZCE_Mysql_STMT_Bind::set_bind_data_length(const size_t paramno, unsigned long bindlen)
{
    (bind_length_[paramno]) = bindlen;
}

#endif //MYSQL_VERSION_ID >= 40100

