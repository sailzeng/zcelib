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

#pragma once

//如果你要用MYSQL的库
#if defined ZCE_USE_MYSQL

//这些函数都是4.1.2后的版本功能

#include "zce_trace_debugging.h"


/*!
* @brief MYSQL_BIND 的包装封装累，
*
* @note  ZCE_Mysql_STMT_Bind里面bind的变量数据，是否为NULL，返回长度，都是指针，
*        外部的保存生命周期，请慎重处理。
*/
class ZCE_Mysql_STMT_Bind
{
public:

    /*!
    * @brief      仅仅是为了适配 ZCE_Mysql_STMT_Bind << 的操作符号
    *             绑定2进制参数数据，用于 mysql_stmt_bind_param
    */
    class BinData_Param
    {
        friend class ZCE_Mysql_STMT_Bind;

    public:

        /*!
        * @brief
        * @param[in] data_type 数据类型，只能是MYSQL_TYPE_BLOB or MYSQL_TYPE_STRING
        * @param[in] pdata 数据指针，就是是写入的存放的地方数据，
        * @param[in] data_len 数据长度的指针，传入参数表示数据长度，使用后保存是表示写入的数据长度
        */
        BinData_Param(enum_field_types data_type, void *pdata, unsigned long data_len) :
            stmt_data_type_(data_type),
            stmt_pdata_(pdata),
            stmt_data_length_(data_len)
        {
            ZCE_ASSERT(MYSQL_TYPE_BLOB == stmt_data_type_
                       || MYSQL_TYPE_STRING == stmt_data_type_);
        };
        //
        ~BinData_Param()
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
    * @brief      仅仅是为了适配 ZCE_Mysql_STMT_Bind << 的操作符号
    *             绑定2进制结果数据，用于 mysql_stmt_bind_result
    */
    class BinData_Result
    {
        friend class ZCE_Mysql_STMT_Bind;

    public:

        /*!
        * @brief
        * @param[in] data_type 数据类型，只能是MYSQL_TYPE_BLOB or MYSQL_TYPE_STRING
        * @param[in] pdata 数据指针，就是是写入的存放的地方数据，
        * @param[in] data_len 数据长度的指针，传入参数表示数据长度，使用后保存是表示写入的数据长度
        */
        BinData_Result(enum_field_types data_type, void *pdata, unsigned long *data_len) :
            stmt_data_type_(data_type),
            stmt_pdata_(pdata),
            stmt_data_length_(data_len)
        {
            ZCE_ASSERT(MYSQL_TYPE_BLOB == stmt_data_type_
                       || MYSQL_TYPE_STRING == stmt_data_type_);
        };
        //
        ~BinData_Result()
        {
        };
    protected:
        //
        enum_field_types  stmt_data_type_;
        //
        void             *stmt_pdata_;
        //
        unsigned long    *stmt_data_length_;
    };

    /*!
    @brief      仅仅是为了适配 ZCE_Mysql_STMT_Bind << 的操作符号

    */
    class TimeData
    {
        friend class ZCE_Mysql_STMT_Bind;
    public:
        //
        TimeData(enum_field_types timetype, MYSQL_TIME *pstmttime) :
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
        ~TimeData()
        {
        };

    protected:
        //
        enum_field_types  stmt_timetype_;
        //
        MYSQL_TIME       *stmt_ptime_;
    };

    /*!
    * @brief      仅仅是为了适配 ZCE_Mysql_STMT_Bind << 的操作符号
    *             绑定一个空参数
    * @note
    */
    class NULL_Param
    {
        friend class ZCE_Mysql_STMT_Bind;

    public:
        NULL_Param(my_bool *is_null):
            is_null_(is_null)
        {
        };
        ~NULL_Param()
        {
        };

        my_bool *is_null_;
    };

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
    * @param      bisnull   是否为NULL,
    * @param      paramdata 参数的数据的指针
    * @param      szparam   参数的长度
    */
    int bind_one_param(size_t paramno,
                       ::enum_field_types paramtype,
                       my_bool *is_null,
                       void *paramdata ,
                       unsigned long szparam = 0);


    /*!
    * @brief
    * @return     int
    * @param[in]     paramno
    * @param[in]     paramtype
    * @param[in]     paramdata
    * @param[in,out] szparam
    */
    int bind_one_result(size_t paramno,
                        ::enum_field_types paramtype,
                        void *paramdata,
                        unsigned long *szparam);

    ///得到STMT HANDLE
    inline MYSQL_BIND *get_stmt_bind_handle()
    {
        return stmt_bind_;
    }

    inline MYSQL_BIND *operator[](size_t paramno) const
    {
        return &stmt_bind_[paramno];
    }

    ///重新设置
    void reset();

    ///将变量绑定
    void bind(size_t bind_col, char val);
    void bind(size_t bind_col, char &val);
    void bind(size_t bind_col, short &val);
    void bind(size_t bind_col, int &val);
    void bind(size_t bind_col, long &val);
    void bind(size_t bind_col, long long &val);

    
    void bind(size_t bind_col, unsigned char &val);
    void bind(size_t bind_col, unsigned short &val);
    void bind(size_t bind_col, unsigned int &val);
    void bind(size_t bind_col, unsigned long &val);
    void bind(size_t bind_col, unsigned long long &val);

    void bind(size_t bind_col, float &val);
    void bind(size_t bind_col, double &val);

    ///为了使用几个类型的适配器
    ///绑定二进制数据，的适配器
    void bind(size_t bind_col, ZCE_Mysql_STMT_Bind::BinData_Param &val);
    ///绑定二进制结果的适配器
    void bind(size_t bind_col, ZCE_Mysql_STMT_Bind::BinData_Result &val);
    ///绑定时间的适配器
    void bind(size_t bind_col, ZCE_Mysql_STMT_Bind::TimeData &val);
    ///绑定空的适配器
    void bind(size_t bind_col, ZCE_Mysql_STMT_Bind::NULL_Param &val);


    template <typename bind_type>
    ZCE_Mysql_STMT_Bind &operator << (bind_type &val)
    {
        bind(current_bind_, val);
        ++current_bind_;
        return *this;
    }

protected:

    ///绑定的变量个数
    size_t           num_bind_;

    ///当前使用的绑定参数序号,用于<<
    size_t            current_bind_;

    ///BIND MySQL的封装方式让我不能用vector,
    MYSQL_BIND       *stmt_bind_;

};



#endif //#if defined ZCE_USE_MYSQL



