//Posix�Ĺ����ڴ���ģ���ʱ����һ�����⣬Linux�µ�Posix�Ĺ����ڴ治���й����ڴ�����ԣ�
//�����ļ������ԣ����һ�ͳһ�ķŵ�һ��Ŀ¼/dev/shmĿ¼��
//���Բ�̫����Window�ķ��ļ�ӳ�乱���ڴ�ģ��.
//����ֻ�л�һ�·�ʽ����Windows��Ҳ��һ������Ŀ¼,

#include "zce_predefine.h"
#include "zce_os_adapt_predefine.h"
#include "zce_boost_non_copyable.h"
#include "zce_os_adapt_shm.h"
#include "zce_os_adapt_file.h"
#include "zce_os_adapt_dirent.h"
#include "zce_os_adapt_error.h"
#include "zce_log_logging.h"
#include "zce_share_mem_posix.h"

//���캯��
ZCE_ShareMem_Posix::ZCE_ShareMem_Posix():
    mmap_addr_(NULL),
    mmap_handle_(ZCE_INVALID_HANDLE),
    shm_size_(0)
{
}

ZCE_ShareMem_Posix::~ZCE_ShareMem_Posix()
{
    if (mmap_addr_)
    {
        close();
    }
}

//���ļ�������ӳ��
int ZCE_ShareMem_Posix::open(const char *shm_name,
                             std::size_t shm_size,
                             int file_open_mode,
                             int file_perms_mode,
                             const void *want_address,
                             int mmap_prot,
                             int mmap_flags,
                             std::size_t offset)
{
    //�����������open������������ֶ��Ա�ʾ��ε���open,
    ZCE_ASSERT(NULL == mmap_addr_);
    ZCE_ASSERT(ZCE_INVALID_HANDLE == mmap_handle_);

    int ret = 0;

    //���������INVALID_HANDLE_VALUE ʱ������ʹ�ò����ļ�ӳ��Ĺ����ڴ�
    //MMAP�ļ��ľ��
    mmap_handle_ = zce::shm_open(shm_name, file_open_mode, file_perms_mode);

    //���û�гɹ����ļ�
    if (mmap_handle_ == ZCE_INVALID_HANDLE)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] Posix memory open fail, name=%s ,zce::shm_open fail. last error =%d",
                shm_name,
                zce::last_error());
        return -1;
    }

    shm_name_ = shm_name;

    //�����ģʽʹ����O_TRUNC����ʾ���´�
    if (file_open_mode & O_TRUNC)
    {
        zce::ftruncate(mmap_handle_, static_cast<long>(shm_size + offset));
    }
    else
    {
        //���ļ��ĳߴ���м�飬������ԣ�Ҳ�����д���
        size_t filelen = 0;
        ret  = zce::filesize(mmap_handle_, &filelen);

        //���ܵõ��ļ��ߴ�
        if ( 0 != ret )
        {
            ZCE_LOG(RS_ERROR, "[zcelib] Posix memory open fail, name=%s ,zce::filesize ret =%ld last error=%d",
                    shm_name,
                    filelen,
                    zce::last_error());
            zce::close(mmap_handle_);
            mmap_handle_ = ZCE_INVALID_HANDLE;
            return -1;
        }
        //�ļ���һ�����ļ������߿��ļ���
        else if ( 0 == filelen )
        {
            zce::ftruncate(mmap_handle_, static_cast<long>(shm_size + offset));
        }
        //�ļ��Ѿ����ڣ����һ�������(��С)��
        else
        {
            //�ļ��Ŀռ䶼���������
            if (filelen < static_cast<size_t> (shm_size + offset))
            {
                ZCE_LOG(RS_ERROR, "[zcelib] Posix memory open fail, name=%s, old file size(%lu) < request file size(%lu). ",
                        shm_name,
                        filelen,
                        shm_size + offset);
                zce::close(mmap_handle_);
                mmap_handle_ = ZCE_INVALID_HANDLE;
                return -1;
            }
        }

    }

    //���й����ڴ�ӳ��
    void *nonconst_addr = const_cast<void *>(want_address);
    mmap_addr_ = zce::mmap (nonconst_addr,
                            shm_size,
                            mmap_prot,
                            mmap_flags,
                            mmap_handle_,
                            offset);

    //
    if (!mmap_addr_)
    {
        zce::close(mmap_handle_);
        mmap_handle_ = ZCE_INVALID_HANDLE;
        return -1;
    }

    shm_size_ = shm_size;
    return 0;
}

//���ļ�������ӳ��, ��
int ZCE_ShareMem_Posix::open(const char *shm_name,
                             std::size_t shm_size,
                             bool if_restore,
                             bool read_only,
                             bool share_file,
                             const void *want_address,
                             std::size_t  offset )
{
    int file_open_mode = (O_CREAT);
    int mmap_prot = PROT_READ;
    int mmap_flags = 0;
    int file_perms_mode = 0;

    //ΪʲôĬ���������û�Ҳ���Զ�ȡ����Ϊ��Щʱ������Ҫ��+s��rootȨ��������������
    if (share_file)
    {
        mmap_flags |= MAP_SHARED;
        file_perms_mode = ZCE_SHARE_FILE_PERMS;
    }
    else
    {
        mmap_flags |= MAP_PRIVATE;
        file_perms_mode = ZCE_PRIVATE_FILE_PERMS;
    }

    //������ǻָ������ļ����нض�
    if (!if_restore)
    {
        file_open_mode |= (O_TRUNC);
    }

    //�������ֻ����
    if (!read_only)
    {
        file_open_mode |= O_RDWR;
        mmap_prot |= PROT_WRITE;
    }
    else
    {
        file_open_mode |= O_RDONLY;
    }

    //�����ϣ�����õĵ�ַ��
    if (want_address)
    {
        mmap_flags |= MAP_FIXED;
    }

    //
    return this->open(shm_name,
                      shm_size,
                      file_open_mode,
                      file_perms_mode,
                      want_address,
                      mmap_prot,
                      mmap_flags,
                      offset);

}

//�ر��ļ�
int ZCE_ShareMem_Posix::close()
{
    //���Ա�֤������û��open�͵���close�����
    ZCE_ASSERT(mmap_addr_ != NULL );
    ZCE_ASSERT(mmap_handle_ != ZCE_INVALID_HANDLE);

    int ret = 0;
    ret = zce::munmap(mmap_addr_, shm_size_);
    mmap_addr_ = NULL;
    shm_size_ = 0;

    zce::close(mmap_handle_);
    mmap_handle_ = ZCE_INVALID_HANDLE;

    if (ret != 0)
    {
        return -1;
    }

    return 0;
}

//ɾ��ӳ����ļ�����Ȼ����ӳ���ʱ����ɾ��
int ZCE_ShareMem_Posix::remove()
{
    return zce::shm_unlink(shm_name_.c_str());
}

//ͬ���ļ�
int ZCE_ShareMem_Posix::flush()
{
    return zce::msync(mmap_addr_, shm_size_, MS_SYNC);
}

//����ӳ����ڴ��ַ
void *ZCE_ShareMem_Posix::addr()
{
    return mmap_addr_;
}
