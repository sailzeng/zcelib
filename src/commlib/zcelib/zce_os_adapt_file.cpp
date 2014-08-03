
#include "zce_predefine.h"
#include "zce_trace_log_debug.h"
#include "zce_os_adapt_predefine.h"
#include "zce_os_adapt_time.h"
#include "zce_os_adapt_error.h"
#include "zce_os_adapt_file.h"

//读取文件
ssize_t ZCE_OS::read(ZCE_HANDLE file_handle, void *buf, size_t count)
{
    //WINDOWS下，长度无法突破32位的，参数限制了ReadFileEx也一样，大概WINDOWS认为没人这样读取文件
#if defined (ZCE_OS_WINDOWS)
    DWORD ok_len;
    BOOL ret_bool = ::ReadFile (file_handle,
                                buf,
                                static_cast<DWORD> (count),
                                &ok_len,
                                NULL);

    if (ret_bool)
    {
        return (ssize_t) ok_len;
    }
    else
    {
        return -1;
    }

#elif defined (ZCE_OS_LINUX)
    return ::read (file_handle, buf, count);
#endif
}

//写如文件，WINDOWS下，长度无法突破32位的,当然有人需要写入4G数据吗？
ssize_t ZCE_OS::write(ZCE_HANDLE file_handle, const void *buf, size_t count)
{
#if defined (ZCE_OS_WINDOWS)
    DWORD ok_len;
    BOOL ret_bool = ::WriteFile (file_handle,
                                 buf,
                                 static_cast<DWORD> (count),
                                 &ok_len,
                                 NULL);

    if (ret_bool)
    {
        return (ssize_t) ok_len;
    }
    else
    {
        return -1;
    }

#elif defined (ZCE_OS_LINUX)
    return ::write (file_handle, buf, count);
#endif
}

//截断文件
int ZCE_OS::truncate(const char *filename, size_t offset)
{
#if defined (ZCE_OS_WINDOWS)

    int ret = 0;
    //打开文件，并且截断，最后关闭
    ZCE_HANDLE file_handle = ZCE_OS::open(filename, (O_CREAT | O_RDWR));

    if ( ZCE_INVALID_HANDLE == file_handle)
    {
        return -1;
    }

    ret = ZCE_OS::ftruncate(file_handle, offset);

    if (0 != ret )
    {
        return ret;
    }

    ZCE_OS::close(file_handle);
    return 0;
#endif

#if defined (ZCE_OS_LINUX)
    return ::truncate (filename, static_cast<off_t>(offset));
#endif
}

//截断文件，倒霉的是WINDOWS下又TMD 没有，用BOOST的又非要遵守他的参数规范，我蛋疼
//其实可以变长，呵呵。
//注意这儿的fd是WIN32 API OpenFile得到的函数，不是你用ISO函数打开的那个fd，
int ZCE_OS::ftruncate(ZCE_HANDLE file_handle, size_t  offset)
{
    //Windows2000以前没有 SetFilePointerEx，我不是ACE，我不支持那么多屁事
#if defined (ZCE_OS_WINDOWS)

    LARGE_INTEGER loff;
    loff.QuadPart = offset;
    BOOL bret = ::SetFilePointerEx (file_handle,
                                    loff,
                                    0,
                                    FILE_BEGIN);

    if (bret == FALSE)
    {
        return  -1;
    }

    //linux ftruncate，后，吧指针放到了末尾
    bret = ::SetEndOfFile (file_handle);

    if (bret == FALSE)
    {
        return -1;
    }

    return 0;
    //
#elif defined (ZCE_OS_LINUX)
    return ::ftruncate (file_handle, static_cast<off_t>(offset));
#endif

}

//在文件内进行偏移
ssize_t ZCE_OS::lseek(ZCE_HANDLE file_handle, ssize_t offset, int whence)
{
#if defined (ZCE_OS_WINDOWS)

    //WINDOWS的lseek是不支持64位的，所以直接用API，完成工作，
    DWORD dwmovemethod = FILE_BEGIN;

    if (whence == SEEK_SET)
    {
        dwmovemethod = FILE_BEGIN;
    }
    else if (whence == SEEK_CUR)
    {
        dwmovemethod = FILE_CURRENT;
    }
    else if (whence == SEEK_END)
    {
        dwmovemethod = FILE_END;
    }
    else
    {
        assert(false);
    }

    LARGE_INTEGER loff;
    loff.QuadPart = offset;

    LARGE_INTEGER new_pos;
    BOOL bret = ::SetFilePointerEx (file_handle,
                                    loff,
                                    &new_pos,
                                    dwmovemethod);

    if (bret == FALSE)
    {
        return  -1;
    }

    return static_cast<ssize_t>(new_pos.QuadPart);

#elif defined (ZCE_OS_LINUX)
    //
    return ::lseek(file_handle,
                   static_cast<off_t> (offset),
                   whence);
#endif

}

//根据文件名称，判断文件的尺寸,如果文件不存在，打不开等，返回-1
int ZCE_OS::filelen(const char *filename, size_t *file_size)
{
    int ret = 0;
    ZCE_HANDLE file_handle = ZCE_OS::open(filename, (O_RDONLY));

    if ( ZCE_INVALID_HANDLE == file_handle)
    {
        return -1;
    }

    ret = ZCE_OS::filesize (file_handle, file_size);
    ZCE_OS::close (file_handle);
    return ret;
}

int ZCE_OS::filesize (ZCE_HANDLE file_handle, size_t *file_size)
{
#if defined (ZCE_OS_WINDOWS)

    LARGE_INTEGER size;
    BOOL ret_bool = ::GetFileSizeEx (file_handle, &size);

    if (!ret_bool)
    {
        return -1;
    }

    //32位平台上可能丢长度，但是我就考虑64位系统，你才会突破4G把
    *file_size = static_cast<size_t> (size.QuadPart);
    return 0;
    //
#elif defined (ZCE_OS_LINUX)
    struct stat sb;
    int ret =  ::fstat(file_handle, &sb);

    if (ret != 0 )
    {
        return ret;
    }

    *file_size = sb.st_size;

    return 0;
#endif
}

//我曾经很自以为是的认为ACE很土鳖，为什么不直接用open函数，然后用_get_osfhandle转换成HANDLE就可以了。
//关闭的时候用_open_osfhandle转换回来就OK了，但其实发现土鳖的是我，
//我完全错误理解了_open_osfhandle函数，这也可能解释了原来pascal原来遇到的问题close触发断言的问题。
//一切都不是RP问题，还是写错了代码。感谢derrickhu和sasukeliu两位，一个隐藏的比较深刻的bug

//为什么要提供这个API呢，因为WINDOWS平台大部分都是采用HANDLE处理的

ZCE_HANDLE ZCE_OS::open (const char *filename,
                         int open_mode,
                         mode_t perms)
{
    //Windows平台
#if defined (ZCE_OS_WINDOWS)

    //将各种LINUX的参数转换成Windows API的参数

    DWORD access = GENERIC_READ;

    if (ZCE_BIT_IS_SET (open_mode, O_WRONLY))
    {
        //如果仅仅只能写
        access = GENERIC_WRITE;
    }
    else if (ZCE_BIT_IS_SET (open_mode, O_RDWR))
    {
        access = GENERIC_READ | GENERIC_WRITE;
    }

    DWORD  creation = OPEN_EXISTING;

    if ( ZCE_BIT_IS_SET (open_mode, O_CREAT) && ZCE_BIT_IS_SET (open_mode, O_EXCL))
    {
        creation = CREATE_NEW;
    }
    else if ( ZCE_BIT_IS_SET (open_mode, O_CREAT) && ZCE_BIT_IS_SET (open_mode, O_TRUNC) )
    {
        creation = CREATE_ALWAYS;
    }
    else if (ZCE_BIT_IS_SET (open_mode, O_CREAT))
    {
        creation = OPEN_ALWAYS;
    }
    else if (ZCE_BIT_IS_SET (open_mode, O_TRUNC))
    {
        creation = TRUNCATE_EXISTING;
    }

    DWORD shared_mode = 0;

    if ( ZCE_BIT_IS_SET(perms, S_IRGRP)
         || ZCE_BIT_IS_SET(perms, S_IROTH)
         || ZCE_BIT_IS_SET(perms, S_IWUSR))
    {
        shared_mode |= FILE_SHARE_READ;
    }

    if ( ZCE_BIT_IS_SET(perms, S_IWGRP)
         || ZCE_BIT_IS_SET(perms, S_IWOTH)
         || ZCE_BIT_IS_SET(perms, S_IWUSR))
    {
        shared_mode |= FILE_SHARE_WRITE;
        shared_mode |= FILE_SHARE_DELETE;
    }

    ZCE_HANDLE openfile_handle = ZCE_INVALID_HANDLE;

    //ACE的代码在这段用一个多线程的互斥保护，
    //因为CreateFileA并不能同时将文件的指针移动到末尾，所以(O_APPEND)这是一个两步操作(先CreateFileA，后SetFilePointerEx)，
    //ACE担心有特殊情况？多线程创建还是？他的没有注释说明这个问题,我暂时不去做保护，

    //CRITICAL_SECTION fileopen_mutex;
    ////VISTAT后没有这个异常了
    //__try
    //{
    //    ::InitializeCriticalSection (&fileopen_mutex);
    //}
    //__except (EXCEPTION_EXECUTE_HANDLER)
    //{
    //    errno = ENOMEM;
    //    return ZCE_INVALID_HANDLE;
    //}

    openfile_handle = ::CreateFileA (filename,
                                     access,
                                     shared_mode,
                                     NULL,
                                     creation,
                                     FILE_ATTRIBUTE_NORMAL,
                                     0);

    //如果打开的文件句柄是无效的

    if (openfile_handle != ZCE_INVALID_HANDLE  && ZCE_BIT_IS_SET (open_mode, O_APPEND))
    {
        LARGE_INTEGER distance_to_move, new_file_pointer;

        distance_to_move.QuadPart = 0;
        new_file_pointer.QuadPart = 0;
        BOOL bret = ::SetFilePointerEx (openfile_handle,
                                        distance_to_move,
                                        &new_file_pointer,
                                        FILE_END);

        if (FALSE == bret)
        {
            ::CloseHandle(openfile_handle);
            openfile_handle = ZCE_INVALID_HANDLE;
        }
    }

    //对应上面的临界区保护
    //::DeleteCriticalSection (&fileopen_mutex);

    return openfile_handle;

#elif defined (ZCE_OS_LINUX)
    return ::open (filename, open_mode, perms);
#endif
}

//关闭一个文件
int ZCE_OS::close (ZCE_HANDLE handle)
{
    //
#if defined (ZCE_OS_WINDOWS)
    BOOL bret = ::CloseHandle(handle);

    if (bret == TRUE)
    {
        return 0;
    }
    else
    {
        return -1;
    }

#elif defined (ZCE_OS_LINUX)
    return ::close (handle);
#endif
}

//用模版名称建立并且打开一个临时文件，
ZCE_HANDLE ZCE_OS::mkstemp(char *template_name)
{
#if defined (ZCE_OS_WINDOWS)
    char *tmp_filename = _mktemp(template_name);
    return ZCE_OS::open(tmp_filename, ZCE_DEFAULT_FILE_PERMS);
#elif defined (ZCE_OS_LINUX)
    return ::mkstemp(template_name);
#endif
}

//通过文件名称得到文件的stat信息，你可以认为zce_os_stat就是stat，只是在WINDOWS下stat64,主要是为了长文件考虑的
int ZCE_OS::stat(const char *path, zce_os_stat *file_stat)
{
#if defined (ZCE_OS_WINDOWS)
    return ::_stat64(path, file_stat);
#elif defined (ZCE_OS_LINUX)
    return ::stat(path, file_stat);
#endif
}

//通过文件的句柄得到文件的stat信息
int ZCE_OS::fstat(ZCE_HANDLE file_handle, zce_os_stat *file_stat)
{
#if defined (ZCE_OS_WINDOWS)

    //这个实现比较痛苦，但也没有办法，其他方法（比如用_open_osfhandle）都会偷鸡不成，反舍一把米

    BOOL ret_bool = FALSE;
    BY_HANDLE_FILE_INFORMATION file_info;
    ret_bool = ::GetFileInformationByHandle(file_handle,
                                            &file_info);

    if (!ret_bool)
    {
        return -1;
    }

    //转换时间
    timeval tv_ct_time = ZCE_OS::make_timeval(&file_info.ftCreationTime);
    timeval tv_ac_time = ZCE_OS::make_timeval(&file_info.ftLastAccessTime);
    timeval tv_wt_time = ZCE_OS::make_timeval(&file_info.ftLastWriteTime);

    LARGE_INTEGER file_size;
    file_size.HighPart = file_info.nFileSizeHigh;
    file_size.LowPart = file_info.nFileSizeLow;

    //_S_IFDIR,

    memset(file_stat, 0, sizeof(zce_os_stat));
    file_stat->st_uid = 0;
    file_stat->st_gid = 0;
    file_stat->st_size = file_size.QuadPart;

    //得到几个时间
    //注意st_ctime这儿呀，这儿的LINUX下和Windows是有些不一样的，st_ctime在LINUX下是状态最后改变时间，而在WINDOWS下是创建时间
    file_stat->st_ctime = tv_ct_time.tv_sec;
    file_stat->st_mtime = tv_wt_time.tv_sec;
    file_stat->st_atime = tv_ac_time.tv_sec;

    //检查是文件还是目录
    file_stat->st_mode = 0;

    if (file_info.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE )
    {
        file_stat->st_mode = S_IFREG;

        if (file_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
        {
            file_stat->st_mode = S_IFDIR;
        }
    }

    return 0;

#elif defined (ZCE_OS_LINUX)
    return ::fstat(file_handle, file_stat);
#endif
}



//路径是否是一个目录，如果是返回TRUE，如果不是返回FALSE
bool ZCE_OS::is_directory(const char *path_name)
{
    int ret = 0;
    zce_os_stat file_stat;
    ret = ZCE_OS::stat(path_name, &file_stat);
    if (0 != ret)
    {
        return false;
    }
    //如果有目录属性，则true
    if (file_stat.st_mode & S_IFDIR )
    {
        return true;
    }
    else
    {
        return false;
    }
}


//删除文件
int ZCE_OS::unlink(const char *filename )
{
#if defined (ZCE_OS_WINDOWS)
    return ::_unlink(filename);
#elif defined (ZCE_OS_LINUX)
    return ::unlink(filename);
#endif
}

//
mode_t ZCE_OS::umask (mode_t cmask)
{
#if defined (ZCE_OS_WINDOWS)
    return ::_umask(cmask);
#elif defined (ZCE_OS_LINUX)
    return ::umask(cmask);
#endif
}

//检查文件是否OK，吼吼
//mode 两个平台都支持F_OK,R_OK,W_OK,R_OK|W_OK，X_OK参数LINUX支持,WIN不支持
int ZCE_OS::access(const char *pathname, int mode)
{
#if defined (ZCE_OS_WINDOWS)
    return ::_access_s(pathname, mode);
#elif defined (ZCE_OS_LINUX)
    return ::access(pathname, mode);
#endif
}

//--------------------------------------------------------------------------------------------------
//非标准函数
//用只读方式读取一个文件的内容，返回的buffer最后填充'\0',buf_len >= 2
int ZCE_OS::read_file_data(const char *filename, char *buffer, size_t buf_len, size_t *read_len)
{
    //参数检查
    ZCE_ASSERT(filename && buffer && buf_len >= 2);

    //打开文件
    ZCE_HANDLE  fd = ZCE_OS::open(filename, O_RDONLY);

    if (ZCE_INVALID_HANDLE == fd)
    {
        ZLOG_ERROR("open file [%s]  fail ,error =%d", filename, ZCE_OS::last_error());
        return -1;
    }

    //读取内容
    ssize_t len = ZCE_OS::read(fd, buffer, buf_len - 1);
    ZCE_OS::close(fd);

    if (len < 0)
    {
        ZLOG_ERROR("read file [%s] fail ,error =%d", filename, ZCE_OS::last_error());
        return -1;
    }

    buffer[len] = 0;
    *read_len = len;

    return 0;
}

