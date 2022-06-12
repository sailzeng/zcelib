/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/os_adapt/semaphore.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2011年10月6日 今天Jobs走了，发现自己居然也到了到了开始缅怀前辈年纪，
* @brief      信号灯的跨平台封装，向LINUX的POSIX的封装靠齐
*
* @details    Windows 和 Linux 在信号灯上比较接近，
*
* @note
*
*/

#pragma once

#include "zce/os_adapt/common.h"

namespace zce
{
/*!
* @brief      初始化，创建一个无名（匿名）信号灯，线程下一般用匿名信号灯就足够了,
*             WINDOWS下的匿名信号灯是无法多进程共享的，
* @return     int         0成功，-1失败
* @param[out] sem         信号灯句柄
* @param[in]  pshared     信号灯的SHARE方式，WINDOWS下此参数无用
* @param[in]  init_value  信号灯初试的值
* @param[in]  max_val     max_val不是标准参数，用默认值修饰了，Windows下特有，可以用于模拟Mutex？
* @note       对应的销毁函数是 @ref sem_destroy
*/
int sem_init(sem_t* sem,
             int pshared,
             unsigned int init_value,
             unsigned int max_val = 0x7FFFFFFF) noexcept;

/*!
* @brief      销毁无名(匿名)信号灯
* @return     int      0成功，-1失败
* @param      sem      信号灯句柄
*/
int sem_destroy(sem_t* sem) noexcept;

/*!
* @brief      打开(有名)信号灯,max_val最大值不是标准参数，所以用默认只修饰了，这个主要用于创建有名信号灯,
*             有名信号灯，可以跨进程访问
* @return     sem_t*     如果成功返回，信号灯句柄 否则返回NULL
* @param      name       信号灯的名字
* @param      oflag      同文件open的创建参数，
* @param      mode       同文件open的访问权限参数
* @param      init_value 信号灯初始化的值
* @param      max_val    max_val不是标准参数，用默认值修饰了，Windows下特有，
* @note       打开后，要使用@ref sem_close，@ref sem_unlink
*/
sem_t* sem_open(const char* name,
                int oflag = O_CREAT,
                mode_t mode = ZCE_DEFAULT_FILE_PERMS,
                unsigned int init_value = 1,
                unsigned int max_val = 0x7FFFFFFF) noexcept;

/*!
* @brief      关闭信号灯
* @return     int 0成功，-1失败
* @param      sem 信号灯对象
*/
int sem_close(sem_t* sem) noexcept;

/*!
* @brief      删除信号灯
* @return     int  0成功，-1失败
* @param      name 信号灯的名字
* @note
*/
int sem_unlink(const char* name) noexcept;

/*!
* @brief      信号灯的V操作,
* @return     int
* @param      sem
* @note
*/
int sem_post(sem_t* sem) noexcept;

/*!
* @brief      非标准函数，信号灯的release_count次V操作，
* @return     int
* @param      sem
* @param      release_count
* @note
*/
int sem_post(sem_t* sem,
             u_int release_count) noexcept;

/*!
* @brief      信号灯,尝试锁定（P操作）,
* @return     int
* @param      sem
* @note
*/
int sem_trywait(sem_t* sem) noexcept;

/*!
* @brief      信号灯锁定（P操作）,如果信号灯的值小于0，
* @return     int
* @param      sem
*/
int sem_wait(sem_t* sem) noexcept;

/*!
* @brief      信号灯带超时等待的锁定操作（P操作），时间是绝对值
* @return     int              0成功，-1失败，如果是超时失败，errno是ETIMEDOUT
* @param      sem              信号灯对象
* @param      abs_timeout_spec 等待的时间点，绝对时间
* @note
*/
int sem_timedwait(sem_t* sem,
                  const ::timespec* abs_timeout_spec) noexcept;

/*!
* @brief      信号灯带超时等待的锁定操作（P操作）,非标准实现,使用timeval结构，时间是绝对值
* @return     int              0成功，-1失败，如果是超时失败，errno是ETIMEDOUT
* @param      sem              信号灯对象
* @param      abs_timeout_time 等待的时间点，绝对时间  timeval是我内部的时长标示通用类型
*/
int sem_timedwait(sem_t* sem,
                  const timeval* abs_timeout_time) noexcept;

/*!
* @brief      返回当前信号灯的当前值,慎用，Windows目前不支持（某种程度上反映出了WINDOWS API的设计的问题）
*             微软号称以后会改进这个API,ReleaseSemaphore,但WIN8没有看见改变
* @return     int   0成功，-1失败，
* @param      sem   信号灯对象
* @param      sval  信号灯的值
*/
int sem_getvalue(sem_t* sem, int* sval) noexcept;
};


