#include "zce/predefine.h"
#include "zce/logger/logging.h"
#include "zce/db/mysql/field.h"

//如果你要用MYSQL的库
#if defined ZCE_USE_MYSQL && ZCE_USE_MYSQL == 1

namespace zce::mysql
{
field::field(const char* fdata,
             unsigned int flength,
             enum_field_types ftype) :
    field_data_(fdata),
    field_length_(flength),
    field_type_(ftype)
{
}

field::field(const field& others)
{
    field_length_ = others.field_length_;
    field_type_ = others.field_type_;
    field_data_ = others.field_data_;
}
field& field::operator=(const field& others)
{
    field_length_ = others.field_length_;
    field_type_ = others.field_type_;
    field_data_ = others.field_data_;
    return *this;
}

//High Speed 得到String
void field::get_string(std::string& tmpstr) const
{
    tmpstr.reserve(field_length_);
    tmpstr.assign(field_data_, field_length_);
}


}

//如果你要用MYSQL的库
#endif //#if defined ZCE_USE_MYSQL