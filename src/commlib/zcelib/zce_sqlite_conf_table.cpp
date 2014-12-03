#include "zce_predefine.h"
#include "zce_sqlite_process.h"
#include "zce_sqlite_stmt_handler.h"
#include "zce_sqlite_conf_table.h"


//目前版本限制只加这一个
#if SQLITE_VERSION_NUMBER >= 3005000


//构造和析构函数
AI_IIJIMA_BINARY_DATA::AI_IIJIMA_BINARY_DATA()
{
    ai_iijima_data_[MAX_LEN_OF_AI_IIJIMA_DATA] = '\0';
}
AI_IIJIMA_BINARY_DATA::~AI_IIJIMA_BINARY_DATA()
{

}
//
void AI_IIJIMA_BINARY_DATA::clear()
{
    index_1_ = 0;
    index_2_ = 0;
    ai_data_length_ = 0;
    ai_iijima_data_[MAX_LEN_OF_AI_IIJIMA_DATA] = '\0';
    last_mod_time_ = 0;
}


/*****************************************************************************************************************
struct General_SQLite_Config 一个很通用的从DB中间得到通用配置信息的方法
*****************************************************************************************************************/
ZCE_General_Config_Table::ZCE_General_Config_Table()
{
    sql_string_ = new char[MAX_SQLSTRING_LEN];
    sqlite_handler_ = new ZCE_SQLite_DB_Handler();
}


ZCE_General_Config_Table::~ZCE_General_Config_Table()
{
    if (sql_string_)
    {
        delete sql_string_;
        sql_string_ = NULL;
    }
    if (sqlite_handler_)
    {
        delete sqlite_handler_;
        sqlite_handler_ = NULL;
    }
}

//打开一个通用的数据库
int ZCE_General_Config_Table::open_dbfile(const char *db_file,
                                          bool create_db)
{
    int ret = sqlite_handler_->open_database(db_file, create_db);
    if (ret != 0)
    {
        return ret;
    }
    return 0;
}

//创建TABLE SQL语句
void ZCE_General_Config_Table::sql_create_table(unsigned  int table_id)
{
    //构造后面的SQL
    char *ptmppoint = sql_string_;
    size_t buflen = MAX_SQLSTRING_LEN;

    //注意里面的?
    int len = snprintf(ptmppoint, buflen,
        "CREATE TABLE IF NOT EXISTS config_table_%u(index_1 INTEGER,"
        "index_2 INTEGER, conf_data BLOB, last_mod_time INTEGER);"
        "CREATE UNIQUE INDEX IF NOT EXISTS cfg_table_idx_%u ON "
        "config_table_%u(index_1, index_2)",
        table_id,
        table_id,
        table_id);
    ptmppoint += len;
    buflen -= len;

    
}

//改写的SQL
void ZCE_General_Config_Table::sql_replace_one(unsigned int table_id,
                                               unsigned int index_1,
                                               unsigned int index_2,
                                               unsigned int last_mod_time)
{
    //构造后面的SQL
    char *ptmppoint = sql_string_;
    size_t buflen = MAX_SQLSTRING_LEN;

    //注意里面的?
    int len = snprintf(ptmppoint, buflen, "REPLACE INTO config_table_%u "
                       "(index_1,index_2,conf_data,last_mod_time ) VALUES "
                       "(%u,%u,?,%u) ",
                       table_id,
                       index_1,
                       index_2,
                       last_mod_time
                      );
    ptmppoint += len;
    buflen -= len;
}

//得到选择一个确定数据的SQL
void ZCE_General_Config_Table::sql_select_one(unsigned int table_id,
                                              unsigned int index_1,
                                              unsigned int index_2)
{
    char *ptmppoint = sql_string_;
    size_t buflen = MAX_SQLSTRING_LEN;

    //构造SQL
    int len = snprintf(ptmppoint, buflen, "SELECT conf_data,last_mod_time "
                       "FROM config_table_%u WHERE ((index_1=%u) AND (index_2=%u)) ",
                       table_id,
                       index_1,
                       index_2);
    ptmppoint += len;
    buflen -= len;
}

//得到删除数据的SQL
void ZCE_General_Config_Table::sql_delete_one(unsigned int table_id,
                                              unsigned int index_1,
                                              unsigned int index_2)
{

    char *ptmppoint = sql_string_;
    size_t buflen = MAX_SQLSTRING_LEN;

    int len = snprintf(ptmppoint,
                       buflen,
                       "DELETE FROM config_table_%u WHERE "
                       " ((index_1=%u) AND (index_2=%u)) ",
                       table_id,
                       index_1,
                       index_2);
    ptmppoint += len;
    buflen -= len;
}
//
void ZCE_General_Config_Table::sql_counter(unsigned int table_id,
                                           unsigned int startno,
                                           unsigned int numquery)
{
    //构造SQL
    char *ptmppoint = sql_string_;
    size_t buflen = MAX_SQLSTRING_LEN;

    int len = snprintf(ptmppoint, buflen, "SELECT COUNT(*) FROM config_table_%u ",
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

//
void ZCE_General_Config_Table::sql_select_array(unsigned int table_id,
                                                unsigned int startno,
                                                unsigned int numquery,
                                                bool order_by_idx)
{
    char *ptmppoint = sql_string_;
    size_t buflen = MAX_SQLSTRING_LEN;

    //构造SQL
    int len = snprintf(ptmppoint, buflen, "SELECT index_1,index_2,conf_data,last_mod_time "
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

    if (order_by_idx)
    {
        len = snprintf(ptmppoint, buflen, "ORDER BY index_1,index_2 ", startno, numquery);
        ptmppoint += len;
        buflen -= len;
    }
}

///创建数据表
int ZCE_General_Config_Table::create_table(unsigned int table_id)
{

    //建表和建立索引
    sql_create_table(table_id);
    ZCE_SQLite_STMTHdl stmt_handler(sqlite_handler_);
    int ret = 0;
    ret = stmt_handler.prepare_sql_string(sql_string_);
    if (ret != 0)
    {
        return ret;
    }
    bool hash_result = false;
    ret = stmt_handler.execute_stmt_sql(hash_result);
    if (ret != 0)
    {
        return ret;
    }
    return 0;
}


//更新一条记录，
int ZCE_General_Config_Table::replace_one(unsigned int table_id,
                                          const AI_IIJIMA_BINARY_DATA &conf_data)
{
    //构造后面的SQL
    sql_replace_one(table_id,
                    conf_data.index_1_,
                    conf_data.index_2_,
                    conf_data.last_mod_time_);
    ZCE_SQLite_STMTHdl stmt_handler(sqlite_handler_);
    int ret = 0;
    ret = stmt_handler.prepare_sql_string(sql_string_);
    if (ret != 0)
    {
        return ret;
    }

    stmt_handler << ZCE_SQLite_STMTHdl::BINARY((void *)conf_data.ai_iijima_data_,
                                               conf_data.ai_data_length_);
    if (ret != 0)
    {
        return ret;
    }

    bool hash_result = false;
    ret = stmt_handler.execute_stmt_sql(hash_result);
    if (ret != 0)
    {
        return ret;
    }
    return 0;
}

//
int ZCE_General_Config_Table::select_one(unsigned int table_id,
                                         AI_IIJIMA_BINARY_DATA &conf_data)
{
    sql_select_one(table_id,
                   conf_data.index_1_,
                   conf_data.index_2_);
    ZCE_SQLite_STMTHdl stmt_handler(sqlite_handler_);
    int ret = 0;
    ret = stmt_handler.prepare_sql_string(sql_string_);
    if (ret != 0)
    {
        return ret;
    }

    bool hash_result = false;
    ret = stmt_handler.execute_stmt_sql(hash_result);
    if (ret != 0 )
    {
        return ret;
    }

    if (false == hash_result)
    {
        return -1;
    }


    stmt_handler >> ZCE_SQLite_STMTHdl::BINARY((void *)conf_data.ai_iijima_data_,
                                               conf_data.ai_data_length_);
    stmt_handler >> conf_data.last_mod_time_;

    return 0;
}

//删除一条记录
int ZCE_General_Config_Table::delete_one(unsigned int table_id,
                                         unsigned int index_1,
                                         unsigned int index_2)
{
    //构造后面的SQL
    sql_delete_one(table_id, index_1, index_2);
    ZCE_SQLite_STMTHdl stmt_handler(sqlite_handler_);
    int ret = 0;
    ret = stmt_handler.prepare_sql_string(sql_string_);
    if (ret != 0)
    {
        return ret;
    }
    bool hash_result = false;
    ret = stmt_handler.execute_stmt_sql(hash_result);
    if (ret != 0)
    {
        return ret;
    }
    return 0;
}

//
int ZCE_General_Config_Table::counter(unsigned int table_id,
                                      unsigned int startno,
                                      unsigned int numquery,
                                      unsigned int &rec_count)
{
    sql_counter(table_id, startno, numquery);
    ZCE_SQLite_STMTHdl stmt_handler(sqlite_handler_);
    int ret = 0;
    ret = stmt_handler.prepare_sql_string(sql_string_);
    if (ret != 0)
    {
        return ret;
    }

    bool hash_result = false;
    ret = stmt_handler.execute_stmt_sql(hash_result);
    if (ret != 0)
    {
        return ret;
    }

    if (false == hash_result)
    {
        return -1;
    }

    stmt_handler >> rec_count;
    return 0;
}

//查询所有的队列
int ZCE_General_Config_Table::select_array(unsigned int table_id,
                                           unsigned int startno,
                                           unsigned int numquery,
                                           bool order_by_idx,
                                           ARRARY_OF_AI_IIJIMA_BINARY &ary_ai_iijma)
{
    int ret = 0;

    //先计算数量
    unsigned int  num_counter = 0;
    ret = counter(table_id, startno, numquery, num_counter);
    if (0 != ret)
    {
        return ret;
    }

    //没有找到数据
    if (num_counter == 0)
    {
        return -1;
    }
    ary_ai_iijma.resize(num_counter);

    sql_select_array(table_id, startno, numquery, order_by_idx);
    ZCE_SQLite_STMTHdl stmt_handler(sqlite_handler_);

    ret = stmt_handler.prepare_sql_string(sql_string_);
    if (ret != 0)
    {
        return ret;
    }

    bool hash_result;
    ret = stmt_handler.execute_stmt_sql(hash_result);
    
    for (size_t i = 0; ret == 0 && hash_result == true; ++i)
    {
        stmt_handler >> ary_ai_iijma[i].index_1_;
        stmt_handler >> ary_ai_iijma[i].index_2_;
        stmt_handler >> ZCE_SQLite_STMTHdl::BINARY((void *)ary_ai_iijma[i].ai_iijima_data_,
                                                   ary_ai_iijma[i].ai_data_length_);
        stmt_handler >> ary_ai_iijma[i].last_mod_time_;

        ret = stmt_handler.execute_stmt_sql(hash_result);
    }

    //出现错误或者没有找到
    if (0 != ret)
    {
        return ret;
    }

    return 0;
}


#endif //SQLITE_VERSION_NUMBER >= 3005000



