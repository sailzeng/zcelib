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
    ///提领操作
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
