
#include "zce_predefine.h"
#include "zce_os_adapt_predefine.h"
#include "zce_boost_non_copyable.h"
#include "zce_os_adapt_shm.h"
#include "zce_os_adapt_file.h"
#include "zce_os_adapt_dirent.h"
#include "zce_os_adapt_error.h"
#include "zce_log_logging.h"
#include "zce_share_mem_systemv.h"

//�������е������Ϊʲôϲ����System V�Ĺ����ڴ��ˣ��򵥡���Ϊû���ļ�ӳ�䣬������ʵ�����˺ܶ����

//���캯��
ZCE_ShareMem_SystemV::ZCE_ShareMem_SystemV():
    sysv_key_(0),
    sysv_shmid_(ZCE_INVALID_HANDLE),
    shm_size_(0),
    shm_addr_(NULL)
{
}

ZCE_ShareMem_SystemV::~ZCE_ShareMem_SystemV()
{
    if (shm_addr_)
    {
        close();
    }
}

//���ļ�������ӳ��
int ZCE_ShareMem_SystemV::open(key_t sysv_key,
                               std::size_t shm_size,
                               int shmget_flg,
                               int shmat_flg,
                               const void *want_address)
{
    //�����������open������������ֶ��Ա�ʾ��ε���open,
    ZCE_ASSERT(NULL == shm_addr_);
    ZCE_ASSERT(ZCE_INVALID_HANDLE == sysv_shmid_);

    sysv_shmid_ = zce::shmget(sysv_key, shm_size, shmget_flg);

    if (ZCE_INVALID_HANDLE == sysv_shmid_ )
    {
        ZCE_LOG(RS_ERROR, "[zcelib] System memory shmget fail ,sysv key = %d,last error =%d. ", sysv_key, zce::last_error());
        return -1;
    }

    shm_addr_ = zce::shmat(sysv_shmid_, want_address, shmat_flg);

    if (shm_addr_ == MAP_FAILED)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] System memory shmat fail ,last error =%d. ", zce::last_error());
        return -1;
    }

    sysv_key_ = sysv_key;
    shm_size_ = shm_size;

    return 0;
}

//���ļ�������ӳ��, ��
int ZCE_ShareMem_SystemV::open(key_t sysv_key,
                               std::size_t shm_size,
                               bool fail_if_exist,
                               bool read_only,
                               const void *want_address)
{

    int shmget_flg = 0;
    int shmat_flg = 0;

    //�����ֻ����ʽ�򿪹����ڴ�
    if (read_only)
    {
        shmget_flg = SHM_R;
        shmat_flg = SHM_RDONLY;
    }
    else
    {
        shmget_flg = SHM_R | SHM_W;
    }

    //ȷ���򿪷�ʽ��
    shmget_flg |= IPC_CREAT;

    if (fail_if_exist)
    {
        shmget_flg |= IPC_EXCL;
    }

    //�����ϣ�����õĵ�ַ��
    if (want_address)
    {
        shmat_flg |= SHM_RND;
    }

    //
    return this->open(sysv_key,
                      shm_size,
                      shmget_flg,
                      shmat_flg,
                      want_address);

}

//�ر��ļ�
int ZCE_ShareMem_SystemV::close()
{
    //���Ա�֤������û��open�͵���close�����
    ZCE_ASSERT(shm_addr_ != NULL );
    ZCE_ASSERT(sysv_shmid_ != ZCE_INVALID_HANDLE);

    int ret = 0;
    ret = zce::shmdt(shm_addr_);
    shm_addr_ = NULL;
    shm_size_ = 0;
    sysv_shmid_ = ZCE_INVALID_HANDLE;

    if (ret != 0)
    {
        return -1;
    }

    return 0;
}

//ɾ��ӳ����ļ�����Ȼ����ӳ���ʱ����ɾ��
int ZCE_ShareMem_SystemV::remove()
{
    return zce::shmctl(sysv_shmid_, IPC_RMID, NULL);
}

//����ӳ����ڴ��ַ
void *ZCE_ShareMem_SystemV::addr()
{
    return shm_addr_;
}

