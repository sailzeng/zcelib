#include "zce_predefine.h"
#include "zce_sqlite_conf_table.h"


//目前版本限制只加这一个
#if SQLITE_VERSION_NUMBER >= 3003000


/*****************************************************************************************************************
struct General_SQLite_Config 一个很通用的从DB中间得到通用配置信息的方法
*****************************************************************************************************************/

//改写的SQL
void General_SQLite_Config::ReplaceSQLString(unsigned int table_id,
                                             unsigned int conf_id_1,
                                             unsigned int conf_id_2,
                                             const AI_IIJIMA_BINARY_DATA &conf_data_1,
                                             const AI_IIJIMA_BINARY_DATA &conf_data_2,
                                             unsigned int last_mod_time,
                                             char *sql_string,
                                             size_t &buflen)
{
    //构造后面的SQL
    char *ptmppoint = sql_string;
    int len = snprintf(ptmppoint, buflen, "REPLACE INTO config_table_%u "
                       "(conf_id_1,conf_id_2,conf_data,last_mod_time ) VALUES "
                       "(%u,%u,%u,'%s','%s',%u) ",
                       table_id,
                       conf_id_1,
                       conf_id_2,
                       conf_data_1.ai_iijima_data_,
                       conf_data_2.ai_iijima_data_,
                       last_mod_time
                      );
    ptmppoint += len;
    buflen -= len;
}

//得到选择一个确定数据的SQL
void General_SQLite_Config::SelectSQLString(unsigned int table_id,
                                            unsigned int conf_id_1,
                                            unsigned int conf_id_2,
                                            char *sql_string,
                                            size_t &buflen)
{
    char *ptmppoint = sql_string;

    //构造SQL
    int len = snprintf(ptmppoint, buflen, "SELECT conf_id_1,conf_id_2,conf_data,last_mod_time "
                       "FROM config_table_%u WHERE ((conf_id_1=%u) AND (conf_id_2=%u)) ",
                       table_id,
                       conf_id_1,
                       conf_id_2);
    ptmppoint += len;
    buflen -= len;
}

//得到删除数据的SQL
void General_SQLite_Config::DeleteSQLString(unsigned int table_id,
                                            unsigned int conf_id_1,
                                            unsigned int conf_id_2,
                                            char *sql_string,
                                            size_t &buflen)
{

    char *ptmppoint = sql_string;
    int len = snprintf(ptmppoint, buflen, "DELETE FROM config_table_%u WHERE ((conf_id_1=%u) AND (conf_id_2=%u)) ",
                       table_id,
                       conf_id_1,
                       conf_id_2);
    ptmppoint += len;
    buflen -= len;
}
//
void General_SQLite_Config::GetCountSQLString(unsigned int table_id,
                                              char *sql_string,
                                              size_t &buflen)
{
    //构造SQL
    char *ptmppoint = sql_string;
    int len = snprintf(ptmppoint, buflen, "SELECT COUNT(*) FROM config_table_%u ) ",
                       table_id);
    ptmppoint += len;
    buflen -= len;
}

//
void General_SQLite_Config::GetArraySQLString(unsigned int table_id,
                                              unsigned int startno,
                                              unsigned int numquery,
                                              char *sql_string,
                                              size_t &buflen)
{
    //构造SQL
    char *ptmppoint = sql_string;
    int len = snprintf(ptmppoint, buflen, "SELECT conf_id_1,conf_id_2,conf_data,last_mod_time "
                       "FROM config_table_%u ",
                       table_id);
    ptmppoint += len;
    buflen -= len;

    //如果要查询LIMIT的数目
    if (numquery != 0)
    {
        len = snprintf(ptmppoint, buflen, "LIMIT %u,%u ", startno, numquery);
        ptmppoint += len;
        buflen -= len;
    }
}




#endif //SQLITE_VERSION_NUMBER >= 3003000



