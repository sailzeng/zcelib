#pragma once

namespace zce
{
/*!
* @brief      SPIN自旋锁，线程互斥锁，
*/
class spin_lock
{
public:

    spin_lock() = default;
    ~spin_lock() = default;

    spin_lock(const spin_lock&) = delete;
    spin_lock& operator=(const spin_lock &) = delete;

    bool lock()
    {
        // acquire spin lock
        while (flag_.test_and_set())
        {
        }
        return true;
    }
    bool try_lock()
    {
        // acquire spin lock
        return (flag_.test_and_set());
    }
    void unlock()
    {
        // release spin lock
        flag_.clear();
    }
private:
    //!atomic 的标志，有atomic的功能
    std::atomic_flag flag_;
};
}