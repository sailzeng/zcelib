#include "zce/predefine.h"
#include "zce/os_adapt/define.h"
#include "zce/util/non_copyable.h"
#include "zce/logger/logging.h"
#include "zce/os_adapt/shm.h"
#include "zce/os_adapt/file.h"
#include "zce/os_adapt/dirent.h"
#include "zce/os_adapt/error.h"

#include "zce/shared_mem/systemv.h"

//我现在有点理解大家为什么喜欢用System V的共享内存了，简单。因为没有文件映射，所以其实他少了很多参数
namespace zce
{
//构造函数
shm_systemv::shm_systemv() :
    sysv_key_(0),
    sysv_shmid_(ZCE_INVALID_HANDLE),
    shm_size_(0),
    shm_addr_(nullptr)
{
}

shm_systemv::~shm_systemv()
{
    if (shm_addr_)
    {
        close();
    }
}

//打开文件，进行映射
int shm_systemv::open(key_t sysv_key,
                      std::size_t shm_size,
                      int shmget_flg,
                      int shmat_flg,
                      const void* want_address)
{
    //避免重入调用open函数，如果出现断言表示多次调用open,
    ZCE_ASSERT(nullptr == shm_addr_);
    ZCE_ASSERT(ZCE_INVALID_HANDLE == sysv_shmid_);

    sysv_shmid_ = zce::shmget(sysv_key, shm_size, shmget_flg);

    if (ZCE_INVALID_HANDLE == sysv_shmid_)
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

//打开文件，进行映射, 简单
int shm_systemv::open(key_t sysv_key,
                      std::size_t shm_size,
                      bool fail_if_exist,
                      bool read_only,
                      const void* want_address)
{
    int shmget_flg = 0;
    int shmat_flg = 0;

    //如果是只读方式打开共享内存
    if (read_only)
    {
        shmget_flg = SHM_R;
        shmat_flg = SHM_RDONLY;
    }
    else
    {
        shmget_flg = SHM_R | SHM_W;
    }

    //确定打开方式，
    shmget_flg |= IPC_CREAT;

    if (fail_if_exist)
    {
        shmget_flg |= IPC_EXCL;
    }

    //如果有希望设置的地址，
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

//关闭文件
int shm_systemv::close()
{
    //断言保证不出现没有open就调用close的情况
    ZCE_ASSERT(shm_addr_ != nullptr);
    ZCE_ASSERT(sysv_shmid_ != ZCE_INVALID_HANDLE);

    int ret = 0;
    ret = zce::shmdt(shm_addr_);
    shm_addr_ = nullptr;
    shm_size_ = 0;
    sysv_shmid_ = ZCE_INVALID_HANDLE;

    if (ret != 0)
    {
        return -1;
    }

    return 0;
}

//删除映射的文件，当然正在映射的时候不能删除
int shm_systemv::remove()
{
    return zce::shmctl(sysv_shmid_, IPC_RMID, nullptr);
}

//返回映射的内存地址
void* shm_systemv::addr()
{
    return shm_addr_;
}
}