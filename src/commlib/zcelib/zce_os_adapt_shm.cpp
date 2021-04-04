
#include "zce_predefine.h"

#include "zce_os_adapt_error.h"
#include "zce_os_adapt_file.h"
#include "zce_os_adapt_dirent.h"
#include "zce_os_adapt_shm.h"


//
void *zce::mmap (void *addr,
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

    DWORD nt_flag_protect = 0;
    DWORD  nt_flags = 0;

    // can not map to address 0
    if (ZCE_BIT_IS_SET (flags, MAP_FIXED) && 0 == addr )
    {
        errno = ENOTSUP;
        return MAP_FAILED;
    }

    //����ʹ�ã������ļ��������Чֵ
    if (ZCE_BIT_IS_SET(flags, MAP_ANONYMOUS) && ZCE_INVALID_HANDLE != file_handle || file_handle == NULL)
    {
        errno = ENOTSUP;
        return MAP_FAILED;
    }


    //����ڸ�����ǽ�mmap��prot����ת����΢��ĵĲ�����TNND��
    if ( PROT_NONE == prot )
    {
        nt_flag_protect = PAGE_NOACCESS;
    }
    else if (PROT_READ == prot)
    {
        nt_flag_protect = PAGE_READWRITE;
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

    //�����˽�еģ��൱�������˶��Ǹ���
    if (ZCE_BIT_IS_SET (flags, MAP_PRIVATE))
    {
        //PAGE_WRITECOPY �ȼ� PAGE_READONLY
        nt_flag_protect |= PAGE_WRITECOPY;
        nt_flags = FILE_MAP_COPY;
    }


    LARGE_INTEGER longlong_value;
    longlong_value.QuadPart = len;

    //file_handle == ZCE_INVALID_HANDLE�󣬴����Ĺ����ڴ治���ļ����棬����ϵͳӳ���ļ��� system paging file
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


int zce::mprotect (const void *addr, size_t len, int prot)
{
#if defined (ZCE_OS_WINDOWS)

    DWORD nt_flag_protect = 0;

    //����ڸ�����ǽ�mmap��prot����ת����΢��ĵĲ�����TNND��
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


int zce::msync (void *addr, size_t len, int sync)
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



int zce::munmap (void *addr, size_t len)
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


ZCE_HANDLE zce::shm_open (const char *file_path,
                          int mode,
                          mode_t perms)
{
    //
#if defined (ZCE_OS_WINDOWS)

    //�Ƚ���һ�����Ŀ¼,ģ��/dev/shm/
    zce::mkdir(ZCE_POSIX_MMAP_DIRECTORY);

    char shm_file_name[PATH_MAX + 1];
    shm_file_name[PATH_MAX] = '\0';
    snprintf(shm_file_name, PATH_MAX, "%s%s", ZCE_POSIX_MMAP_DIRECTORY, file_path);

    return zce::open (shm_file_name, mode, perms);

#elif defined (ZCE_OS_LINUX)
    return ::shm_open (file_path, mode, perms);
#endif
}

int zce::shm_unlink (const char *file_path)
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
//SystemV�Ĺ����ڴ�

//�ṩ����ģ�������˵������Ϊ�˺��棬����ȻҲ����System V�й������Ⱥ�ڻ���������������ֲ��������һ�㣩
//�Ҹ��˶�System V��IPCû�а���һ����Ͼ�����POSIX IPC�ڱ�׼��վס�˽ţ�System V��IPC�ⷽ��Ҫ��һ�㣬��һ����System V IPC �Ľӿ����Ҳ����POSIX��ô���ţ�

//�������߷���һ�������ڴ���
ZCE_HANDLE zce::shmget(key_t sysv_key, size_t size, int oflag)
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

    //file_handle == ZCE_INVALID_HANDLE�󣬴����Ĺ����ڴ治���ļ����棬����ϵͳӳ���ļ��� system paging file
    ZCE_HANDLE shm_handle = ::CreateFileMappingA (ZCE_INVALID_HANDLE,
                                                  NULL,
                                                  nt_flag_protect,
                                                  longlong_value.HighPart,
                                                  longlong_value.LowPart,
                                                  (IPC_PRIVATE == sysv_key) ? NULL : map_file_name);

    //����һ���������һ�еĲ�������ʹ��keyΪIPC_PRIVATE��ÿ�ζ�����һ�������Ĺ����ڴ棬
    //���Key��ΪIPC_PRIVATE����ͳһ������һ������

    //������ִ���
    if (shm_handle == 0)
    {
        return ZCE_INVALID_HANDLE;
    }

    //�����ȷҪ������Ǵ��������Ҳ������Ѿ����ڵķ���
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

//���Ѿ�shmget�Ĺ����ڴ���
void *zce::shmat(ZCE_HANDLE shmid, const void *shmaddr, int shmflg)
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

    //ƫ����
    LARGE_INTEGER longlong_value;
    longlong_value.QuadPart = 0;

    //size����Ϊ0��ʶȫ���ڴ�ӳ��
    void *addr_mapping = ::MapViewOfFileEx (shmid,
                                            nt_flags,
                                            longlong_value.HighPart,
                                            longlong_value.LowPart,
                                            0,
                                            (LPVOID)shmaddr);

    //���ӳ��ʧ��
    if (NULL == addr_mapping)
    {
        return MAP_FAILED;
    }

    return addr_mapping;


#elif defined (ZCE_OS_LINUX)
    return ::shmat(shmid, shmaddr, shmflg);
#endif
}

//�̽�����ڴ���
int zce::shmdt(const void *shmaddr)
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

//�Թ����ڴ����ṩ���ֲ���
int zce::shmctl(ZCE_HANDLE shmid, int cmd, struct shmid_ds *buf)
{
#if defined (ZCE_OS_WINDOWS)

    ZCE_UNUSED_ARG(shmid);
    ZCE_UNUSED_ARG(buf);

    //΢���ӳ���ڴ浱���ü���Ϊ0ʱ���Լ��ͷŵģ������ҹ���������ʹ����ȷ��
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



