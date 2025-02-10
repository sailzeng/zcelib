#pragma once

namespace zce::db
{
template <typename CONNECT, typename RESULT, typename STMT>
class database
{
public:
    //!构造函数，不用处理什么，相关的成员变量的析构都进行了处理
    database() = default;
    ~database() = default;

    //!避免拷贝
    database(const database &) = delete;
    database& operator=(const database&) = delete;

    /*!
    * @brief      初始化服务器,使用hostname进行连接,可以不立即连接和立即连接，你自己控制。
    * @return     int     0成功，-1失败
    * @param      host    DB HOST名称,IP地址
    * @param      user    DB USER名称
    * @param      pwd     DB PWD密码
    * @param      port    端口
    * @param      atonce  是否立即连接服务器
    */
    int initiate(const char* host,
                 const char* user,
                 const char* pwd,
                 unsigned int port,
                 bool atonce = false)
    {
        db_host_ = host;
        db_user_name_ = user;
        db_password_ = pwd;
        db_port_ = port;

        if (connect_atonce)
        {
            return connect();
        }

        return 0;
    }

    //!连接Query 服务器,如果希望初始化后进行连接,使用这个函数
    int connect()
    {
        int ret = 0;

        //连接数据库
        if (db_connect_.is_connected() == false)
        {
            //如果设置过HOST，用HOST NAME进行连接
            if (db_host_.length() > 0)
            {
                ret = db_connect_.connect_by_host(db_host_.c_str(),
                                                  db_user_name_.c_str(),
                                                  db_password_.c_str(),
                                                  nullptr,
                                                  db_port_);
            }
            else
            {
                ZCE_ASSERT(false);
            }

            //如果错误
            if (ret != 0)
            {
                ZCE_LOG(RS_ERROR, "[zcelib] DB Error : [%u]:%s.",
                        db_connect_.error_no(),
                        db_connect_.error_message()
                );
                return -1;
            }

            //
            db_command_.set_connect(&db_connect_);
        }

        return  0;
    }
    //!断开连接
    void disconnect();

    /*!
    * @brief      用于非SELECT语句(INSERT,UPDATE)，
    * @return     int
    * @param      sql
    * @param      sql_len SQL语句长度
    * @param      num_affect 返回的收到影响的记录条数
    * @param      insert_id  返回的插入的LAST_INSERT_ID
    */
    int query(const char* sql,
              size_t sql_len,
              uint64_t& num_affect,
              uint64_t& insert_id);

    /*!
    * @brief      执行家族的SQL语句,用于SELECT语句,直接转储结果集合的方法
    * @return     int
    * @param      sql  SQL语句
    * @param      sql_len
    * @param      num_affect  返回参数,返回的查询的记录个数
    * @param      db_result  返回参数,查询的结果集合
    * @note       几个query函数连接周期不会关闭链接,zce::mysql::Connect对象再析构时断链接
    */
    int query(const char* sql,
              size_t sql_len,
              uint64_t& num_affect,
              RESULT& db_result);

protected:

    ///数据库IP地址
    std::string       db_host_;
    ///DB数据库的端口号码
    unsigned int      db_port_ = 0;

    ///数据库用户名称
    std::string       db_user_name_;
    ///DB数据库密码ID
    std::string       db_password_;

    ///DB 文件
    std::string       db_file_;

    ///MYSQL数据库连接对象
    CONNECT db_connect_;

    ///MYSQL命令执行对象
    COMMAND db_command_;
};

class execute
{
    ///得到MYSQL定义的错误返回
    unsigned int error_message(char* szerr,
                               size_t buflen);

    ///错误语句Str
    const char* error_message();
    ///DB返回的错误ID
    unsigned int error_id();

    ///得到DB访问的语句
    const char* get_query_sql(void);

    ///得到Real Escape String ,Real表示根据当前的MYSQL Connet的字符集,得到Escape String
    ///Escape String 为将字符传中的相关字符进行转义后的语句,比如',",\等字符
    unsigned int make_real_escape_string(char* tostr,
                                         const char* fromstr,
                                         unsigned int fromlen);
};
}