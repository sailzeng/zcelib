#include "zce/predefine.h"
#include "zce/log/logging.h"
#include "zce/os_adapt/file.h"
#include "zce/lock/record_lock.h"

//��¼���ķ�װ��

//���캯��
ZCE_Record_Lock::ZCE_Record_Lock(ZCE_HANDLE file_handle)
{
    open(file_handle);
}

ZCE_Record_Lock::ZCE_Record_Lock()
{

}
ZCE_Record_Lock::~ZCE_Record_Lock()
{
    close();
}

//��һ���ļ�,ͬʱ��ʼ��������lock����
int ZCE_Record_Lock::open(const char *file_name,
                          int open_mode,
                          mode_t perms)
{
    //�����ظ��򿪣��ö��Ա���
    ZCE_ASSERT(record_lock_.handle_ == ZCE_INVALID_HANDLE);
    //������ļ�
    ZCE_HANDLE file_handle = zce::open(file_name,
                                      open_mode,
                                      perms);

    if (file_handle == ZCE_INVALID_HANDLE)
    {
        return -1;
    }
    //��ʶ���Լ��򿪵�
    open_by_self_ = true;

    return zce::file_lock_init(&record_lock_,
                                file_handle);
}

//��һ���ļ�Handle��ʼ��,�ⲿ�����ZCE_HANDLE���Ҳ���رգ��ļ�
int ZCE_Record_Lock::open(ZCE_HANDLE file_handle)
{
    return zce::file_lock_init(&record_lock_, file_handle);
}

//�õ����ļ��ľ��
ZCE_HANDLE ZCE_Record_Lock::get_file_handle()
{
    return record_lock_.handle_;
}

//�ر�֮�������ZCE_Record_Lock�ڲ��Լ��򿪵��ļ��������ļ�������������ر�ʱ��ر��ļ�
void ZCE_Record_Lock::close()
{
    if (open_by_self_)
    {
        zce::close(record_lock_.handle_);
    }
    return;
}

//�ļ���¼��д��
int ZCE_Record_Lock::flock_rdlock (int  whence,
                                   ssize_t start,
                                   ssize_t len)
{
    return zce::fcntl_rdlock(&record_lock_,
                             whence,
                             start,
                             len);
}

//���Զ��ļ���¼���мӶ�ȡ��
int ZCE_Record_Lock::flock_tryrdlock (int  whence,
                                      ssize_t start,
                                      ssize_t len)
{
    return zce::fcntl_tryrdlock(&record_lock_,
                                whence,
                                start,
                                len);
}

//���ļ���¼ֱ������
int ZCE_Record_Lock::flock_wrlock (int  whence,
                                   ssize_t start,
                                   ssize_t len)
{
    return zce::fcntl_wrlock(&record_lock_,
                             whence,
                             start,
                             len);
}

//���Զ��ļ���¼���м�дȡ��
int ZCE_Record_Lock::flock_trywrlock (int  whence,
                                      ssize_t start,
                                      ssize_t len)
{
    return zce::fcntl_trywrlock(&record_lock_,
                                whence,
                                start,
                                len);
}

//����
int ZCE_Record_Lock::flock_unlock (int  whence,
                                   ssize_t start,
                                   ssize_t len)
{
    return zce::fcntl_unlock(&record_lock_,
                             whence,
                             start,
                             len);
}

