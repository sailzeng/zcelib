/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_mysql_stmtbind.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2005��10��17��
* @brief
*
* @details
*
* @note
*
*/

#ifndef ZCE_LIB_MYSQL_STMT_RESULT_H_
#define ZCE_LIB_MYSQL_STMT_RESULT_H_

//�����Ҫ��MYSQL�Ŀ�
#if defined ZCE_USE_MYSQL

//��Щ��������4.1.2��İ汾����

#include "zce_log_logging.h"


/*!
* @brief MYSQL_BIND �İ�װ��װ�ۣ�
*
* @note  ZCE_Mysql_STMT_Bind����bind�ı������ݣ��Ƿ�ΪNULL�����س��ȣ�����ָ�룬
*        �ⲿ�ı����������ڣ������ش���
*/
class ZCE_Mysql_STMT_Bind
{
public:

    /*!
    * @brief      ������Ϊ������ ZCE_Mysql_STMT_Bind << �Ĳ�������
    *             ��2���Ʋ������ݣ����� mysql_stmt_bind_param
    */
    class BinData_Param
    {
        friend class ZCE_Mysql_STMT_Bind;

    public:

        /*!
        * @brief
        * @param[in] data_type �������ͣ�ֻ����MYSQL_TYPE_BLOB or MYSQL_TYPE_STRING
        * @param[in] pdata ����ָ�룬������д��Ĵ�ŵĵط����ݣ�
        * @param[in] data_len ���ݳ��ȵ�ָ�룬���������ʾ���ݳ��ȣ�ʹ�ú󱣴��Ǳ�ʾд������ݳ���
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
    * @brief      ������Ϊ������ ZCE_Mysql_STMT_Bind << �Ĳ�������
    *             ��2���ƽ�����ݣ����� mysql_stmt_bind_result
    */
    class BinData_Result
    {
        friend class ZCE_Mysql_STMT_Bind;

    public:

        /*!
        * @brief
        * @param[in] data_type �������ͣ�ֻ����MYSQL_TYPE_BLOB or MYSQL_TYPE_STRING
        * @param[in] pdata ����ָ�룬������д��Ĵ�ŵĵط����ݣ�
        * @param[in] data_len ���ݳ��ȵ�ָ�룬���������ʾ���ݳ��ȣ�ʹ�ú󱣴��Ǳ�ʾд������ݳ���
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
    @brief      ������Ϊ������ ZCE_Mysql_STMT_Bind << �Ĳ�������

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
    * @brief      ������Ϊ������ ZCE_Mysql_STMT_Bind << �Ĳ�������
    *             ��һ���ղ���
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

    //���������ʵ��,�����޷���,�кܶ�ط����ҷ����ָ��,���ܸ���ǳ�ȸ���
    ZCE_Mysql_STMT_Bind &operator=(const ZCE_Mysql_STMT_Bind &others);

public:

    /*!
    * @brief      ���캯��
    * @param      numbind  Ҫ�󶨱���,����ĸ���
    */
    ZCE_Mysql_STMT_Bind(size_t numbind);
    //
    ~ZCE_Mysql_STMT_Bind();

    /*!
    * @brief      ��һ������
    * @return     int
    * @param      paramno   �����ı��
    * @param      paramtype ��������
    * @param      bisnull   �Ƿ�ΪNULL,
    * @param      paramdata ���������ݵ�ָ��
    * @param      szparam   �����ĳ���
    */
    int bind_one_param(size_t paramno,
                       ::enum_field_types paramtype,
                       my_bool *is_null,
                       void *paramdata,
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

    ///�õ�STMT HANDLE
    inline MYSQL_BIND *get_stmt_bind_handle()
    {
        return stmt_bind_;
    }

    inline MYSQL_BIND *operator[](size_t paramno) const
    {
        return &stmt_bind_[paramno];
    }

    ///��������
    void reset();

    ///��������
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

    ///Ϊ��ʹ�ü������͵�������
    ///�󶨶��������ݣ���������
    void bind(size_t bind_col, ZCE_Mysql_STMT_Bind::BinData_Param &val);
    ///�󶨶����ƽ����������
    void bind(size_t bind_col, ZCE_Mysql_STMT_Bind::BinData_Result &val);
    ///��ʱ���������
    void bind(size_t bind_col, ZCE_Mysql_STMT_Bind::TimeData &val);
    ///�󶨿յ�������
    void bind(size_t bind_col, ZCE_Mysql_STMT_Bind::NULL_Param &val);


    template <typename bind_type>
    ZCE_Mysql_STMT_Bind &operator << (bind_type &val)
    {
        bind(current_bind_, val);
        ++current_bind_;
        return *this;
    }

protected:

    ///�󶨵ı�������
    size_t           num_bind_;

    ///��ǰʹ�õİ󶨲������,����<<
    size_t            current_bind_;

    ///BIND MySQL�ķ�װ��ʽ���Ҳ�����vector,
    MYSQL_BIND       *stmt_bind_;

};



#endif //#if defined ZCE_USE_MYSQL

#endif //ZCE_LIB_MYSQL_STMT_RESULT_H_

