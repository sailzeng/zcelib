#include "stdafx.h"
#include "illusion_excel_file.h"
#include "illusion_excel_config.h"


//处理的单子实例
Illusion_Excel_Config *Illusion_Excel_Config::instance_ = NULL;

//
Illusion_Excel_Config::Illusion_Excel_Config()
{
}


Illusion_Excel_Config::~Illusion_Excel_Config()
{
}


//清理所有的读取数据
void Illusion_Excel_Config::clear()
{
    file_cfg_map_.clear();
}



//读取所有的枚举值
int Illusion_Excel_Config::read_enumconfig(EXCEL_FILE_DATA & file_Cfg_data)
{
    //前面检查过了
    BOOL bret =  illusion_excel_file_.load_sheet(_T("ENUM_CONFIG"),TRUE);
    if (bret == FALSE)
    {
        return -1;
    }

    //答应行列
    long row_count = illusion_excel_file_.row_count();
    long col_count = illusion_excel_file_.column_count();
    TRACE("ENUM_CONFIG table have col_count = %u row_count =%u\n",col_count,row_count);

    //注意行列的下标都是从1开始。
    const long COL_ENUM_KEY = 1;
    const long COL_ENUM_VALUE = 2;
    const TCHAR ENUM_FIRST_CHAR = _T('[');

    size_t read_enum = 0;
    //读取所有的行
    for (long i=1;i<=row_count;++i)
    {

        long   row_no = i;
        //字符串
        CString enum_key = illusion_excel_file_.get_cell_cstring(row_no,COL_ENUM_KEY);

        //如果第一个字符是[
        if (enum_key[0] == ENUM_FIRST_CHAR )
        {
            CString enum_vlaue = illusion_excel_file_.get_cell_cstring(row_no, COL_ENUM_VALUE);
            file_Cfg_data.file_enum_[enum_key] = enum_vlaue;

            ++read_enum;
        }
    }

    return 0;
}


//读取表格配置
int Illusion_Excel_Config::read_tableconfig(EXCEL_FILE_DATA & file_Cfg_data)
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

    for (long i = 1; i <= row_count; ++i)
    {
    }


    return 0;
}


int Illusion_Excel_Config::process_excelfile(const CString &open_file)
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
    if (illusion_excel_file_.load_sheet(_T("TABLE_CONFIG")) == FALSE ||
        illusion_excel_file_.load_sheet(_T("ENUM_CONFIG")) == FALSE)
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

    //让其他人修改这个配置
    EXCEL_FILE_DATA &cfg_data = (*result.first).second;
    int ret = read_enumconfig(cfg_data);
    if (0 != ret)
    {
        ::AfxMessageBox(_T("你选择的配置EXCEL文件中的ENUM_CONFIG表不正确，请重现检查后打开。!"));
        return ret;
    }

    return 0;
}










