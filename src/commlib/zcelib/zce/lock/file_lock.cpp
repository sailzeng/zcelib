#include "zce/predefine.h"
#include "zce/log/logging.h"
#include "zce/os_adapt/error.h"
#include "zce/os_adapt/file.h"
#include "zce/os_adapt/flock.h"
#include "zce/lock/file_lock.h"

//���캯��
ZCE_File_Lock::ZCE_File_Lock():
    open_by_self_(false),
    file_len_(0)
{
}

//��������
ZCE_File_Lock::~ZCE_File_Lock()
{
    close();
}

//ͨ���ļ����Ʋ�����ʼ���ļ������������ļ�
int ZCE_File_Lock::open(const char *file_name,
                        int open_mode,
                        mode_t perms)
{
    int ret = 0;
    //�����ظ��򿪣��ö��Ա���
    ZCE_ASSERT(file_lock_.handle_ == ZCE_INVALID_HANDLE);

    //������ļ�
    ZCE_HANDLE file_handle = zce::open(file_name,
                                   open_mode,
                                   perms);

    if (file_handle == ZCE_INVALID_HANDLE)
    {
        return -1;
    }

    ret = zce::filesize(file_handle, &file_len_);
    if (0 != ret)
    {
        return ret;
    }

    zce::file_lock_init(&file_lock_,file_handle);

    //��ʶ���Լ��򿪵�
    open_by_self_ = true;

    return 0;
}

//ͨ���ļ������ʼ���ļ���
int ZCE_File_Lock::open(ZCE_HANDLE file_handle)
{
    int ret = 0;
    ret = zce::filesize(file_handle, &file_len_);
    if (0 != ret)
    {
        return ret;
    }

    zce::file_lock_init(&file_lock_,file_handle);

    return 0;
}

//�ر��ļ���
int ZCE_File_Lock::close()
{
    unlock();

    //������Լ��򿪵ģ��ر�֮
    if (open_by_self_)
    {
        zce::close(file_lock_.handle_);
    }
    return 0;
}

//�õ����ļ��ľ��
ZCE_HANDLE ZCE_File_Lock::get_file_handle()
{
    return file_lock_.handle_;
}

//��ȡ��
void ZCE_File_Lock::lock_read()
{
    int ret = 0;
    ret = zce::fcntl_rdlock(&file_lock_, SEEK_SET, 0, file_len_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::flock LOCK_SH", ret);
        return;
    }

    return;
}
//���Զ�ȡ��
bool ZCE_File_Lock::try_lock_read()
{
    int ret = 0;

    ret = zce::fcntl_tryrdlock(&file_lock_, SEEK_SET, 0, file_len_);
    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::flock LOCK_SH|LOCK_NB", ret);
        return false;
    }

    return true;
}

//д����
void ZCE_File_Lock::lock_write()
{
    int ret = 0;
    ret = zce::fcntl_wrlock(&file_lock_, SEEK_SET, 0, file_len_);
    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::fcntl_wrlock LOCK_EX", ret);
        return;
    }
}
//���Զ�ȡ��
bool ZCE_File_Lock::try_lock_write()
{
    int ret = 0;

    ret = zce::fcntl_trywrlock(&file_lock_, SEEK_SET, 0, file_len_);
    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::try_lock_write LOCK_EX|LOCK_NB", ret);
        return false;
    }

    return true;
}

//����,����Ƕ�д��Ҳֻ��Ҫ��һ������
void ZCE_File_Lock::unlock()
{
    int ret = 0;

    ret = zce::fcntl_unlock(&file_lock_, SEEK_SET, 0, file_len_);
    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::fcntl_unlock LOCK_UN", ret);
        return;
    }
}

