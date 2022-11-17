#pragma once

#include "zce/os_adapt/file.h"
#include "zce/os_adapt/socket.h"
#include "zce/os_adapt/dirent.h"

namespace zce
{
template <class Exister>
class scope_guard
{
public:
    scope_guard() = delete;
    scope_guard(const scope_guard& anther) = delete;

    inline explicit scope_guard(Exister&& exiter) :
        exiter_(std::move(exiter)),
        dismissed_(false)

    {
    }
    inline explicit scope_guard(const Exister& exiter) :
        exiter_(exiter),
        dismissed_(false)
    {
    }

    inline explicit scope_guard(scope_guard&& anther) :
        exiter_(std::move(anther.exiter_)),
        dismissed_(anther.dismissed_)
    {
    }

    inline ~scope_guard()
    {
        if (!dismissed_)
        {
            exiter_();
        }
    }

    inline void dismiss()
    {
        dismissed_ = true;
    }
private:

    Exister exiter_;
    bool dismissed_;
};

//范围guard类，
//!写到后面，发现重载的数量
template <class T, class Exister>
class scope_guard_t
{
public:
    scope_guard_t() = delete;
    scope_guard_t(const scope_guard_t& another) = delete;

    inline scope_guard_t(T&& value,
                         Exister &&exister) :
        value_(std::move(value)),
        exister_(std::move(exister)),
        dismissed_(false)
    {
    }

    inline scope_guard_t(const T& value,
                         const Exister &exister) :
        value_(value),
        exister_(exister),
        dismissed_(false)
    {
    }

    inline scope_guard_t(scope_guard_t&& another) :
        value_(std::move(another.value_)),
        exister_(std::move(another.exister_)),
        dismissed_(another.dismissed_)
    {
    }

    inline scope_guard_t(ZCE_HANDLE value,
                         decltype(zce::close)* exister = zce::close) :
        value_(value),
        exister_(exister),
        dismissed_(false)
    {
    }

    inline scope_guard_t(ZCE_SOCKET value,
                         decltype(zce::close_socket)* exister = zce::close_socket) :
        value_(value),
        exister_(exister),
        dismissed_(false)
    {
    }

    inline scope_guard_t(std::FILE* value,
                         decltype(::fclose)* exister = std::fclose) :
        value_(value),
        exister_(exister),
        dismissed_(false)
    {
    }

    inline scope_guard_t(DIR* value,
                         decltype(zce::closedir)* exister = zce::closedir) :
        value_(value),
        exister_(exister),
        dismissed_(false)
    {
    }

    //不能写::FILE *,FILE *,冲突。

    inline ~scope_guard_t() noexcept
    {
        if (!dismissed_)
        {
            exister_(value_);
        }
    }

    inline void reset(const T& value) noexcept
    {
        if (!dismissed_)
        {
            exister_(value_);
        }
        value_ = value;
    }

    T get()
    {
        return value_;
    }

    inline void dismiss()  noexcept
    {
        dismissed_ = true;
    }
private:
    T value_;
    Exister exister_;
    bool dismissed_;
};

//!辅助类，利用析构关闭ZCE_HANDLE,ZCE_HANDLE 不能用unique_ptr

//! 自动释放的ZCE_HANDLE
//! 这样使用 zce::auto_file fd(::fopen("xxx"));
using auto_handle =
scope_guard_t<ZCE_HANDLE, decltype(zce::close)*>;

//! 辅助SOCKET自动关闭类 自动释放SOCKET指针
using auto_socket =
scope_guard_t<ZCE_SOCKET, decltype(zce::close_socket)*>;

//FILE* 用unique_ptr也能实现
using auto_file =
scope_guard_t<FILE*, decltype(::fclose)*>;

using auto_stdfile =
scope_guard_t<std::FILE *, decltype(std::fclose)*>;

using auto_dir =
scope_guard_t<DIR *, decltype(zce::closedir)*>;

//! 下面使用unique_ptr怎么写的教程，
//! 这样使用 zce::auto_file fd(::fopen("xxx"),::fclose);,::fclose必须写
//using auto_file =
//std::unique_ptr<FILE, decltype(::fclose)*>;
//using auto_stdfile =
//std::unique_ptr<std::FILE, decltype(std::fclose)*>;
}
