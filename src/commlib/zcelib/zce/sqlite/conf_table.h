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
* @note       AI_IIJIMA ������
*             ����ȽϷ����㷳Ҳ���㷳�����μ��ͷ�����һ����ͣ�
*             ���еĲ����ƹ�̫�谵���ұ�Ѱ��������ɫ��Сҩ��
*             ����е�����ൽ�˼��㣬����Ҳ���ĵ��˼��㣬
*             �������������������߰���Ķ���Խ��Խ��.
*             ˵�˺ܶ෹�����ķϻ��������ڴ����������ɣ�
*             һ������ì���˵ģ������а������������������룬ȴһ���ް���
*             ������ʢ������������������֪��
*             ���������ȴ����sina������ҳ�������й�����Ķ��ձ�һ��Ů�����
*             ���Ƽ�����С˵������ͼʽ�԰�����
*             С˵��һ�����淴ӳ�˺ڰ����ʱ���������һ��СŮ�����ĵĴݲУ������Ƽ���Ҷ�����
*             ���Ҫ˵���ǣ��������ף�ϣ�������ϧ��
*             �����Ƕΰ�һ����Ʒ����3�����ʷ��
*/


/*
�����������壬
CREATE TABLE IF NOT EXISTS config_table_8(index_1 INTEGER,index_2 INTEGER,conf_data BLOB ,last_mod_time INTEGER);
CREATE UNIQUE INDEX IF NOT EXISTS cfg_table_idx_8 ON config_table_8 (index_1,index_2)

*/

#ifndef ZCE_LIB_SQLITE_CONF_TABLE_H_
#define ZCE_LIB_SQLITE_CONF_TABLE_H_

//Ŀǰ�汾����ֻ����һ��
#if SQLITE_VERSION_NUMBER >= 3005000



/*!
* @brief      ���ڽ�һЩ�ṹת��Ϊ���������ݣ�����������ֶ���
*
* @note
*/
struct AI_IIJIMA_BINARY_DATA
{


public:

    //!�������������
    AI_IIJIMA_BINARY_DATA();
    ~AI_IIJIMA_BINARY_DATA();

    //!
    void clear();

    //!�ȽϺ���
    bool operator < (const AI_IIJIMA_BINARY_DATA &right) const;

#if defined ZCE_USE_PROTOBUF && ZCE_USE_PROTOBUF == 1

    //!��һ���ṹ���б���
    int protobuf_encode(unsigned int index_1,
                        unsigned int index_2,
                        const google::protobuf::MessageLite *msg);

    //!��һ���ṹ���н���
    int protobuf_decode(unsigned int *index_1,
                        unsigned int *index_2,
                        google::protobuf::MessageLite *msg);

#endif

public:

    //!��̬����������������
    static const int MAX_LEN_OF_AI_IIJIMA_DATA = 32 * 1024 - 1;

public:

    //!����1
    unsigned int index_1_ = 0;
    //!����2 Ĭ��Ϊ0
    unsigned int index_2_ = 0;

    //!����������
    int ai_data_length_ = 0;
    //!��̬����ȡ
    char ai_iijima_data_[MAX_LEN_OF_AI_IIJIMA_DATA + 1];

    //!����޸�ʱ��
    unsigned int last_mod_time_ = 0;
};

typedef std::vector <AI_IIJIMA_BINARY_DATA>   ARRARY_OF_AI_IIJIMA_BINARY;



/******************************************************************************************
struct BaobaoGeneralDBConf һ����ͨ�õĴ�DB�м�õ�ͨ��������Ϣ�ķ���
******************************************************************************************/
class ZCE_SQLite_DB_Handler;

//һ����ͨ�õĴ�DB�м�õ�ͨ��������Ϣ�Ľṹ
class ZCE_General_Config_Table
{

public:

    ZCE_General_Config_Table();
    ~ZCE_General_Config_Table();

protected:

    //!����TABLE SQL���
    void sql_create_table(unsigned  int table_id);
    //!����INDEX SQL���
    void sql_create_index(unsigned  int table_id);


    //!��д��STMT SQL
    void sql_replace_bind(unsigned  int table_id);

    //!��д��SQL,�ı���ʽ����x
    void sql_replace_one(unsigned  int table_id,
                         unsigned int index_1,
                         unsigned int index_2,
                         size_t blob_len,
                         const char *blob_data,
                         unsigned int last_mod_time);


    //!�õ�ѡ��һ��ȷ�����ݵ�SQL
    void sql_select_one(unsigned int table_id,
                        unsigned int index_1,
                        unsigned int index_2);

    //�õ�ɾ�����ݵ�SQL
    void sql_delete_one(unsigned int table_id,
                        unsigned int index_1,
                        unsigned int index_2);


    //!�����ѯ������
    void sql_counter(unsigned int table_id,
                     unsigned int startno,
                     unsigned int numquery);

    //!��ѯ���ݶ��У��������ݻ���ȫ������
    void sql_select_array(unsigned int table_id,
                          unsigned int startno,
                          unsigned int numquery);

    //!

    /*!
    * @brief      base16���㷨��
    * @return     int
    * @param      in
    * @param      in_len
    * @param      out
    * @param[in/out] out_len ����ʱ��ʶoutbuf�ĳ��ȣ����ʱ��ʶ������ɵ��ַ�������
    * @note       �����Ϊ�˱�������̫���ļ�������base16ʵ�ֽ�Ϊ�򵥣�����û����zce��encode����
    */
    int base16_encode(const char *in,
                      size_t in_len,
                      char *out,
                      size_t *out_len);
public:

    //!��һ��ͨ�õ����ݿ�
    int open_dbfile(const char *db_file,
                    bool read_only,
                    bool create_db);

    //!�������ݱ�
    int create_table(unsigned int table_id);

    //!�رմ򿪵����ݿ�
    void close_dbfile();

    //!UPDATE ���� INSERT һ����¼
    int replace_one(unsigned int table_id,
                    const AI_IIJIMA_BINARY_DATA *conf_data);

    //UPDATE ���� INSERT һ���¼
    int replace_array(unsigned int table_id,
                      const ARRARY_OF_AI_IIJIMA_BINARY *ary_ai_iijma);

    //!��ѯ��һ����¼
    int select_one(unsigned int table_id,
                   AI_IIJIMA_BINARY_DATA *conf_data);

    //!ɾ��һ����¼
    int delete_one(unsigned int table_id,
                   unsigned int index_1,
                   unsigned int index_2);

    //!��ѯ��¼����
    int counter(unsigned int table_id,
                unsigned int startno,
                unsigned int numquery,
                unsigned int *rec_count);

    //!��ѯ���ݶ��У��������ݣ����Ʋ�ѯ����������ȫ������
    int select_array(unsigned int table_id,
                     unsigned int startno,
                     unsigned int numquery,
                     ARRARY_OF_AI_IIJIMA_BINARY *ary_ai_iijma);

    /*!
    * @brief      �Ա��������ݱ���ҳ����죬Ȼ���ҳ������SQL�����������ļ����µȹ���
    * @return     int      �Ƿ�ɹ�������˱Ƚ�
    * @param      old_db   �Ƚ����ݿ�old�ļ�����
    * @param      new_db   �Ƚ����ݿ�new�ļ�����
    * @param      table_id �Ƚϵ�table id
    * @param[out] update_sql ���صĸ���SQL���
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

    //! SQL���
    char *sql_string_ = NULL;
    //!
    ZCE_SQLite_DB_Handler *sqlite_handler_;


};


#endif //SQLITE_VERSION_NUMBER >= 3005000

#endif //ZCE_LIB_SQLITE_CONF_TABLE_H_
