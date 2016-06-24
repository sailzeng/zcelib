#include "zce_predefine.h"
#include "zce_trace_debugging.h"
#include "zce_sqlite_db_handler.h"
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

//清理函数
void AI_IIJIMA_BINARY_DATA::clear()
{
    index_1_ = 0;
    index_2_ = 0;
    ai_data_length_ = 0;
    ai_iijima_data_[MAX_LEN_OF_AI_IIJIMA_DATA] = '\0';
    last_mod_time_ = 0;
}

//比较函数
bool AI_IIJIMA_BINARY_DATA::operator < (const AI_IIJIMA_BINARY_DATA right) const
{
    if (this->index_1_ < right.index_1_)
    {
        return true;
    }
    else if (this->index_1_ == right.index_1_)
    {
        if (this->index_2_ < right.index_2_)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}


#if defined ZCE_USE_PROTOBUF && ZCE_USE_PROTOBUF == 1


int AI_IIJIMA_BINARY_DATA::protobuf_encode(unsigned int index_1,
                                           unsigned int index_2,
                                           const google::protobuf::MessageLite *msg)
{

    if (!msg->IsInitialized())
    {
        ZCE_LOG(RS_ERROR, "class [%s] protobuf encode fail, IsInitialized return false.error string [%s].",
            msg->GetTypeName().c_str(),
            msg->InitializationErrorString().c_str());
        return -1;
    }

    index_1_ = index_1;
    index_2_ = index_2;

    int protobuf_len = msg->ByteSize();
    if (protobuf_len > MAX_LEN_OF_AI_IIJIMA_DATA)
    {
        ZCE_LOG(RS_ERROR, "Config [%d|%d] class %s protobuf encode fail, ByteSize return %d >"
                " MAX_LEN_OF_AI_IIJIMA_DATA %d.\n",
                index_1,
                index_2,
                typeid(msg).name(),
                protobuf_len);
        return -1;
    }

    bool bret = msg->SerializeToArray(ai_iijima_data_, MAX_LEN_OF_AI_IIJIMA_DATA);
    if (!bret)
    {
        ZCE_LOG(RS_ERROR, "Config [%d|%d] class %s protobuf encode fail, SerializeToArray return false.",
                index_1,
                index_2,
                typeid(msg).name());
        return -1;
    }
    ai_data_length_ = protobuf_len;
    last_mod_time_ = static_cast<unsigned int>( ::time(NULL));
    return 0;
}


int AI_IIJIMA_BINARY_DATA::protobuf_decode(unsigned int *index_1,
                                           unsigned int *index_2,
                                           google::protobuf::MessageLite *msg)
{
    bool bret = msg->ParseFromArray(ai_iijima_data_, ai_data_length_);

    if (false == bret)
    {
        ZCE_LOG(RS_ERROR, "Class [%s] protobuf decode fail,ParseFromArray return false.", msg->GetTypeName().c_str());
        return -1;
    }
    *index_1 = index_1_;
    *index_2 = index_2_;
    if (!msg->IsInitialized())
    {
        ZCE_LOG(RS_ERROR, "class [%s] protobuf encode fail, IsInitialized return false.error string [%s].",
            msg->GetTypeName().c_str(),
            msg->InitializationErrorString().c_str());
        return -1;
    }
    return 0;
}

#endif

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

    sqlite_handler_->close_database();
    if (sqlite_handler_)
    {
        delete sqlite_handler_;
        sqlite_handler_ = NULL;
    }
}

//打开一个通用的数据库
int ZCE_General_Config_Table::open_dbfile(const char *db_file,
                                          bool read_only,
                                          bool create_db)
{
    int ret = sqlite_handler_->open_database(db_file, read_only, create_db);
    if (ret != 0)
    {
        return ret;
    }
    return 0;
}


void ZCE_General_Config_Table::close_dbfile()
{
    sqlite_handler_->close_database();
}

//创建TABLE SQL语句
void ZCE_General_Config_Table::sql_create_table(unsigned  int table_id)
{
    //构造后面的SQL
    char *ptmppoint = sql_string_;
    size_t buflen = MAX_SQLSTRING_LEN;

    int len = snprintf(ptmppoint, buflen,
                       "DROP TABLE IF EXISTS config_table_%u;"
                       "DROP INDEX IF EXISTS cfg_table_idx_%u;"
                       "CREATE TABLE IF NOT EXISTS config_table_%u(index_1 INTEGER,"
                       "index_2 INTEGER, conf_data BLOB, last_mod_time INTEGER);"
                       "CREATE UNIQUE INDEX IF NOT EXISTS cfg_table_idx_%u ON "
                       "config_table_%u(index_1, index_2);",
                       table_id,
                       table_id,
                       table_id,
                       table_id,
                       table_id);
    ptmppoint += len;
    buflen -= len;


}

//改写的SQL
void ZCE_General_Config_Table::sql_replace_bind(unsigned int table_id)
{
    //构造后面的SQL
    char *ptmppoint = sql_string_;
    size_t buflen = MAX_SQLSTRING_LEN;

    //注意里面的?
    int len = snprintf(ptmppoint, buflen, "REPLACE INTO config_table_%u "
                       "(index_1,index_2,conf_data,last_mod_time ) VALUES "
                       "(?,?,?,?) ;",
                       table_id
                      );
    ptmppoint += len;
    buflen -= len;
}


//!改写的SQL,文本格式，用x
//!此函数保留主要是用于文件比较，产生更新SQL，因为是更新SQL，所以全部用的x
void ZCE_General_Config_Table::sql_replace_one(unsigned  int table_id,
                                               unsigned int index_1,
                                               unsigned int index_2,
                                               size_t blob_len,
                                               const char * blob_data,
                                               unsigned int last_mod_time)
{
    //构造后面的SQL
    char *ptmppoint = sql_string_;
    size_t buflen = MAX_SQLSTRING_LEN;

    //对于空间，我们是预留了足够的空间的，就不检查边界了
	//对于x,x的作用是说明里面的数据''用base 16的编码处理，视作二进制
    int len = snprintf(ptmppoint, buflen, "REPLACE INTO config_table_%u "
                       "(index_1,index_2,conf_data,last_mod_time ) VALUES "
                       "(%u,%u,x'",
                       table_id,
                       index_1,
                       index_2 );

    ptmppoint += len;
    buflen -= len;

    size_t out_len = buflen;
    base16_encode(blob_data,
                  blob_len,
                  ptmppoint,
                  &out_len);
    ptmppoint += out_len;
    buflen -= out_len;

    len = snprintf(ptmppoint, buflen, "',%u);", last_mod_time);
    ptmppoint += out_len;
    buflen -= out_len;

}


//BASE16的编码
int ZCE_General_Config_Table::base16_encode(const char *in,
											size_t in_len,
											char *out,
											size_t *out_len)
{
	//
	static const char BASE16_ENC_MAP[] = "0123456789abcdef";

	ZCE_ASSERT(in != NULL && out != NULL && out_len != NULL);

	size_t output_len = in_len * 2;
	if (*out_len < output_len + 1)
	{
		*out_len = output_len + 1;
		errno = ENOMEM;
		return -1;
	}

	const char *p = in;
	char *q = out;

	for (size_t i = 0; i < in_len; i++)
	{
		char c = p[i] & 0xff;
		*q++ = BASE16_ENC_MAP[(c >> 4) & 0xf];
		*q++ = BASE16_ENC_MAP[c & 0xf];
	}
	out[output_len] = '\0';
	*out_len = output_len;
	return 0;
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
                       " ((index_1=%u) AND (index_2=%u)) ;",
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
                                                unsigned int numquery)
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
}

//!创建数据表
int ZCE_General_Config_Table::create_table(unsigned int table_id)
{

    //建表和建立索引
    sql_create_table(table_id);

    int ret = 0;
    ret = sqlite_handler_->execute(sql_string_);
    if (ret != 0)
    {
        return ret;
    }

    return 0;
}


//更新一条记录，
int ZCE_General_Config_Table::replace_one(unsigned int table_id,
                                          const AI_IIJIMA_BINARY_DATA *conf_data)
{
    //构造后面的SQL
    sql_replace_bind(table_id);
    ZCE_SQLite_STMTHdl stmt_handler(sqlite_handler_);
    int ret = 0;

    ret = stmt_handler.begin_transaction();
    if (ret != 0)
    {
        return ret;
    }

    ret = stmt_handler.prepare(sql_string_);
    if (ret != 0)
    {
        return ret;
    }

    ZCE_SQLite_STMTHdl::BIN_Param binary_data((void *)conf_data->ai_iijima_data_,
                                           conf_data->ai_data_length_);
    stmt_handler << conf_data->index_1_;
    stmt_handler << conf_data->index_2_;
    stmt_handler << binary_data;
    stmt_handler << conf_data->last_mod_time_;

    bool hash_result = false;
    ret = stmt_handler.execute_stmt_sql(hash_result);
    if (ret != 0)
    {
        return ret;
    }
    ret = stmt_handler.commit_transction();
    if (ret != 0)
    {
        return ret;
    }
    return 0;
}


int ZCE_General_Config_Table::replace_array(unsigned int table_id,
                                            const ARRARY_OF_AI_IIJIMA_BINARY *ary_ai_iijma)
{
    //构造后面的SQL
    sql_replace_bind(table_id);
    ZCE_SQLite_STMTHdl stmt_handler(sqlite_handler_);
    int ret = 0;

    ret = stmt_handler.begin_transaction();
    if (ret != 0)
    {
        return ret;
    }

    const size_t ary_size = ary_ai_iijma->size();
    for (size_t i = 0; i < ary_size; ++i)
    {
        //感觉SQLite3的 STMT欠火候，第二次使用还要prepare
        ret = stmt_handler.prepare(sql_string_);
        if (ret != 0)
        {
            return ret;
        }

        ZCE_SQLite_STMTHdl::BIN_Param binary_data((void *)(*ary_ai_iijma)[i].ai_iijima_data_,
                                               (*ary_ai_iijma)[i].ai_data_length_);
        stmt_handler << (*ary_ai_iijma)[i].index_1_;
        stmt_handler << (*ary_ai_iijma)[i].index_2_;
        stmt_handler << binary_data;
        stmt_handler << (*ary_ai_iijma)[i].last_mod_time_;

        bool hash_result = false;
        ret = stmt_handler.execute_stmt_sql(hash_result);
        if (ret != 0)
        {
            return ret;
        }
    }

    ret = stmt_handler.commit_transction();
    if (ret != 0)
    {
        return ret;
    }
    return 0;
}

//
int ZCE_General_Config_Table::select_one(unsigned int table_id,
                                         AI_IIJIMA_BINARY_DATA *conf_data)
{
    sql_select_one(table_id,
                   conf_data->index_1_,
                   conf_data->index_2_);
    ZCE_SQLite_STMTHdl stmt_handler(sqlite_handler_);
    int ret = 0;
    ret = stmt_handler.prepare(sql_string_);
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

    ZCE_SQLite_STMTHdl::BIN_Result binary_data((void *)conf_data->ai_iijima_data_,
                                           &(conf_data->ai_data_length_));
    stmt_handler >> binary_data;
    stmt_handler >> conf_data->last_mod_time_;

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
    ret = stmt_handler.prepare(sql_string_);
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
                                      unsigned int *rec_count)
{
    sql_counter(table_id, startno, numquery);
    ZCE_SQLite_STMTHdl stmt_handler(sqlite_handler_);
    int ret = 0;
    ret = stmt_handler.prepare(sql_string_);
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

    stmt_handler >> *rec_count;
    return 0;
}

//查询所有的队列
int ZCE_General_Config_Table::select_array(unsigned int table_id,
                                           unsigned int startno,
                                           unsigned int numquery,
                                           ARRARY_OF_AI_IIJIMA_BINARY *ary_ai_iijma)
{
    int ret = 0;

    //先计算数量
    unsigned int  num_counter = 0;
    ret = counter(table_id, startno, numquery, &num_counter);
    if (0 != ret)
    {
        return ret;
    }

    //没有找到数据
    if (num_counter == 0)
    {
        return -1;
    }
    ary_ai_iijma->resize(num_counter);

    sql_select_array(table_id, startno, numquery);
    ZCE_SQLite_STMTHdl stmt_handler(sqlite_handler_);

    ret = stmt_handler.prepare(sql_string_);
    if (ret != 0)
    {
        return ret;
    }

    bool hash_result;
    ret = stmt_handler.execute_stmt_sql(hash_result);

    for (size_t i = 0; ret == 0 && hash_result == true; ++i)
    {
        stmt_handler >> (*ary_ai_iijma)[i].index_1_;
        stmt_handler >> (*ary_ai_iijma)[i].index_2_;

        int blob_len = stmt_handler.cur_column_bytes();
        if (blob_len > AI_IIJIMA_BINARY_DATA::MAX_LEN_OF_AI_IIJIMA_DATA)
        {
            ZCE_LOG(RS_ERROR, "Error current column bytes length [%u] > "
                    "AI_IIJIMA_BINARY_DATA::MAX_LEN_OF_AI_IIJIMA_DATA [%u]." ,
                    blob_len, AI_IIJIMA_BINARY_DATA::MAX_LEN_OF_AI_IIJIMA_DATA);
            return -1;
        }

        ZCE_SQLite_STMTHdl::BIN_Result binary_data((void *)(*ary_ai_iijma)[i].ai_iijima_data_,
                                               &((*ary_ai_iijma)[i].ai_data_length_));

        stmt_handler >> binary_data;
        stmt_handler >> (*ary_ai_iijma)[i].last_mod_time_;

        ret = stmt_handler.execute_stmt_sql(hash_result);
    }

    //出现错误或者没有找到
    if (0 != ret)
    {
        return ret;
    }

    return 0;
}


//对比两个数据表格，找出差异，然后找出差异的SQL
int ZCE_General_Config_Table::compare_table(const char *old_db,
                                            const char *new_db,
                                            unsigned int table_id,
                                            std::string *update_sql)
{
    int ret = 0;



    //读取旧数据
    ret = open_dbfile(old_db, true , false);
    if (0 != ret)
    {
        return ret;
    }
    ARRARY_OF_AI_IIJIMA_BINARY old_ai_iijma;
    ret = select_array(table_id,
                       0,
                       0,
                       &old_ai_iijma);
    if (0 != ret)
    {
        return ret;
    }

    //读取新数据
    ret = open_dbfile(new_db, true, false);
    if (0 != ret)
    {
        return ret;
    }
    ARRARY_OF_AI_IIJIMA_BINARY new_ai_iijma;
    ret = select_array(table_id,
                       0,
                       0,
                       &new_ai_iijma);
    if (0 != ret)
    {
        return ret;
    }

    //把新旧数据排序，方便比较
    std::sort(old_ai_iijma.begin(), old_ai_iijma.end());
    std::sort(new_ai_iijma.begin(), new_ai_iijma.end());

    update_sql->reserve(1024*1024*8);

    //两个都有序，找出差异的元素
    size_t p = 0, q = 0;
    for (; p < old_ai_iijma.size();)
    {
        //如果对比的两者相等
        if (old_ai_iijma[p].index_1_ == new_ai_iijma[q].index_1_ &&
            old_ai_iijma[p].index_2_ == new_ai_iijma[q].index_2_)
        {
            if (old_ai_iijma[p].ai_data_length_ == new_ai_iijma[q].ai_data_length_ &&
                0 == ::memcmp(old_ai_iijma[p].ai_iijima_data_,
                              new_ai_iijma[q].ai_iijima_data_,
                              old_ai_iijma[p].ai_data_length_))
            {
                //old[p] = new[q]相同，
                ++p;
                ++q;
            }
            else
            {
                //old[p] != new[q],对比的记录不相同，REPLACE
                sql_replace_one(table_id,
                                new_ai_iijma[q].index_1_,
                                new_ai_iijma[q].index_2_,
                                new_ai_iijma[q].ai_data_length_,
                                new_ai_iijma[q].ai_iijima_data_,
                                new_ai_iijma[q].last_mod_time_);
                *update_sql += sql_string_;

                ++p;
                ++q;
            }

        }
        //继续在 new 里面寻找 old[p]
        else
        {
            size_t r = q++;
            for (; r < new_ai_iijma.size();)
            {
                if (old_ai_iijma[p].index_1_ == new_ai_iijma[r].index_1_ &&
                    old_ai_iijma[p].index_2_ == new_ai_iijma[r].index_2_)
                {
                    bool r_is_equal = false;
                    if (old_ai_iijma[p].ai_data_length_ == new_ai_iijma[r].ai_data_length_ &&
                        0 == ::memcmp(old_ai_iijma[p].ai_iijima_data_,
                                      new_ai_iijma[r].ai_iijima_data_,
                                      old_ai_iijma[p].ai_data_length_))
                    {
                        r_is_equal = true;
                    }

                    //index相同的位置的数据是否一致的，决定这个位置是否更新
                    size_t end_pos = r_is_equal ? r - 1 : r;
                    for (size_t s = q; s < end_pos; ++s)
                    {
                        //new[q] 到 new[r] 都是新赠的，REPLACE
                        sql_replace_one(table_id,
                                        new_ai_iijma[s].index_1_,
                                        new_ai_iijma[s].index_2_,
                                        new_ai_iijma[s].ai_data_length_,
                                        new_ai_iijma[s].ai_iijima_data_,
                                        new_ai_iijma[s].last_mod_time_);
                        *update_sql += sql_string_;
                    }

                    break;

                }
            }

            if (r < new_ai_iijma.size())
            {
                ++p;
                q = r++;
            }
            //old[p] 是多出的，DELETE
            else
            {
                sql_delete_one(table_id, old_ai_iijma[p].index_1_, old_ai_iijma[p].index_2_);
                *update_sql += sql_string_;
                ++p;
            }
        }

    }

    //如果new[q] 不是 最后一个节点，那么new[q]到尾部都是新增的数据 REPLACE
    for (; q < new_ai_iijma.size(); ++q)
    {
        //new[q] 到 new[r] 都是新赠的，REPLACE
        sql_replace_one(table_id,
                        new_ai_iijma[q].index_1_,
                        new_ai_iijma[q].index_2_,
                        new_ai_iijma[q].ai_data_length_,
                        new_ai_iijma[q].ai_iijima_data_,
                        new_ai_iijma[q].last_mod_time_);
        *update_sql += sql_string_;
    }

    return 0;
}





#endif //SQLITE_VERSION_NUMBER >= 3005000



