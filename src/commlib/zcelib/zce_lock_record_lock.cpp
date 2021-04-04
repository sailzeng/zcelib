#include "zce_predefine.h"
#include "zce_os_adapt_file.h"
#include "zce_lock_record_lock.h"

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
    //������ļ�
    ZCE_HANDLE lock_hadle = zce::open(file_name,
                                      open_mode,
                                      perms);

    return zce::flock_init(&record_lock_,
                           lock_hadle);
}

//��һ���ļ�Handle��ʼ��,�ⲿ�����ZCE_HANDLE���Ҳ���رգ��ļ�
int ZCE_Record_Lock::open(ZCE_HANDLE file_handle)
{
    return zce::flock_init(&record_lock_, file_handle);
}

//�õ����ļ��ľ��
ZCE_HANDLE ZCE_Record_Lock::get_file_handle()
{
    return record_lock_.handle_;
}

//�ر�֮�������ZCE_Record_Lock�ڲ��Լ��򿪵��ļ��������ļ�������������ر�ʱ��ر��ļ�
void ZCE_Record_Lock::close()
{
    zce::flock_destroy(&record_lock_);
    return;
}

//�ļ���¼��д��
int ZCE_Record_Lock::flock_rdlock (int  whence,
                                   ssize_t start,
                                   ssize_t len)
{
    return zce::flock_rdlock(&record_lock_,
                             whence,
                             start,
                             len);
}

//���Զ��ļ���¼���мӶ�ȡ��
int ZCE_Record_Lock::flock_tryrdlock (int  whence,
                                      ssize_t start,
                                      ssize_t len)
{
    return zce::flock_tryrdlock(&record_lock_,
                                whence,
                                start,
                                len);
}

//���ļ���¼ֱ������
int ZCE_Record_Lock::flock_wrlock (int  whence,
                                   ssize_t start,
                                   ssize_t len)
{
    return zce::flock_wrlock(&record_lock_,
                             whence,
                             start,
                             len);
}

//���Զ��ļ���¼���м�дȡ��
int ZCE_Record_Lock::flock_trywrlock (int  whence,
                                      ssize_t start,
                                      ssize_t len)
{
    return zce::flock_trywrlock(&record_lock_,
                                whence,
                                start,
                                len);
}

//����
int ZCE_Record_Lock::flock_unlock (int  whence,
                                   ssize_t start,
                                   ssize_t len)
{
    return zce::flock_unlock(&record_lock_,
                             whence,
                             start,
                             len);
}

