
#include "zce_predefine.h"

#include "zce_os_adapt_error.h"
#include "zce_os_adapt_file.h"
#include "zce_os_adapt_dirent.h"
#include "zce_os_adapt_shm.h"


//
void *ZCE_OS::mmap (void *addr,
                    size_t len,
                    int prot,
                    int flags,
                    ZCE_HANDLE file_handle,
                    size_t off)
{

#if defined (ZCE_OS_WINDOWS)

    if (ZCE_BIT_ISNOT_SET (flags, MAP_FIXED))
    {
        addr = 0;
    }

    // can not map to address 0
    if (ZCE_BIT_IS_SET (flags, MAP_FIXED) && 0 == addr )
    {
        errno = ENOTSUP;
        return MAP_FAILED;
    }

    DWORD nt_flag_protect = 0;
    DWORD  nt_flags = 0;

    //这段在干嘛，就是将mmap的prot参数转换成微软的的参数，TNND，
    if ( PROT_NONE == prot )
    {
        nt_flag_protect = PAGE_NOACCESS;
    }
    else if (PROT_READ == prot)
    {
        nt_flag_protect = PAGE_READONLY;
        nt_flags = FILE_MAP_READ;
    }
    else if ( ZCE_BIT_IS_SET(prot, PROT_READ ) && ZCE_BIT_IS_SET(prot, PROT_WRITE) )
    {
        nt_flag_protect = PAGE_READWRITE;
        nt_flags = FILE_MAP_WRITE;
    }
    else if (PROT_EXEC == prot)
    {
        nt_flag_protect = PAGE_EXECUTE;
        nt_flags = FILE_MAP_EXECUTE;
    }
    else if (ZCE_BIT_IS_SET(prot, PROT_EXEC) && ZCE_BIT_IS_SET(prot, PROT_READ) )
    {
        nt_flag_protect = PAGE_EXECUTE_READ;
        nt_flags = FILE_MAP_EXECUTE | FILE_MAP_READ;
    }
    else if (ZCE_BIT_IS_SET(prot, PROT_EXEC) && ZCE_BIT_IS_SET(prot, PROT_READ) && ZCE_BIT_IS_SET(prot, PROT_WRITE ) )
    {
        nt_flag_protect = PAGE_EXECUTE_READWRITE;
        nt_flags = FILE_MAP_EXECUTE | FILE_MAP_WRITE;
    }
    else
    {
        errno = EOPNOTSUPP;
        return MAP_FAILED;
    }

    //如果是私有的，相当于所有人都是副本
    if (ZCE_BIT_IS_SET (flags, MAP_PRIVATE))
    {
        nt_flags = FILE_MAP_COPY;
    }


    LARGE_INTEGER longlong_value;
    longlong_value.QuadPart = len;

    //file_handle == ZCE_INVALID_HANDLE后，创建的共享内存不再文件里面，而在系统映射文件中 system paging file
    ZCE_HANDLE file_mapping = ::CreateFileMappingA (file_handle,
                                                    NULL,
                                                    nt_flag_protect,
                                                    (file_handle == ZCE_INVALID_HANDLE) ? longlong_value.HighPart : 0,
                                                    (file_handle == ZCE_INVALID_HANDLE) ? longlong_value.LowPart : 0,
                                                    NULL);

    if (file_mapping == 0)
    {
        return MAP_FAILED;
    }

    //
    longlong_value.QuadPart = off;
    void *addr_mapping = ::MapViewOfFileEx (file_mapping,
                                            nt_flags,
                                            longlong_value.HighPart,
                                            longlong_value.LowPart,
                                            len,
                                            addr);

    // Only close this down if we used the temporary.
    ::CloseHandle (file_mapping);


    if (addr_mapping == 0)
    {
        return MAP_FAILED;
    }
    else
    {
        return addr_mapping;
    }

#elif defined (ZCE_OS_LINUX)
    //
    return ::mmap ( addr,
                    len,
                    prot,
                    flags,
                    file_handle,
                    off);

#endif
}


int ZCE_OS::mprotect (const void *addr, size_t len, int prot)
{
#if defined (ZCE_OS_WINDOWS)

    DWORD nt_flag_protect = 0;

    //这段在干嘛，就是将mmap的prot参数转换成微软的的参数，TNND，
    if ( PROT_NONE == prot )
    {
        nt_flag_protect = PAGE_NOACCESS;
    }
    else if (PROT_READ == prot)
    {
        nt_flag_protect = PAGE_READONLY;
    }
    else if ( ZCE_BIT_IS_SET(prot, PROT_READ) && ZCE_BIT_IS_SET(prot, PROT_WRITE) )
    {
        nt_flag_protect = PAGE_READWRITE;
    }
    else if (PROT_EXEC == prot)
    {
        nt_flag_protect = PAGE_EXECUTE;
    }
    else if (ZCE_BIT_IS_SET(prot, PROT_EXEC) && ZCE_BIT_IS_SET(prot, PROT_READ) )
    {
        nt_flag_protect = PAGE_EXECUTE_READ;
    }
    else if (ZCE_BIT_IS_SET(prot, PROT_EXEC) && ZCE_BIT_IS_SET(prot, PROT_READ) && ZCE_BIT_IS_SET(prot, PROT_WRITE) )
    {
        nt_flag_protect = PAGE_EXECUTE_READWRITE;
    }
    else
    {
        errno = EOPNOTSUPP;
        return -1;
    }

    DWORD dummy = 0;
    BOOL ret_bool = ::VirtualProtect((LPVOID)addr, len, prot, &dummy);

    if (ret_bool == FALSE)
    {
        return -1;
    }

    return 0;

#elif defined (ZCE_OS_LINUX)
    return ::mprotect ((void *) addr, len, prot);

#endif
}


int ZCE_OS::msync (void *addr, size_t len, int sync)
{
#if defined (ZCE_OS_WINDOWS)
    ZCE_UNUSED_ARG (sync);

    BOOL ret_bool = ::FlushViewOfFile (addr, len);

    if (ret_bool == FALSE)
    {
        return -1;
    }

    return 0;

#elif defined (ZCE_OS_LINUX)
    return ::msync (addr, len, sync);
#endif
}



int ZCE_OS::munmap (void *addr, size_t len)
{
#if defined (ZCE_OS_WINDOWS)
    ZCE_UNUSED_ARG (len);

    BOOL ret_bool = ::UnmapViewOfFile (addr);

    if (ret_bool == FALSE)
    {
        return -1;
    }

    return 0;
#elif defined (ZCE_OS_LINUX)
    return ::munmap (addr, len);
#endif
}


ZCE_HANDLE ZCE_OS::shm_open (const char *file_path,
                             int mode,
                             mode_t perms)
{
    //
#if defined (ZCE_OS_WINDOWS)

    //先建立一下这个目录,模拟/dev/shm/
    ZCE_OS::mkdir(ZCE_POSIX_MMAP_DIRECTORY);

    char shm_file_name[PATH_MAX + 1];
    shm_file_name[PATH_MAX] = '\0';
    snprintf(shm_file_name, PATH_MAX, "%s%s", ZCE_POSIX_MMAP_DIRECTORY, file_path);

    return ZCE_OS::open (shm_file_name, mode, perms);

#elif defined (ZCE_OS_LINUX)
    return ::shm_open (file_path, mode, perms);
#endif
}

int ZCE_OS::shm_unlink (const char *file_path)
{
#if defined (ZCE_OS_WINDOWS)

    char shm_file_name[PATH_MAX + 1];
    shm_file_name[PATH_MAX] = '\0';
    snprintf(shm_file_name, PATH_MAX, "%s%s", ZCE_POSIX_MMAP_DIRECTORY, file_path);

    return ::unlink (shm_file_name);

#elif defined (ZCE_OS_LINUX)
    return ::shm_unlink (file_path);
#endif
}

//-------------------------------------------------------------------------------------------------
//SystemV的共享内存

//提供这组模拟对我来说仅仅是为了好玩，（当然也由于System V有广大的深厚群众基础，至少让你移植起来容易一点）
//我个人对System V的IPC没有爱，一方面毕竟不如POSIX IPC在标准上站住了脚，System V的IPC这方面要弱一点，另一方面System V IPC 的接口设计也不如POSIX那么优雅，

//创建或者访问一个共享内存区
ZCE_HANDLE ZCE_OS::shmget(key_t sysv_key, size_t size, int oflag)
{
#if defined (ZCE_OS_WINDOWS)

    DWORD nt_flag_protect = 0;

    if (ZCE_BIT_IS_SET(oflag, SHM_R))
    {
        nt_flag_protect = PAGE_READONLY;
    }

    if ( ZCE_BIT_IS_SET(oflag, SHM_W) )
    {
        nt_flag_protect = PAGE_READWRITE;
    }

    char map_file_name[PATH_MAX + 1];
    map_file_name[PATH_MAX] = 0;
    snprintf(map_file_name, PATH_MAX, "ZCELIB.SYSV_IPC_SHM_%d.$$$", sysv_key);

    LARGE_INTEGER longlong_value;
    longlong_value.QuadPart = size;

    //file_handle == ZCE_INVALID_HANDLE后，创建的共享内存不再文件里面，而在系统映射文件中 system paging file
    ZCE_HANDLE shm_handle = ::CreateFileMappingA (ZCE_INVALID_HANDLE,
                                                  NULL,
                                                  nt_flag_protect,
                                                  longlong_value.HighPart,
                                                  longlong_value.LowPart,
                                                  (IPC_PRIVATE == sysv_key) ? NULL : map_file_name);

    //解释一下上面最后一行的参数，当使用key为IPC_PRIVATE，每次都创建一个无名的贡献内存，
    //如果Key不为IPC_PRIVATE，我统一给他起一个名字

    //如果出现错误
    if (shm_handle == 0)
    {
        return ZCE_INVALID_HANDLE;
    }

    //如果明确要求必须是创建，而且不能是已经存在的访问
    if ( ZCE_BIT_IS_SET(oflag, IPC_CREAT)  &&  ZCE_BIT_IS_SET(oflag, IPC_EXCL) )
    {
        if (ERROR_ALREADY_EXISTS == ::GetLastError() )
        {
            errno = EEXIST;
            return ZCE_INVALID_HANDLE;
        }
    }

    return shm_handle;

#elif defined (ZCE_OS_LINUX)
    return ::shmget(sysv_key, size, oflag);
#endif
}

//打开已经shmget的共享内存区
void *ZCE_OS::shmat(ZCE_HANDLE shmid, const void *shmaddr, int shmflg)
{
#if defined (ZCE_OS_WINDOWS)

    if (ZCE_BIT_ISNOT_SET (shmflg, SHM_RND))
    {
        shmaddr = 0;
    }

    // can not map to address 0
    if (ZCE_BIT_IS_SET (shmflg, SHM_RND) && 0 == shmaddr )
    {
        errno = ENOTSUP;
        return MAP_FAILED;
    }

    DWORD nt_flags = FILE_MAP_WRITE;

    if (ZCE_BIT_IS_SET (shmflg, SHM_RDONLY))
    {
        nt_flags = FILE_MAP_READ;
    }

    if (ZCE_BIT_IS_SET (shmflg, SHM_EXEC))
    {
        nt_flags = FILE_MAP_READ | FILE_MAP_EXECUTE;
    }

    //偏移量
    LARGE_INTEGER longlong_value;
    longlong_value.QuadPart = 0;

    //size参数为0标识全部内存映射
    void *addr_mapping = ::MapViewOfFileEx (shmid,
                                            nt_flags,
                                            longlong_value.HighPart,
                                            longlong_value.LowPart,
                                            0,
                                            (LPVOID)shmaddr);

    //如果映射失败
    if (NULL == addr_mapping)
    {
        return MAP_FAILED;
    }

    return addr_mapping;


#elif defined (ZCE_OS_LINUX)
    return ::shmat(shmid, shmaddr, shmflg);
#endif
}

//短接这个内存区
int ZCE_OS::shmdt(const void *shmaddr)
{
#if defined (ZCE_OS_WINDOWS)
    BOOL ret_bool = ::UnmapViewOfFile (shmaddr);

    if (ret_bool == FALSE)
    {
        return -1;
    }

    return 0;

#elif defined (ZCE_OS_LINUX)
    return ::shmdt(shmaddr);
#endif
}

//对共享内存区提供多种操作
int ZCE_OS::shmctl(ZCE_HANDLE shmid, int cmd, struct shmid_ds *buf)
{
#if defined (ZCE_OS_WINDOWS)

    ZCE_UNUSED_ARG(shmid);
    ZCE_UNUSED_ARG(buf);

    //微软的映射内存当引用计数为0时会自己释放的，所以我姑且相信你使用正确把
    if (IPC_RMID == cmd)
    {
        return 0;
    }

    errno = ENOTSUP;
    return -1;

#elif defined (ZCE_OS_LINUX)
    return ::shmctl(shmid, cmd, buf);
#endif
}



