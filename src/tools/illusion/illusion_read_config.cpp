#include "stdafx.h"
#include "illusion_excel_file.h"
#include "illusion_read_config.h"


//处理的单子实例
Illusion_Read_Config *Illusion_Read_Config::instance_ = NULL;

//
Illusion_Read_Config::Illusion_Read_Config()
{
    
    cvt_utf16_buf_ = new wchar_t[64*1024];
    cvt_utf8_buf_ = new char [64*1024];
    cvt_mbcs_buf_ = new char[64 * 1024];
}


Illusion_Read_Config::~Illusion_Read_Config()
{
    if (cvt_utf16_buf_)
    {
        delete cvt_utf16_buf_;
        cvt_utf16_buf_ = NULL;
    }
    if (cvt_utf8_buf_)
    {
        delete cvt_utf8_buf_;
        cvt_utf8_buf_ = NULL;
    }
    if (cvt_mbcs_buf_)
    {
        delete cvt_mbcs_buf_;
        cvt_mbcs_buf_ = NULL;
    }
}


Illusion_Read_Config *Illusion_Read_Config::instance()
{
    if (instance_ == NULL)
    {
        instance_ = new Illusion_Read_Config();
    }
    return instance_;
}

void Illusion_Read_Config::clean_instance()
{
    if (instance_)
    {
        delete instance_;
        instance_ = NULL;
    }
}

//
BOOL Illusion_Read_Config::initialize()
{
    return illusion_excel_file_.init_excel();
}
//
void Illusion_Read_Config::finalize()
{
    return illusion_excel_file_.release_excel();
}

//清理所有的读取数据
void Illusion_Read_Config::clear()
{
    file_cfg_map_.clear();
}



//读取所有的枚举值
int Illusion_Read_Config::read_enum_data(EXCEL_FILE_DATA &file_cfg_data)
{
    //前面检查过了
    BOOL bret =  illusion_excel_file_.load_sheet(_T("ENUM_CONFIG"), TRUE);
    if (bret == FALSE)
    {
        return -1;
    }

    //答应行列
    long row_count = illusion_excel_file_.row_count();
    long col_count = illusion_excel_file_.column_count();
    TRACE("ENUM_CONFIG table have col_count = %u row_count =%u\n", col_count, row_count);

    //注意行列的下标都是从1开始。
    const long COL_ENUM_KEY = 1;
    const long COL_ENUM_VALUE = 2;
    const TCHAR ENUM_FIRST_CHAR = _T('[');

    size_t read_enum = 0;
    //读取所有的行
    for (long i = 1; i <= row_count; ++i)
    {

        long   row_no = i;
        //字符串
        CString enum_key = illusion_excel_file_.get_cell_cstring(row_no, COL_ENUM_KEY);

        //如果第一个字符是[
        if (enum_key[0] == ENUM_FIRST_CHAR )
        {
            CString enum_vlaue = illusion_excel_file_.get_cell_cstring(row_no, COL_ENUM_VALUE);
            file_cfg_data.file_enum_[enum_key] = enum_vlaue;

            ++read_enum;
        }
    }

    return 0;
}


//读取表格配置
int Illusion_Read_Config::read_table_config(EXCEL_FILE_DATA &file_cfg_data)
{
    //前面检查过了
    BOOL bret = illusion_excel_file_.load_sheet(_T("TABLE_CONFIG"), TRUE);
    if (bret == FALSE)
    {
        return -1;
    }

    long row_count = illusion_excel_file_.row_count();
    long col_count = illusion_excel_file_.column_count();
    TRACE("TABLE_CONFIG table have col_count = %u row_count =%u\n", col_count, row_count);

    //注意行列的下标都是从1开始。
    const long COL_TC_KEY = 1;
    const long COL_TC_VALUE = 2;

    for (long row_no = 1; row_no <= row_count; ++row_no)
    {

        CString tc_key = illusion_excel_file_.get_cell_cstring(row_no, COL_TC_KEY);


        TABLE_CONFIG tc_data;

        if (tc_key == _T("表格名称"))
        {
            tc_data.table_name_ = illusion_excel_file_.get_cell_cstring(row_no, COL_TC_VALUE);
            if (tc_data.table_name_.IsEmpty())
            {
                return -1;
            }
            //检查EXCEL文件中是否有这个表格
            if (illusion_excel_file_.load_sheet(tc_data.table_name_, FALSE) == FALSE)
            {
                return -3;
            }

            ++row_no;
            if (row_no > row_no)
            {
                return -1;
            }
            tc_data.read_data_start_ = illusion_excel_file_.get_cell_int(row_no, COL_TC_VALUE);
            if (tc_data.read_data_start_ <= 0)
            {
                return -1;
            }

            ++row_no;
            if (row_no > row_no)
            {
                return -1;
            }
            tc_data.protobuf_message_ = illusion_excel_file_.get_cell_cstring(row_no, COL_TC_VALUE);
            if (tc_data.protobuf_message_.IsEmpty())
            {
                return -1;
            }

            ++row_no;
            if (row_no > row_no)
            {
                return -1;
            }
            tc_data.protobuf_cfg_line_ = illusion_excel_file_.get_cell_int(row_no, COL_TC_VALUE);
            if (tc_data.protobuf_cfg_line_ <= 0)
            {
                return -1;
            }

            ++row_no;
            if (row_no > row_no)
            {
                return -1;
            }
            tc_data.sqlite3_db_name_ = illusion_excel_file_.get_cell_cstring(row_no, COL_TC_VALUE);
            if (tc_data.sqlite3_db_name_.IsEmpty())
            {
                return -1;
            }

            ++row_no;
            if (row_no > row_no)
            {
                return -1;
            }
            tc_data.table_id_ = illusion_excel_file_.get_cell_int(row_no, COL_TC_VALUE);
            if (tc_data.table_id_ <= 0)
            {
                return -1;
            }

            ++row_no;
            if (row_no > row_no)
            {
                return -1;
            }
            tc_data.index1_column_ = illusion_excel_file_.get_cell_int(row_no, COL_TC_VALUE);
            if (tc_data.index1_column_ <= 0)
            {
                return -1;
            }

            ++row_no;
            if (row_no > row_no)
            {
                return -1;
            }
            tc_data.index2_column_ = illusion_excel_file_.get_cell_int(row_no, COL_TC_VALUE);
            if (tc_data.index2_column_ <= 0)
            {
                return -1;
            }

            auto result = file_cfg_data.file_table_cfg_.insert(std::make_pair(tc_data.table_name_, tc_data));
            if (false == result.second)
            {
                return -2;
            }
        }
        else
        {
            continue;
        }
    }

    return 0;
}


//读取表格数据read_table_data
int Illusion_Read_Config::read_table_data(TABLE_CONFIG &tc_data)
{
    //检查EXCEL文件中是否有这个表格
    if (illusion_excel_file_.load_sheet(tc_data.table_name_, TRUE) == FALSE)
    {
        return -3;
    }
    long row_count = illusion_excel_file_.row_count();
    long col_count = illusion_excel_file_.column_count();
    TRACE("%s table have col_count = %u row_count =%u\n", tc_data.table_name_, col_count, row_count);

    CString proto_item_name;
    std::string std_item_name;
    for (long col_no = 1; col_no <= col_count; ++col_no)
    {
        proto_item_name = illusion_excel_file_.get_cell_cstring(col_no, tc_data.protobuf_cfg_line_);


#if defined UNICODE || defined _UNICODE

#else
        std_item_name = (LPCTSTR)proto_item_name;
#endif

    }

    return 0;
}

int Illusion_Read_Config::read_excelconfig(const CString &open_file)
{
    BOOL bret = illusion_excel_file_.open_excelfile(open_file);
    //Excel文件打开失败
    if (bret != TRUE)
    {
        return -1;
    }
    //
    TRACE("Dream excecl file have sheet num[%d].\n", illusion_excel_file_.sheets_count());

    //表格错误
    if (illusion_excel_file_.load_sheet(_T("TABLE_CONFIG"), FALSE) == FALSE ||
        illusion_excel_file_.load_sheet(_T("ENUM_CONFIG"), FALSE) == FALSE)
    {
        //
        ::AfxMessageBox(_T("你选择的配置EXCEL不是能读取的配置表，请重现检查后打开。!"));
        return -1;
    }

    //file_cfg_map_[open_file] = excel_data;
    EXCEL_FILE_DATA excel_data;
    auto result = file_cfg_map_.insert(make_pair(open_file, excel_data));
    if (!result.second)
    {
        return -1;
    }

    //
    EXCEL_FILE_DATA &cfg_data = (*result.first).second;
    int ret = read_enum_data(cfg_data);
    if (0 != ret)
    {
        ::AfxMessageBox(_T("你选择的配置EXCEL文件中的ENUM_CONFIG表不正确，请重现检查后打开。!"));
        return ret;
    }

    //
    ret = read_table_config(cfg_data);
    if (0 != ret)
    {
        ::AfxMessageBox(_T("你选择的配置EXCEL文件中的TABLE_CONFIG表不正确，请重现检查后打开。!"));
        return ret;
    }

    return 0;
}


//根据当前默认的字符编码方式，转换为UTF8
int Illusion_Read_Config::convert_to_utf8(CString &src, std::string &dst)
{
#if defined UNICODE || defined _UNICODE
     
    DWORD ret = 0;
    ret = ::WideCharToMultiByte(CP_UTF8, 
        NULL, 
        (LPCTSTR)src,
        src.GetLength(),
        cvt_utf8_buf_,
        CONVERT_BUFFER_LEN,
        NULL,
        0);
    if (ret == 0)
    {
        return -1;
    }
    return 0;

#else
    // MBCS ===> UTF16 ===> UTF8

    // 第一次先把MBCS码转换成UTF-16
    DWORD ret = 0;
    ret = ::MultiByteToWideChar(CP_ACP, 
        0,
        (LPCTSTR)src,
        src.GetLength(),
        cvt_utf16_buf_,
        CONVERT_BUFFER_LEN);
    if (ret == 0)
    {
        return -1;
    }

    // 第二次再把UTF-16编码转换为UTF-8编码
    ret = ::WideCharToMultiByte(CP_UTF8, 
        NULL, 
        cvt_utf16_buf_,
        -1, 
        cvt_utf8_buf_,
        CONVERT_BUFFER_LEN,
        NULL,
        0);
    if (ret == 0)
    {
        return -1;
    }
    return 0;
#endif
}

//根据当前默认的字符编码方式，转换为UTF16(UNICODE)
int Illusion_Read_Config::convert_to_utf16(CString &src, std::string &dst)
{
#if defined UNICODE || defined _UNICODE
    // UTF16 == UTF16
    if (src.GetLength() > CONVERT_BUFFER_LEN)
    {
        return -1;
    }
    dst.assign(((const char *)((LPCTSTR)src)),(src.GetLength()*( sizeof(wchar_t))) );
    return 0;
#else
    // MBCS ===> UTF16
    DWORD ret = 0;
    ret = ::MultiByteToWideChar(CP_ACP,
        0,
        (LPCTSTR)src,
        src.GetLength(),
        cvt_utf16_buf_,
        CONVERT_BUFFER_LEN);
    if (ret == 0)
    {
        return -1;
    }
    return 0;
#endif
}

//根据当前默认的字符编码方式，转换为MBCS
int Illusion_Read_Config::convert_to_mbcs(CString &src, std::string &dst)
{
#if defined UNICODE || defined _UNICODE
    //UTF16 == > MBCS
    DWORD ret = 0;
    ret = ::WideCharToMultiByte(CP_ACP, NULL, (LPCTSTR)src, src.GetLength(), cvt_mbcs_buf_, CONVERT_BUFFER_LEN, NULL, 0);
    if (ret == 0)
    {
        return -1;
    }
    return 0;
#else
    // MBCS ===> MBCS
    if (src.GetLength() > CONVERT_BUFFER_LEN)
    {
        return -1;
    }
    dst.assign(((const char *)((LPCTSTR)src)), (src.GetLength()*(sizeof(wchar_t))));
    return 0;
#endif
}







