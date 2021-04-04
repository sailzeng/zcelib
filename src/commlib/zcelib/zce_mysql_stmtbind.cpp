
#include "zce_predefine.h"
#include "zce_mysql_stmtbind.h"

//��Щ��������4.1.2��İ汾����
#if MYSQL_VERSION_ID >= 40100

//���캯��
ZCE_Mysql_STMT_Bind::ZCE_Mysql_STMT_Bind(size_t numbind):
    num_bind_(numbind),
    current_bind_(0),
    stmt_bind_(NULL)
{
    stmt_bind_ = new MYSQL_BIND[num_bind_];
    memset(stmt_bind_, 0, sizeof(MYSQL_BIND)* num_bind_);

}

//��������
ZCE_Mysql_STMT_Bind::~ZCE_Mysql_STMT_Bind()
{
    if (stmt_bind_)
    {
        delete [] stmt_bind_;
    }

}

//��������
void ZCE_Mysql_STMT_Bind::reset()
{
    memset(stmt_bind_, 0, sizeof(MYSQL_BIND)* num_bind_);
    current_bind_ = 0;
}

//��һ������
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

int ZCE_Mysql_STMT_Bind::bind_one_result(size_t result_no,
                                         ::enum_field_types paramtype,
                                         void *paramdata,
                                         unsigned long *szparam )
{
    ZCE_ASSERT(result_no < num_bind_);
    if (result_no >= num_bind_)
    {
        return -1;
    }

    stmt_bind_[result_no].buffer_type = paramtype;
    stmt_bind_[result_no].buffer = paramdata;

    stmt_bind_[result_no].buffer_length = *szparam;
    //����ָ�뱣�淵��ֵ
    stmt_bind_[result_no].length = szparam;
    return 0;
}



//��һ��char
void ZCE_Mysql_STMT_Bind::bind(size_t bind_col, char &val)
{
    stmt_bind_[bind_col].buffer_type = MYSQL_TYPE_TINY;
    stmt_bind_[bind_col].buffer = (void *)(&val);
    stmt_bind_[bind_col].buffer_length = sizeof(char);

    return ;
}

//
void ZCE_Mysql_STMT_Bind::bind(size_t bind_col, short &val)
{
    stmt_bind_[bind_col].buffer_type = MYSQL_TYPE_SHORT;
    stmt_bind_[bind_col].buffer = (void *)(&val);
    stmt_bind_[bind_col].buffer_length = sizeof(short);

    return;
}

//
void ZCE_Mysql_STMT_Bind::bind(size_t bind_col, int &val)
{
    stmt_bind_[bind_col].buffer_type = MYSQL_TYPE_LONG;
    stmt_bind_[bind_col].buffer = (void *)(&val);

    return;
}
//
void ZCE_Mysql_STMT_Bind::bind(size_t bind_col, long &val)
{
    stmt_bind_[bind_col].buffer_type = MYSQL_TYPE_LONG;
    stmt_bind_[bind_col].buffer = (void *)(&val);

    return;
}
//
void ZCE_Mysql_STMT_Bind::bind(size_t bind_col, long long &val)
{
    stmt_bind_[bind_col].buffer_type = MYSQL_TYPE_LONGLONG;
    stmt_bind_[bind_col].buffer = (void *)(&val);

    return;
}

//
void ZCE_Mysql_STMT_Bind::bind(size_t bind_col, unsigned char &val)
{
    stmt_bind_[bind_col].buffer_type = MYSQL_TYPE_TINY;
    stmt_bind_[bind_col].buffer = (void *)(&val);

    //�޷���,�󶨽��ʱӦ�ò���
    stmt_bind_[bind_col].is_unsigned = 1;

    return;
}

//
void ZCE_Mysql_STMT_Bind::bind(size_t bind_col, unsigned short &val)
{
    stmt_bind_[bind_col].buffer_type = MYSQL_TYPE_SHORT;
    stmt_bind_[bind_col].buffer = (void *)(&val);

    //�޷���,�󶨽��ʱӦ�ò���
    stmt_bind_[bind_col].is_unsigned = 1;

    return;
}
//
void ZCE_Mysql_STMT_Bind::bind(size_t bind_col, unsigned int &val)
{
    stmt_bind_[bind_col].buffer_type = MYSQL_TYPE_LONG;
    stmt_bind_[bind_col].buffer = (void *)(&val);

    //�޷���,�󶨽��ʱӦ�ò���
    stmt_bind_[bind_col].is_unsigned = 1;

    return;
}
//
void ZCE_Mysql_STMT_Bind::bind(size_t bind_col, unsigned long &val)
{
    stmt_bind_[bind_col].buffer_type = MYSQL_TYPE_LONG;
    stmt_bind_[bind_col].buffer = (void *)(&val);
    stmt_bind_[bind_col].buffer_length = sizeof(unsigned long);
    //�޷���,�󶨽��ʱӦ�ò���
    stmt_bind_[bind_col].is_unsigned = 1;

    return;
}
//
void ZCE_Mysql_STMT_Bind::bind(size_t bind_col, unsigned long long &val)
{
    stmt_bind_[bind_col].buffer_type = MYSQL_TYPE_LONGLONG;
    stmt_bind_[bind_col].buffer = reinterpret_cast<void *>(&val);
    stmt_bind_[bind_col].buffer_length = sizeof(unsigned long long);
    //�޷���,�󶨽��ʱӦ�ò���
    stmt_bind_[bind_col].is_unsigned = 1;

    return;
}
//
void ZCE_Mysql_STMT_Bind::bind(size_t bind_col, float &val)
{
    stmt_bind_[bind_col].buffer_type = MYSQL_TYPE_FLOAT;
    stmt_bind_[bind_col].buffer = reinterpret_cast<void *>(&val);

    return;
}

//
void ZCE_Mysql_STMT_Bind::bind(size_t bind_col, double &val)
{
    stmt_bind_[bind_col].buffer_type = MYSQL_TYPE_DOUBLE;
    stmt_bind_[bind_col].buffer = reinterpret_cast<void *>(&val);

    return;
}

//
void ZCE_Mysql_STMT_Bind::bind(size_t bind_col, ZCE_Mysql_STMT_Bind::BinData_Param &bin_data)
{
    stmt_bind_[bind_col].buffer_type = bin_data.stmt_data_type_;
    stmt_bind_[bind_col].buffer = bin_data.stmt_pdata_;

    //������ܼ��ǰ󶨲���,Ҳ�ǰ󶨽��
    stmt_bind_[bind_col].buffer_length = bin_data.stmt_data_length_;
    stmt_bind_[bind_col].length = NULL;

    return;
}

//�󶨶����ƽ����������
void ZCE_Mysql_STMT_Bind::bind(size_t bind_col, ZCE_Mysql_STMT_Bind::BinData_Result &val)
{
    stmt_bind_[bind_col].buffer_type = val.stmt_data_type_;
    stmt_bind_[bind_col].buffer = val.stmt_pdata_;

    //������ܼ��ǰ󶨲���,Ҳ�ǰ󶨽��
    stmt_bind_[bind_col].buffer_length = *val.stmt_data_length_;
    stmt_bind_[bind_col].length = val.stmt_data_length_;

    return;
}

//
void ZCE_Mysql_STMT_Bind::bind(size_t bind_col, ZCE_Mysql_STMT_Bind::TimeData &val)
{
    stmt_bind_[bind_col].buffer_type = val.stmt_timetype_;
    stmt_bind_[bind_col].buffer = reinterpret_cast<void *>(val.stmt_ptime_);

    stmt_bind_[bind_col].buffer_length = sizeof(MYSQL_TIME);
    stmt_bind_[bind_col].length = NULL;

    return;
}


//��һ���ղ���
void ZCE_Mysql_STMT_Bind::bind(size_t bind_col, ZCE_Mysql_STMT_Bind::NULL_Param &val)
{
    stmt_bind_[bind_col].buffer_type = MYSQL_TYPE_NULL;
    stmt_bind_[bind_col].is_null = val.is_null_;

    return;
}



#endif //MYSQL_VERSION_ID >= 40100

