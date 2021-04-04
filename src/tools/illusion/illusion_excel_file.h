/*!
* @copyright  2004-2014  Apache License, Version 2.0 FULLSAIL
* @filename   illusion_excel_file.cpp
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2009��4��3��
* @brief
*
*
* @details
*
*
*
* @note           ������Ǵ��������صģ���������ɣ���лԭ�������ߣ���ֻ������
*                 ����һ��������
*                 ��������һЩ������ʹ�ò�������bool ��ΪBOOL��,���ڶ����ϵ��
*                 �Ҹ���һ���֣��о�ԭ�������߶���OO��˼·���ֲ��Ǻ������
*                 �������ණ��OLE������ȫ���˽⣬�ñ��˷�װ�Ķ����о����Ƿ�����
*                 �ܶ࣬��Դ��ΰ��Ŀ�Դ���� C++
*                 http://blog.csdn.net/gyssoft/archive/2007/04/29/1592104.aspx
*
*                 OLE��дEXCEL���Ƚ���������Ӧ�þ�������OLE�Ĵ���
*                 ���ڶ�ȡ�����н��������������һ��Ԥ���صķ�ʽ���������һ�μ���
*                 ���еĶ�ȡ����,����ٶȾͷɿ��ˡ�
*                 ��˵д������û��ʲô�����ӿ��
*                 http://topic.csdn.net/t/20030626/21/1962211.html
*
*                 ������һЩд�뷽ʽ�Ĵ��룬��֤����д��EXCEL�����������Ƕ��ڱ��棬
*                 �ҷ����������CLOSE���ұ���ķ�ʽ���ٶȷǳ������Ҳ����Ϊʲô��
*                 �����Ұ�EXCEL���ˣ�������к�������,
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

    //���һ��CELL�Ƿ����ַ���
    BOOL    is_cell_string(long iRow, long iColumn);
    //���һ��CELL�Ƿ�����ֵ
    BOOL    is_cell_number(long iRow, long iColumn);

    //�õ�һ��CELL��CString
    CString get_cell_cstring(long iRow, long iColumn);
    //�õ�����
    int     get_cell_int(long iRow, long iColumn);
    //�õ�double������
    double  get_celldouble(long iRow, long iColumn);

    //ȡ���е�����
    long row_count();
    //ȡ���е�����
    long column_count();

    //ʹ��ĳ��shet��shit��shit
    BOOL load_sheet(long table_index, BOOL pre_load = FALSE);
    //ͨ������ʹ��ĳ��sheet��
    BOOL load_sheet(const CString &sheet, BOOL pre_load = FALSE);
    //ͨ�����ȡ��ĳ��Sheet������
    CString sheet_name(long table_index);

    //�õ�Sheet������
    int sheets_count();

    //���ļ�
    BOOL open_excelfile(const CString &file_name);
    //�رմ򿪵�Excel �ļ�����ʱ���EXCEL�ļ���Ҫ
    void close_excelfile(BOOL if_save = FALSE);
    //���Ϊһ��EXCEL�ļ�
    void saveas_excelfile(const CString &xls_file);
    //ȡ�ô��ļ�������
    CString open_filename();

    //
    void set_cell_int(long iline, long icolumn, int new_int);
    //
    void set_cell_string(long iline, long icolumn, const CString &new_string);

public:
    //��ʼ��EXCEL OLE
    static BOOL init_excel();
    //�ͷ�EXCEL�� OLE
    static void release_excel();

    //ȡ���е����ƣ�����27->AA
    static TCHAR *column_name(long column_no);

    //����Ҫ���Ƕ���һЩ��ӡ����������
    static char *column_mbcs_name(long column_no);

protected:

    //Ԥ�ȼ���
    void preload_sheet();

protected:
    //�򿪵�EXCEL�ļ�����
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

    ///�Ƿ����Ԥ�ȼ��أ�Ԥ���ظ��������ڴ棬����ȡЧ����߼�ʮ��
    BOOL          already_preload_;

    //Create the SAFEARRAY from the VARIANT ret.
    COleSafeArray ole_safe_array_;
};
