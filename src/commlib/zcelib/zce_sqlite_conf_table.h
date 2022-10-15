/*!
* @copyright  2004-2014  Apache License, Version 2.0 FULLSAIL
* @filename   zce_sqlite_conf_table.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       Thursday, November 27, 2009
* @brief
*
*
* @details
*
*
*
* @note       AI_IIJIMA 饭岛爱
*             最近比较烦比你烦也比你烦，我梦见和饭岛爱一起晚餐；
*             梦中的餐厅灯光太昏暗，我遍寻不著那蓝色的小药丸
*             最近有点事情多到了极点，但是也无聊到了极点，
*             发现最近代码的名字乱七八糟的东西越来越多.
*             说了很多饭岛爱的废话。不如在代码里面纪念吧，
*             一个充满矛盾人的，名字有爱，被无数人无数幻想，却一生无爱，
*             被李宗盛传唱，但不被大众所知，
*             结果在死后却上了sina网的首页，引发中国人民的对日本一个女孩怀念。
*             半推荐她的小说《柏拉图式性爱》，
*             小说在一个侧面反映了黑暗的资本主义社会对一个小女孩身心的摧残，所以推荐大家读读，
*             最后要说的是，生命不易，希望大家珍惜。
*             想起那段把一个产品做了3遍的历史。
*/


/*
表格和索引定义，
CREATE TABLE IF NOT EXISTS config_table_8(index_1 INTEGER,index_2 INTEGER,conf_data BLOB ,last_mod_time INTEGER);
CREATE UNIQUE INDEX IF NOT EXISTS cfg_table_idx_8 ON config_table_8 (index_1,index_2)

*/

#ifndef ZCE_LIB_SQLITE_CONF_TABLE_H_
#define ZCE_LIB_SQLITE_CONF_TABLE_H_

//目前版本限制只加这一个
#if SQLITE_VERSION_NUMBER >= 3005000



/*!
* @brief      用于将一些结构转换为二进制数据，存放于数据字段中
*
* @note
*/
struct AI_IIJIMA_BINARY_DATA
{


public:

    //!构造和析构函数
    AI_IIJIMA_BINARY_DATA();
    ~AI_IIJIMA_BINARY_DATA();

    //!
    void clear();

    //!比较函数
    bool operator < (const AI_IIJIMA_BINARY_DATA &right) const;

#if defined ZCE_USE_PROTOBUF && ZCE_USE_PROTOBUF == 1

    //!将一个结构进行编码
    int protobuf_encode(unsigned int index_1,
                        unsigned int index_2,
                        const google::protobuf::MessageLite *msg);

    //!将一个结构进行解码
    int protobuf_decode(unsigned int *index_1,
                        unsigned int *index_2,
                        google::protobuf::MessageLite *msg);

#endif

public:

    //!动态编解码的数据区长度
    static const int MAX_LEN_OF_AI_IIJIMA_DATA = 32 * 1024 - 1;

public:

    //!索引1
    unsigned int index_1_ = 0;
    //!索引2 默认为0
    unsigned int index_2_ = 0;

    //!数据区长度
    int ai_data_length_ = 0;
    //!动态数据取
    char ai_iijima_data_[MAX_LEN_OF_AI_IIJIMA_DATA + 1];

    //!最后修改时间
    unsigned int last_mod_time_ = 0;
};

typedef std::vector <AI_IIJIMA_BINARY_DATA>   ARRARY_OF_AI_IIJIMA_BINARY;



/******************************************************************************************
struct BaobaoGeneralDBConf 一个很通用的从DB中间得到通用配置信息的方法
******************************************************************************************/
class ZCE_SQLite_DB_Handler;

//一个很通用的从DB中间得到通用配置信息的结构
class ZCE_General_Config_Table
{

public:

    ZCE_General_Config_Table();
    ~ZCE_General_Config_Table();

protected:

    //!创建TABLE SQL语句
    void sql_create_table(unsigned  int table_id);
    //!创建INDEX SQL语句
    void sql_create_index(unsigned  int table_id);


    //!改写的STMT SQL
    void sql_replace_bind(unsigned  int table_id);

    //!改写的SQL,文本格式，用x
    void sql_replace_one(unsigned  int table_id,
                         unsigned int index_1,
                         unsigned int index_2,
                         size_t blob_len,
                         const char *blob_data,
                         unsigned int last_mod_time);


    //!得到选择一个确定数据的SQL
    void sql_select_one(unsigned int table_id,
                        unsigned int index_1,
                        unsigned int index_2);

    //得到删除数据的SQL
    void sql_delete_one(unsigned int table_id,
                        unsigned int index_1,
                        unsigned int index_2);


    //!计算查询的总数
    void sql_counter(unsigned int table_id,
                     unsigned int startno,
                     unsigned int numquery);

    //!查询数据队列，部分数据或者全部数据
    void sql_select_array(unsigned int table_id,
                          unsigned int startno,
                          unsigned int numquery);

    //!

    /*!
    * @brief      base16的算法，
    * @return     int
    * @param      in
    * @param      in_len
    * @param      out
    * @param[in/out] out_len 收入时标识outbuf的长度，输出时标识最后生成的字符串长度
    * @note       这儿是为了避免引入太多文件，而且base16实现较为简单，所以没有用zce的encode代码
    */
    int base16_encode(const char *in,
                      size_t in_len,
                      char *out,
                      size_t *out_len);
public:

    //!打开一个通用的数据库
    int open_dbfile(const char *db_file,
                    bool read_only,
                    bool create_db);

    //!创建数据表
    int create_table(unsigned int table_id);

    //!关闭打开的数据库
    void close_dbfile();

    //!UPDATE 或者 INSERT 一个记录
    int replace_one(unsigned int table_id,
                    const AI_IIJIMA_BINARY_DATA *conf_data);

    //UPDATE 或者 INSERT 一组记录
    int replace_array(unsigned int table_id,
                      const ARRARY_OF_AI_IIJIMA_BINARY *ary_ai_iijma);

    //!查询了一条记录
    int select_one(unsigned int table_id,
                   AI_IIJIMA_BINARY_DATA *conf_data);

    //!删除一条记录
    int delete_one(unsigned int table_id,
                   unsigned int index_1,
                   unsigned int index_2);

    //!查询记录总数
    int counter(unsigned int table_id,
                unsigned int startno,
                unsigned int numquery,
                unsigned int *rec_count);

    //!查询数据队列，部分数据（限制查询数量）或者全部数据
    int select_array(unsigned int table_id,
                     unsigned int startno,
                     unsigned int numquery,
                     ARRARY_OF_AI_IIJIMA_BINARY *ary_ai_iijma);

    /*!
    * @brief      对比两个数据表格，找出差异，然后找出差异的SQL，用于数据文件更新等功能
    * @return     int      是否成功的完成了比较
    * @param      old_db   比较数据库old文件名称
    * @param      new_db   比较数据库new文件名称
    * @param      table_id 比较的table id
    * @param[out] update_sql 返回的更新SQL语句
    * @note
    */
    int compare_table(const char *old_db,
                      const char *new_db,
                      unsigned int table_id,
                      std::string *update_sql);
protected:
    //
    const static size_t MAX_SQLSTRING_LEN = AI_IIJIMA_BINARY_DATA::MAX_LEN_OF_AI_IIJIMA_DATA * 2 + 1024;

public:

    //! SQL语句
    char *sql_string_ = NULL;
    //!
    ZCE_SQLite_DB_Handler *sqlite_handler_;


};


#endif //SQLITE_VERSION_NUMBER >= 3005000

#endif //ZCE_LIB_SQLITE_CONF_TABLE_H_
