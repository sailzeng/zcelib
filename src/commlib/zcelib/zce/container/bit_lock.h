#pragma once

namespace zce
{
template< std::size_t N >
class bit_lock
{
    bit_lock();
    ~bit_lock();

    int open()
    {
        bit_lock_ = new std::atom<char>[N / 8 + 1];
        return 0;
    }

    bool operator[](std::size_t pos);

    bool test(std::size_t pos);

protected:

    std::atom<char> *bit_lock_ = nullptr;
};
}