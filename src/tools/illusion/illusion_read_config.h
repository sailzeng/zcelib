


#pragma once




class Illusion_Read_Config
{
public:

    enum CVT_CODING
    {
        CVT_UTF8,
        CVT_UTF16,
        CVT_MBCS,
    };


    struct TABLE_CONFIG
    {
        ///表格名称
        CString excel_table_name_;

        ///表格数据从第几行读取
        long read_data_start_ = 3;

        ///表格对应的protobuf的message名称
        std::string   pb_msg_name_;

        ///表格的第几行描述字段对应的protobuf
        long protobuf_item_line_ = 2;

        ///表格存放的数据库（SQLite）文件名称
        std::string sqlite3_db_name_;

        ///表格对应的table id
        unsigned int table_id_ = 0;
        ///表格索引的字段1的列号
        unsigned int index1_column_ = 0;
        ///表格索引的字段2的列号
        unsigned int index2_column_ = 0;


        ///Protobuf item定义的数据
        std::vector<std::string>  proto_field_ary_;
        
        ///假设结构如下，record是一个repeated 的message，
        ///phonebook.master
        ///phonebook.record.name
        ///phonebook.record.tele_number
        ///phonebook.record.email
        ///phonebook.record.name
        ///phonebook.record.tele_number
        ///phonebook.record.email
        ///那么phonebook.record.name出现的位置会被标识为item_msg_firstshow_ 为1
        std::vector<int> item_msg_firstshow_;

        ///在上面的例子  会被记录为phonebook.record.name
        std::string firstshow_field_;
        ///在上面的例子 会被记录为phonebook.record
        std::string firstshow_msg_;

    };

    ///枚举值的对应关系表
    typedef std::map <CString, CString >  MAP_CSTRING_TO_CSTRING;

    ///
    typedef std::map <CString, TABLE_CONFIG> MAP_TABLE_TO_CONFIG;

    ///
    struct EXCEL_FILE_DATA
    {
        MAP_CSTRING_TO_CSTRING  xls_enum_;

        MAP_TABLE_TO_CONFIG  xls_table_cfg_;
    };

    typedef std::map <CString, EXCEL_FILE_DATA> MAP_FNAME_TO_CFGDATA;

protected: // 仅从序列化创建
    Illusion_Read_Config();
protected:
    virtual ~Illusion_Read_Config();


public:

    ///
    static Illusion_Read_Config *instance();

    ///
    static void clean_instance();

public:

    ///初始化
    BOOL initialize();
    //
    void finalize();

    /*!
    * @brief      
    * @return     int
    * @param      open_file 打开的EXCEL文件名称，名称MFC
    */
    int read_excel_byucname(const CString &open_file);

    /*!
    * @brief      
    * @return     int
    * @param      proto_fname EXCEL文件名称。
    */
    int read_excel(const std::string &excel_fname);

    /*!
    * @brief      
    * @param      path_name
    */
    void map_proto_path(const std::string &path_name);

    /*!
    * @brief      
    * @return     int
    * @param      mbcs_name
    * @note       
    */
    int read_proto(const std::string &mbcs_name);


    //清理所有的读取数据
    void clear();

protected:

    //读枚举值
    int read_table_enum(EXCEL_FILE_DATA &file_cfg_data);

    ///读取表格配置
    int read_table_config(EXCEL_FILE_DATA &file_cfg_data);

    ///读取表格数据
    int read_table_cfgdata(TABLE_CONFIG &table_cfg);


    /*!
    * @brief      根据当前默认的字符编码方式，转换为UTF8
    * @return     int == 0表示转换成功
    * @param      src 源字符串，CString结构，根据MFC的字符集编码集决定
    * @param      dst 转后的的字符串，这个函数默认转换为UTF8的字符集合
    * @note       
    */
    int convert_to_utf8(CString &src, std::string &dst);
    
    //
    int convert_to_utf16(CString &src, std::string &dst);
    //
    int convert_to_mbcs(CString &src, std::string &dst);

protected:
    //
    static const size_t CONVERT_BUFFER_LEN = 64 * 1024 -1;

protected:

    //单子实例
    static Illusion_Read_Config  *instance_;

protected:

    ///Excel的处理对象,EXCEL的处理类
    Illusion_ExcelFile ils_excel_file_;

    ///
    ZCE_Protobuf_Reflect ils_proto_reflect_;

    ///文件对应的配置数据，用于我的查询
    MAP_FNAME_TO_CFGDATA   file_cfg_map_;

    ///
    CVT_CODING cur_cvt_coding_ = CVT_UTF8;

    ///
    wchar_t *cvt_utf16_buf_ = NULL;

    ///
    char *cvt_utf8_buf_ = NULL;

    ///
    char *cvt_mbcs_buf_ = NULL;

};


