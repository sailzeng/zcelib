//=========================================================================================
#include "zce/predefine.h"
#include "zce/db/sqlite/result.h"

#if defined ZCE_USE_SQLITE && ZCE_USE_SQLITE == 1

namespace zce::sqlite
{
result::~result()
{
    free();
}

bool result::is_null()
{
    return (result_ == nullptr);
}

const char* result::field_name(size_t column)
{
    return result_[column];
}

const char* result::field_data(size_t row, size_t column)
{
    return result_[(row + 1) * column_ + column];
}

//释放结果集合
void result::free()
{
    if (result_)
    {
        ::sqlite3_free_table(result_);
        result_ = nullptr;
    }

    column_ = 0;
    row_ = 0;
}
}

#endif //#if defined ZCE_USE_SQLITE && ZCE_USE_SQLITE == 1
