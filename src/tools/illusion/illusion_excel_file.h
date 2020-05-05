锘塊敇?*!
* @copyright  2004-2014  Apache License, Version 2.0 FULLSAIL
* @filename   illusion_excel_file.cpp
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
<<<<<<< HEAD
* @date       2009楠?閾?闀?* @brief
=======
* @date       2009年4月3日
* @brief
>>>>>>> ecb76a1a4aa8381667ced3cb31202915f48ca78b
*
*
* @details
*
*
*
<<<<<<< HEAD
* @note           鏉╂瑤閲滅猾緇樻Ц嫻犲海綞夋稉濞勭瑓鏉炵晫孌戦敍灞懼灉閸ф劒闊╅崗鑸靛灇閿涘本鍔呯拫銏犲笀閺夈儳孌戞擔婊嗘粏
=======
* @note           这个类是从网上下载的，我坐享其成，感谢原来的作者，我只试试是
*                 做了一下修正。
*                 修正包括一些参数的使用不谨慎，bool 改为BOOL等,对于对象关系，
*                 我改了一部分，感觉原来的作者对于OO的思路部分不是很清楚。
*                 对于这类东西OLE，我完全不了解，用别人封装的东西感觉还是放心了
*                 很多，开源，伟大的开源加上 C++
*                 http://blog.csdn.net/gyssoft/archive/2007/04/29/1592104.aspx
*
*                 OLE读写EXCEL都比较慢，所以应该尽量减少OLE的次数
*                 对于读取，还有解决方法，请试用一下预加载的方式，这个方法一次加载
*                 所有的读取数据,如此速度就飞快了。
*                 据说写数据是没有什么方法加快的
*                 http://topic.csdn.net/t/20030626/21/1962211.html
*
*                 增加了一些写入方式的代码，保证可以写入EXCEL数据区，但是对于保存，
*                 我发现如果调用CLOSE并且保存的方式，速度非常慢，我不理解为什么。
*                 所以我吧EXCEL打开了，让你进行后续管理,
*
*/
#pragma once


class Illusion_ExcelFile
{

public:

    //
    Illusion_ExcelFile();
    virtual ~Illusion_ExcelFile();

protected:
    //
    static CApplication excel_application_;
public:
    //
    void show_in_excel(BOOL bShow);

    //检查一个CELL是否是字符串
    BOOL    is_cell_string(long iRow, long iColumn);
    //检查一个CELL是否是数值
    BOOL    is_cell_number(long iRow, long iColumn);

    //得到一个CELL的CString
    CString get_cell_cstring(long iRow, long iColumn);
    //得到整数
    int     get_cell_int(long iRow, long iColumn);
    //得到double的数据
    double  get_celldouble(long iRow, long iColumn);

    //取得行的总数
    long row_count();
    //取得列的总数
    long column_count();

    //使用某个shet，shit，shit
    BOOL load_sheet(long table_index, BOOL pre_load = FALSE);
    //通过名称使用某个sheet，
    BOOL load_sheet(const CString &sheet, BOOL pre_load = FALSE);
    //通过序号取得某个Sheet的名称
    CString sheet_name(long table_index);

    //得到Sheet的总数
    int sheets_count();

    //打开文件
    BOOL open_excelfile(const CString &file_name);
    //关闭打开的Excel 文件，有时候打开EXCEL文件就要
    void close_excelfile(BOOL if_save = FALSE);
    //另存为一个EXCEL文件
    void saveas_excelfile(const CString &xls_file);
    //取得打开文件的名称
    CString open_filename();

    //
    void set_cell_int(long iline, long icolumn, int new_int);
    //
    void set_cell_string(long iline, long icolumn, const CString &new_string);

public:
    //初始化EXCEL OLE
    static BOOL init_excel();
    //释放EXCEL的 OLE
    static void release_excel();

    //取得列的名称，比如27->AA
    static TCHAR *column_name(long column_no);

    //还是要考虑对于一些打印输出的情况。
    static char *column_mbcs_name(long column_no);

protected:

    //预先加载
    void preload_sheet();

protected:
    //打开的EXCEL文件名称
    CString       open_excel_file_;

    //
    CWorkbooks    excel_books_;
    //
    CWorkbook     excel_work_book_;
    //
    CWorksheets   excel_sheets_;
    //
    CWorksheet    excel_work_sheet_;
    //
    CRange        excel_current_range_;

    ///是否进行预先加载，预加载更加消耗内存，但读取效率提高几十倍
    BOOL          already_preload_;

    //Create the SAFEARRAY from the VARIANT ret.
    COleSafeArray ole_safe_array_;
};
>>>>>>> ecb76a1a4aa8381667ced3cb31202915f48ca78b
