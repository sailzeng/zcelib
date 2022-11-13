//=========================================================================================
#include "zce/predefine.h"
#include "zce/sqlite/sqlite_result.h"

#if SQLITE_VERSION_NUMBER >= 3005000

namespace zce
{
sqlite_result::~sqlite_result()
{
    free_result();
}

bool sqlite_result::is_null()
{
    return (result_ == nullptr);
}

const char* sqlite_result::field_name(size_t column)
{
    return result_[column - 1];
}

const char* sqlite_result::field_cstr(size_t row, size_t column)
{
    return result_[row * column_ + column - 1];
}

//释放结果集合
void sqlite_result::free_result()
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

#endif //#if SQLITE_VERSION_NUMBER >= 3005000