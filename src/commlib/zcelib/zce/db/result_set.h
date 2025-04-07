#pragma once

#include "zce/db/mysql/result.h"
#include "zce/db/pq/result.h"

namespace zce::db
{
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
    result_set_iterator(result_set_t* res, size_t row) :
        result_(res),
        row_(row)
    {
    }
    iterator_type(const iterator_type&) = default;
    iterator_type& operator=(const result_set_iterator&) = default;
    //移动构造函数
    iterator_type(iterator_type&&) noexcept = default;
    iterator_type& operator=(iterator_type&&) noexcept = default;

    iterator_type& operator++()
    {
        ++row_;
        return *this;
    }
    iterator_type operator++(int)
    {
        result_set_iterator tmp = *this;
        ++row_;
        return tmp;
    }
    bool operator==(const iterator_type& other) const
    {
        return ((row_ == other.row_) && (result_ == other.result_));
    }
    bool operator!=(const iterator_type& other) const
    {
        return !(*this == other);
    }
    ///提领操作
    value_type operator*() const
    {
        return result_->make_tuple(row_);
    }

protected:

    result_set_t* result_ = nullptr;

    size_t row_ = 0;
};

template <typename Dbres, typename... Types>
class result_set
{
public:
    using result_set_t = result_set<Dbres, Types...>;
    using iterator = result_set<Dbres, Types...>;
    using value_type = std::tuple<Types...>;

    result_set() = default;
    result_set(Dbres&& res) : result_(std::move(res))
    {
    }
    result_set(const result_set&) = delete;
    result_set& operator=(const result_set&) = delete;
    //移动构造函数
    result_set(result_set&&) noexcept = default;
    result_set& operator=(result_set&&) noexcept = default;

    iterator begin()
    {
        return iterator(this, 0);
    }

    iterator end()
    {
        return iterator(this, result_.num_of_rows());
    }

protected:
    //
    Dbres result_;
};
}
