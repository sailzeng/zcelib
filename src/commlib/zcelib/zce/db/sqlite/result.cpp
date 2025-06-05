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
    cursor_row_ = 0;
}
result::~result()
{
    release();
}

result::result(result&& res) noexcept
{
    //移动资源
    move_result(std::move(res));
}

result& result::operator=(result&& res) noexcept
{
    //清理
    release();
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
    already_fetched_ = res.already_fetched_;
    res.clear();
}

//释放结果集合
void result::release()
{
    if (sq_result_)
    {
        ::sqlite3_free_table(sq_result_);
    }
    clear();
}

void result::clear()
{
    sq_result_ = nullptr;
    num_result_column_ = 0;
    num_result_row_ = 0;
    cursor_row_ = 0;
    already_fetched_ = false;
}

bool result::is_null()
{
    return (sq_result_ == nullptr);
}

//! do{ dosomesth; }while(cursor_next())
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

bool result::fetch_next()
{
    if (already_fetched_ == false)
    {
        already_fetched_ = true;
        return true;
    }
    return cursor_next();
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
    cmd_ = &cmd;
    num_result_column_ = ::sqlite3_column_count(cmd_->statement_);
}

//移动构造函数
stmt_result::stmt_result(stmt_result&& res) noexcept
{
    //移动资源
    cmd_ = res.cmd_;
    num_result_column_ = res.num_result_column_;
    res.clear();
}
stmt_result& stmt_result::operator=(stmt_result&& res) noexcept
{
    cmd_ = res.cmd_;
    num_result_column_ = res.num_result_column_;
    res.clear();
    return *this;
}

void stmt_result::clear()
{
    num_result_column_ = 0;
    cursor_row_ = 0;
    already_fetched_ = false;
}

bool stmt_result::reset()
{
    int ret = ::sqlite3_reset(cmd_->statement_);
    if (ret != SQLITE_OK)
    {
        ZCE_LOG(RS_ERROR, "sqlite3_reset error:%d, %s",
                ::sqlite3_errcode(::sqlite3_db_handle(cmd_->statement_)),
                ::sqlite3_errmsg(::sqlite3_db_handle(cmd_->statement_)));
        return false;
    }
    ret = ::sqlite3_step(cmd_->statement_);
    if (ret != SQLITE_ROW)
    {
        ZCE_LOG(RS_ERROR, "sqlite3_reset error:%d, %s",
                ::sqlite3_errcode(::sqlite3_db_handle(cmd_->statement_)),
                ::sqlite3_errmsg(::sqlite3_db_handle(cmd_->statement_)));
        return false;
    }
    cursor_row_ = 0;
    already_fetched_ = false;
    return true;
}

// do{ dosometh; }while(cursor_next())
bool stmt_result::cursor_next()
{
    if (cmd_->statement_ == nullptr)
    {
        return false;
    }
    int ret = ::sqlite3_step(cmd_->statement_);
    if (ret == SQLITE_DONE)
    {
        //这儿加入sqlite3_reset操作，是想你再次判定
        cmd_->bind_reset();
        return false;
    }
    else if (ret == SQLITE_ROW)
    {
        ++cursor_row_;
        return true;
    }
    else
    {
        ZCE_LOG(RS_ERROR, "sqlite3_step error:%d, %s",
                ::sqlite3_errcode(::sqlite3_db_handle(cmd_->statement_)),
                ::sqlite3_errmsg(::sqlite3_db_handle(cmd_->statement_)));
        return false;
    }
}

// while(fetch_next()) { dosometh; }
bool stmt_result::fetch_next()
{
    if (already_fetched_ == false)
    {
        already_fetched_ = true;
        return true;
    }
    return cursor_next();
}

void stmt_result::field(size_t col, char& val) const
{
    val = static_cast<char>(::sqlite3_column_int(cmd_->statement_, (int)col));
    return;
}

void stmt_result::field(size_t col, short& val) const
{
    val = static_cast<short>(::sqlite3_column_int(cmd_->statement_,
                                                  (int)col));
    return;
}

void stmt_result::field(size_t col, int& val) const
{
    val = ::sqlite3_column_int(cmd_->statement_,
                               (int)col);
    return;
}

void stmt_result::field(size_t col, long& val) const
{
    val = ::sqlite3_column_int(cmd_->statement_,
                               (int)col);
    return;
}

void stmt_result::field(size_t col, long long& val) const
{
    val = ::sqlite3_column_int64(cmd_->statement_,
                                 (int)col);
    return;
}

void stmt_result::field(size_t col, unsigned char& val) const
{
    val = static_cast<unsigned char>(::sqlite3_column_int(cmd_->statement_,
                                                          (int)col));
    return;
}

void stmt_result::field(size_t col, unsigned short& val) const
{
    val = static_cast<unsigned short>(::sqlite3_column_int(cmd_->statement_,
                                                           (int)col));
    return;
}

void stmt_result::field(size_t col, unsigned int& val) const
{
    val = static_cast<unsigned int>(sqlite3_column_int(cmd_->statement_,
                                                       (int)col));
    return;
}

void stmt_result::field(size_t col, unsigned long& val) const
{
    val = static_cast<unsigned long>(sqlite3_column_int(cmd_->statement_,
                                                        (int)col));

    return;
}

void stmt_result::field(size_t col, unsigned long long& val) const
{
    val = static_cast<unsigned long long> (sqlite3_column_int64(cmd_->statement_,
                                                                (int)col));
    return;
}

void stmt_result::field(size_t col, float& val) const
{
    val = static_cast<float> (sqlite3_column_double(cmd_->statement_,
                                                    (int)col));

    return;
}

void stmt_result::field(size_t col, double& val) const
{
    val = sqlite3_column_double(cmd_->statement_,
                                (int)col);
    return;
}

void stmt_result::field(size_t col, char* val) const
{
    //Fisk这个变态让我改了地方，为了安全检查。
    strncpy(val,
            reinterpret_cast<const char*>(sqlite3_column_text(cmd_->statement_,
                                                              (int)col)),
            static_cast<size_t>(sqlite3_column_bytes(cmd_->statement_, (int)col)));
    return;
}

//二进制的数据要特别考虑一下,字符串都特别+1了,而二进制数据不要这样考虑
void stmt_result::field(size_t col, zce::string_buf& val) const
{
    val.assign((char*)::sqlite3_column_blob(cmd_->statement_, (int)col),
               ::sqlite3_column_bytes(cmd_->statement_, (int)col));
    return;
}

void stmt_result::field(size_t col, std::string& val) const
{
    val.assign(reinterpret_cast<const char*>(sqlite3_column_text(cmd_->statement_,
                                                                 (int)col)),
               ::sqlite3_column_bytes(cmd_->statement_, (int)col));
    return;
}

const char* stmt_result::field_name(size_t col)
{
    return ::sqlite3_column_name(cmd_->statement_, (int)col);
}

int stmt_result::field_length(size_t col)
{
    return ::sqlite3_column_bytes(cmd_->statement_, (int)col);
}
}
#endif //#if defined ZCE_USE_SQLITE && ZCE_USE_SQLITE == 1
