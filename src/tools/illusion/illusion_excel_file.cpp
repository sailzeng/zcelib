锘?#include "stdafx.h"
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


//鍒濆