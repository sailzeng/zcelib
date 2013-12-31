/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_sqlite3_process.h
* @author     Viviliu，Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2008年3月12日
* @brief      用于SQLite的一些操作处理，
* 
* @details    为啥加了一个3，因为SQlite当时还在2的版本
*             和3的版本过渡期间，而且SQlite残忍的不兼容了2（该恨就要狠，ACE这点上
*             婆婆妈妈的，），
*             SQLite的处理类，08年吗？感觉应该更早一点把，应该是07把，
*             当时大约是我要寻找一种数据库来存放我们当时的数据表，本来想搞ACCESS的，
*             但用ACCESS却不能拜托微软的魔咒，
*             后来living大大告诉我们RTX用了SQLite存放聊天信息，于是我们找来看看。
*             发现的确很好，后来发现这个数据库一发不可收拾，逐步在手机上成为了移动
*             终端数据库霸主。也算有缘，看见一个开源好东东的成长。
* 
*             6年以后，重新上了SQLite的网站，发现网站进步了不少，代码风格一样，直接
*             源码，随便你怎么玩。
* 
*             关于Sqlite的速度，请参考http://www.sqlite.org/speed.html，
* 
*/
#ifndef ZCE_LIB_SQLITE3_PRCOESS_H_
#define ZCE_LIB_SQLITE3_PRCOESS_H_

//目前版本限制只加这一个
#if SQLITE_VERSION_NUMBER >= 3003000

/*!
@brief      连接处理一个SQLite3数据库的，打开一个SQLite3数据库就得到Handler
            用Handler完成后面各种数据库操作。
*/
class ZCELIB_EXPORT SQLite3_DB_Handler
{

public:

    ///构造函数，
    SQLite3_DB_Handler();
    ///析构函数
    ~SQLite3_DB_Handler();

    /*!
    @brief      打开数据库，注意文件名称的路径要用UTF8编码，所以最好不要用中文?
    @return     int      打开数据库是否成功 0 成功，其他失败
    @param      db_file  数据库文件的名称，
                         由于整体代码的关系，这儿假设在WINDOWS你传入的是ANSI的
                         字符串，在LINUX，你传入的是UTF8
    */
    int open_database(const char *db_file);

    ///这个特性要3.5以后的版本才可以用
#if SQLITE_VERSION_NUMBER >= 3005000

    /*!
    @brief      只读方式打开数据库,这个特性要3.5以后的版本才可以用。
    @return     int      打开数据库是否成功 0 成功，其他失败
    @param      db_file  数据库文件的名称,ANSI 字符串
    */
    int open_readonly_db(const char *db_file);
#endif

    ///关闭数据库
    void close_database();

    ///取得错误语句Str
    const char *get_dbret_errstr();
    ///取得DB返回的错误ID
    unsigned int get_dbret_errid();

    ///取得SQLite的句柄
    inline sqlite3 *get_sqlite_handler()
    {
        return sqlite3_handler_;
    };

    ///开始一个事务
    int begin_transaction();
    ///提交一个事务，或者说结束一个事务
    int commit_transction();

    ///将同步选项关闭，建议不要使用
    int turn_off_synch();

    //执行INSERT、REPLACE的插入语句，支持多条记录一次性插入,
    //int execute_insert_sql(const char *sql_string);

protected:

    ///sqlite3的处理Handler
    sqlite3         *sqlite3_handler_;

};

#endif //SQLITE_VERSION_NUMBER >= 3003000

#endif //ZCE_LIB_SQLITE3_PRCOESS_H_

