/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_sqlite3_process.h
* @author     Viviliu��Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2008��3��12��
* @brief      ����SQLite��һЩ��������
*             ������ڲ���װ��ʵ����ȫ��UTF8�ĸ�ʽ������������UTF16
* @details    ԭ��Ϊɶ����һ����׺3����ΪSQlite��ʱ����2�İ汾
*             ��3�İ汾�����ڼ䣬����SQlite���̵Ĳ�������2���ú޾�Ҫ�ݣ�ACE�����
*             ��������ģ�����
*             SQLite�Ĵ����࣬08���𣿸о�Ӧ�ø���һ��ѣ�Ӧ����07�ѣ�
*             ��ʱ��Լ����ҪѰ��һ�����ݿ���������ǵ�ʱ�����ݱ��������ACCESS�ģ�
*             ����ACCESSȴ���ܰ���΢���ħ�䣬
*             ����living����������RTX����SQLite���������Ϣ��������������������
*             ���ֵ�ȷ�ܺã���������������ݿ�һ��������ʰ�������ֻ��ϳ�Ϊ���ƶ�
*             �ն����ݿ������Ҳ����Ե������һ����Դ�ö����ĳɳ���
*
*             6���Ժ���������SQLite����վ��������վ�����˲��٣�������һ����ֱ��
*             Դ�룬�������ô�档
*
*             ����Sqlite���ٶȣ���ο�http://www.sqlite.org/speed.html��
*
*             �Ҹ����ǣ������Ӧ�ü�ֵģ�������Ӧ�÷����ģ���ϧ����ӵ�еģ�������Ѿ�������
*
*/
#ifndef ZCE_LIB_SQLITE_DBHANDLE_H_
#define ZCE_LIB_SQLITE_DBHANDLE_H_

#include "zce_os_adapt_string.h"

//Ŀǰ�汾����ֻ����һ��
#if SQLITE_VERSION_NUMBER >= 3005000


//==============================================================================================
class ZCE_SQLite_Result;

/*!
@brief      ���Ӵ���һ��SQLite3���ݿ�ģ���һ��SQLite3���ݿ�͵õ�Handler
            ��Handler��ɺ���������ݿ������
*/
class ZCE_SQLite_DB_Handler
{

public:

    //!���캯����
    ZCE_SQLite_DB_Handler();
    //!��������
    ~ZCE_SQLite_DB_Handler();

    /*!
    @brief      �����ݿ⣬ע���ļ����Ƶ�·��Ҫ��UTF8���룬������ò�Ҫ������?
    @return     int      �����ݿ��Ƿ�ɹ� 0 �ɹ�������ʧ��
    @param      db_file  ���ݿ��ļ������ƣ�
                         �����������Ĺ�ϵ�����������WINDOWS�㴫�����ANSI��
                         �ַ�������LINUX���㴫�����UTF8
    @param      read_only ֻ��
    @param      create_db �Ƿ���Ҫ�������ݿ⣬
    */
    int open_database(const char *db_file,
                      bool read_only,
                      bool create_db);


#if defined ZCE_OS_WINDOWS
    /*!
    @brief      ��MBCS(Windows��˵�� multibyte character set )��·�����ƴ�һ��Ŀ¼,
    *           ��Ϊ���ڲ��õ�ȫ����UTF8�ĺ�����������������⣬��������ת�����룬
    */
    int open_mbcs_path_db(const char *utf16_db_path,
                          bool read_only,
                          bool create_db = false);


#endif


    //!�ر����ݿ�
    void close_database();

    //!ȡ�ô������Str
    const char *error_message();
    //!ȡ��DB���صĴ���ID
    int error_code();

    //!ȡ��SQLite�ľ��
    inline sqlite3 *get_sqlite_handler()
    {
        return sqlite3_handler_;
    };

    //!��ʼһ������
    int begin_transaction();
    //!�ύһ�����񣬻���˵����һ������
    int commit_transction();

    //!��ͬ��ѡ��رգ����鲻Ҫʹ��
    int turn_off_synch();

    //!ִ��DDL�Ȳ���Ҫ�����SQL
    int execute(const char *sql_string);

    /*!
    * @brief      ִ��SQL���µķ�װ,�������ƵĲ��У�
    * @return     int ����0��ʾ�ɹ���
    * @param      sql_string SQL���
    * @param      ִ�еĽ��������ֵ
    * @note       �ڲ������sqlite3_get_table,sqlite3_free_table��
    *             ���������SQLite�в��Ǳ��Ƽ��ĺ���������ʹ��ʱ����һ�£���Ȼ��
    *             ִ�в�ѯ��ȷʵ��sqlite3_exec������
    *             ���⣬�������Ӧ�ò��ܴ�����������ݣ���Ϊ���޷���֪�������
    */
    int get_table(const char *sql_string,
                  ZCE_SQLite_Result *result);

protected:

    //!sqlite3�Ĵ���Handler
    sqlite3         *sqlite3_handler_;

};



//==============================================================================================

/*!
* @brief      get_table �������صĽ������
*             ��ʵ����sqlite3_get_table �Ľ�������ķ�װ
* @note       ��ע�⣬sqlite3_get_table ֻ��Ӧ�������ݵĺ���
*/
class ZCE_SQLite_Result
{
    friend class ZCE_SQLite_DB_Handler;

public:

    ZCE_SQLite_Result();
    ~ZCE_SQLite_Result();

    //!��������ͷ�ΪNULL
    inline bool is_null()
    {
        return (result_ == NULL);
    }


    //!�ͷŽ������
    void free_result();

    /*!
    * @brief      ����һ���ֶε�����
    * @return     const char* �ֶε�����
    * @param      column �ֶε��к�,��1��ʼ
    */
    const char *field_name(int column)
    {
        return result_[column - 1];
    }

    /*!
    * @brief      ����һ���ֶε����ݣ�
    * @return     const char* �ֶε�����
    * @param      row    �ֶε��к�,��1��ʼ
    * @param      column �ֶε��к�,��1��ʼ
    */
    const char *field_cstr(int row, int column)
    {
        return result_[row * column_ + column - 1];
    }

    /*!
    * @brief      �������ͣ�����һ���ֶε����ݣ�
    * @tparam     value_type
    * @return     value_type
    * @param      row    �ֶε��к�,��1��ʼ
    * @param      column �ֶε��к�,��1��ʼ
    * @note
    */
    template <typename value_type>
    value_type field_data(int row, int column)
    {
        return zce::str_to_value<value_type>( result_[row * column_ + column - 1] );
    }

    //!�е�����
    inline int row_number()
    {
        return row_;
    }

    //!�е�����
    inline int column_number()
    {
        return column_;
    }


protected:

    //! Results of the query
    char **result_ = NULL;
    //! Number of result rows written here ��row_Ҳ�Ǵ�1��ʼ
    int row_ = 0;

    //! Number of result columns written here ,column_ ��1��ʼ
    int column_ = 0;

    //! Error msg written here
    char *err_msg_ = NULL;
};

#endif //SQLITE_VERSION_NUMBER >= 3005000

#endif //ZCE_LIB_SQLITE_DBHANDLE_H_

