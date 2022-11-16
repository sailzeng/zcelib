#include "zce/predefine.h"
#include "zce/logger/logging.h"
#include "zce/os_adapt/define.h"
#include "zce/os_adapt/time.h"
#include "zce/os_adapt/error.h"
#include "zce/os_adapt/file.h"
#include "zce/util/scope_guard.h"

namespace zce
{
//我曾经很自以为是的认为ACE很土鳖，为什么不直接用open函数，然后用_get_osfhandle转换成HANDLE就可以了。
//关闭的时候用_open_osfhandle转换回来就OK了，但其实发现土鳖的是我，
//我完全错误理解了_open_osfhandle函数，这也可能解释了原来pascal原来遇到的问题close触发断言的问题。
//一切都不是RP问题，还是写错了代码。感谢derrickhu和sasukeliu两位，一个隐藏的比较深刻的bug
//_open_osfhandle其实是用生成一个int 描述符和HANDLE关联。
//为什么要提供这个API呢，因为WINDOWS平台大部分都是采用HANDLE处理的

ZCE_HANDLE open(const char* filename,
                int flags,
                mode_t mode)
{
    //Windows平台
#if defined (ZCE_OS_WINDOWS)

    //将各种LINUX的参数转换成Windows API的参数

    DWORD access = GENERIC_READ;

    if (flags & O_WRONLY)
    {
        //如果仅仅只能写
        access = GENERIC_WRITE;
    }
    else if (flags & O_RDWR)
    {
        access = GENERIC_READ | GENERIC_WRITE;
    }

    DWORD  creation = OPEN_EXISTING;

    if ((flags & O_CREAT) && (flags & O_EXCL))
    {
        creation = CREATE_NEW;
    }
    else if ((flags & O_CREAT) && (flags & O_TRUNC))
    {
        creation = CREATE_ALWAYS;
    }
    else if ((flags & O_CREAT))
    {
        creation = OPEN_ALWAYS;
    }
    else if ((flags & O_TRUNC))
    {
        creation = TRUNCATE_EXISTING;
    }

    DWORD shared_mode = 0;

    if ((mode & S_IRGRP)
        || (mode & S_IROTH)
        || (mode & S_IWUSR))
    {
        shared_mode |= FILE_SHARE_READ;
    }

    if ((mode & S_IWGRP)
        || (mode & S_IWOTH)
        || (mode & S_IWUSR))
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

    openfile_handle = ::CreateFileA(filename,
                                    access,
                                    shared_mode,
                                    nullptr,
                                    creation,
                                    FILE_ATTRIBUTE_NORMAL,
                                    0);

    //如果打开的文件句柄是无效的

    if (openfile_handle != ZCE_INVALID_HANDLE && (flags & O_APPEND))
    {
        LARGE_INTEGER distance_to_move, new_file_pointer;

        distance_to_move.QuadPart = 0;
        new_file_pointer.QuadPart = 0;
        auto bret = ::SetFilePointerEx(openfile_handle,
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
    return ::open(filename, flags, mode);
#endif
}

int open2(ZCE_HANDLE& handle,
          const char* filename,
          int flags,
          mode_t mode)
{
    handle = zce::open(filename,
                       flags,
                       mode);
    if (handle == ZCE_INVALID_HANDLE)
    {
        return -1;
    }
    return 0;
}

//关闭一个文件
int close(ZCE_HANDLE handle)
{
    //
#if defined (ZCE_OS_WINDOWS)
    auto bret = ::CloseHandle(handle);

    if (bret == TRUE)
    {
        return 0;
    }
    else
    {
        return -1;
    }

#elif defined (ZCE_OS_LINUX)
    return ::close(handle);
#endif
}

//读取文件
ssize_t read(ZCE_HANDLE file_handle,
             void* buf,
             size_t count) noexcept
{
    //WINDOWS下，长度无法突破32位的，参数限制了ReadFileEx也一样，大概WINDOWS认为没人这样读取文件
    //位置当然你是可以调整的
    //ReadFileEx只是能异步读取
#if defined (ZCE_OS_WINDOWS)
    DWORD ok_len;
    BOOL ret_bool = ::ReadFile(file_handle,
                               buf,
                               static_cast<DWORD> (count),
                               &ok_len,
                               nullptr);

    if (ret_bool)
    {
        return static_cast<ssize_t>(ok_len);
    }
    else
    {
        return -1;
    }

#elif defined (ZCE_OS_LINUX)
    return ::read(file_handle, buf, count);
#endif
}

//写文件，WINDOWS下，长度无法突破32位的,当然有人需要写入4G数据吗？
//Windows下尽量向POSIX 靠拢了
ssize_t write(ZCE_HANDLE file_handle,
              const void* buf,
              size_t count) noexcept
{
#if defined (ZCE_OS_WINDOWS)

    DWORD ok_len;
    auto ret_bool = ::WriteFile(file_handle,
                                buf,
                                static_cast<DWORD> (count),
                                &ok_len,
                                nullptr);

    if (ret_bool)
    {
        //注意Windows 下的write是有缓冲的，这个和Linux下的略有区别，
        //如果需要立即看到，可以用FlushFileBuffers,我暂时看不出一定要这样做的必要，
        //这个地方为了和POSIX统一，还是调用了这个函数
        //另外一个方法是在CreateFile 时增加属性 FILE_FLAG_NO_BUFFERING and
        // FILE_FLAG_WRITE_THROUGH
        ::FlushFileBuffers(file_handle);
        return (ssize_t)ok_len;
    }
    else
    {
        return -1;
    }

#elif defined (ZCE_OS_LINUX)
    return ::write(file_handle, buf, count);
#endif
}

// 读取文件，从起始文件的偏移量的文职开始读取
int read(ZCE_HANDLE file_handle,
         void* buf,
         size_t buf_count,
         size_t* read_count,
         off_t offset,
         int whence) noexcept
{
    if (whence != SEEK_CUR || offset != 0)
    {
        off_t off = zce::lseek(file_handle, offset, whence);
        if (off == -1)
        {
            return -1;
        }
    }
    *read_count = 0;
    auto result = read(file_handle,
                       buf,
                       buf_count);
    if (result < 0)
    {
        return -1;
    }
    *read_count = buf_count;
    return 0;
}

//写文件,从起始文件的偏移量的文职开始写入
int write(ZCE_HANDLE file_handle,
          const void* buf,
          size_t buf_count,
          size_t* write_count,
          off_t offset,
          int whence) noexcept
{
    *write_count = 0;
    if (whence != SEEK_CUR || offset != 0)
    {
        off_t off = zce::lseek(file_handle, offset, whence);
        if (off == -1)
        {
            return -1;
        }
    }
    auto result = zce::write(file_handle,
                             buf,
                             buf_count);
    if (result < 0)
    {
        return -1;
    }
    *write_count = buf_count;
    return 0;
}

//在文件内进行偏移
off_t lseek(ZCE_HANDLE file_handle,
            off_t offset,
            int whence) noexcept
{
#if defined (ZCE_OS_WINDOWS)

    //WINDOWS的lseek是不支持64位的，所以直接用API，完成工作，（后来有了_lseeki64）
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
    auto bret = ::SetFilePointerEx(file_handle,
                                   loff,
                                   &new_pos,
                                   dwmovemethod);

    if (bret == FALSE)
    {
        return  -1;
    }

    return static_cast<off_t>(new_pos.QuadPart);

#elif defined (ZCE_OS_LINUX)
    //
    return ::lseek(file_handle,
                   static_cast<off_t> (offset),
                   whence);
#endif
}

//在文件内进行偏移
int lseek(ZCE_HANDLE file_handle,
          off_t offset,
          int whence,
          off_t& result_off) noexcept
{
    result_off = 0;
    off_t off = zce::lseek(file_handle, offset, whence);
    if (off == -1)
    {
        return -1;
    }
    result_off = off;
    return 0;
}

//截断文件，倒霉的是WINDOWS下又TMD 没有，用BOOST的又非要遵守他的参数规范，我蛋疼
//其实可以变长，呵呵。
//注意这儿的fd是WIN32 API OpenFile得到的函数，不是你用ISO函数打开的那个fd，
int ftruncate(ZCE_HANDLE file_handle, size_t  offset)
{
    //Windows2000以前没有 SetFilePointerEx，我不是ACE，我不支持那么多屁事
#if defined (ZCE_OS_WINDOWS)

    LARGE_INTEGER loff;
    loff.QuadPart = offset;
    BOOL bret = ::SetFilePointerEx(file_handle,
                                   loff,
                                   0,
                                   FILE_BEGIN);

    if (bret == FALSE)
    {
        return  -1;
    }

    //linux ftruncate，后，吧指针放到了末尾
    bret = ::SetEndOfFile(file_handle);
    if (bret == FALSE)
    {
        return -1;
    }
    return 0;
    //
#elif defined (ZCE_OS_LINUX)
    return ::ftruncate(file_handle,
                       static_cast<off_t>(offset));
#endif
}

///截断文件,根据文件名称
int truncate(const char* filename,
             size_t offset)
{
#if defined (ZCE_OS_WINDOWS)
    auto_handle fh(zce::open(filename, (O_WRONLY)));
    if (ZCE_INVALID_HANDLE == fh.get())
    {
        return -1;
    }
    return ftruncate(fh.get(), offset);
#elif defined (ZCE_OS_LINUX)
    return truncate(filename, offset);
#endif
}

//根据文件名称，判断文件的尺寸,如果文件不存在，打不开等，返回-1
int filelen(const char* filename, size_t* file_size)
{
    auto_handle fh(zce::open(filename, (O_RDONLY)));
    if (ZCE_INVALID_HANDLE == fh.get())
    {
        return -1;
    }
    return zce::filesize(fh.get(), file_size);
}

int filesize(ZCE_HANDLE file_handle, size_t* file_size)
{
#if defined (ZCE_OS_WINDOWS)

    LARGE_INTEGER size;
    auto ret_bool = ::GetFileSizeEx(file_handle, &size);

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
    int ret = ::fstat(file_handle, &sb);

    if (ret != 0)
    {
        return ret;
    }

    *file_size = sb.st_size;
    return 0;
#endif
}

//用模版名称建立并且打开一个临时文件，
ZCE_HANDLE mkstemp(char* template_name)
{
#if defined (ZCE_OS_WINDOWS)
    char* tmp_filename = _mktemp(template_name);
    return zce::open(tmp_filename, ZCE_DEFAULT_FILE_PERMS);
#elif defined (ZCE_OS_LINUX)
    return ::mkstemp(template_name);
#endif
}

//通过文件名称得到文件的stat信息，你可以认为zce_os_stat就是stat，只是在WINDOWS下stat64,
//主要是为了长文件考虑的
int zce::stat(const char* path, struct stat* file_stat)
{
#if defined (ZCE_OS_WINDOWS)
    return ::stat(path, file_stat);
#elif defined (ZCE_OS_LINUX)
    return ::stat(path, file_stat);
#endif
}

//通过文件的句柄得到文件的stat信息
int fstat(ZCE_HANDLE file_handle, struct stat* file_stat)
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
    timeval tv_ct_time = zce::make_timeval(&file_info.ftCreationTime);
    timeval tv_ac_time = zce::make_timeval(&file_info.ftLastAccessTime);
    timeval tv_wt_time = zce::make_timeval(&file_info.ftLastWriteTime);

    LARGE_INTEGER file_size;
    file_size.HighPart = file_info.nFileSizeHigh;
    file_size.LowPart = file_info.nFileSizeLow;

    //_S_IFDIR,

    memset(file_stat, 0, sizeof(struct stat));
    file_stat->st_uid = 0;
    file_stat->st_gid = 0;
    file_stat->st_size = static_cast<_off_t>(file_size.QuadPart);

    //得到几个时间
    //注意st_ctime这儿呀，这儿的LINUX下和Windows是有些不一样的，
    //st_ctime在LINUX下是状态最后改变时间，而在WINDOWS下是创建时间
    file_stat->st_ctime = tv_ct_time.tv_sec;
    file_stat->st_mtime = tv_wt_time.tv_sec;
    file_stat->st_atime = tv_ac_time.tv_sec;

    //检查是文件还是目录
    file_stat->st_mode = 0;

    if (file_info.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE)
    {
        file_stat->st_mode = S_IFREG;

        if (file_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
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
bool zce::is_directory(const char* path_name)
{
    int ret = 0;
    struct stat file_stat;
    ret = zce::stat(path_name, &file_stat);
    if (0 != ret)
    {
        return false;
    }
    //如果有目录属性，则true
    if (file_stat.st_mode & S_IFDIR)
    {
        return true;
    }
    else
    {
        return false;
    }
}

//删除文件
int unlink(const char* filename)
{
#if defined (ZCE_OS_WINDOWS)
    return ::_unlink(filename);
#elif defined (ZCE_OS_LINUX)
    return ::unlink(filename);
#endif
}

//
mode_t umask(mode_t cmask)
{
#if defined (ZCE_OS_WINDOWS)
    return ::_umask(cmask);
#elif defined (ZCE_OS_LINUX)
    return ::umask(cmask);
#endif
}

//检查文件是否OK，吼吼
//mode 两个平台都支持F_OK,R_OK,W_OK,R_OK|W_OK，X_OK参数LINUX支持,WIN不支持
int access(const char* pathname, int mode)
{
#if defined (ZCE_OS_WINDOWS)
    return ::_access_s(pathname, mode);
#elif defined (ZCE_OS_LINUX)
    return ::access(pathname, mode);
#endif
}

//--------------------------------------------------------------------------------------------------
//非标准函数
//用只读方式读取一个文件的内容，
//返回的buffer最后没有，没有，没有填充'\0',buf_len >= 1
int read_file(const char* filename,
              char* buffer,
              size_t buf_len,
              size_t* read_len,
              size_t offset)
{
    //参数检查
    ZCE_ASSERT(filename && buffer && buf_len >= 1);
    *read_len = 0;
    //打开文件
    ZCE_HANDLE  fd = zce::open(filename, O_RDONLY);
    if (ZCE_INVALID_HANDLE == fd)
    {
        ZCE_LOG(RS_ERROR, "open file [%s]  fail ,error =%d", filename, zce::last_error());
        return -1;
    }
    zce::lseek(fd, static_cast<off_t>(offset), SEEK_SET);
    //读取内容
    ssize_t len = zce::read(fd, buffer, buf_len);
    zce::close(fd);
    if (len < 0)
    {
        ZCE_LOG(RS_ERROR, "read file [%s] fail ,error =%d",
                filename,
                zce::last_error());
        return -1;
    }

    *read_len = len;
    return 0;
}

//读取文件的全部数据，
std::pair<int, std::shared_ptr<char> > read_file(const char* filename,
                                                 size_t* file_len,
                                                 off_t offset)
{
    int ret = -1;
    std::shared_ptr<char> null_ptr;
    //打开文件
    zce::auto_handle fd(zce::open(filename, O_RDONLY));
    if (ZCE_INVALID_HANDLE == fd.get())
    {
        ZCE_LOG(RS_ERROR, "read_file open file [%s]  fail ,error =%d", filename, zce::last_error());
        return std::make_pair(ret, null_ptr);
    }
    *file_len = zce::lseek(fd.get(), 0, SEEK_END);
    if (static_cast<size_t>(-1) == *file_len)
    {
        ZCE_LOG(RS_ERROR, "read_file open file [%s]  fail ,error =%d",
                filename, zce::last_error());
        return std::make_pair(ret, null_ptr);
    }
    std::shared_ptr<char> ptr(new char[*file_len + 1], std::default_delete<char[]>());
    *(ptr.get() + *file_len) = '\0';
    //调整偏移，读取内容
    if (offset)
    {
        off_t oft = zce::lseek(fd.get(), static_cast<off_t>(offset), SEEK_SET);
        if (oft < 0)
        {
            ZCE_LOG(RS_ERROR, "read_file lseek file [%s]  fail ,error =%d",
                    filename, zce::last_error());
            return std::make_pair(-1, null_ptr);
        }
    }

    ssize_t len = zce::read(fd.get(), ptr.get(), *file_len);
    if (len < 0)
    {
        ZCE_LOG(RS_ERROR, "read_file read file [%s] fail ,error =%d",
                filename,
                zce::last_error());
        return std::make_pair(ret, null_ptr);
    }

    ret = 0;
    return std::make_pair(ret, ptr);
}

int write_file(const char* filename,
               const char* buff,
               size_t buf_len,
               size_t* write_len,
               off_t offset)
{
    //参数检查
    ZCE_ASSERT(filename && buff && buf_len >= 1);
    *write_len = 0;
    //打开文件
    zce::auto_handle  fd(zce::open(filename, O_CREAT | O_WRONLY));
    if (ZCE_INVALID_HANDLE == fd.get())
    {
        ZCE_LOG(RS_ERROR, "write_file open file [%s]  fail ,error =%d",
                filename, zce::last_error());
        return -1;
    }
    if (offset)
    {
        off_t oft = zce::lseek(fd.get(), static_cast<off_t>(offset), SEEK_SET);
        if (oft < 0)
        {
            ZCE_LOG(RS_ERROR, "write_file lseek file [%s]  fail ,error =%d",
                    filename, zce::last_error());
            return -1;
        }
    }

    //读取内容
    ssize_t len = zce::write(fd.get(), buff, buf_len);
    if (len < 0)
    {
        ZCE_LOG(RS_ERROR, "write_file write file [%s] fail ,error =%d",
                filename, zce::last_error());
        return -1;
    }

    *write_len = len;
    return 0;
}
}