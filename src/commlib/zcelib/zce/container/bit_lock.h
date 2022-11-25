#pragma once

namespace zce
{
template< std::size_t N >
class bit_lock
{
    bit_lock();
    ~bit_lock();

    bool operator[](std::size_t pos);

    bool test(std::size_t pos);

    std::atom<char> *test_;
};
}