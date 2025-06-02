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
* @note       
*/



#pragma once

#include "zce/db/mysql/result.h"
#include "zce/db/pq/result.h"

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
        sq_result_(res),
        num_result_row_(row)
    {
    }
    result_set_iterator(const result_set_iterator&) = default;
    result_set_iterator& operator=(const result_set_iterator&) = default;
    //移动构造函数
    result_set_iterator(result_set_iterator&&) noexcept = default;
    result_set_iterator& operator=(result_set_iterator&&) noexcept = default;

    result_set_iterator& operator++()
    {
        ++num_result_row_;
        return *this;
    }
    result_set_iterator operator++(int)
    {
        result_set_iterator tmp = *this;
        ++num_result_row_;
        return tmp;
    }
    bool operator==(const result_set_iterator& other) const
    {
        return ((num_result_row_ == other.num_result_row_) && (sq_result_ == other.sq_result_));
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
        return sq_result_->make_tuple<Types...>(num_result_row_);
    }

    value_type* operator->() const
    {
        //
        return &(operator*());
    }
protected:

    Dbres* sq_result_ = nullptr;

    size_t num_result_row_ = 0;
};

template <typename Dbres, typename... Types>
class result_set
{
public:
    using result_set_t = result_set<Dbres, Types...>;
    using iterator = result_set_iterator<Dbres, Types...>;
    using value_type = std::tuple<Types...>;

    result_set() = default;
    result_set(Dbres&& res) : sq_result_(std::move(res))
    {
    }
    result_set(const result_set&) = delete;
    result_set& operator=(const result_set&) = delete;
    //移动构造函数
    result_set(result_set&&) noexcept = default;
    result_set& operator=(result_set&&) noexcept = default;

    iterator begin()
    {
        return iterator(&sq_result_, 0);
    }

    iterator end()
    {
        return iterator(&sq_result_, sq_result_.num_of_rows());
    }

protected:
    //
    Dbres sq_result_;
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
