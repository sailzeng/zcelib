#ifndef ZCE_LIB_SHARE_MEMORY_MMAP_H_
#define ZCE_LIB_SHARE_MEMORY_MMAP_H_

#include "zce_os_adapt_predefine.h"
#include "zce_boost_non_copyable.h"

/*********************************************************************************
class ZCE_ShareMem_Mmap ��װMMAP�����ڴ�ӳ���ļ�����
*********************************************************************************/
class ZCE_ShareMem_Mmap: public ZCE_NON_Copyable
{
public:
    //���캯��
    ZCE_ShareMem_Mmap();
    ~ZCE_ShareMem_Mmap();

public:
    //���ļ�������ӳ��
    int open(const char *file_name,
             std::size_t shm_size,
             int file_open_mode = O_CREAT | O_RDWR,
             int file_perms_mode = ZCE_SHARE_FILE_PERMS,
             const void *want_address = NULL,
             int mmap_prot = PROT_READ | PROT_WRITE,
             int mmap_flags = MAP_SHARED,
             std::size_t offset = 0
            );

    //���ļ�������ӳ��, �򵥣��Ƽ�ʹ���������
    int open(const char *file_name,
             std::size_t shm_size,
             bool if_restore,
             bool read_only = false,
             bool share_file = true,
             const void *want_address = NULL,
             std::size_t  offset = 0
            );

    //�ر��ļ�
    int close();

    //ɾ��ӳ����ļ�����Ȼ����ӳ���ʱ����ɾ��
    int remove();

    //ͬ���ļ�
    int flush();

    ///����ӳ����ڴ��ַ
    inline void *addr()
    {
        return mmap_addr_;
    }

    ///�����ļ�����
    inline const char *file_name()
    {
        return mmap_file_name_.c_str();
    }
protected:

    //ӳ���ļ�������
    std::string         mmap_file_name_;

    //ӳ����ڴ��ַ
    void               *mmap_addr_;

    // ӳ����ļ����
    ZCE_HANDLE          mmap_handle_;

    //ӳ��Ĺ����ڴ��С
    std::size_t         shm_size_;
};

#endif //ZCE_LIB_SHARE_MEMORY_MMAP_H_

