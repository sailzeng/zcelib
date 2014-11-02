// illusion_doc.h :  CillusionDoc 类的接口
//


#pragma once


struct EXCEL_TABLE_CONFIG
{
    //表格名称
    CString table_name_;

    //表格数据从第几行读取
    unsigned int read_data_start_ = 3;

    //表格对应的protobuf的message名称
    CString  protobuf_message_;

    //表格的第几行描述字段对应的protobuf
    unsigned int protobuf_cfg_line_ = 2;

    //表格存放的数据库（SQLite）文件名称
    CString sqlite3_db_name_;
    
    //表格对应的table id
    unsigned int table_id_ = 0;
    //表格索引的字段1的列号
    unsigned int index1_column_ = 0;
    
    //表格索引的字段2的列号
    unsigned int index2_column_ = 0;
    
    //空字段的默认值

    
};

class Illusion_Excel_Config 
{
protected: 
    //枚举值的对应关系表
    typedef std::map <CString ,int >              MAP_STRING_TO_INT;

    ///
    


protected: // 仅从序列化创建
	Illusion_Excel_Config();
protected:
    virtual ~Illusion_Excel_Config();

    
protected:


    //枚举值的MAP，用于我的查询
    MAP_STRING_TO_INT             map_enum_conf_;

    //Excel的处理对象,EXCEL的处理类
    Illusion_ExcelFile             illusion_excel_file_;

    //SQL语句的缓冲区
    char                         *sql_string_;

protected:
    //单子实例
    static Illusion_Excel_Config  *instance_;
    
public:

    //处理EXCEL文件
    virtual int ProcessExcelFile(const CString &open_file) = 0;

    //清理所有的读取数据
    virtual void ClearAllReadConf();

protected:

    //读枚举值
    int ReadEnumTable();

};


