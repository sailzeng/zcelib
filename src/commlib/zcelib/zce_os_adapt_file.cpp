
#include "zce_predefine.h"
#include "zce_log_logging.h"
#include "zce_os_adapt_predefine.h"
#include "zce_os_adapt_time.h"
#include "zce_os_adapt_error.h"
#include "zce_os_adapt_file.h"

//��ȡ�ļ�
ssize_t zce::read(ZCE_HANDLE file_handle, void *buf, size_t count)
{
    //WINDOWS�£������޷�ͻ��32λ�ģ�����������ReadFileExҲһ�������WINDOWS��Ϊû��������ȡ�ļ�
    //λ�õ�Ȼ���ǿ��Ե�����
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

//д���ļ���WINDOWS�£������޷�ͻ��32λ��,��Ȼ������Ҫд��4G������
//Windows�¾�����POSIX ��£��
ssize_t zce::write(ZCE_HANDLE file_handle, const void *buf, size_t count)
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
        //ע��zce Windows �µ�write���л���ģ������Linux�µ���������
        //�����Ҫ����������������FlushFileBuffers,����ʱ������һ��Ҫ�������ı�Ҫ��
        //����ط�Ϊ�˺�POSIXͳһ�����ǵ������������
        //����һ����������CreateFile ʱ�������� FILE_FLAG_NO_BUFFERING and FILE_FLAG_WRITE_THROUGH
        ::FlushFileBuffers(file_handle);
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

//�ض��ļ�
int zce::truncate(const char *filename, size_t offset)
{
#if defined (ZCE_OS_WINDOWS)

    int ret = 0;
    //���ļ������ҽضϣ����ر�
    ZCE_HANDLE file_handle = zce::open(filename, (O_CREAT | O_RDWR));
    if ( ZCE_INVALID_HANDLE == file_handle)
    {
        return -1;
    }

    ret = zce::ftruncate(file_handle, offset);

    if (0 != ret )
    {
        return ret;
    }

    zce::close(file_handle);
    return 0;
#endif

#if defined (ZCE_OS_LINUX)
    return ::truncate (filename, static_cast<off_t>(offset));
#endif
}

//�ض��ļ�����ù����WINDOWS����TMD û�У���BOOST���ַ�Ҫ�������Ĳ����淶���ҵ���
//��ʵ���Ա䳤���Ǻǡ�
//ע�������fd��WIN32 API OpenFile�õ��ĺ�������������ISO�����򿪵��Ǹ�fd��
int zce::ftruncate(ZCE_HANDLE file_handle, size_t  offset)
{
    //Windows2000��ǰû�� SetFilePointerEx���Ҳ���ACE���Ҳ�֧����ô��ƨ��
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

    //linux ftruncate���󣬰�ָ��ŵ���ĩβ
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

//���ļ��ڽ���ƫ��
ssize_t zce::lseek(ZCE_HANDLE file_handle, ssize_t offset, int whence)
{
#if defined (ZCE_OS_WINDOWS)

    //WINDOWS��lseek�ǲ�֧��64λ�ģ�����ֱ����API����ɹ���������������_lseeki64��
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

//�����ļ����ƣ��ж��ļ��ĳߴ�,����ļ������ڣ��򲻿��ȣ�����-1
int zce::filelen(const char *filename, size_t *file_size)
{
    int ret = 0;
    ZCE_HANDLE file_handle = zce::open(filename, (O_RDONLY));

    if ( ZCE_INVALID_HANDLE == file_handle)
    {
        return -1;
    }

    ret = zce::filesize (file_handle, file_size);
    zce::close (file_handle);
    return ret;
}

int zce::filesize (ZCE_HANDLE file_handle, size_t *file_size)
{
#if defined (ZCE_OS_WINDOWS)

    LARGE_INTEGER size;
    BOOL ret_bool = ::GetFileSizeEx (file_handle, &size);

    if (!ret_bool)
    {
        return -1;
    }

    //32λƽ̨�Ͽ��ܶ����ȣ������ҾͿ���64λϵͳ����Ż�ͻ��4G��
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

//������������Ϊ�ǵ���ΪACE�����Ϊʲô��ֱ����open������Ȼ����_get_osfhandleת����HANDLE�Ϳ����ˡ�
//�رյ�ʱ����_open_osfhandleת��������OK�ˣ�����ʵ������������ң�
//����ȫ���������_open_osfhandle��������Ҳ���ܽ�����ԭ��pascalԭ������������close�������Ե����⡣
//һ�ж�����RP���⣬����д���˴��롣��лderrickhu��sasukeliu��λ��һ�����صıȽ���̵�bug

//ΪʲôҪ�ṩ���API�أ���ΪWINDOWSƽ̨�󲿷ֶ��ǲ���HANDLE�����

ZCE_HANDLE zce::open (const char *filename,
                      int open_mode,
                      mode_t perms)
{
    //Windowsƽ̨
#if defined (ZCE_OS_WINDOWS)

    //������LINUX�Ĳ���ת����Windows API�Ĳ���

    DWORD access = GENERIC_READ;

    if (ZCE_BIT_IS_SET (open_mode, O_WRONLY))
    {
        //�������ֻ��д
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

    //ACE�Ĵ����������һ�����̵߳Ļ��Ᵽ����
    //��ΪCreateFileA������ͬʱ���ļ���ָ���ƶ���ĩβ������(O_APPEND)����һ����������(��CreateFileA����SetFilePointerEx)��
    //ACE������������������̴߳������ǣ�����û��ע��˵���������,����ʱ��ȥ��������

    //CRITICAL_SECTION fileopen_mutex;
    ////VISTAT��û������쳣��
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

    //����򿪵��ļ��������Ч��

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

    //��Ӧ������ٽ�������
    //::DeleteCriticalSection (&fileopen_mutex);

    return openfile_handle;

#elif defined (ZCE_OS_LINUX)
    return ::open (filename, open_mode, perms);
#endif
}

//�ر�һ���ļ�
int zce::close (ZCE_HANDLE handle)
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

//��ģ�����ƽ������Ҵ�һ����ʱ�ļ���
ZCE_HANDLE zce::mkstemp(char *template_name)
{
#if defined (ZCE_OS_WINDOWS)
    char *tmp_filename = _mktemp(template_name);
    return zce::open(tmp_filename, ZCE_DEFAULT_FILE_PERMS);
#elif defined (ZCE_OS_LINUX)
    return ::mkstemp(template_name);
#endif
}

//ͨ���ļ����Ƶõ��ļ���stat��Ϣ���������Ϊzce_os_stat����stat��ֻ����WINDOWS��stat64,��Ҫ��Ϊ�˳��ļ����ǵ�
int zce::stat(const char *path, zce_os_stat *file_stat)
{
#if defined (ZCE_OS_WINDOWS)
    return ::_stat64(path, file_stat);
#elif defined (ZCE_OS_LINUX)
    return ::stat(path, file_stat);
#endif
}

//ͨ���ļ��ľ���õ��ļ���stat��Ϣ
int zce::fstat(ZCE_HANDLE file_handle, zce_os_stat *file_stat)
{
#if defined (ZCE_OS_WINDOWS)

    //���ʵ�ֱȽ�ʹ�࣬��Ҳû�а취������������������_open_osfhandle������͵�����ɣ�����һ����

    BOOL ret_bool = FALSE;
    BY_HANDLE_FILE_INFORMATION file_info;
    ret_bool = ::GetFileInformationByHandle(file_handle,
                                            &file_info);

    if (!ret_bool)
    {
        return -1;
    }

    //ת��ʱ��
    timeval tv_ct_time = zce::make_timeval(&file_info.ftCreationTime);
    timeval tv_ac_time = zce::make_timeval(&file_info.ftLastAccessTime);
    timeval tv_wt_time = zce::make_timeval(&file_info.ftLastWriteTime);

    LARGE_INTEGER file_size;
    file_size.HighPart = file_info.nFileSizeHigh;
    file_size.LowPart = file_info.nFileSizeLow;

    //_S_IFDIR,

    memset(file_stat, 0, sizeof(zce_os_stat));
    file_stat->st_uid = 0;
    file_stat->st_gid = 0;
    file_stat->st_size = file_size.QuadPart;

    //�õ�����ʱ��
    //ע��st_ctime���ѽ�������LINUX�º�Windows����Щ��һ���ģ�st_ctime��LINUX����״̬���ı�ʱ�䣬����WINDOWS���Ǵ���ʱ��
    file_stat->st_ctime = tv_ct_time.tv_sec;
    file_stat->st_mtime = tv_wt_time.tv_sec;
    file_stat->st_atime = tv_ac_time.tv_sec;

    //������ļ�����Ŀ¼
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



//·���Ƿ���һ��Ŀ¼������Ƿ���TRUE��������Ƿ���FALSE
bool zce::is_directory(const char *path_name)
{
    int ret = 0;
    zce_os_stat file_stat;
    ret = zce::stat(path_name, &file_stat);
    if (0 != ret)
    {
        return false;
    }
    //�����Ŀ¼���ԣ���true
    if (file_stat.st_mode & S_IFDIR )
    {
        return true;
    }
    else
    {
        return false;
    }
}


//ɾ���ļ�
int zce::unlink(const char *filename )
{
#if defined (ZCE_OS_WINDOWS)
    return ::_unlink(filename);
#elif defined (ZCE_OS_LINUX)
    return ::unlink(filename);
#endif
}

//
mode_t zce::umask (mode_t cmask)
{
#if defined (ZCE_OS_WINDOWS)
    return ::_umask(cmask);
#elif defined (ZCE_OS_LINUX)
    return ::umask(cmask);
#endif
}

//����ļ��Ƿ�OK�����
//mode ����ƽ̨��֧��F_OK,R_OK,W_OK,R_OK|W_OK��X_OK����LINUX֧��,WIN��֧��
int zce::access(const char *pathname, int mode)
{
#if defined (ZCE_OS_WINDOWS)
    return ::_access_s(pathname, mode);
#elif defined (ZCE_OS_LINUX)
    return ::access(pathname, mode);
#endif
}

//--------------------------------------------------------------------------------------------------
//�Ǳ�׼����
//��ֻ����ʽ��ȡһ���ļ������ݣ����ص�buffer������'\0',buf_len >= 1
int zce::read_file_data(const char *filename, 
                        char *buffer, 
                        size_t buf_len, 
                        size_t *read_len,
                        size_t offset)
{
    //�������
    ZCE_ASSERT(filename && buffer && buf_len >= 1);

    //���ļ�
    ZCE_HANDLE  fd = zce::open(filename, O_RDONLY);
    if (ZCE_INVALID_HANDLE == fd)
    {
        ZCE_LOG(RS_ERROR, "open file [%s]  fail ,error =%d", filename, zce::last_error());
        return -1;
    }
    zce::lseek(fd,static_cast<ssize_t>(offset),SEEK_SET);
    //��ȡ����
    ssize_t len = zce::read(fd, buffer, buf_len - 1);
    zce::close(fd);

    if (len < 0)
    {
        ZCE_LOG(RS_ERROR, "read file [%s] fail ,error =%d", filename, zce::last_error());
        return -1;
    }

    buffer[len] = 0;
    *read_len = len;

    return 0;
}

//��ȡ�ļ���ȫ�����ݣ�
std::pair<int,std::shared_ptr<char>> zce::read_file_all(const char* filename,
                                                        size_t* file_len,
                                                        size_t offset)
{
    int ret=-1;
    std::shared_ptr<char> null_ptr;
    //���ļ�
    ZCE_HANDLE  fd=zce::open(filename,O_RDONLY);
    if(ZCE_INVALID_HANDLE==fd)
    {
        ZCE_LOG(RS_ERROR,"open file [%s]  fail ,error =%d",filename,zce::last_error());
        return std::make_pair(ret,null_ptr);
    }
    *file_len = zce::lseek(fd,0,SEEK_END);
    if(static_cast<size_t>(-1) == *file_len)
    {
        zce::close(fd);
        ZCE_LOG(RS_ERROR,"open file [%s]  fail ,error =%d",filename,zce::last_error());
        return std::make_pair(ret,null_ptr);
    }
    std::shared_ptr<char> ptr(new char [*file_len+1],std::default_delete<char []>());
    *(ptr.get() +*file_len)='\0';
    //����ƫ�ƣ���ȡ����
    zce::lseek(fd,static_cast<ssize_t>(offset),SEEK_SET);
    ssize_t len=zce::read(fd,ptr.get(),*file_len);
    zce::close(fd);
    if(len<0)
    {
        ZCE_LOG(RS_ERROR,"read file [%s] fail ,error =%d",filename,zce::last_error());
        return std::make_pair(ret,null_ptr);
    }

    ret=0;
    return std::make_pair(ret,ptr);
}
