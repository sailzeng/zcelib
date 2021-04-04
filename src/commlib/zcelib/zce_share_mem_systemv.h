
#ifndef ZCE_LIB_SHARE_MEMORY_SYSTEM_V_H_
#define ZCE_LIB_SHARE_MEMORY_SYSTEM_V_H_

#include "zce_os_adapt_predefine.h"

/*********************************************************************************
class ZCE_ShareMem_SystemV ��װSystemV�����ڴ�ӳ����룬��POSIX��MMAP����������ǣ�û��ӳ���ļ�
*********************************************************************************/
class ZCE_ShareMem_SystemV : public ZCE_NON_Copyable
{

public:
    //���캯��
    ZCE_ShareMem_SystemV();
    ~ZCE_ShareMem_SystemV();

public:
    //���ļ�������ӳ��
    int open(key_t sysv_key,
             std::size_t shm_size,
             int shmget_flg = IPC_CREAT | SHM_R | SHM_W,
             int shmat_flg = 0,
             const void *want_address = NULL
            );

    //���ļ�������ӳ��, �򵥣��Ƽ�ʹ���������
    int open(key_t sysv_key,
             std::size_t shm_size,
             bool fail_if_exist,
             bool read_only = false,
             const void *want_address = NULL
            );

    //�ر�ӳ��(�ļ�)
    int close();

    //ɾ��ӳ����ļ�����Ȼ����ӳ��(���߻�������ӳ��)��ʱ����ɾ��
    int remove();

    //����ӳ����ڴ��ַ
    void *addr();

protected:

    //key_t�������ڴ��Ψһ��ʶ�������Ǹ������ѡ�
    key_t               sysv_key_;

    // ӳ����ļ����
    ZCE_HANDLE          sysv_shmid_;

    //ӳ��Ĺ����ڴ��С
    std::size_t         shm_size_;

    //ӳ����ڴ��ַ
    void               *shm_addr_;
};

#endif //ZCE_LIB_SHARE_MEMORY_SYSTEM_V_H_

