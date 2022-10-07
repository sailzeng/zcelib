/*!
* @copyright  2004-2021  Apache License, Version 2.0 FULLSAIL
* @filename   zce/net/dns_resolve.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2022.08
* @brief
* @details
*
*
*
* @note
*
* 祝酒歌
* 美酒飘香啊歌声飞
* 朋友啊请你干一杯请你干一杯
* 胜利的十月永难忘
* 杯中洒满幸福泪
* 来来来来来
* 十月里响春雷
* 八亿神州举金杯
* 舒心的酒啊浓又美
* 千杯万盏也不醉
* 手捧美酒啊望北京
* 豪情啊胜过长江水胜过长江水
* 祖国前程似锦绣
* 万里山河尽朝晖
* 来来来来来
* 瞻未来无限美
* 人人胸中春风吹
* 美酒浇旺心头火
* 燃得斗志永不退
* 今天啊畅饮胜利酒
* 明日啊上阵劲百倍
* 为了实现四个现代化
* 甘洒热血和汗水
* 来来来来来
* 征途上战鼓擂
* 条条战线捷报飞
* 待到理想化宏图
* 再重摆美酒再相会
* 来来来来来
* 再重摆美酒再相会
*/

#pragma once

#include "zce/event/handle_base.h"
#include "zce/os_adapt/common.h"

namespace zce
{
class time_value;
}
//前向声明
namespace zce::mysql
{
class connect;
class result;
}

//!
namespace zce::aio
{
class worker;

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
    //MYSQL
    MYSQL_BEGIN = 200,
    MYSQL_CONNECT,
    MYSQL_DISCONNECT,
    MYSQL_QUERY_NOSELECT,
    MYSQL_QUERY_SELECT,
    MYSQL_END = 299,
    //通过域名取得地址
    HOST_BEGIN = 300,
    HOST_GETADDRINFO_ONE,
    HOST_GETADDRINFO_ARY,
    HOST_END = 399,
    //
    SOCKET_BEGIN = 400,
    SOCKET_CONNECT_ADDR,
    SOCKET_CONNECT_HOST,
    SOCKET_SEND,
    SOCKET_RECV,
    SOCKET_ACCEPT,
    SOCKET_RECVFROM,
    SOCKET_SENDTO,  //sendto是非阻塞函数，直接调用也行
    SOCKET_END = 399,
};

//! AIO异步操作的原子
struct AIO_ATOM
{
    virtual void clear() = 0;

    //!
    AIO_TYPE  aio_type_;
    //!
    uint32_t id_;
    //!
    std::function<void(AIO_ATOM*)> call_back_;
};

//=========================================================================
//! FS文件操作的原子
struct FS_ATOM :public AIO_ATOM
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

//!异步打开某个文件，完成后回调函数call_back
int fs_open(zce::aio::worker* worker,
            const char* path,
            int flags,
            int mode,
            std::function<void(AIO_ATOM*)> call_back);

//!异步关闭某个文件，完成后回调函数call_back
int fs_close(zce::aio::worker* worker,
             ZCE_HANDLE handle,
             std::function<void(AIO_ATOM*)> call_back);

//!移动文件的读写位置,
int fs_lseek(zce::aio::worker* worker,
             ZCE_HANDLE handle,
             off_t offset,
             int whence,
             std::function<void(AIO_ATOM*)> call_back);

//!异步读取文件内容
int fs_read(zce::aio::worker* worker,
            ZCE_HANDLE handle,
            char* read_bufs,
            size_t nbufs,
            std::function<void(AIO_ATOM*)> call_back,
            ssize_t offset = 0,
            int whence = SEEK_CUR);

//!异步写入文件内容
int fs_write(zce::aio::worker* worker,
             ZCE_HANDLE handle,
             const char* write_bufs,
             size_t nbufs,
             std::function<void(AIO_ATOM*)> call_back,
             ssize_t offset = 0,
             int whence = SEEK_CUR);

//!异步截断文件
int fs_ftruncate(zce::aio::worker* worker,
                 ZCE_HANDLE handle,
                 size_t offset,
                 std::function<void(AIO_ATOM*)> call_back);

//!异步打开文件，读取文件内容，然后关闭
int fs_read_file(zce::aio::worker* worker,
                 const char* path,
                 char* read_bufs,
                 size_t nbufs,
                 std::function<void(AIO_ATOM*)> call_back,
                 ssize_t offset = 0);

//!异步打开文件，写入文件内容，然后关闭
int fs_write_file(zce::aio::worker* worker,
                  const char* path,
                  const char* write_bufs,
                  size_t nbufs,
                  std::function<void(AIO_ATOM*)> call_back,
                  ssize_t offset = 0);

//!异步删除文件
int fs_unlink(zce::aio::worker* worker,
              const char* path,
              std::function<void(AIO_ATOM*)> call_back);

//!异步改名
int fs_rename(zce::aio::worker* worker,
              const char* path,
              const char* new_path,
              std::function<void(AIO_ATOM*)> call_back);

//!异步获取stat
int fs_stat(zce::aio::worker* worker,
            const char* path,
            struct stat* file_stat,
            std::function<void(AIO_ATOM*)> call_back);

//=========================================================================
//! 目录操作的原子
struct DIR_ATOM :public AIO_ATOM
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

//! 异步scandir,参数参考scandir，namelist可以用free_scandir_list要释放
int dir_scandir(zce::aio::worker* worker,
                const char* dirname,
                struct dirent*** namelist,
                std::function<void(AIO_ATOM*)> call_back);

//!异步建立dir
int dir_mkdir(zce::aio::worker* worker,
              const char* dirname,
              int mode,
              std::function<void(AIO_ATOM*)> call_back);

//!异步删除dir
int dir_rmdir(zce::aio::worker* worker,
              const char* dirname,
              std::function<void(AIO_ATOM*)> call_back);

//=========================================================================
//! 数据库操作原子
struct MYSQL_ATOM :public AIO_ATOM
{
    //!
    virtual void clear() override;
    //!
    int result_ = -1;
    zce::mysql::connect* db_connect_ = nullptr;
    const char* host_name_ = nullptr;
    const char* user_ = nullptr;
    const char* pwd_ = nullptr;
    unsigned int port_ = MYSQL_PORT;
    const char* sql_ = nullptr;
    size_t sql_len_ = 0;
    uint64_t* num_affect_ = nullptr;
    uint64_t* insert_id_ = nullptr;
    zce::mysql::result* db_result_ = nullptr;
};

//!链接数据
int mysql_connect(zce::aio::worker* worker,
                  zce::mysql::connect* db_connect,
                  const char* host_name,
                  const char* user,
                  const char* pwd,
                  unsigned int port, //默认填写MYSQL_PORT
                  std::function<void(AIO_ATOM*)> call_back);

//!断开数据库链接
int mysql_disconnect(zce::aio::worker* worker,
                     zce::mysql::connect* db_connect,
                     std::function<void(AIO_ATOM*)> call_back);

//!查询，非SELECT语句
int mysql_query(zce::aio::worker* worker,
                zce::mysql::connect* db_connect,
                const char* sql,
                size_t sql_len,
                uint64_t* num_affect,
                uint64_t* insert_id,
                std::function<void(AIO_ATOM*)> call_back);

//!查询，SELECT语句
int mysql_query(zce::aio::worker* worker,
                zce::mysql::connect* db_connect,
                const char* sql,
                size_t sql_len,
                uint64_t* num_affect,
                zce::mysql::result* db_result,
                std::function<void(AIO_ATOM*)> call_back);

//=========================================================================
//!
struct HOST_ATOM :public AIO_ATOM
{
    //!清理
    virtual void clear();
    //!参数
    int result_ = -1;
    const char* hostname_ = nullptr;
    const char* service_ = nullptr;
    size_t* ary_addr_num_ = nullptr;
    ::sockaddr_in* ary_addr_ = nullptr;
    size_t* ary_addr6_num_ = nullptr;
    ::sockaddr_in6* ary_addr6_ = nullptr;
    //
    sockaddr* addr_ = nullptr;
    socklen_t addr_len_ = 0;
};

//!host 函数，用于获取地址，AIO_Atom 可以转化为Host_Atom使用

//!获得host对应的多个地址信息，包括IPV4，IPV6，类似getaddrinfo_ary，
int host_getaddr_ary(zce::aio::worker* worker,
                     const char* hostname,
                     const char* service,
                     size_t* ary_addr_num,
                     sockaddr_in* ary_addr,
                     size_t* ary_addr6_num,
                     sockaddr_in6* ary_addr6,
                     std::function<void(AIO_ATOM*)> call_back);

//!获得host对应的一个地址信息，类似getaddrinfo_one
int host_getaddr_one(zce::aio::worker* worker,
                     const char* hostname,
                     const char* service,
                     sockaddr* addr,
                     socklen_t addr_len,
                     std::function<void(AIO_ATOM*)> call_back);

//=========================================================================
//! Socket atom
struct SOCKET_ATOM :public AIO_ATOM
{
    //!清理
    virtual void clear();
    //!参数
    int result_ = -1;
    size_t result_count_ = 0;
    //
    ZCE_SOCKET handle_ = ZCE_INVALID_SOCKET;
    const sockaddr* addr_ = nullptr;
    socklen_t addr_len_ = 0;
    const void* snd_buf_ = nullptr;
    void* rcv_buf_ = nullptr;
    size_t len_ = 0;
    zce::time_value* timeout_tv_ = nullptr;

    int flags_ = 0;
    sockaddr* from_ = nullptr;
    socklen_t* from_len_ = nullptr;
    const char* host_name_ = nullptr;
    uint16_t host_port_ = 0;
    sockaddr* host_addr_ = nullptr;
    ZCE_SOCKET accept_hdl_ = ZCE_INVALID_SOCKET;
};

//! ST = socket timeout
//! 等待若干时间进行connect，直至超时
int st_connect(zce::aio::worker* worker,
               ZCE_SOCKET handle,
               const sockaddr* addr,
               socklen_t addr_len,
               zce::time_value* timeout_tv,
               std::function<void(AIO_ATOM*)> call_back);

int st_connect(zce::aio::worker* worker,
               ZCE_SOCKET handle,
               const char* host_name,
               uint16_t port,
               sockaddr* host_addr,
               socklen_t addr_len,
               zce::time_value& timeout_tv,
               std::function<void(AIO_ATOM*)> call_back);

//! 等待若干时间进行accept，直至超时
int st_accept(zce::aio::worker* worker,
              ZCE_SOCKET handle,
              sockaddr* addr,
              socklen_t* addr_len,
              zce::time_value* timeout_tv,
              std::function<void(AIO_ATOM*)> call_back);

//! 等待若干时间进行recv，直至超时
int st_recv(zce::aio::worker* worker,
            ZCE_SOCKET handle,
            void* buf,
            size_t len,
            zce::time_value* timeout_tv,
            std::function<void(AIO_ATOM*)> call_back,
            int flags = 0);

//!等待若干时间进行send，直至超时
int st_send(zce::aio::worker* worker,
            ZCE_SOCKET handle,
            const void* buf,
            size_t len,
            zce::time_value* timeout_tv,
            std::function<void(AIO_ATOM*)> call_back,
            int flags = 0);

//!等待若干时间进行recv数据，直至超时
int st_recvfrom(zce::aio::worker* worker,
                ZCE_SOCKET handle,
                void* buf,
                size_t len,
                sockaddr* from,
                socklen_t* from_len,
                zce::time_value* timeout_tv,
                std::function<void(AIO_ATOM*)> call_back,
                int flags = 0);

//!用超时机制发起send数据,注意，注意，UDP，直接用sendto就可以了。

//=========================================================================
//!
struct AIO_EVENT :public zce::event_handler
{
    //读取事件触发调用函数
    virtual int read_event();

    //写入事件触发调用函数，用于写入事件
    virtual int write_event();

    //发生了链接的事件
    virtual int connect_event(bool success);

    //发生了accept的事件是调用
    virtual int accept_event();

    //!清理
    virtual void clear();
    //!参数
    int result_ = -1;
    size_t result_count_ = 0;
    //
    ZCE_SOCKET handle_ = ZCE_INVALID_SOCKET;
    const sockaddr* addr_ = nullptr;
    socklen_t addr_len_ = 0;
    const void* snd_buf_ = nullptr;
    void* rcv_buf_ = nullptr;
    size_t len_ = 0;
    zce::time_value* timeout_tv_ = nullptr;

    int flags_ = 0;
    sockaddr* from_ = nullptr;
    socklen_t* from_len_ = nullptr;
    const char* host_name_ = nullptr;
    uint16_t host_port_ = 0;
    sockaddr* host_addr_ = nullptr;
    ZCE_SOCKET accept_hdl_ = ZCE_INVALID_SOCKET;

    virtual ZCE_HANDLE get_handle() const
    {
        return (ZCE_HANDLE)handle_;
    }
};

//! ER = event reactor
//! 等待若干时间进行connect，直至超时
int er_connect(zce::aio::worker* worker,
               ZCE_SOCKET handle,
               const sockaddr* addr,
               socklen_t addr_len,
               std::function<void(AIO_EVENT*)> call_back);

//! 等待若干时间进行accept，直至超时
int er_accept(zce::aio::worker* worker,
              ZCE_SOCKET handle,
              sockaddr* addr,
              socklen_t* addr_len,
              std::function<void(AIO_EVENT*)> call_back);

//! 等待若干时间进行recv，直至超时
int er_recv(zce::aio::worker* worker,
            ZCE_SOCKET handle,
            void* buf,
            size_t len,
            std::function<void(AIO_EVENT*)> call_back);

//!等待若干时间进行send，直至超时
int er_send(zce::aio::worker* worker,
            ZCE_SOCKET handle,
            const void* buf,
            size_t len,
            std::function<void(AIO_EVENT*)> call_back);

//!等待若干时间进行recv数据，直至超时
int er_recvfrom(zce::aio::worker* worker,
                ZCE_SOCKET handle,
                void* buf,
                size_t len,
                sockaddr* from,
                socklen_t* from_len,
                std::function<void(AIO_EVENT*)> call_back);
}//namespace zce::aio