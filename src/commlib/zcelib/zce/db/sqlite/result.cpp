//=========================================================================================
#include "zce/predefine.h"
#include "zce/logger/logging.h"
#include "zce/db/sqlite/command.h"
#include "zce/db/sqlite/result.h"

#if defined ZCE_USE_SQLITE && ZCE_USE_SQLITE == 1

namespace zce::sqlite
{
///****************************************************************************************
/// class result
///****************************************************************************************
result::result(char** res, int row_num, int col_num)
{   //初始化
    sq_result_ = res;
    num_result_row_ = row_num;
    num_result_column_ = col_num;
}
result::~result()
{
    free();
}

result::result(result&& res) noexcept
{
    //移动资源
    move_result(std::move(res));
}

result& result::operator=(result&& res) noexcept
{
    //清理
    free();
    move_result(std::move(res));
    return *this;
}

void result::move_result(result&& res) noexcept
{
    //移动资源
    sq_result_ = res.sq_result_;
    num_result_column_ = res.num_result_column_;
    num_result_row_ = res.num_result_row_;
    cursor_row_ = res.cursor_row_;
    //清理原来的资源
    res.sq_result_ = nullptr;
    res.num_result_column_ = 0;
    res.num_result_row_ = 0;
    res.cursor_row_ = size_t(-1);
}

//释放结果集合
void result::free()
{
    if (sq_result_)
    {
        ::sqlite3_free_table(sq_result_);
        sq_result_ = nullptr;
    }
    num_result_column_ = 0;
    num_result_row_ = 0;
    cursor_row_ = size_t(-1);
}

bool result::is_null()
{
    return (sq_result_ == nullptr);
}

bool result::cursor_next()
{
    if (sq_result_ == nullptr || cursor_row_ + 1 >= num_result_row_)
    {
        return false;
    }
    ++cursor_row_;
    return true;
}

bool result::cursor_seek(size_t row_id)
{
    if (sq_result_ == nullptr || row_id >= num_result_row_)
    {
        return false;
    }
    cursor_row_ = row_id;
    return true;
}

const char* result::field_name(size_t field)
{
    ZCE_ASSERT(field < num_result_column_);
    return sq_result_[field];
}

const char* result::field_data(size_t row, size_t field)
{
    ZCE_ASSERT(sq_result_ != nullptr && field < num_result_column_ && row < num_result_row_);
    return sq_result_[(row + 1) * num_result_column_ + field];
}

///****************************************************************************************
/// class stmt_result
///****************************************************************************************
stmt_result::stmt_result(zce::sqlite::command& cmd)
{
    statement_ = cmd.get_sqlite3_stmt();
    num_result_column_ = ::sqlite3_column_count(statement_);
}

//移动构造函数
stmt_result::stmt_result(stmt_result&& res) noexcept
{
    //移动资源
    statement_ = res.statement_;
    num_result_column_ = res.num_result_column_;
    res.statement_ = nullptr;
    res.num_result_column_ = 0;
}
stmt_result& stmt_result::operator=(stmt_result&& res) noexcept
{
    //清理
    statement_ = res.statement_;
    num_result_column_ = res.num_result_column_;
    res.statement_ = nullptr;
    res.num_result_column_ = 0;
    return *this;
}

stmt_result& stmt_result::operator=(sqlite3_stmt* statement) noexcept
{
    statement_ = statement;
    num_result_column_ = ::sqlite3_column_count(statement_);
    return *this;
}

// 取下一行数据
bool stmt_result::fectch_next()
{
    if (statement_ == nullptr)
    {
        return false;
    }
    return ::sqlite3_step(statement_) == SQLITE_ROW;
}

// 将光标至于下一行
bool stmt_result::cursor_next()
{
    if (statement_ == nullptr)
    {
        return false;
    }
    if (cursor_row_ == size_t(-1))
    {
        ++cursor_row_;
        return true;
    }
    else
    {
        ++cursor_row_;
        return ::sqlite3_step(statement_) == SQLITE_ROW;
    }
}

void stmt_result::field(size_t col, char& val)
{
    val = static_cast<char>(::sqlite3_column_int(statement_, (int)col));
    return;
}

void stmt_result::field(size_t col, short& val)
{
    val = static_cast<short>(::sqlite3_column_int(statement_,
                             (int)col));
    return;
}

void stmt_result::field(size_t col, int& val)
{
    val = ::sqlite3_column_int(statement_,
                               (int)col);
    return;
}

void stmt_result::field(size_t col, long& val)
{
    val = ::sqlite3_column_int(statement_,
                               (int)col);
    return;
}

void stmt_result::field(size_t col, long long& val)
{
    val = ::sqlite3_column_int64(statement_,
                                 (int)col);
    return;
}

void stmt_result::field(size_t col, unsigned char& val)
{
    val = static_cast<unsigned char>(::sqlite3_column_int(statement_,
                                     (int)col));
    return;
}

void stmt_result::field(size_t col, unsigned short& val)
{
    val = static_cast<unsigned short>(::sqlite3_column_int(statement_,
                                      (int)col));
    return;
}

void stmt_result::field(size_t col, unsigned int& val)
{
    val = static_cast<unsigned int>(sqlite3_column_int(statement_,
                                    (int)col));
    return;
}

void stmt_result::field(size_t col, unsigned long& val)
{
    val = static_cast<unsigned long>(sqlite3_column_int(statement_,
                                     (int)col));

    return;
}

void stmt_result::field(size_t col, unsigned long long& val)
{
    val = static_cast<unsigned long long> (sqlite3_column_int64(statement_,
                                           (int)col));
    return;
}

void stmt_result::field(size_t col, float& val)
{
    val = static_cast<float> (sqlite3_column_double(statement_,
                              (int)col));

    return;
}

void stmt_result::field(size_t col, double& val)
{
    val = sqlite3_column_double(statement_,
                                (int)col);
    return;
}

void stmt_result::field(size_t col, char* val)
{
    //Fisk这个变态让我改了地方，为了安全检查。
    strncpy(val,
            reinterpret_cast<const char*>(sqlite3_column_text(statement_,
            (int)col)),
            static_cast<size_t>(sqlite3_column_bytes(statement_, (int)col)));
    return;
}

//二进制的数据要特别考虑一下,字符串都特别+1了,而二进制数据不要这样考虑
void stmt_result::field(size_t col, zce::string_buf& val)
{
    val.assign((char*)::sqlite3_column_blob(statement_, (int)col),
               ::sqlite3_column_bytes(statement_, (int)col));
    return;
}

void stmt_result::field(size_t col, std::string& val)
{
    val.assign(reinterpret_cast<const char*>(sqlite3_column_text(statement_,
               (int)col)),
               ::sqlite3_column_bytes(statement_, (int)col));
    return;
}

const char* stmt_result::field_name(size_t col)
{
    return ::sqlite3_column_name(statement_, (int)col);
}

int stmt_result::field_length(size_t col)
{
    return ::sqlite3_column_bytes(statement_, (int)col);
}
}
#endif //#if defined ZCE_USE_SQLITE && ZCE_USE_SQLITE == 1
