/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_mysql_stmtbind.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2005年10月17日
* @brief
*
* @details
*
* @note
*
*/

#ifndef ZCE_LIB_MYSQL_STMT_RESULT_H_
#define ZCE_LIB_MYSQL_STMT_RESULT_H_

//如果你要用MYSQL的库
#if defined ZCE_USE_MYSQL

//这些函数都是4.1.2后的版本功能
#if MYSQL_VERSION_ID >= 40100

#include "zce_trace_log_debug.h"


class STMT_BindData_Adaptor;
class STMT_BindTime_Adaptor;
class ZCE_Mysql_STMT_Bind;

/*!
* @brief      仅仅是为了适配 ZCE_Mysql_STMT_Bind << 的操作符号
*             绑定2进制数据
*/
class ZCELIB_EXPORT STMT_BindData_Adaptor
{

    friend class ZCE_Mysql_STMT_Bind;

public:
    //
    STMT_BindData_Adaptor(enum_field_types strtype, void *strdata, unsigned long szstr):
        stmt_data_type_(strtype),
        stmt_pdata_(strdata),
        stmt_data_length_(szstr)
    {
    };
    //
    ~STMT_BindData_Adaptor()
    {
    };
protected:
    //
    enum_field_types  stmt_data_type_;
    //
    void             *stmt_pdata_;
    //
    unsigned long     stmt_data_length_;
};

/*!
@brief      仅仅是为了适配 ZCE_Mysql_STMT_Bind << 的操作符号

*/
class ZCELIB_EXPORT STMT_BindTime_Adaptor
{
    friend class ZCE_Mysql_STMT_Bind;
protected:
    //
    enum_field_types  stmt_timetype_;
    //
    MYSQL_TIME       *stmt_ptime_;

public:
    //
    STMT_BindTime_Adaptor(enum_field_types timetype, MYSQL_TIME *pstmttime):
        stmt_timetype_(timetype),
        stmt_ptime_(pstmttime)
    {
        ZCE_ASSERT(stmt_timetype_ == MYSQL_TYPE_TIMESTAMP ||
                   stmt_timetype_ == MYSQL_TYPE_DATE ||
                   stmt_timetype_ == MYSQL_TYPE_DATETIME ||
                   stmt_timetype_ == MYSQL_TYPE_TIMESTAMP
                  );
    };
    //
    ~STMT_BindTime_Adaptor()
    {
    };
};

/*!
* @brief      仅仅是为了适配 ZCE_Mysql_STMT_Bind << 的操作符号
*             绑定一个空参数
* @note
*/
class ZCELIB_EXPORT STMT_BindNULL_Adaptor
{
    friend class ZCE_Mysql_STMT_Bind;

public:
    STMT_BindNULL_Adaptor()
    {
    };
    ~STMT_BindNULL_Adaptor()
    {
    };
};



/*!
* @brief
*
* @note
*/
class ZCELIB_EXPORT ZCE_Mysql_STMT_Bind
{


protected:

    //定义出来不实现,让你无法用,有很多地方有我分配的指针,不能给你浅度复制
    ZCE_Mysql_STMT_Bind &operator=(const ZCE_Mysql_STMT_Bind &others);

public:

    /*!
    * @brief      构造函数
    * @param      numbind  要绑定变量,结果的个数
    */
    ZCE_Mysql_STMT_Bind(size_t numbind);
    //
    ~ZCE_Mysql_STMT_Bind();

    /*!
    * @brief      绑定一个参数
    * @return     int
    * @param      paramno   参数的编号
    * @param      paramtype 参数类型
    * @param      bisnull   是否为NULL
    * @param      paramdata 参数的数据的指针
    * @param      szparam   参数的长度
    */
    int bind_one_param(size_t paramno,
                       ::enum_field_types paramtype,
                       bool bisnull,
                       void *paramdata ,
                       unsigned long szparam = 0);

    //得到STMT HANDLE
    inline MYSQL_BIND *get_stmt_bind_handle();

    inline MYSQL_BIND *operator[](unsigned int paramno) const;

    //理论应该为Const定义一组，用于绑定变量

    //将变量绑定
    ZCE_Mysql_STMT_Bind &operator << (char &);
    ZCE_Mysql_STMT_Bind &operator << (short &);
    ZCE_Mysql_STMT_Bind &operator << (int &);
    ZCE_Mysql_STMT_Bind &operator << (long &);
    ZCE_Mysql_STMT_Bind &operator << (long long &);

    ZCE_Mysql_STMT_Bind &operator << (unsigned char &);
    ZCE_Mysql_STMT_Bind &operator << (unsigned short &);
    ZCE_Mysql_STMT_Bind &operator << (unsigned int &);
    ZCE_Mysql_STMT_Bind &operator << (unsigned long &);
    ZCE_Mysql_STMT_Bind &operator << (unsigned long long &);

    ZCE_Mysql_STMT_Bind &operator << (float &);
    ZCE_Mysql_STMT_Bind &operator << (double &);

    //ZCE_Mysql_STMT_Bind& operator << (char *);

    //为了使用几个类型的适配器
    ZCE_Mysql_STMT_Bind &operator << (STMT_BindData_Adaptor &);
    ZCE_Mysql_STMT_Bind &operator << (STMT_BindTime_Adaptor &);
    ZCE_Mysql_STMT_Bind &operator << (STMT_BindNULL_Adaptor &);

    //检查第几个绑定的参数是否为NULL
    bool get_bind_is_null(const size_t paramno) const;
    //设置第几个绑定的参数是否为NULL
    void set_bind_is_null(const size_t paramno, const bool isnull);

    //得到第几个绑定的参数Buf的长度
    unsigned long get_bind_buf_length(const size_t paramno) const;
    //设置第几个绑定的参数Buf的长度
    void set_bind_buf_length(const size_t paramno, unsigned long buflen);

    //得到BIND数据的实际长度
    unsigned long get_bind_data_length(const size_t paramno) const;
    //
    void set_bind_data_length(const size_t paramno, unsigned long buflen);


protected:

    //绑定的变量个数
    size_t           num_bind_;

    //当前使用的绑定参数序号,用于<<
    size_t            current_bind_;

    //BIND MySQL的封装方式让我不能用vector,
    MYSQL_BIND       *stmt_bind_;
    //是否为NULL
    my_bool          *is_bind_null_;

    //BIND的数据实际的长度
    unsigned long    *bind_length_;
};

inline MYSQL_BIND *ZCE_Mysql_STMT_Bind::get_stmt_bind_handle()
{
    return stmt_bind_;
}

#endif //MYSQL_VERSION_ID >= 40100

#endif //#if defined ZCE_USE_MYSQL

#endif //ZCE_LIB_MYSQL_STMT_RESULT_H_

