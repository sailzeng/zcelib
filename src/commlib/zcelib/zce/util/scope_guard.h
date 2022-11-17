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
template <class T>
class auto_close
{
public:
    auto_close() = delete;
    auto_close(const auto_close& another) = delete;

    inline auto_close(T&& value) :
        value_(std::move(value)),
        dismissed_(false)
    {
    }

    inline auto_close(const T& value) :
        value_(value),
        dismissed_(false)
    {
    }

    inline auto_close(auto_close&& another) :
        value_(std::move(another.value_)),
        dismissed_(another.dismissed_)
    {
        another.dismissed_ = false;
    }

    inline ~auto_close() noexcept
    {
        if (!dismissed_)
        {
            exister(value_);
        }
    }

    inline void reset(const T& value) noexcept
    {
        if (!dismissed_)
        {
            exister(value_);
        }
        value_ = value;
    }

    T get()
    {
        return value_;
    }

    void exister(ZCE_HANDLE value)
    {
        if (value != ZCE_INVALID_HANDLE)
        {
            zce::close(value);
        }
    }
    void exister(ZCE_SOCKET value)
    {
        if (value != ZCE_INVALID_SOCKET)
        {
            zce::close_socket(value);
        }
    }
    void exister(FILE* value)
    {
        if (value)
        {
            ::fcloset(value);
        }
    }
    //不能写std::FILE *,FILE *,冲突。
    void exister(DIR* value)
    {
        if (value)
        {
            zce::closedir(value);
        }
    }
    inline void dismiss()  noexcept
    {
        dismissed_ = true;
    }
private:
    T value_;
    bool dismissed_;
};

//!辅助类，利用析构关闭ZCE_HANDLE,ZCE_HANDLE 不能用unique_ptr

//! 自动释放的ZCE_HANDLE
//! 这样使用 zce::auto_file fd(::fopen("xxx"));
using auto_handle = auto_close<ZCE_HANDLE>;

//! 辅助SOCKET自动关闭类 自动释放SOCKET指针
using auto_socket = auto_close<ZCE_SOCKET>;

//FILE* 用unique_ptr也能实现
using auto_file = auto_close<FILE*>;

using auto_stdfile = auto_close<std::FILE *>;

using auto_dir = auto_close<DIR *>;

//! 下面使用unique_ptr怎么写的教程，
//! 这样使用 zce::auto_file fd(::fopen("xxx"),::fclose);,::fclose必须写
//using auto_file =
//std::unique_ptr<FILE, decltype(::fclose)*>;
//using auto_stdfile =
//std::unique_ptr<std::FILE, decltype(std::fclose)*>;
}
