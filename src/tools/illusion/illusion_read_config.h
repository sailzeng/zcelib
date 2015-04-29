#pragma once

#include "illusion_protobuf_reflect.h"


class Illusion_Read_Config
{
public:



    struct TABLE_CONFIG
    {
        ///表格名称
        CString excel_table_name_;

        ///表格数据从第几行读取
        long read_data_start_ = 3;

        ///表格对应的protobuf的message名称
        std::string   pb_msg_name_;

        ///表格的第几行描述字段对应的protobuf
        long pb_fieldname_line_ = 2;

        ///表格存放的数据库（SQLite）文件名称
        std::string sqlite3_db_name_;

        ///表格对应的table id
        unsigned int table_id_ = 0;
        ///表格索引的字段1的列号
        long index1_column_ = 0;
        ///表格索引的字段2的列号
        long index2_column_ = 0;


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
    bool initialize(bool need_open_excel,
                    const std::string &config_path);
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



    ///从DB3文件里面读取某个配置表的配置
    int read_db3_conftable(const std::string &db3_fname,
                           const std::string &conf_message_name,
                           unsigned int table_id,
                           unsigned int index_1 = 0,
                           unsigned int index_2 = 0);

protected:

    //读枚举值
    int read_table_enum(EXCEL_FILE_DATA &file_cfg_data);

    ///读取表格配置
    int read_table_config(EXCEL_FILE_DATA &file_cfg_data);

    ///读取表格数据
    int read_table_cfgdata(TABLE_CONFIG &table_cfg,
                           ARRARY_OF_AI_IIJIMA_BINARY *aiiijma_ary);

    ///将数据保存到SQLite3 DB文件里面
    int save_to_sqlitedb(const TABLE_CONFIG &table_cfg,
                         const ARRARY_OF_AI_IIJIMA_BINARY *aiiijma_ary);



protected:

    //单子实例
    static Illusion_Read_Config  *instance_;

protected:

    ///配置路径
    std::string config_path_;

    ///日志输出的目录
    std::string outlog_dir_path_;

    ///DB3文件输出的目录
    std::string sqlitedb_pah_;

    bool need_open_excel_ = false;

    ///Excel的处理对象,EXCEL的处理类
    Illusion_ExcelFile ils_excel_file_;

    ///
    Illusion_Protobuf_Reflect ils_proto_reflect_;

    ///文件对应的配置数据，用于我的查询
    MAP_FNAME_TO_CFGDATA   file_cfg_map_;









};


