/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/os_adapt/common.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2011年5月1日
* @brief      定义各种跨平台的结构，枚举，宏，全局变量定义
*
* @details    当时设计时有2个选择，一个是，把各个定义分散放到具体的每个zce/os_adapt/XXXXXX.h文件中
*             一个是放在一个头文件里面，开始是分散，但最后发现部分定义要多人一起使用，所以最好还是搞了
*             统一定义这个文件。所以几乎所有的跨平台的基础结构都放在这儿，
*             但写到后面，发现还是有意外情况，少量代码如果放到这儿会和其他一些人的跨平台代码冲突，所以
*             少量定义还是放到各自的文件中。并不是因为混乱。
* @note       我尽量不使用off_t这个类型，因为在WIN平台下，他被定义成long，而WIN平台下long是不扩展的，
*             WINDOWS下的types.h已经有定义，
*             typedef off_t long;
*             大部分情况我用ssize_t替换off_t
*/

#pragma once

//因为我我用预编译头文件的缘故，所以这儿不用包括这个文件

/*!
* @namespace zce 跨平台的函数的名字空间定义，其中的函数主要向LINUX，POSIX看起，也有少量
*                    自己写的函数，
*                    主要覆盖面，包括网络，文件，目录，动态库，同步对象，线程，共享内存，时间
*                    字符串，机器性能等。
*/

#if !defined (ZCE_INVALID_FILE_DESCRIBE)
# define ZCE_INVALID_FILE_DESCRIBE  -1
#endif

//------------------------------------------------------------------------------------------
//WINDOWS 的版本如何搞
#if defined (ZCE_OS_WINDOWS)

//多进程间共享
#ifndef PTHREAD_PROCESS_SHARED
#define PTHREAD_PROCESS_SHARED          0x1
#endif

//进程内部
#ifndef PTHREAD_PROCESS_PRIVATE
#define PTHREAD_PROCESS_PRIVATE         0x2
#endif

//普通的
#ifndef PTHREAD_MUTEX_NORMAL
#define PTHREAD_MUTEX_NORMAL            0x0
#endif
//错误检查的互斥量，
#ifndef PTHREAD_MUTEX_ERRORCHECK
#define PTHREAD_MUTEX_ERRORCHECK        0x4
#endif

//递归的互斥量
#ifndef PTHREAD_MUTEX_RECURSIVE
#define PTHREAD_MUTEX_RECURSIVE         0x8
#endif

//自定义的，超时处理，WIN下的临界区不支持超时
#ifndef PTHREAD_MUTEX_TIMEOUT
#define PTHREAD_MUTEX_TIMEOUT           0x100000
#endif

#ifdef ZCE_OS_WINDOWS
#pragma warning ( disable : 26495)
#endif

typedef struct sem_t
{
    //信号量的句柄
    HANDLE  sem_hanlde_ = INVALID_HANDLE_VALUE;

    //信号量是否是一个无名的信号灯
    bool    sem_unnamed_ = false;
};

#if !defined (SEM_FAILED)
#  define SEM_FAILED ((sem_t *) -1)
#endif

typedef struct
{
    // Either PTHREAD_PROCESS_SHARED or PTHREAD_PROCESS_PRIVATE
    int lock_shared_;
    //锁定类型，主要检查是否是PTHREAD_MUTEX_RECURSIVE，
    int lock_type_;

    //互斥量的名字，如果是多进程的互斥量，就必须有名字
    char mutex_name_[PATH_MAX + 1];
} pthread_mutexattr_t;

///
struct pthread_mutex_t
{
    /// Either PTHREAD_PROCESS_SHARED or PTHREAD_PROCESS_PRIVATE
    int lock_shared_;
    //锁定类型，主要检查是否是PTHREAD_MUTEX_RECURSIVE，
    ///这个PTHREAD_MUTEX_TIMEOUT是我们自己为WIN定制的
    int lock_type_;

    ///线程使用了2种不同的方式，如果不需要非递归，和超时，我们用临界区
    ///否则用互斥量
    union
    {
        //MUTEX的HANDLE,进程，线程都可以用，有递归特性
        HANDLE           recursive_mutex_;
        //临界区性能好，如果不要超时，而且不要多进程，不要非递归时选择
        CRITICAL_SECTION thr_nontimeout_mutex_;
        //非递归锁，用信号灯模拟，
        sem_t* non_recursive_mutex_;
    };
};

//在WINDOWS下用临界区（进程内）+SPIN 或者Mutex（进程间）模拟SPIN lock
//注意这个临界区的行为我加入了SPIN功能，
typedef pthread_mutex_t  pthread_spinlock_t;

struct pthread_condattr_t
{
    // 外部锁定类型，如果需要timeout,增加这个 PTHREAD_MUTEX_TIMEOUT，否则会使用临界区
    int outer_lock_type_;

    //互斥量的名字，如果是多进程的互斥量，就必须有名字
    char cv_name_[PATH_MAX + 1];
};

struct win_simulate_cv_t
{
    /// 等待者的数量
    int                  waiters_ = 0;

    /// 保存进行的解锁操作是broadcast进行的还是signal进行
    bool                 was_broadcast_ = false;

    /// waiters 的计数的保护锁
    pthread_mutex_t      waiters_lock_;

    /// 信号灯，阻塞排队等待的线程直到 signaled.
    sem_t* block_sema_ = NULL;

    ///完成广播后的通知，这个地方用sema其实并不利于公平性，用EVENT更好一点。
    ///但由于要求广播的时候外部锁必现加上，所以问题也不太大，
    sem_t* finish_broadcast_ = NULL;
};

struct pthread_cond_t
{
    ///外部锁定类型，是否需要TIMEOUT，等，PTHREAD_MUTEX_TIMEOUT
    int                      outer_lock_type_ = 0;

    //两个架构，实际起作用的只有一个

    ///WINDOWS的条件变量在WINSERVER2008，VISTA后才支持
    ///这个条件变量只能单进程内部使用，其外部互斥量，只支持临界区
#if defined ZCE_SUPPORT_WINSVR2008 && ZCE_SUPPORT_WINSVR2008 == 1
    CONDITION_VARIABLE   cv_object_;
#endif
    ///采用两个信号灯和一个互斥量模拟的条件变量，同时支持signal和broadcast操作，
    ///也支持外部互斥量是多进程共享，也支持外部互斥量是MUTEX（信号灯），临界区模拟
    ///的，pthread_mutex_t
    win_simulate_cv_t     simulate_cv_;
};

//读写锁的代码来自UNP V2

///读写锁的属性结构，
typedef struct
{
    //
    bool            priority_to_write_;
} pthread_rwlockattr_t;

///读写锁的对象结构，利用互斥量，条件变量实现的读写锁
struct pthread_rwlock_t
{
    ///模拟的
    ///是否是唤醒写入优先，（是就是写入优先，否则读取优先）这是一个问题，我把抉择权利给你
    bool            priority_to_write_ = true;

    ///保护这个结构在多线程中读写的互斥量，主要下面那些整数的修改
    pthread_mutex_t rw_mutex_;

    /// 读者等待的条件变量
    pthread_cond_t  rw_condreaders_;
    /// 写入等待的条件变量
    pthread_cond_t  rw_condwriters_;

    ///等待读的线程数量
    int             rw_nwaitreaders_ = 0;
    ///等待写的线程数量
    int             rw_nwaitwriters_ = 0;

    ///锁的持有状态，如果有一个写者持有锁-1 如果>0表示多少个读者持有这个锁
    int             rw_refcount_ = 0;
};

#ifdef ZCE_OS_WINDOWS
#pragma warning ( default : 26495)
#endif

typedef unsigned int          mode_t;

typedef struct __stat64       zce_os_stat;

//注意S_IFDIR 和 S_ISDIR 的区别，S_ISDIR是一个宏用于判定是否是一个目录

#if !defined(S_IFDIR)
#define S_IFDIR  _S_IFDIR
#endif

#if !defined(S_IFREG)
#define S_IFREG  _S_IFREG
#endif

//zce_os_stat 在WINDOWS下被定义成__stat64，LINUX下被定义成stat,
//我简单介绍一下两个平台共有，而且大家一般会用到的属性

//文件模式 S_IFREG 普通文件 S_IFDIR 目录,
//[Linux]    mode_t    st_mode;
//[WIN]      unsigned short st_mode;
//    文件长度,字节
//[Linux]    off_t     st_size;
//[WIN]      __int64    st_size;

//最后访问时间
//[Linux]    time_t    st_atime;
//[WIN]      __time64_t st_atime;
//最后修改时间
//[Linux]    time_t    st_mtime;
//[WIN]      __time64_t st_mtime;
//WIN创建时间,LINUX,状态修改时间
//其实这个值在LINUX下不是创建时间，而是change time，标识状态改变的时间，inode修改时，这个值改变，
//由于文件的各种属性也在INODE里面就，各种属性(chmod，chown)修改时，这个时间也会变化
//[Linux]    time_t    st_ctime;
//[WIN]      __time64_t st_ctime;

/* Share changes */
#define MAP_SHARED      0x01
/* Changes are private */
#define MAP_PRIVATE     0x02
/* Interpret addr exactly */
#define MAP_FIXED       0x04
/* don't use a file */
#define MAP_ANONYMOUS   0x10

/* page can not be accessed */
#define PROT_NONE       0x0
/* page can be read */
#define PROT_READ       0x1
/* page can be written */
#define PROT_WRITE      0x2
/* page can be executed */
#define PROT_EXEC       0x4

//同步的参数
/* synchronous memory sync */
#define MS_SYNC         1
/* sync memory asynchronously */
#define MS_ASYNC        2
/* invalidate the caches */
#define MS_INVALIDATE   4

#if !defined (MAP_FAILED)
#define MAP_FAILED ((void *) -1)
#endif

// Define the pathname separator characters for Win32 (ugh).
#define WIN_DIRECTORY_SEPARATOR_CHAR1 '\\'
#define WIN_DIRECTORY_SEPARATOR_CHAR2 '/'

//WINDOWS平台现在这两个字符都支持
#define ZCE_IS_DIRECTORY_SEPARATOR(ch) (('\\'==(ch)) || ('/'==(ch)))

#define ZCE_DIRECTORY_SEPARATOR_CHAR '\\'
#define ZCE_DIRECTORY_SEPARATOR_STR  "\\"
#define ZCE_CURRENT_DIRECTORY_STR    ".\\"

//Windows下的POSOX文件统一放一个目录,

#define ZCE_POSIX_MMAP_DIRECTORY   "C:\\dev.shm\\"

//The file type is unknown.
#ifndef DT_UNKNOWN
#define DT_UNKNOWN      0
#endif
//This is a named pipe(FIFO).
#ifndef DT_FIFO
#define DT_FIFO         1
#endif
//This is a character device.
#ifndef DT_CHR
#define DT_CHR          2
#endif
//This is a directory.
#ifndef DT_DIR
#define DT_DIR          4
#endif
//This is a block device.
#ifndef DT_BLK
#define DT_BLK          6
#endif
//This is a regular file.
#ifndef DT_REG
#define DT_REG          8
#endif
//This is a symbolic link.
#ifndef DT_LNK
#define DT_LNK          10
#endif
//This is a UNIX domain socket.
#ifndef DT_SOCK
#define DT_SOCK         12
#endif

// If we are using winsock2 then the SO_REUSEADDR feature is broken
// SO_REUSEADDR=1 behaves like SO_REUSEPORT=1. (SO_REUSEPORT is an
// extension to sockets on some platforms)
// We define SO_REUSEPORT here so that zce::setsockopt() can still
// allow the user to specify that a socketaddr can *always* be reused.
#if ! defined(SO_REUSEPORT)
#define SO_REUSEPORT 0x0400  // We just have to pick a value that won't conflict
#endif

//LINUX下的定义好像是1024吧。
#if !defined (IOV_MAX)
#  define IOV_MAX 32
#endif

//统一shutdown的3个参数
#if !defined (SHUT_RD)
#define SHUT_RD SD_RECEIVE
#endif

#if !defined (SHUT_WR)
#define SHUT_WR SD_SEND
#endif

#if !defined (SHUT_RDWR)
#define SHUT_RDWR SD_BOTH
#endif

typedef SOCKET ZCE_SOCKET;

#if !defined (ZCE_INVALID_SOCKET)
# define ZCE_INVALID_SOCKET  INVALID_SOCKET
#endif

#ifndef ZCE_DEFAULT_BACKLOG
#define ZCE_DEFAULT_BACKLOG SOMAXCONN
#endif

#ifndef O_NONBLOCK
#define O_NONBLOCK      00004000
#endif

struct iovec
{
    //Starting address
    void* iov_base;
    //Number of bytes
    size_t iov_len;
};

struct msghdr
{
    void* msg_name;       /* optional address */
    socklen_t     msg_namelen;    /* size of address */
    struct iovec* msg_iov;        /* scatter/gather array */
    size_t        msg_iovlen;     /* # elements in msg_iov */
    void* msg_control;    /* ancillary data, see below */
    socklen_t     msg_controllen; /* ancillary data buffer len */
    int           msg_flags;      /* flags on received message */
};

struct sched_param
{
    int sched_priority;
};
//pthread 的属性结构,其实真实的pthread_attr_t不是这样，我这儿简化了很多地方
struct pthread_attr_t
{
    //
    int             detachstate;
    //
    int             inheritsched;
    //调度策略其实没法使用,我的代码不会用于REAL TIME系统
    //int           schedpolicy;
    //调度的优先级
    sched_param     schedparam;
    //堆栈的地址，在WINDOWS下没法用
    //void *        stackaddr
    //堆栈的尺寸
    size_t          stacksize;
};

#ifndef PTHREAD_CREATE_DETACHED
#define PTHREAD_CREATE_DETACHED 1
#endif
#ifndef PTHREAD_CREATE_JOINABLE
#define PTHREAD_CREATE_JOINABLE 2
#endif
#ifndef PTHREAD_INHERIT_SCHED
#define PTHREAD_INHERIT_SCHED   3
#endif
#ifndef PTHREAD_EXPLICIT_SCHED
#define PTHREAD_EXPLICIT_SCHED  4
#endif

//线程ID
typedef unsigned int ZCE_THREAD_ID;
//线程的句柄
typedef HANDLE       ZCE_THREAD_HANDLE;
//
typedef unsigned int ZCE_THR_FUNC_RETURN;

//
typedef DWORD        pid_t;

//动态库方面的跨平台定义
#if !defined (ZCE_SHLIB_INVALID_HANDLE)
#  define ZCE_SHLIB_INVALID_HANDLE 0
#endif //

#if !defined (ZCE_DEFAULT_SHLIB_MODE)
#  define ZCE_DEFAULT_SHLIB_MODE 0
#endif //

//还是说明一下为什么要在WINDOWS的兼容代码使用HANDLE,而不是int，
//因为WINDOWs的API大部分都是使用HANDLE，如果每次都要转换，这个，有点违背我对性能的爱好。
typedef HANDLE   ZCE_HANDLE;

#if !defined (ZCE_INVALID_HANDLE)
# define ZCE_INVALID_HANDLE  INVALID_HANDLE_VALUE
#endif

//WINDOWS下默认支持的权限熟悉是S_IREAD|S_IWRITE ，由于俺全面兼容LINUX，我会根据你的LINUX参数进行转换
#if !defined S_IRWXU
#define S_IRWXU 00700
#endif
#if !defined S_IRUSR
#define S_IRUSR 00400
#endif
#if !defined S_IWUSR
#define S_IWUSR 00200
#endif
#if !defined S_IXUSR
#define S_IXUSR 00100
#endif

#if !defined S_IRWXG
#define S_IRWXG 00070
#endif
#if !defined S_IRGRP
#define S_IRGRP 00040
#endif
#if !defined S_IWGRP
#define S_IWGRP 00020
#endif
#if !defined S_IXGRP
#define S_IXGRP 00010
#endif

#if !defined S_IRWXO
#define S_IRWXO 00007
#endif
#if !defined S_IROTH
#define S_IROTH 00004
#endif
#if !defined S_IWOTH
#define S_IWOTH 00002
#endif
#if !defined S_IXOTH
#define S_IXOTH 00001
#endif

//默认文件的权限属性0660
#if !defined (ZCE_DEFAULT_FILE_PERMS)
#define ZCE_DEFAULT_FILE_PERMS  (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP)
#endif

//共享的文件权限属性0666
#if !defined (ZCE_SHARE_FILE_PERMS)
#define ZCE_SHARE_FILE_PERMS  (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)
#endif

//私有的文件权限熟悉
#if !defined (ZCE_PRIVATE_FILE_PERMS)
#define ZCE_PRIVATE_FILE_PERMS  (0)
#endif

//默认目录的权限属性0755，其实在WINDOWS下没用
#if !defined (ZCE_DEFAULT_DIR_PERMS)
#define ZCE_DEFAULT_DIR_PERMS (S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH)
#endif

///文件是否存在
#if !defined(F_OK)
#define F_OK 0
#endif

///文件是否可写
#if !defined (W_OK)
#define W_OK 2
#endif

///文件是否可读
#if !defined (R_OK)
#define R_OK 4
#endif

//

typedef HMODULE   ZCE_SHLIB_HANDLE;

//定义这些只是为了让你的代码不用定义两份，在WINDOWS下这些定义没用。
#if !defined (RTLD_LAZY)
#define RTLD_LAZY 1
#endif /* !RTLD_LAZY */

#if !defined (RTLD_NOW)
#define RTLD_NOW 2
#endif /* !RTLD_NOW */

#if !defined (RTLD_GLOBAL)
#define RTLD_GLOBAL 3
#endif /* !RTLD_GLOBAL */

typedef int clockid_t;

#ifndef  CLOCK_REALTIME
#define CLOCK_REALTIME 0
#endif
#ifndef  CLOCK_MONOTONIC
#define CLOCK_MONOTONIC  1
#endif

#if _MSC_VER <= 1800

//POSIX的时间，
typedef struct
{
    /// Seconds
    time_t    tv_sec;
    /// Nanoseconds
    long      tv_nsec;
} timespec;

#endif

//flock函数用的的4个参数,
// shared lock
#ifndef LOCK_SH
#define LOCK_SH         1
#endif
// exclusive lock
#ifndef LOCK_EX
#define LOCK_EX         2
#endif
// or'd with one of the above to prevent  blocking
#ifndef LOCK_NB
#define LOCK_NB         4
#endif
//remove lock
#ifndef LOCK_UN
#define LOCK_UN         8
#endif

//在Windows 下就是一个索引值
typedef    DWORD  pthread_key_t;

//TIME VAL的时间，这个在WINDOW下为了select居然有定义了，难得,难得，虽然里面都是long
//struct timeval
//{
//    // seconds
//    time_t    tv_sec;
//    //and microseconds
//    long      tv_usec;
//};

#if !defined (SIGHUP)
#  define SIGHUP 0
#endif /* SIGHUP */

#if !defined (SIGINT)
#  define SIGINT 0
#endif /* SIGINT */

#if !defined (SIGSEGV)
#  define SIGSEGV 0
#endif /* SIGSEGV */

#if !defined (SIGIO)
#  define SIGIO 0
#endif /* SIGSEGV */

#if !defined (SIGUSR1)
#  define SIGUSR1 0
#endif /* SIGUSR1 */

#if !defined (SIGUSR2)
#  define SIGUSR2 0
#endif /* SIGUSR2 */

#if !defined (SIGCHLD)
#  define SIGCHLD 0
#endif /* SIGCHLD */

#if !defined (SIGCLD)
#  define SIGCLD SIGCHLD
#endif /* SIGCLD */

#if !defined (SIGQUIT)
#  define SIGQUIT 0
#endif /* SIGQUIT */

#if !defined (SIGPIPE)
#  define SIGPIPE 0
#endif /* SIGPIPE */

#if !defined (SIGALRM)
#  define SIGALRM 0
#endif /* SIGALRM */

#if !defined (SIGABRT)
#  define SIGABRT 0
#endif /* SIGABRT */

#if !defined (SIGTERM)
#  define SIGTERM 0
#endif /* SIGTERM */

#if !defined (SIG_DFL)
#  define SIG_DFL ((sighandler_t) 0)
#endif /* SIG_DFL */

#if !defined (SIG_IGN)
#  define SIG_IGN ((sighandler_t) 1)     /* ignore signal */
#endif /* SIG_IGN */

#if !defined (SIG_ERR)
#  define SIG_ERR ((sighandler_t) -1)    /* error return from signal */
#endif /* SIG_ERR */

typedef void (*sighandler_t)(int);

//epoll的这些变量提供出来不是为了模拟epoll，是方便我编译测试的
typedef union epoll_data
{
    void* ptr;
    ZCE_HANDLE fd;
    uint32_t u32;
    uint64_t u64;
} epoll_data_t;

struct epoll_event
{
    uint32_t events; /* Epoll events */
    epoll_data_t data; /* User data variable */
};

//EPOLLIN：表示对应的文件描述符可以读；
#define EPOLLIN     (1<<1)
//EPOLLOUT：表示对应的文件描述符可以写；
#define EPOLLOUT    (1<<2)
//EPOLLPRI：表示对应的文件描述符有紧急的数据可读
#define EPOLLPRI    (1<<3)
//EPOLLERR：表示对应的文件描述符发生错误；
#define EPOLLERR    (1<<4)
//EPOLLHUP：表示对应的文件描述符被挂断；
#define EPOLLHUP    (1<<5)
//EPOLLET：表示对应的文件描述符有事件发生；
#define EPOLLET     (1<<6)

//为SYSTEM V定义的一些常量

typedef int key_t;

#ifndef IPC_CREAT
#define IPC_CREAT  00001000   /* create if key is nonexistent */
#endif

#ifndef IPC_EXCL
#define IPC_EXCL   00002000   /* fail if key exists */
#endif

// /* permission flag for shmget */
#ifndef SHM_R
#define SHM_R           0400    /* or S_IRUGO from <linux/stat.h> */
#endif
#ifndef SHM_W
#define SHM_W           0200    /* or S_IWUGO from <linux/stat.h> */
#endif

/* mode for attach */
#ifndef SHM_RDONLY
#define SHM_RDONLY      010000  /* read-only access */
#endif
#ifndef SHM_RND
#define SHM_RND         020000  /* round attach address to SHMLBA boundary */
#endif
#ifndef SHM_EXEC
#define SHM_EXEC        0100000 /* execution access */
#endif

#ifndef IPC_PRIVATE
#define IPC_PRIVATE     0
#endif

// command for shmctl
#ifndef IPC_RMID
#define IPC_RMID        0     /* remove resource */
#endif
#ifndef IPC_SET
#define IPC_SET         1     /* set ipc_perm options */
#endif
#ifndef IPC_STAT
#define IPC_STAT        2     /* get ipc_perm options */
#endif

// no use ,仅仅为了编译通过
struct shmid_ds
{
    size_t shm_segsz;
};

//Inotify的各种监听选项,Windows费力死能监控到的事件也就这几个

/// File was modified
#ifndef IN_MODIFY
#define IN_MODIFY               0x00000002
#endif
/// File was moved from X
#ifndef IN_MOVED_FROM
#define IN_MOVED_FROM           0x00000040
#endif
/// File was moved to Y
#ifndef IN_MOVED_TO
#define IN_MOVED_TO             0x00000080
#endif
/// Subfile was created
#ifndef IN_CREATE
#define IN_CREATE               0x00000100
#endif
/// Subfile was deleted
#ifndef IN_DELETE
#define IN_DELETE               0x00000200
#endif

//================================================================================================
#elif defined (ZCE_OS_LINUX)

//为了兼容Windows部分代码的宏，
#if !defined MAXIMUM_WAIT_OBJECTS
# define MAXIMUM_WAIT_OBJECTS 64
#endif

#if !defined (ZCE_DEFAULT_FILE_PERMS)
#define ZCE_DEFAULT_FILE_PERMS  (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP)
#endif

#if !defined (ZCE_SHARE_FILE_PERMS)
#define ZCE_SHARE_FILE_PERMS  (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)
#endif

#if !defined (ZCE_PRIVATE_FILE_PERMS)
#define ZCE_PRIVATE_FILE_PERMS  (S_IRUSR|S_IWUSR)
#endif

#if !defined (ZCE_DEFAULT_DIR_PERMS)
#define ZCE_DEFAULT_DIR_PERMS (S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH)
#endif

#define LINUX_DIRECTORY_SEPARATOR_CHAR '/'

//WINDOWS平台现在这两个字符都支持
#define ZCE_IS_DIRECTORY_SEPARATOR(ch) ('/'==(ch))

// Define the pathname separator characters for UNIX.
#define ZCE_DIRECTORY_SEPARATOR_STR   "/"
#define ZCE_DIRECTORY_SEPARATOR_CHAR  '/'
#define ZCE_CURRENT_DIRECTORY_STR     "./"

#define ZCE_POSIX_MMAP_DIRECTORY "/dev/shm/"

//网络方面的跨平台定义
typedef int ZCE_SOCKET;

#if !defined (ZCE_INVALID_SOCKET)
# define ZCE_INVALID_SOCKET  -1
#endif

#ifndef ZCE_DEFAULT_BACKLOG
#define ZCE_DEFAULT_BACKLOG 128
#endif

//线程的通用化定义pthread_t一般在的定义是unsigned long
typedef pthread_t   ZCE_THREAD_ID;
//
typedef pthread_t   ZCE_THREAD_HANDLE;
//
typedef void* ZCE_THR_FUNC_RETURN;

typedef int   ZCE_HANDLE;

typedef struct stat   zce_os_stat;

typedef void* ZCE_SHLIB_HANDLE;

#if !defined (ZCE_INVALID_HANDLE)
# define ZCE_INVALID_HANDLE -1
#endif

#if !defined (ZCE_SHLIB_INVALID_HANDLE)
#  define ZCE_SHLIB_INVALID_HANDLE 0
#endif

#if !defined (ZCE_DEFAULT_SHLIB_MODE)
#  define ZCE_DEFAULT_SHLIB_MODE RTLD_LAZY
#endif //

#endif //#elif defined (ZCE_OS_LINUX)

///////////////////////////////////////////////////////////////////////////////////////////////////////////
