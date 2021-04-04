#ifndef ZCE_LIB_SQLITE_STMT_HANDLER_H_
#define ZCE_LIB_SQLITE_STMT_HANDLER_H_

//Ŀǰ�汾����ֻ����һ��
#if SQLITE_VERSION_NUMBER >= 3005000

/*!
@brief      SQlite STMT�ľ��
            ����SQL�Ĵ���ȣ�STMT�Ǹ��ö���������������鷳һ�㡣
*/
class ZCE_SQLite_STMTHdl
{
public:

    //!������������ݽṹ�����ڸ�����BLOB���͵Ĳ�������
    struct BIN_Param
    {
        /*!
        * @brief
        * @param      binary_data ����������BUFFER
        * @param      binary_len  ���ݳ��ȣ�
        */
        BIN_Param(void *binary_data, int binary_len)
            : binary_data_(binary_data)
            , binary_len_(binary_len)
        {
        }
        ~BIN_Param()
        {
        }

        //!2�������ݵ�ָ��
        void   *binary_data_;
        //!���������ݵĳ���,
        int    binary_len_;
    };

    //!������������ݽṹ�����ڸ�����BLOB���͵����ݽ��
    struct BIN_Result
    {
        /*!
        * @brief
        * @param      binary_data ����������BUFFER
        * @param      binary_len  ���ݳ��ȣ���ʼ��Ϊ���ݳ��ȣ�ʹ�ú��¼���ݽ������
        */
        BIN_Result(void *binary_data, int *binary_len)
            : binary_data_(binary_data)
            , binary_len_(binary_len)
        {
        }
        ~BIN_Result()
        {
        }

        //!2�������ݵ�ָ��
        void   *binary_data_;
        //!���������ݵĳ���,ע��󶨽��ʱ�������ֵ��λ����󶨵�ʱ�򣬻Ḩ�����س���
        int    *binary_len_;
    };


public:
    /*!
    * @brief      ���캯��
    * @param      sqlite3_handler  SQlite3��DB��װ�����
    */
    ZCE_SQLite_STMTHdl(ZCE_SQLite_DB_Handler *sqlite3_handler);
    /*!
    * @brief      ��������
    */
    ~ZCE_SQLite_STMTHdl();

public:

    /*!
    * @brief      Ԥ����SQL���
    * @return     int
    * @param      sql_string
    */
    int prepare(const char *sql_string);

    /*!
    * @brief      ���³�ʼ��STMT��Handler
    * @return     int
    */
    int reset();

    /*!
    * @brief      ����SQLITE3��STMT HANDLER,�ָ���ʼ��ֵ�ȡ�
    * @return     int
    */
    int finalize();

    /*!
    * @brief      ִ��SQL����һ����ִ��SQL�������ƶ��α�
    * @return     int         0�ɹ�������ʧ��
    * @param[out] hash_reuslt ����ֵ,�Ƿ��н��
    * note        Ҫִ�ж�Σ���һ�εõ�������ϣ������ƶ��αꡣ
    */
    int execute_stmt_sql(bool &hash_reuslt);

    //!��ʼһ������
    int begin_transaction();
    //!�ύһ������
    int commit_transction();
    //!��ͬ��ѡ��رգ����鲻Ҫʹ��
    int turn_off_synch();

    //!
    sqlite3_stmt *get_sqlite3_stmt_handler()
    {
        return sqlite3_stmt_handler_;
    }

    //!�������Str
    inline const char *error_message()
    {
        return sqlite_handler_->error_message();
    }
    //!DB���صĴ���ID
    inline  unsigned int error_code()
    {
        return sqlite_handler_->error_code();
    }

    /*!
    * @brief      �õ���ǰ�����еĳ���
    * @return     int ����
    * @param[in]  result_col
    * @note
    */
    inline int column_bytes(int result_col)
    {
        return ::sqlite3_column_bytes(sqlite3_stmt_handler_, result_col);
    }

    /*!
    * @brief      ȡ���е�����
    * @return     int
    * @param      num_col
    */
    int column_count()
    {
        return ::sqlite3_column_count(sqlite3_stmt_handler_);
    }

    //!��ǰcolumn�����ݳ���
    inline int cur_column_bytes()
    {
        return ::sqlite3_column_bytes(sqlite3_stmt_handler_, current_col_);
    }

    /*!
    * @brief      ����SQL����?���������а󶨣�
    * @tparam     bind_type �󶨵Ĳ�������
    * @return     int       ���� 0 ��ʾ�ɹ���
    * @param      bind_index �󶨵��±꣬��1��ʼ
    * @param      val       SQL���󶨵Ĳ���
    * @note       SQLite STMT��MYSQL��API������һЩ�������𣬿������ĺ���,����û������,
    *             SQLite��Bind�������õ�ʱ���ȡ����ֵ�����ٴӺ����Ĳ����Ͽ�����������
    *             ����Ҫbind blob���ݣ�ʹ��BINARY
    */
    template <class bind_type>
    int bind(int bind_col, bind_type val);

    /*!
    * @brief      ȡ���еĽ��
    * @tparam     value_type ���������
    * @param      result_col �кţ���0��ʼ
    * @param      val ȡ���Ľ��
    * @note       �����Ƶ�����Ҫ�ر���һ��,�ַ������ر�+1��,�����������ݲ�Ҫ��������
    */
    template <class value_type>
    void column(int result_col, value_type val);


    //!�������,�к��Զ�++
    template <class value_type>
    ZCE_SQLite_STMTHdl &operator >> (value_type &val)
    {
        column<value_type &>(current_col_, val);
        ++current_col_;
        return *this;
    }



    //!bind�󶨲���,�к��Զ�++
    template <class bind_type>
    ZCE_SQLite_STMTHdl &operator << (bind_type val)
    {
        bind<bind_type>(current_bind_, val);
        ++current_bind_;
        return *this;
    }

    //���������͵�<<����ʹ�õ������ã���������һ�£�
    ZCE_SQLite_STMTHdl &operator << (const ZCE_SQLite_STMTHdl::BIN_Param &val);
    ZCE_SQLite_STMTHdl &operator << (const std::string &val);


protected:

    //!SQLite��DB���
    ZCE_SQLite_DB_Handler *sqlite_handler_;

    //!SQLiteԭ����STMT�ľ��
    sqlite3_stmt *sqlite3_stmt_handler_;

    //!��ǰȡ�������,����>>����,��0��ʼ
    int current_col_;

    //!��ǰbind��SQL���������±꣬����>>����,,��1��ʼ
    int current_bind_;
};

#endif //SQLITE_VERSION_NUMBER >= 3005000

#endif //ZCE_LIB_SQLITE3_STMT_HANDLER_H_

