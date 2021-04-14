
#include "zce/predefine.h"
#include "zce/string/helper.h"

//-----------------------------------------------------------------------------------------------------



//DOUBLE格式化输出辅助
zce::Double_Out_Helper::Double_Out_Helper(double out_data,
                                          size_t width,
                                          size_t precision,
                                          int flags):
    out_data_(out_data),
    width_(width),
    precision_(precision),
    flags_(flags)
{
}

zce::Double_Out_Helper::~Double_Out_Helper()
{
}

//String格式化输出辅助类
zce::String_Out_Helper::String_Out_Helper(const char *out_str_ptr,
                                          size_t out_str_len,
                                          size_t width,
                                          size_t precision,
                                          int flags ):
    out_str_ptr_(out_str_ptr),
    out_str_len_(out_str_len),
    width_(width),
    precision_(precision),
    flags_(flags)
{

}

//
zce::String_Out_Helper::String_Out_Helper(const std::string &out_str,
                                          size_t width,
                                          size_t precision,
                                          int flags):
    out_str_ptr_(out_str.c_str()),
    out_str_len_(out_str.length()),
    width_(width),
    precision_(precision),
    flags_(flags)
{
}

zce::String_Out_Helper::String_Out_Helper(const char *out_str_ptr,
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

zce::String_Out_Helper::~String_Out_Helper()
{
}

