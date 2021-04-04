/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_os_adapt_predefine.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2011��5��1��
* @brief      ������ֿ�ƽ̨�Ľṹ��ö�٣��꣬ȫ�ֱ�������
*
* @details    ��ʱ���ʱ��2��ѡ��һ���ǣ��Ѹ��������ɢ�ŵ������ÿ��zce_os_adapt_XXXXXX.h�ļ���
*             һ���Ƿ���һ��ͷ�ļ����棬��ʼ�Ƿ�ɢ��������ֲ��ֶ���Ҫ����һ��ʹ�ã�������û��Ǹ���
*             ͳһ��������ļ������Լ������еĿ�ƽ̨�Ļ����ṹ�����������
*             ��д�����棬���ֻ��������������������������ŵ�����������һЩ�˵Ŀ�ƽ̨�����ͻ������
*             �������廹�Ƿŵ����Ե��ļ��С���������Ϊ���ҡ�
* @note       �Ҿ�����ʹ��off_t������ͣ���Ϊ��WINƽ̨�£����������long����WINƽ̨��long�ǲ���չ�ģ�
*             WINDOWS�µ�types.h�Ѿ��ж��壬
*             typedef off_t long;
*             �󲿷��������ssize_t�滻off_t
*/

#ifndef ZCE_LIB_OS_ADAPT_PREDEFINE_H_
#define ZCE_LIB_OS_ADAPT_PREDEFINE_H_

//��Ϊ������Ԥ����ͷ�ļ���Ե�ʣ�����������ð�������ļ�

/*!
* @namespace zce ��ƽ̨�ĺ��������ֿռ䶨�壬���еĺ�����Ҫ��LINUX��POSIX����Ҳ������
*                    �Լ�д�ĺ�����
*                    ��Ҫ�����棬�������磬�ļ���Ŀ¼����̬�⣬ͬ�������̣߳������ڴ棬ʱ��
*                    �ַ������������ܵȡ�
*/

#if !defined (ZCE_INVALID_FILE_DESCRIBE)
# define ZCE_INVALID_FILE_DESCRIBE  -1
#endif

//------------------------------------------------------------------------------------------
//WINDOWS �İ汾��θ�
#if defined (ZCE_OS_WINDOWS)

//����̼乲��
#ifndef PTHREAD_PROCESS_SHARED
#define PTHREAD_PROCESS_SHARED          0x1
#endif

//�����ڲ�
#ifndef PTHREAD_PROCESS_PRIVATE
#define PTHREAD_PROCESS_PRIVATE         0x2
#endif

//��ͨ��
#ifndef PTHREAD_MUTEX_NORMAL
#define PTHREAD_MUTEX_NORMAL            0x0
#endif
//������Ļ�������
#ifndef PTHREAD_MUTEX_ERRORCHECK
#define PTHREAD_MUTEX_ERRORCHECK        0x4
#endif

//�ݹ�Ļ�����
#ifndef PTHREAD_MUTEX_RECURSIVE
#define PTHREAD_MUTEX_RECURSIVE         0x8
#endif

//�Զ���ģ���ʱ����WIN�µ��ٽ�����֧�ֳ�ʱ
#ifndef PTHREAD_MUTEX_TIMEOUT
#define PTHREAD_MUTEX_TIMEOUT           0x100000
#endif


#ifdef ZCE_OS_WINDOWS
#pragma warning ( disable : 26495)
#endif

typedef struct sem_t
{
    //�ź����ľ��
    HANDLE  sem_hanlde_ = INVALID_HANDLE_VALUE;

    //�ź����Ƿ���һ���������źŵ�
    bool    sem_unnamed_ = false;

} ;

#if !defined (SEM_FAILED)
#  define SEM_FAILED ((sem_t *) -1)
#endif

typedef struct
{
    // Either PTHREAD_PROCESS_SHARED or PTHREAD_PROCESS_PRIVATE
    int lock_shared_;
    //�������ͣ���Ҫ����Ƿ���PTHREAD_MUTEX_RECURSIVE��
    int lock_type_;

    //�����������֣�����Ƕ���̵Ļ��������ͱ���������
    char mutex_name_[PATH_MAX + 1];

} pthread_mutexattr_t;

///
struct pthread_mutex_t
{
    /// Either PTHREAD_PROCESS_SHARED or PTHREAD_PROCESS_PRIVATE
    int lock_shared_;
    //�������ͣ���Ҫ����Ƿ���PTHREAD_MUTEX_RECURSIVE��
    ///���PTHREAD_MUTEX_TIMEOUT�������Լ�ΪWIN���Ƶ�
    int lock_type_;

    ///�߳�ʹ����2�ֲ�ͬ�ķ�ʽ���������Ҫ�ǵݹ飬�ͳ�ʱ���������ٽ���
    ///�����û�����
    union
    {
        //MUTEX��HANDLE,���̣��̶߳������ã��еݹ�����
        HANDLE           recursive_mutex_;
        //�ٽ������ܺã������Ҫ��ʱ�����Ҳ�Ҫ����̣���Ҫ�ǵݹ�ʱѡ��
        CRITICAL_SECTION thr_nontimeout_mutex_;
        //�ǵݹ��������źŵ�ģ�⣬
        sem_t           *non_recursive_mutex_;
    };
} ;

//��WINDOWS�����ٽ����������ڣ�+SPIN ����Mutex�����̼䣩ģ��SPIN lock
//ע������ٽ�������Ϊ�Ҽ�����SPIN���ܣ�
typedef pthread_mutex_t  pthread_spinlock_t;

struct pthread_condattr_t
{
    // �ⲿ�������ͣ������Ҫtimeout,������� PTHREAD_MUTEX_TIMEOUT�������ʹ���ٽ���
    int outer_lock_type_;

    //�����������֣�����Ƕ���̵Ļ��������ͱ���������
    char cv_name_[PATH_MAX + 1];

} ;

struct win_simulate_cv_t
{

    /// �ȴ��ߵ�����
    int                  waiters_ = 0;

    /// ������еĽ���������broadcast���еĻ���signal����
    bool                 was_broadcast_ = false;

    /// waiters �ļ����ı�����
    pthread_mutex_t      waiters_lock_;

    /// �źŵƣ������Ŷӵȴ����߳�ֱ�� signaled.
    sem_t               *block_sema_ = NULL;

    ///��ɹ㲥���֪ͨ������ط���sema��ʵ�������ڹ�ƽ�ԣ���EVENT����һ�㡣
    ///������Ҫ��㲥��ʱ���ⲿ�����ּ��ϣ���������Ҳ��̫��
    sem_t               *finish_broadcast_ = NULL;

} ;

struct pthread_cond_t
{

    ///�ⲿ�������ͣ��Ƿ���ҪTIMEOUT���ȣ�PTHREAD_MUTEX_TIMEOUT
    int                      outer_lock_type_ = 0;

    //�����ܹ���ʵ�������õ�ֻ��һ��

    ///WINDOWS������������WINSERVER2008��VISTA���֧��
    ///�����������ֻ�ܵ������ڲ�ʹ�ã����ⲿ��������ֻ֧���ٽ���
#if defined ZCE_SUPPORT_WINSVR2008 && ZCE_SUPPORT_WINSVR2008 == 1
    CONDITION_VARIABLE   cv_object_;
#endif
    ///���������źŵƺ�һ��������ģ�������������ͬʱ֧��signal��broadcast������
    ///Ҳ֧���ⲿ�������Ƕ���̹���Ҳ֧���ⲿ��������MUTEX���źŵƣ����ٽ���ģ��
    ///�ģ�pthread_mutex_t
    win_simulate_cv_t     simulate_cv_;

} ;

//��д���Ĵ�������UNP V2

///��д�������Խṹ��
typedef struct
{
    //
    bool            priority_to_write_;

} pthread_rwlockattr_t;

///��д���Ķ���ṹ�����û���������������ʵ�ֵĶ�д��
struct pthread_rwlock_t
{
    ///ģ���
    ///�Ƿ��ǻ���д�����ȣ����Ǿ���д�����ȣ������ȡ���ȣ�����һ�����⣬�ҰѾ���Ȩ������
    bool            priority_to_write_ = true;

    ///��������ṹ�ڶ��߳��ж�д�Ļ���������Ҫ������Щ�������޸�
    pthread_mutex_t rw_mutex_;

    /// ���ߵȴ�����������
    pthread_cond_t  rw_condreaders_;
    /// д��ȴ�����������
    pthread_cond_t  rw_condwriters_;

    ///�ȴ������߳�����
    int             rw_nwaitreaders_ = 0;
    ///�ȴ�д���߳�����
    int             rw_nwaitwriters_ = 0;

    ///���ĳ���״̬�������һ��д�߳�����-1 ���>0��ʾ���ٸ����߳��������
    int             rw_refcount_ = 0;

} ;


#ifdef ZCE_OS_WINDOWS
#pragma warning ( default : 26495)
#endif

typedef unsigned int          mode_t;

typedef struct __stat64       zce_os_stat;


//ע��S_IFDIR �� S_ISDIR ������S_ISDIR��һ���������ж��Ƿ���һ��Ŀ¼

#if !defined(S_IFDIR)
#define S_IFDIR  _S_IFDIR
#endif

#if !defined(S_IFREG)
#define S_IFREG  _S_IFREG
#endif

//zce_os_stat ��WINDOWS�±������__stat64��LINUX�±������stat,
//�Ҽ򵥽���һ������ƽ̨���У����Ҵ��һ����õ�������

//�ļ�ģʽ S_IFREG ��ͨ�ļ� S_IFDIR Ŀ¼,
//[Linux]    mode_t    st_mode;
//[WIN]      unsigned short st_mode;
//    �ļ�����,�ֽ�
//[Linux]    off_t     st_size;
//[WIN]      __int64    st_size;

//������ʱ��
//[Linux]    time_t    st_atime;
//[WIN]      __time64_t st_atime;
//����޸�ʱ��
//[Linux]    time_t    st_mtime;
//[WIN]      __time64_t st_mtime;
//WIN����ʱ��,LINUX,״̬�޸�ʱ��
//��ʵ���ֵ��LINUX�²��Ǵ���ʱ�䣬����change time����ʶ״̬�ı��ʱ�䣬inode�޸�ʱ�����ֵ�ı䣬
//�����ļ��ĸ�������Ҳ��INODE����ͣ���������(chmod��chown)�޸�ʱ�����ʱ��Ҳ��仯
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

//ͬ���Ĳ���
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

//WINDOWSƽ̨�����������ַ���֧��
#define ZCE_IS_DIRECTORY_SEPARATOR(ch) (('\\'==(ch)) || ('/'==(ch)))

#define ZCE_DIRECTORY_SEPARATOR_CHAR '\\'
#define ZCE_DIRECTORY_SEPARATOR_STR  "\\"
#define ZCE_CURRENT_DIRECTORY_STR    ".\\"

//Windows�µ�POSOX�ļ�ͳһ��һ��Ŀ¼,

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

//LINUX�µĶ��������1024�ɡ�
#if !defined (IOV_MAX)
#  define IOV_MAX 32
#endif

//ͳһshutdown��3������
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
    void *iov_base;
    //Number of bytes
    size_t iov_len;
};

struct msghdr
{
    void         *msg_name;       /* optional address */
    socklen_t     msg_namelen;    /* size of address */
    struct iovec *msg_iov;        /* scatter/gather array */
    size_t        msg_iovlen;     /* # elements in msg_iov */
    void         *msg_control;    /* ancillary data, see below */
    socklen_t     msg_controllen; /* ancillary data buffer len */
    int           msg_flags;      /* flags on received message */
};

struct sched_param
{
    int sched_priority;
};
//pthread �����Խṹ,��ʵ��ʵ��pthread_attr_t������������������˺ܶ�ط�
struct pthread_attr_t
{
    //
    int             detachstate;
    //
    int             inheritsched;
    //���Ȳ�����ʵû��ʹ��,�ҵĴ��벻������REAL TIMEϵͳ
    //int           schedpolicy;
    //���ȵ����ȼ�
    sched_param     schedparam;
    //��ջ�ĵ�ַ����WINDOWS��û����
    //void *        stackaddr
    //��ջ�ĳߴ�
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

//�߳�ID
typedef unsigned int ZCE_THREAD_ID;
//�̵߳ľ��
typedef HANDLE       ZCE_THREAD_HANDLE;
//
typedef unsigned int ZCE_THR_FUNC_RETURN;

//
typedef DWORD        pid_t;

//��̬�ⷽ��Ŀ�ƽ̨����
#if !defined (ZCE_SHLIB_INVALID_HANDLE)
#  define ZCE_SHLIB_INVALID_HANDLE 0
#endif //

#if !defined (ZCE_DEFAULT_SHLIB_MODE)
#  define ZCE_DEFAULT_SHLIB_MODE 0
#endif //

//����˵��һ��ΪʲôҪ��WINDOWS�ļ��ݴ���ʹ��HANDLE,������int��
//��ΪWINDOWs��API�󲿷ֶ���ʹ��HANDLE�����ÿ�ζ�Ҫת����������е�Υ���Ҷ����ܵİ��á�
typedef HANDLE   ZCE_HANDLE;

#if !defined (ZCE_INVALID_HANDLE)
# define ZCE_INVALID_HANDLE  INVALID_HANDLE_VALUE
#endif

//WINDOWS��Ĭ��֧�ֵ�Ȩ����Ϥ��S_IREAD|S_IWRITE �����ڰ�ȫ�����LINUX���һ�������LINUX��������ת��
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

//Ĭ���ļ���Ȩ������0660
#if !defined (ZCE_DEFAULT_FILE_PERMS)
#define ZCE_DEFAULT_FILE_PERMS  (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP)
#endif

//������ļ�Ȩ������0666
#if !defined (ZCE_SHARE_FILE_PERMS)
#define ZCE_SHARE_FILE_PERMS  (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)
#endif

//˽�е��ļ�Ȩ����Ϥ
#if !defined (ZCE_PRIVATE_FILE_PERMS)
#define ZCE_PRIVATE_FILE_PERMS  (0)
#endif

//Ĭ��Ŀ¼��Ȩ������0755����ʵ��WINDOWS��û��
#if !defined (ZCE_DEFAULT_DIR_PERMS)
#define ZCE_DEFAULT_DIR_PERMS (S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH)
#endif

///�ļ��Ƿ����
#if !defined(F_OK)
#define F_OK 0
#endif

///�ļ��Ƿ��д
#if !defined (W_OK)
#define W_OK 2
#endif

///�ļ��Ƿ�ɶ�
#if !defined (R_OK)
#define R_OK 4
#endif

//

typedef HMODULE   ZCE_SHLIB_HANDLE;

//������Щֻ��Ϊ������Ĵ��벻�ö������ݣ���WINDOWS����Щ����û�á�
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

//POSIX��ʱ�䣬
typedef struct
{
    /// Seconds
    time_t    tv_sec;
    /// Nanoseconds
    long      tv_nsec;
} timespec;

#endif

//flock�����õĵ�4������,
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

//��Windows �¾���һ������ֵ
typedef    DWORD  pthread_key_t;

//TIME VAL��ʱ�䣬�����WINDOW��Ϊ��select��Ȼ�ж����ˣ��ѵ�,�ѵã���Ȼ���涼��long
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

//epoll����Щ�����ṩ��������Ϊ��ģ��epoll���Ƿ����ұ�����Ե�
typedef union epoll_data
{
    void    *ptr;
    ZCE_HANDLE fd;
    uint32_t u32;
    uint64_t u64;
} epoll_data_t;

struct epoll_event
{
    uint32_t events; /* Epoll events */
    epoll_data_t data; /* User data variable */
};

//EPOLLIN����ʾ��Ӧ���ļ����������Զ���
#define EPOLLIN     (1<<1)
//EPOLLOUT����ʾ��Ӧ���ļ�����������д��
#define EPOLLOUT    (1<<2)
//EPOLLPRI����ʾ��Ӧ���ļ��������н��������ݿɶ�
#define EPOLLPRI    (1<<3)
//EPOLLERR����ʾ��Ӧ���ļ���������������
#define EPOLLERR    (1<<4)
//EPOLLHUP����ʾ��Ӧ���ļ����������Ҷϣ�
#define EPOLLHUP    (1<<5)
//EPOLLET����ʾ��Ӧ���ļ����������¼�������
#define EPOLLET     (1<<6)

//ΪSYSTEM V�����һЩ����

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

// no use ,����Ϊ�˱���ͨ��
struct shmid_ds
{
    size_t shm_segsz;
};

//Inotify�ĸ��ּ���ѡ��,Windows�������ܼ�ص����¼�Ҳ���⼸��

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

//Ϊ�˼���Windows���ִ���ĺ꣬
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

//WINDOWSƽ̨�����������ַ���֧��
#define ZCE_IS_DIRECTORY_SEPARATOR(ch) ('/'==(ch))

// Define the pathname separator characters for UNIX.
#define ZCE_DIRECTORY_SEPARATOR_STR   "/"
#define ZCE_DIRECTORY_SEPARATOR_CHAR  '/'
#define ZCE_CURRENT_DIRECTORY_STR     "./"

#define ZCE_POSIX_MMAP_DIRECTORY "/dev/shm/"

//���緽��Ŀ�ƽ̨����
typedef int ZCE_SOCKET;

#if !defined (ZCE_INVALID_SOCKET)
# define ZCE_INVALID_SOCKET  -1
#endif

#ifndef ZCE_DEFAULT_BACKLOG
#define ZCE_DEFAULT_BACKLOG 128
#endif

//�̵߳�ͨ�û�����pthread_tһ���ڵĶ�����unsigned long
typedef pthread_t   ZCE_THREAD_ID;
//
typedef pthread_t   ZCE_THREAD_HANDLE;
//
typedef void       *ZCE_THR_FUNC_RETURN;

typedef int   ZCE_HANDLE;

typedef struct stat   zce_os_stat;

typedef void     *ZCE_SHLIB_HANDLE;

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

#endif //ZCE_LIB_OS_ADAPT_PREDEFINE_H_

