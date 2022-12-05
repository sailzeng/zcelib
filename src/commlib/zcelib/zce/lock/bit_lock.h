/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/event/handle_base.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2022年11月29日
* @brief      bit lock ，用于大量的spin lock的地方。
*
* @details
*
* @note
*
*/
#pragma once

#include "zce/shared_mem/mmap.h"

namespace zce
{
class bit_lock
{
public:

    bit_lock() = default;
    ~bit_lock() = default;

    //!
    int open(std::size_t bits_size);
    //!
    int open(std::size_t bits_size,
             const char* file_name,
             bool if_restore);

    bool test(std::size_t pos);

    //
    bool lock(std::size_t pos);

    //
    bool unlock(std::size_t pos);

    //
    bool try_lock(std::size_t pos);

protected:
    //!
    std::size_t bits_size_ = 0;
    //!
    zce::shm_mmap shm_mmap_;
    //!
    std::atomic_char *bit_lock_ = nullptr;
};
}