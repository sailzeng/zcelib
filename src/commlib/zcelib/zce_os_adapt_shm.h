
#ifndef ZCE_LIB_OS_ADAPT_SHARE_MEMORY_H_
#define ZCE_LIB_OS_ADAPT_SHARE_MEMORY_H_



#include "zce_os_adapt_predefine.h"

namespace zce
{
//-------------------------------------------------------------------------------------------------
//POSIX�Ĺ����ڴ�

//�������ڴ���ļ�����ӳ��
void *mmap (void *addr,
            size_t len,
            int prot,
            int flags,
            ZCE_HANDLE handle,
            size_t off = 0);

//
int mprotect (const void *addr,
              size_t len,
              int prot);

//���ڴ�仯ͬ�����ļ���ȥ����������Ƶ���������������OS���Զ���������飬
//һ������������˳�ʱ����һ�ξ�OK�ˡ�
int msync (void *addr,
           size_t len,
           int sync = MS_SYNC);

//����ļ�ӳ��
int munmap (void *addr,
            size_t len);

//int madvise (caddr_t addr,
//    size_t len,
//    int map_advice);

//Posix�ڴ��open����shm_open�����ļ����ƣ���û��ʵ���ļ���ӳ��
//Create/open POSIX shared memory objects
ZCE_HANDLE shm_open (const char *filename,
                     int mode,
                     mode_t perms = 0);

//Posix�ڴ��ɾ���ļ�
////Create/open POSIX shared memory objects
int shm_unlink (const char *path);


//-------------------------------------------------------------------------------------------------
//SystemV�Ĺ����ڴ�

//�ṩ����ģ�������˵������Ϊ�˺��棬����ȻҲ����System V�й������Ⱥ�ڻ���������������ֲ��������һ�㣩
//�Ҹ��˶�System V��IPCû�а���һ����Ͼ�����POSIX IPC�ڱ�׼��վס�˽ţ�System V��IPC�ⷽ��Ҫ��һ�㣬
//��һ����System V IPC �Ľӿ����Ҳ����POSIX��ô���ţ�


/*!
* @brief      �������߷���һ�������ڴ���
* @return     ZCE_HANDLE
* @param      sysv_key
* @param      size
* @param      shmflg
* @note
*/
ZCE_HANDLE shmget(key_t sysv_key, size_t size, int shmflg);


/*!
* @brief      ���Ѿ�shmget�Ĺ����ڴ���
* @return     void*
* @param      shmid
* @param      shmaddr
* @param      shmflg
* @note
*/
void *shmat(ZCE_HANDLE shmid, const void *shmaddr, int shmflg);


/*!
* @brief      �̽�����ڴ���
* @return     int
* @param      shmaddr
* @note
*/
int shmdt(const void *shmaddr);


/*!
* @brief      �Թ����ڴ����ṩ���ֲ���
* @return     int
* @param      shmid
* @param      cmd
* @param      buf
* @note
*/
int shmctl(ZCE_HANDLE shmid, int cmd, struct shmid_ds *buf);


};



#endif //ZCE_LIB_OS_ADAPT_SHARE_MEMORY_H_


