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
void Illusion_Excel_Config::ClearAllReadConf()
{
    map_enum_conf_.clear();
}



//读取所有的枚举值
int Illusion_Excel_Config::ReadEnumTable()
{
    ////调试信息
    //CMainFrame::Instance()->GetOutputView()->FillOutputInfo(
    //    CIllusionOutView::OUT_TYPE_DEBUG,
    //    _T("Start process file[%s] ENUM_DESC sheet.\n"),
    //    illusion_excel_file_.GetOpenFileName());

    //前面检查过了
    illusion_excel_file_.load_sheet("ENUM_DESC",TRUE);

    //答应行列
    long row_count = illusion_excel_file_.GetRowCount();
    long col_count = illusion_excel_file_.GetColumnCount();
    TRACE("col_count = %u row_count =%u\n",col_count,row_count);

    //打印
    //CMainFrame::Instance()->GetOutputView()->FillOutputInfo(
    //    CIllusionOutView::OUT_TYPE_INFO,
    //    _T("ENUM_DESC sheet have row_count =%u col_count = %u\n"),
    //    row_count,
    //    col_count);

    //注意行列的下标都是从1开始。
    const long COL_ENUM_KEY = 1;
    const long COL_ENUM_VALUE = 2;
    const char ENUM_FIRST_CHAR = '[';

    size_t read_enum = 0;
    //读取所有的行
    for (long i=1;i<=row_count;++i)
    {

        long   row_no = i;
        //字符串
        CString enum_key = illusion_excel_file_.GetCellString(row_no,COL_ENUM_KEY);

        //如果第一个字符是[
        if (enum_key[0] == ENUM_FIRST_CHAR )
        {
            int enum_vlaue = illusion_excel_file_.GetCellInt(row_no,COL_ENUM_VALUE);
            map_enum_conf_[enum_key] = enum_vlaue;

            ++read_enum;
        }
    }

    ////打印
    //CMainFrame::Instance()->GetOutputView()->FillOutputInfo(
    //    CIllusionOutView::OUT_TYPE_INFO,
    //    _T("ENUM_DESC sheet have enum count = %u\n"),
    //    read_enum);
    //
    return 0;
}












