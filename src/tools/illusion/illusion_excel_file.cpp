
#include "stdafx.h"
#include "illusion_excel_file.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

_variant_t CONST_VARIANT_TRUE((short)TRUE);
_variant_t CONST_VARIANT_FALSE((short)FALSE);
_variant_t CONST_VARIANT_OPTIONAL((long)DISP_E_PARAMNOTFOUND, VT_ERROR);


//
CApplication Illusion_ExcelFile::excel_application_;


Illusion_ExcelFile::Illusion_ExcelFile():
    already_preload_(FALSE)
{
}

Illusion_ExcelFile::~Illusion_ExcelFile()
{
    //
    close_excelfile();
}


//��ʼ��EXCEL�ļ���
BOOL Illusion_ExcelFile::init_excel()
{

    HRESULT hret = ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    //��ʵ�����ʼ����������������ú���
    if (FAILED(hret))
    {
        ::AfxMessageBox(_T("��ʼ��COM֧�ֿ�ʧ��!"));
        return FALSE;
    }

    //����Excel 2000������(����Excel)
    if (!excel_application_.CreateDispatch(_T("Excel.Application"), NULL))
    {
        ::AfxMessageBox(_T("�޷�����EXCEL,�����Ƿ�װ��Office EXCEL�����߼��ע���"));
        return FALSE;
    }

    excel_application_.put_DisplayAlerts(FALSE);
    return TRUE;
}

//
void Illusion_ExcelFile::release_excel()
{
    excel_application_.Quit();
    excel_application_.ReleaseDispatch();
    excel_application_ = NULL;

    ::CoUninitialize();
}

//��excel�ļ�
BOOL Illusion_ExcelFile::open_excelfile(const CString &file_name)
{
    //�ȹر�
    close_excelfile();

    //����ģ���ļ��������ĵ�
    excel_books_.AttachDispatch(excel_application_.get_Workbooks(), true);

    LPDISPATCH lpDis = NULL;
    _variant_t v_fn(file_name);
    lpDis = excel_books_.Open(file_name,
                              vtMissing, vtMissing, vtMissing, vtMissing, vtMissing,
                              vtMissing, vtMissing, vtMissing, vtMissing, vtMissing,
                              vtMissing, vtMissing, vtMissing, vtMissing);
    if (lpDis)
    {
        excel_work_book_.AttachDispatch(lpDis);
        //�õ�Worksheets
        excel_sheets_.AttachDispatch(excel_work_book_.get_Worksheets(), true);

        //��¼�򿪵��ļ�����
        open_excel_file_ = file_name;

        return TRUE;
    }

    return FALSE;

}

//�رմ򿪵�Excel �ļ�,Ĭ������������ļ�
void Illusion_ExcelFile::close_excelfile(BOOL if_save)
{
    //����Ѿ��򿪣��ر��ļ�
    if (open_excel_file_.IsEmpty() == FALSE)
    {
        //�������,�����û�����,���û��Լ��棬����Լ�SAVE�������Ī���ĵȴ�
        if (if_save)
        {
            show_in_excel(TRUE);
        }
        else
        {
            //
            _variant_t v_excel_file(open_excel_file_);
            excel_work_book_.Close(CONST_VARIANT_OPTIONAL,
                                   CONST_VARIANT_OPTIONAL,
                                   CONST_VARIANT_OPTIONAL);
            excel_books_.Close();
        }

        //���ļ����������
        open_excel_file_.Empty();
    }

    excel_sheets_.ReleaseDispatch();
    excel_work_sheet_.ReleaseDispatch();
    excel_current_range_.ReleaseDispatch();
    excel_work_book_.ReleaseDispatch();
    excel_books_.ReleaseDispatch();
}

void Illusion_ExcelFile::saveas_excelfile(const CString &xls_file)
{
    _variant_t v_xls_file(xls_file);
    excel_work_book_.SaveAs(v_xls_file,
                            CONST_VARIANT_OPTIONAL,
                            CONST_VARIANT_OPTIONAL,
                            CONST_VARIANT_OPTIONAL,
                            CONST_VARIANT_OPTIONAL,
                            CONST_VARIANT_OPTIONAL,
                            0,
                            CONST_VARIANT_OPTIONAL,
                            CONST_VARIANT_OPTIONAL,
                            CONST_VARIANT_OPTIONAL,
                            CONST_VARIANT_OPTIONAL,
                            CONST_VARIANT_OPTIONAL);
    return;
}


int Illusion_ExcelFile::sheets_count()
{
    return excel_sheets_.get_Count();
}


CString Illusion_ExcelFile::sheet_name(long table_index)
{
    CWorksheet sheet;
    _variant_t v_t_idx((long)table_index);
    sheet.AttachDispatch(excel_sheets_.get_Item(v_t_idx), true);
    CString name = sheet.get_Name();
    sheet.ReleaseDispatch();
    return name;
}

//������ż���Sheet���,������ǰ�������еı���ڲ�����
BOOL Illusion_ExcelFile::load_sheet(long table_index, BOOL pre_load)
{
    LPDISPATCH lpDis = NULL;
    excel_current_range_.ReleaseDispatch();
    excel_work_sheet_.ReleaseDispatch();
    _variant_t v_t_idx((long)table_index);
    try
    {
        lpDis = excel_sheets_.get_Item(v_t_idx);
    }
    catch (...)
    {
        lpDis = NULL;
    }

    if (lpDis)
    {
        excel_work_sheet_.AttachDispatch(lpDis, TRUE);
        excel_current_range_.AttachDispatch(excel_work_sheet_.get_Cells(), TRUE);
    }
    else
    {
        return FALSE;
    }

    already_preload_ = FALSE;
    //�������Ԥ�ȼ���
    if (pre_load)
    {
        preload_sheet();
        already_preload_ = TRUE;
    }

    return TRUE;
}

//�������Ƽ���Sheet���,������ǰ�������еı���ڲ�����
BOOL Illusion_ExcelFile::load_sheet(const CString &sheet, BOOL pre_load)
{
    LPDISPATCH lpDis = NULL;
    excel_current_range_.ReleaseDispatch();
    excel_work_sheet_.ReleaseDispatch();
    _variant_t  v_sheet(sheet);
    try
    {
        lpDis = excel_sheets_.get_Item(v_sheet);
    }
    catch (...)
    {
        lpDis = NULL;
    }

    if (lpDis)
    {
        excel_work_sheet_.AttachDispatch(lpDis, TRUE);
        excel_current_range_.AttachDispatch(excel_work_sheet_.get_Cells(), TRUE);

    }
    else
    {
        return FALSE;
    }
    //
    already_preload_ = FALSE;
    //�������Ԥ�ȼ���
    if (pre_load)
    {
        already_preload_ = TRUE;
        preload_sheet();
    }

    return TRUE;
}

//�õ��е�����
long Illusion_ExcelFile::column_count()
{
    CRange range;
    CRange usedRange;
    usedRange.AttachDispatch(excel_work_sheet_.get_UsedRange(), true);
    range.AttachDispatch(usedRange.get_Columns(), true);
    long count = range.get_Count();
    usedRange.ReleaseDispatch();
    range.ReleaseDispatch();
    return count;
}

//�õ��е�����
long Illusion_ExcelFile::row_count()
{
    CRange range;
    CRange usedRange;
    usedRange.AttachDispatch(excel_work_sheet_.get_UsedRange(), true);
    range.AttachDispatch(usedRange.get_Rows(), true);
    long count = range.get_Count();
    usedRange.ReleaseDispatch();
    range.ReleaseDispatch();
    return count;
}

//���һ��CELL�Ƿ����ַ���
BOOL Illusion_ExcelFile::is_cell_string(long irow, long icolumn)
{
    CRange range;
    _variant_t v_row((long)irow);
    _variant_t v_column((long)icolumn);
    range.AttachDispatch(excel_current_range_.get_Item(v_row, v_column).pdispVal, true);
    COleVariant vResult = range.get_Value2();
    //VT_BSTR��ʾ�ַ���
    if (vResult.vt == VT_BSTR)
    {
        return TRUE;
    }
    return FALSE;
}

//���һ��CELL�Ƿ�����ֵ
BOOL Illusion_ExcelFile::is_cell_number(long iline, long icolumn)
{
    CRange range;
    _variant_t v_row((long)iline);
    _variant_t v_column((long)icolumn);
    range.AttachDispatch(excel_current_range_.get_Item(v_row, v_column).pdispVal, true);
    VARIANT vResult = range.get_Value2();
    //����һ�㶼��VT_R8
    if (vResult.vt == VT_INT || vResult.vt == VT_R8)
    {
        return TRUE;
    }
    return FALSE;
}

//
CString Illusion_ExcelFile::get_cell_cstring(long iline, long icolumn)
{

    COleVariant vResult ;
    CString str;
    //�ַ���
    if (already_preload_ == FALSE)
    {
        CRange range;
        _variant_t v_row(iline);
        _variant_t v_column(icolumn);
        range.AttachDispatch(excel_current_range_.get_Item(v_row, v_column).pdispVal, true);
        vResult = range.get_Value2();
        range.ReleaseDispatch();
    }
    //�����������Ԥ�ȼ�����
    else
    {
        long read_address[2];
        VARIANT val;
        read_address[0] = iline;
        read_address[1] = icolumn;
        ole_safe_array_.GetElement(read_address, &val);
        vResult = val;
    }

    if (vResult.vt == VT_BSTR)
    {
        str = vResult.bstrVal;
    }
    //����
    else if (vResult.vt == VT_INT)
    {
        str.Format(_T("%d"), vResult.pintVal);
    }
    //8�ֽڵ�����
    else if (vResult.vt == VT_R8)
    {
        str.Format(_T("%.6f"), vResult.dblVal);
    }
    //ʱ���ʽ
    else if (vResult.vt == VT_DATE)
    {
        SYSTEMTIME st;
        VariantTimeToSystemTime(vResult.date, &st);
        CTime tm(st);
        str = tm.Format(_T("%Y-%m-%d"));

    }
    //��Ԫ��յ�
    else if (vResult.vt == VT_EMPTY)
    {
        str = "";
    }

    return str;
}

//VT_R8
int Illusion_ExcelFile::get_cell_int(long iline, long icolumn)
{
    int num;
    COleVariant vresult;

    if (already_preload_ == FALSE)
    {
        CRange range;
        _variant_t v_row((long)iline);
        _variant_t v_column((long)icolumn);
        range.AttachDispatch(excel_current_range_.get_Item(v_row, v_column).pdispVal, true);
        vresult = range.get_Value2();
        range.ReleaseDispatch();
    }
    else
    {
        long read_address[2];
        VARIANT val;
        read_address[0] = iline;
        read_address[1] = icolumn;
        ole_safe_array_.GetElement(read_address, &val);
        vresult = val;
    }
    //
    num = static_cast<int>(vresult.dblVal);

    return num;
}

void Illusion_ExcelFile::set_cell_string(long iline, long icolumn, const CString &new_string)
{
    _variant_t new_value(new_string);
    _variant_t v_pos("A1");
    CRange start_range = excel_work_sheet_.get_Range(v_pos, CONST_VARIANT_OPTIONAL);

    _variant_t v_row((long)iline - 1);
    _variant_t v_column((long)icolumn - 1);

    CRange write_range = start_range.get_Offset(v_row, v_column);
    write_range.put_Value2(new_value);
    start_range.ReleaseDispatch();
    write_range.ReleaseDispatch();

}

void Illusion_ExcelFile::set_cell_int(long iline, long icolumn, int new_int)
{
    _variant_t new_value((long)new_int);

    _variant_t v_pos("A1");
    CRange start_range = excel_work_sheet_.get_Range(v_pos, CONST_VARIANT_OPTIONAL);

    _variant_t v_row((long)iline - 1);
    _variant_t v_column((long)icolumn - 1);

    CRange write_range = start_range.get_Offset(v_row, v_column);
    write_range.put_Value2(new_value);
    start_range.ReleaseDispatch();
    write_range.ReleaseDispatch();
}


//
void Illusion_ExcelFile::show_in_excel(BOOL bShow)
{
    excel_application_.put_Visible(bShow);
    excel_application_.put_UserControl(bShow);
}

//���ش򿪵�EXCEL�ļ�����
CString Illusion_ExcelFile::open_filename()
{
    return open_excel_file_;
}

//ȡ���е����ƣ�����27->AA
TCHAR *Illusion_ExcelFile::column_name(long column_no)
{
    static TCHAR column_name[64];
    size_t str_len = 0;

    while (column_no > 0)
    {
        int num_data = column_no % 26;
        column_no /= 26;
        if (num_data == 0)
        {
            num_data = 26;
            column_no--;
        }
        //��֪������Բ���
        column_name[str_len] = (TCHAR)((num_data - 1) + _T('A') );
        str_len ++;
    }
    column_name[str_len] = '\0';
    //��ת
    _tcsrev(column_name);

    return column_name;
}

char *Illusion_ExcelFile::column_mbcs_name(long column_no)
{
    static unsigned char column_name[64];
    size_t str_len = 0;

    while (column_no > 0)
    {
        int num_data = column_no % 26;
        column_no /= 26;
        if (num_data == 0)
        {
            num_data = 26;
            column_no--;
        }
        //��֪������Բ���
        column_name[str_len] = (char)((num_data - 1) + ('A'));
        str_len++;
    }
    column_name[str_len] = '\0';
    //��ת
    _mbsrev(column_name);

    return(char *) column_name;
}

//Ԥ�ȼ���
void Illusion_ExcelFile::preload_sheet()
{

    CRange used_range;

    used_range = excel_work_sheet_.get_UsedRange();


    VARIANT ret_ary = used_range.get_Value2();
    if (!(ret_ary.vt & VT_ARRAY))
    {
        return;
    }
    //
    ole_safe_array_.Clear();
    ole_safe_array_.Attach(ret_ary);
}

