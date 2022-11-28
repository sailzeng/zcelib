#include "zce/predefine.h"
#include "zce/lock/bit_lock.h"

namespace zce
{
const unsigned char BIT_FIELD[8] =
{
    0x01,
    0x02,
    0x04,
    0x08,
    0x10,
    0x20,
    0x40,
    0x80,
};
//!
int bit_lock::open(std::size_t bits_size)
{
    bit_lock_ = new std::atomic_char[bits_size / 8 + 8];
    return 0;
}

//!
int bit_lock::open(std::size_t bits_size,
                   const char* file_name,
                   bool if_restore)
{
    int ret = 0;
    size_t room_size = (bits_size / 8 + 8) * sizeof(std::atomic_char);
    ret = shm_mmap_.open(file_name, room_size, if_restore);
    if (ret != 0)
    {
        return ret;
    }
    if (if_restore)
    {
        bit_lock_ = (std::atomic_char*)shm_mmap_.addr();
    }
    else
    {
        bit_lock_ = new (shm_mmap_.addr())std::atomic_char[room_size];
    }

    return 0;
}

bool bit_lock::test(std::size_t pos)
{
    std::size_t bit_pos = pos / 8;
    std::size_t bit_offset = pos % 8;
    return bit_lock_[bit_pos] & BIT_FIELD[bit_offset];
}

//
bool bit_lock::try_lock(std::size_t pos)
{
    std::size_t bit_pos = pos / 8;
    std::size_t bit_offset = pos % 8;
    char old_char = bit_lock_[bit_pos];
    if (old_char & BIT_FIELD[bit_offset])
    {
        return false;
    }
    char new_char = old_char | BIT_FIELD[bit_offset];
    return  bit_lock_[bit_pos].compare_exchange_strong(old_char, new_char);
}

//
bool bit_lock::lock(std::size_t pos)
{
    std::size_t bit_pos = pos / 8;
    std::size_t bit_offset = pos % 8;
    bool bret = false;
    while (true)
    {
        char old_char = bit_lock_[bit_pos];
        if (old_char & BIT_FIELD[bit_offset])
        {
            continue;
        }
        char new_char = old_char | BIT_FIELD[bit_offset];
        bret = bit_lock_[bit_pos].compare_exchange_strong(old_char, new_char);
        if (bret)
        {
            break;
        }
    }
    return true;
}
//
bool bit_lock::unlock(std::size_t pos)
{
    std::size_t bit_pos = pos / 8;
    std::size_t bit_offset = pos % 8;
    bool bret = false;
    while (true)
    {
        char old_char = bit_lock_[bit_pos];
        if (!(old_char | BIT_FIELD[bit_offset]))
        {
            break;
        }
        char new_char = old_char & (~BIT_FIELD[bit_offset]);
        bret = bit_lock_[bit_pos].compare_exchange_strong(old_char,
                                                          new_char);
        if (bret)
        {
            break;
        }
    }
    return true;
}
}