/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_sqlite3_process.h
* @author     Viviliu，Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2008年3月12日
* @brief      用于SQLite的一些操作处理，
*             在这个内部包装的实现完全用UTF8的格式函数，不考虑UTF16
* @details    原来为啥加了一个后缀3，因为SQlite当时还在2的版本
*             和3的版本过渡期间，而且SQlite残忍的不兼容了2（该恨就要狠，ACE这点上
*             婆婆妈妈的，），
*             SQLite的处理类，08年吗？感觉应该更早一点把，应该是07把，
*             当时大约是我要寻找一种数据库来存放我们当时的数据表，本来想搞ACCESS的，
*             但用ACCESS却不能摆脱微软的魔咒，
*             后来living大大告诉我们RTX用了SQLite存放聊天信息，于是我们找来看看。
*             发现的确很好，后来发现这个数据库一发不可收拾，逐步在手机上成为了移动
*             终端数据库霸主。也算有缘，看见一个开源好东东的成长。
*
*             6年以后，重新上了SQLite的网站，发现网站进步了不少，代码风格一样，直接
*             源码，随便你怎么玩。
*
*             关于Sqlite的速度，请参考http://www.sqlite.org/speed.html，
*
*             幸福就是，坚持了应该坚持的，放弃了应该放弃的，珍惜现在拥有的，不后悔已
*             经决定的
*
*/
#pragma once

//目前版本限制只加这一个
#if SQLITE_VERSION_NUMBER >= 3005000

namespace zce
{
//==============================================================================================
class sqlite_result;
/*!
@brief      连接处理一个SQLite3数据库的，打开一个SQLite3数据库就得到Handler
            用Handler完成后面各种数据库操作。
*/
class sqlite_hdl
{
public:

    //!构造函数，
    sqlite_hdl();
    //!析构函数
    ~sqlite_hdl();

    /*!
    @brief      打开数据库，注意文件名称的路径要用UTF8编码，所以最好不要用中文?
    @return     int      打开数据库是否成功 0 成功，其他失败
    @param      db_file  数据库文件的名称，
                         由于整体代码的关系，这儿假设在WINDOWS你传入的是ANSI的
                         字符串，在LINUX，你传入的是UTF8
    @param      read_only 只读
    @param      create_db 是否需要创建数据库，(db不存在时)
    */
    int open_database(const char* db_file,
                      bool read_only,
                      bool create_db);

    //!关闭数据库
    void close_database();

    //!取得错误语句Str
    const char* error_message();
    //!取得DB返回的错误ID
    int error_code();

    //!取得SQLite的句柄
    inline sqlite3* get_sqlite_handler()
    {
        return sqlite3_handler_;
    };

    //!开始一个事务
    int begin_transaction();
    //!提交一个事务，或者说结束一个事务
    int commit_transction();

    //!将同步选项关闭，建议不要使用
    int turn_off_synch();

    //!执行DDL等不需要结果的SQL
    int exe(const char* sql_string);

    /*!
    * @brief      执行SQL查下的封装,（二进制的不行）
    * @return     int 返回0表示成功，
    * @param      sql_string SQL语句
    * @param      执行的结果，返回值
    * @note       内部会调用sqlite3_get_table,sqlite3_free_table，
    *             这个函数在SQLite中不是被推荐的函数，建议使用时考虑一下，虽然其
    *             执行查询，确实比sqlite3_exec，方便
    *             另外，这个函数应该不能处理二进制数据，因为你无法得知结果长度
    */
    int get_table(const char* sql_string,
                  zce::sqlite_result* result);

protected:

    //!sqlite3的处理Handler
    sqlite3* sqlite3_handler_;
};
}

#endif //SQLITE_VERSION_NUMBER >= 3005000
