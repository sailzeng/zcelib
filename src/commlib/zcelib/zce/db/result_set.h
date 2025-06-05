/*!
* @copyright  2004-2024  Apache License, Version 2.0 FULLSAIL
* @filename   zce_sqlite_conf_table.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @brief      这是一个数据库结果集的迭代器类和结果集类
*             你可以通过使用它对数据库的查询结果进行迭代和访问。
*
* @details  使用方法大致如下：
*           1. 首先，你需要定义一个结果集类型，指定你要提取的字段类型。
*           using db_res_set = dbt::template res_set<int, short, int, int, float, double, std::string, zce::ztm>;
* 		    2. 然后，你可以使用数据库查询结果来初始化这个结果集。
*           db_res_set  rs(std::move(mysql_result));
*  	        3. 最后，你可以使用范围基于的for循环，迭代器，来迭代结果集。
*           for (auto [f1, f2, f3, f4, f5, f6, f7, f8] : rs)
*
* @note     这玩意的使用也有一个禁忌，就是DB的数据别动，因为有的数据库的
*           结果集是和数据库连接绑定的，如果你在迭代过程中修改了数据库，
*           可能会导致迭代器失效。
*/

#pragma once

#include "zce/db/mysql/result.h"
#include "zce/db/pq/result.h"
#include "zce/db/sqlite/result.h"

namespace zce::db
{
template < typename Dbres, typename... Types > class result_set;

template <typename Dbres, typename... Types>
class result_set_iterator
{
public:
    using result_set_t = result_set<Dbres, Types...>;
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using size_type = size_t;
    using value_type = std::tuple<Types...>;
    using iterator_type = result_set_iterator<Dbres, Types...>;

    result_set_iterator() = default;
    result_set_iterator(Dbres* res, size_t row) :
        db_result_(res),
        iter_row_(row)
    {
    }
    result_set_iterator(const result_set_iterator&) = default;
    result_set_iterator& operator=(const result_set_iterator&) = default;
    //移动构造函数
    result_set_iterator(result_set_iterator&&) noexcept = default;
    result_set_iterator& operator=(result_set_iterator&&) noexcept = default;

    result_set_iterator& operator++()
    {
        if constexpr (std::is_same_v<Dbres, zce::sqlite::stmt_result>)
        {
            db_result_->cursor_next();
        }
        else
        {
            ++iter_row_;
        }
        return *this;
    }

    //! @brief 后置++操作符重载。不返回迭代器的原因是，某些数据库的结果集不支持
    void operator++(int)
    {
        ++(*this);
        return;
    }
    bool operator==(const result_set_iterator& other) const
    {
        if constexpr (std::is_same_v<Dbres, zce::sqlite::stmt_result>)
        {
            //iter_row_ == -1 表示end()的状态
            if (iter_row_ == -1 && (db_result_ == other.db_result_) && db_result_.is_end() == true)
            {
                return true;
            }
            else if (other.iter_row_ == -1 && db_result_.is_end() == true)
            {
                return true;
            }
            else
            {
                return ((iter_row_ == other.iter_row_) && (db_result_ == other.db_result_));
            }
        }
        else
        {
            return ((iter_row_ == other.iter_row_) && (db_result_ == other.db_result_));
        }
    }
    bool operator!=(const result_set_iterator& other) const
    {
        return !(*this == other);
    }

    ///提领操作，通过这个方法可以获取当前迭代器指向的结果集行数据。
    ///using db_res_set = dbt::template res_set<int, short, int, int, float, double, std::string, zce::ztm>;
    ///db_res_set  rs(std::move(mysql_result));
    ///for (auto [f1, f2, f3, f4, f5, f6, f7, f8] : rs)
    value_type operator*()
    {
        if constexpr (std::is_same_v<Dbres, zce::sqlite::stmt_result>)
        {
            return db_result_->make_tuple<Types...>();
        }
        else
        {
            return db_result_->make_tuple<Types...>(iter_row_);
        }
    }

    value_type* operator->() const
    {
        //
        return &(operator*());
    }
protected:
    //! @brief 指向数据库结果集的指针。
    Dbres* db_result_ = nullptr;

    size_t iter_row_ = 0;
};

template <typename Dbres, typename... Types>
class result_set
{
public:
    using result_set_t = result_set<Dbres, Types...>;
    using iterator = result_set_iterator<Dbres, Types...>;
    using value_type = std::tuple<Types...>;

    result_set() = default;
    result_set(Dbres&& res) : db_result_(std::move(res))
    {
    }
    result_set(const result_set&) = delete;
    result_set& operator=(const result_set&) = delete;
    //移动构造函数
    result_set(result_set&&) noexcept = default;
    result_set& operator=(result_set&&) noexcept = default;

    iterator begin()
    {
        return iterator(&db_result_, 0);
    }

    iterator end()
    {
        if constexpr (std::is_same_v<Dbres, zce::sqlite::stmt_result>)
        {
            return iterator(&db_result_, (size_t)-1);
        }
        else
        {
            return iterator(&db_result_, db_result_.num_of_rows());
        }
    }

protected:
    //
    Dbres db_result_;
};
}
#if defined ZCE_USE_MYSQL

#include "zce/db/mysql/result.h"

namespace zce::mysql
{
template <typename... Types>
using result_set = zce::db::result_set<zce::mysql::result, Types...>;
}
#endif

#if defined ZCE_USE_PQ

#include "zce/db/pq/result.h"

namespace zce::pq
{
template <typename... Types>
using result_set = zce::db::result_set<zce::pq::result, Types...>;
}

#endif

#if defined ZCE_USE_SQLITE

#include "zce/db/sqlite/result.h"

namespace zce::sqlite
{
template <typename... Types>
using result_set = zce::db::result_set<zce::sqlite::result, Types...>;

template <typename... Types>
using stmt_resset = zce::db::result_set<zce::sqlite::stmt_result, Types...>;
}

#endif
