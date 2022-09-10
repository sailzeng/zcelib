#pragma once
#include "zce/os_adapt/common.h"

//前向声明
namespace zce::mysql
{
class Connect;
class Result;
}

//!
namespace zce::aio
{
class Worker;

//!
enum AIO_TYPE
{
    AIO_INVALID = 0,
    //文件处理
    FS_BEGIN = 1,
    FS_OPEN,
    FS_CLOSE,
    FS_LSEEK,
    FS_READ,
    FS_WRITE,
    FS_READFILE,
    FS_WRITEFILE,
    FS_STAT,
    FS_FTRUNCATE,
    FS_UNLINK,
    FS_RENAME,
    FS_END = 99,

    DIR_BEGIN = 100,
    DIR_RMDIR,
    DIR_MKDIR,
    DIR_SCANDIR,
    DIR_END = 199,
    //
    MYSQL_BEGIN = 200,
    MYSQL_CONNECT,
    MYSQL_DISCONNECT,
    MYSQL_QUERY_NOSELECT,
    MYSQL_QUERY_SELECT,
    MYSQL_END = 299,
    //
    HOST_BEGIN = 300,
    HOST_GETADDRINFO_ONE,
    HOST_GETADDRINFO_ARY,
    HOST_END = 399,
};

//! AIO异步操作的原子
struct AIO_Atom
{
    virtual void clear() = 0;

    //!
    AIO_TYPE  aio_type_;
    //!
    uint32_t id_;
    //!
    std::function<void(AIO_Atom*)> call_back_;
};

//! FS文件操作的原子
struct FS_Atom :public AIO_Atom
{
    //!清理
    virtual void clear();
public:
    //!结果
    int result_ = -1;
    //路径，注意，这儿保存的是指针，在返回前对应的数据要能使用
    const char* path_ = nullptr;
    //!打开文件标志
    int flags_ = 0;
    //!打开文件模式
    int mode_ = 0;
    //!
    ZCE_HANDLE handle_ = ZCE_INVALID_HANDLE;

    //!文件偏移的参数
    ssize_t offset_ = 0;
    //!
    int whence_ = SEEK_CUR;

    //!读取，写入的buf，buf的长度，结果的长度
    char* read_bufs_ = nullptr;
    const char* write_bufs_ = nullptr;
    size_t bufs_count_ = 0;
    size_t result_count_ = 0;

    //!改名的路径
    const char* new_path_ = nullptr;
    //!文件stat
    struct stat* file_stat_ = nullptr;
};

//! 目录操作的原子
struct Dir_Atom :public AIO_Atom
{
    //!清理
    virtual void clear();
    //!结果
    int result_ = -1;
    //! 打开,处理的目录
    const char* dirname_ = nullptr;
    //!打开文件模式
    int mode_ = 0;
    //! scandir返回的dirent，数量看result_,你需要自己清理
    struct dirent*** namelist_ = nullptr;
};

//! 数据库操作原子
struct MySQL_Atom :public AIO_Atom
{
    //!
    virtual void clear() override;
    //!
    int result_ = -1;
    zce::mysql::Connect* db_connect_ = nullptr;
    const char* host_name_ = nullptr;
    const char* user_ = nullptr;
    const char* pwd_ = nullptr;
    unsigned int port_ = MYSQL_PORT;
    const char* sql_ = nullptr;
    size_t sql_len_ = 0;
    uint64_t* num_affect_ = nullptr;
    uint64_t* insert_id_ = nullptr;
    zce::mysql::Result* db_result_ = nullptr;
};

//!
struct Host_Atom :public AIO_Atom
{
    //!清理
    virtual void clear();
    //!参数
    int result_ = -1;
    const char* hostname_ = nullptr;
    const char* service_ = nullptr;
    size_t* ary_addr_num_ = nullptr;
    sockaddr_in* ary_addr_ = nullptr;
    size_t* ary_addr6_num_ = nullptr;
    sockaddr_in6* ary_addr6_ = nullptr;
    //
    sockaddr* addr_ = nullptr;
    socklen_t addr_len_ = 0;
};

//====================================================

//!异步打开某个文件，完成后回调函数call_back
int fs_open(zce::aio::Worker* worker,
            const char* path,
            int flags,
            int mode,
            std::function<void(AIO_Atom*)> call_back);

//!异步关闭某个文件，完成后回调函数call_back
int fs_close(zce::aio::Worker* worker,
             ZCE_HANDLE handle,
             std::function<void(AIO_Atom*)> call_back);

//!移动文件的读写位置,
int fs_lseek(zce::aio::Worker* worker,
             ZCE_HANDLE handle,
             off_t offset,
             int whence,
             std::function<void(AIO_Atom*)> call_back);

//!异步读取文件内容
int fs_read(zce::aio::Worker* worker,
            ZCE_HANDLE handle,
            char* read_bufs,
            size_t nbufs,
            std::function<void(AIO_Atom*)> call_back,
            ssize_t offset = 0,
            int whence = SEEK_CUR);

//!异步写入文件内容
int fs_write(zce::aio::Worker* worker,
             ZCE_HANDLE handle,
             const char* write_bufs,
             size_t nbufs,
             std::function<void(AIO_Atom*)> call_back,
             ssize_t offset = 0,
             int whence = SEEK_CUR);

//!异步截断文件
int fs_ftruncate(zce::aio::Worker* worker,
                 ZCE_HANDLE handle,
                 size_t offset,
                 std::function<void(AIO_Atom*)> call_back);

//!异步打开文件，读取文件内容，然后关闭
int fs_read_file(zce::aio::Worker* worker,
                 const char* path,
                 char* read_bufs,
                 size_t nbufs,
                 std::function<void(AIO_Atom*)> call_back,
                 ssize_t offset = 0);

//!异步打开文件，写入文件内容，然后关闭
int fs_write_file(zce::aio::Worker* worker,
                  const char* path,
                  const char* write_bufs,
                  size_t nbufs,
                  std::function<void(AIO_Atom*)> call_back,
                  ssize_t offset = 0);

//!异步删除文件
int fs_unlink(zce::aio::Worker* worker,
              const char* path,
              std::function<void(AIO_Atom*)> call_back);

//!异步改名
int fs_rename(zce::aio::Worker* worker,
              const char* path,
              const char* new_path,
              std::function<void(AIO_Atom*)> call_back);

//!异步获取stat
int fs_stat(zce::aio::Worker* worker,
            const char* path,
            struct stat* file_stat,
            std::function<void(AIO_Atom*)> call_back);

//! 异步scandir,参数参考scandir
//! namelist请使用，可以用free_scandir_list函数释放
int dir_scandir(zce::aio::Worker* worker,
                const char* dirname,
                struct dirent*** namelist,
                std::function<void(AIO_Atom*)> call_back);

//!异步建立dir
int dir_mkdir(zce::aio::Worker* worker,
              const char* dirname,
              int mode,
              std::function<void(AIO_Atom*)> call_back);

//!异步删除dir
int dir_rmdir(zce::aio::Worker* worker,
              const char* dirname,
              std::function<void(AIO_Atom*)> call_back);

//!链接数据
int mysql_connect(zce::aio::Worker* worker,
                  zce::mysql::Connect* db_connect,
                  const char* host_name,
                  const char* user,
                  const char* pwd,
                  unsigned int port, //默认填写MYSQL_PORT
                  std::function<void(AIO_Atom*)> call_back);

//!断开数据库链接
int mysql_disconnect(zce::aio::Worker* worker,
                     zce::mysql::Connect* db_connect,
                     std::function<void(AIO_Atom*)> call_back);

//!查询，非SELECT语句
int mysql_query(zce::aio::Worker* worker,
                zce::mysql::Connect* db_connect,
                const char* sql,
                size_t sql_len,
                uint64_t* num_affect,
                uint64_t* insert_id,
                std::function<void(AIO_Atom*)> call_back);

//!查询，SELECT语句
int mysql_query(zce::aio::Worker* worker,
                zce::mysql::Connect* db_connect,
                const char* sql,
                size_t sql_len,
                uint64_t* num_affect,
                zce::mysql::Result* db_result,
                std::function<void(AIO_Atom*)> call_back);

//!host 函数，用于获取地址，AIO_Atom 可以转化为Host_Atom使用

//!获得host对应的多个地址信息，包括IPV4，IPV6，类似getaddrinfo_ary，
int host_getaddr_ary(zce::aio::Worker* worker,
                     const char* hostname,
                     const char* service,
                     size_t* ary_addr_num,
                     sockaddr_in* ary_addr,
                     size_t* ary_addr6_num,
                     sockaddr_in6* ary_addr6,
                     std::function<void(AIO_Atom*)> call_back);

//!获得host对应的一个地址信息，类似getaddrinfo_one
int host_getaddr_one(zce::aio::Worker* worker,
                     const char* hostname,
                     const char* service,
                     sockaddr* addr,
                     socklen_t addr_len,
                     std::function<void(AIO_Atom*)> call_back);

//========================================================================================
//
//AIO FS文件处理相关的awaiter等待体
template <typename RA>
struct awaiter
{
    typedef awaiter<RA> self;

    awaiter(zce::aio::Worker* worker,
            RA* request_atom) :
        worker_(worker),
        request_atom_(request_atom)
    {
    }
    ~awaiter() = default;

    //请求进行AIO操作，如果请求成功.return false挂起协程
    bool await_ready()
    {
        //绑定回调函数
        request_atom_->call_back_ = std::bind(&self::resume,
                                              this,
                                              std::placeholders::_1);
        //将一个文件操作句柄放入请求队列
        bool succ_req = worker_->request(request_atom_);
        if (succ_req)
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    //挂起操作
    void await_suspend(std::coroutine_handle<> awaiting)
    {
        awaiting_ = awaiting;
    }
    //!回复后的操作。恢复后返回结果
    RA await_resume()
    {
        return return_atom_;
    }
    //!回调函数，AIO操作完成后恢复时调用
    void resume(AIO_Atom* return_hdl)
    {
        RA* fs_hdl = (RA*)return_hdl;
        return_atom_ = *fs_hdl;
        awaiting_.resume();
        return;
    }

    //!工作者，具有请求，应答管道，处理IO多线程的管理者
    zce::aio::Worker* worker_ = nullptr;
    //!请求的文件操作句柄
    RA* request_atom_ = nullptr;
    //!完成后返回的句柄
    RA return_atom_;
    //!协程的句柄（调用者）
    std::coroutine_handle<> awaiting_;
};

typedef zce::aio::awaiter<zce::aio::FS_Atom> awaiter_fs;
typedef zce::aio::awaiter<zce::aio::Dir_Atom> awaiter_dir;
typedef zce::aio::awaiter<zce::aio::MySQL_Atom> awaiter_mysql;
typedef zce::aio::awaiter<zce::aio::Host_Atom> awaiter_host;

//========================================================================================
//AIO 协程的co_await 函数

//!协程co_await AIO读取文件
awaiter_fs co_read_file(zce::aio::Worker* worker,
                        const char* path,
                        char* read_bufs,
                        size_t nbufs,
                        ssize_t offset = 0);
//!协程co_await AIO写入文件
awaiter_fs co_write_file(zce::aio::Worker* worker,
                         const char* path,
                         const char* write_bufs,
                         size_t nbufs,
                         ssize_t offset = 0);

//!协程co_await 链接数据
awaiter_mysql co_mysql_connect(zce::aio::Worker* worker,
                               zce::mysql::Connect* db_connect,
                               const char* host_name,
                               const char* user,
                               const char* pwd,
                               unsigned int port);

//!协程co_await 断开数据库链接
awaiter_mysql co_mysql_disconnect(zce::aio::Worker* worker,
                                  zce::mysql::Connect* db_connect);

//!协程co_await 查询，非SELECT语句
awaiter_mysql co_mysql_query(zce::aio::Worker* worker,
                             zce::mysql::Connect* db_connect,
                             const char* sql,
                             size_t sql_len,
                             uint64_t* num_affect,
                             uint64_t* insert_id);

//!协程co_await 查询，SELECT语句
awaiter_mysql co_mysql_query(zce::aio::Worker* worker,
                             zce::mysql::Connect* db_connect,
                             const char* sql,
                             size_t sql_len,
                             uint64_t* num_affect,
                             zce::mysql::Result* db_result);

//!协程co_await getaddrinfo_ary
awaiter_host co_host_getaddr_ary(zce::aio::Worker* worker,
                                 const char* hostname,
                                 const char* service,
                                 size_t* ary_addr_num,
                                 sockaddr_in* ary_addr,
                                 size_t* ary_addr6_num,
                                 sockaddr_in6* ary_addr6);

//!协程co_await ，类似getaddrinfo_one
awaiter_host co_host_getaddr_one(zce::aio::Worker* worker,
                                 const char* hostname,
                                 const char* service,
                                 sockaddr* addr,
                                 socklen_t addr_len);
}//namespace zce::aio