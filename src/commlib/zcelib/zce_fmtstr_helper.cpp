
#include "zce_predefine.h"
#include "zce_fmtstr_helper.h"

//-----------------------------------------------------------------------------------------------------

//INT输出辅助类的构造函数
ZCE_LIB::Int_Out_Helper::Int_Out_Helper(int64_t out_data,
                                        BASE_NUMBER_SYSTEM base,
                                        size_t width,
                                        size_t precision,
                                        int flags):
    out_data_(out_data),
    base_(base),
    width_(width),
    precision_(precision),
    flags_(flags)
{
}

ZCE_LIB::Int_Out_Helper::~Int_Out_Helper()
{
}

//DOUBLE格式化输出辅助
ZCE_LIB::Double_Out_Helper::Double_Out_Helper(double out_data,
                                              size_t width,
                                              size_t precision,
                                              int flags):
    out_data_(out_data),
    width_(width),
    precision_(precision),
    flags_(flags)
{
}

ZCE_LIB::Double_Out_Helper::~Double_Out_Helper()
{
}

//String格式化输出辅助类
ZCE_LIB::String_Out_Helper::String_Out_Helper(const char *out_str_ptr,
                                              size_t out_str_len,
                                              size_t width,
                                              size_t precision ,
                                              int flags ):
    out_str_ptr_(out_str_ptr),
    out_str_len_(out_str_len),
    width_(width),
    precision_(precision),
    flags_(flags)
{

}

//
ZCE_LIB::String_Out_Helper::String_Out_Helper(const std::string &out_str,
                                              size_t width  ,
                                              size_t precision ,
                                              int flags):
    out_str_ptr_(out_str.c_str()),
    out_str_len_(out_str.length()),
    width_(width),
    precision_(precision),
    flags_(flags)
{
}

ZCE_LIB::String_Out_Helper::String_Out_Helper(const char *out_str_ptr,
                                              size_t width,
                                              size_t precision,
                                              int flags):
    out_str_ptr_(out_str_ptr),
    out_str_len_(strlen(out_str_ptr)),
    width_(width),
    precision_(precision),
    flags_(flags)
{
}

ZCE_LIB::String_Out_Helper::~String_Out_Helper()
{
}

