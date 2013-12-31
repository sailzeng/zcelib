#include "zce_predefine.h"
#include "zce_mysql_field.h"

//如果你要用MYSQL的库
#if defined MYSQL_VERSION_ID

//High Speed 得到String
void ZCE_Mysql_Field::get_string(std::string &tmpstr) const
{
    tmpstr.reserve(field_length_);
    tmpstr.assign(field_data_, field_length_);
}

//如果你要用MYSQL的库
#endif //#if defined MYSQL_VERSION_ID

