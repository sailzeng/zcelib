/*!
* @copyright  2004-2014  Apache License, Version 2.0 FULLSAIL
* @filename   soar/predefine.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2006年6月22日
* @brief      大规模工程文件的基础头文件真的是一件讨厌的事情.
*
*
* @details
*
*
*
* @note       为了避免一些保持部分代码的纯洁，
*
*  最后一枪
*  崔建
*  一颗流弹打入我胸膛
*  刹那间往事涌在我心上
*  哦
*  只有泪水
*  哦
*  没有悲伤
*  如果这是最后的一枪
*  我愿接受这莫大的荣光
*  哦
*  最后一枪
*  哦
*  最后一枪
*  不知道有多少 有多少话还没讲
*  不知道有多少 有多少欢乐没享
*  不知道有多少 多少人和我一样
*  不知道有多少 多少个最后一枪
*  安睡在这温暖的土地上
*  朝露夕阳花木自芬芳
*  哦哦 只有一句话 留在世界上
*  不知道有多少 有多少话还没讲
*  不知道有多少 有多少欢乐没享
*  不知道有多少 多少人和我一样
*  不知道有多少 多少个最后一枪
*  一颗流弹打入我的胸膛
*  刹那间往事涌在我的心上
*  哦哦 最后一枪
*  哦哦 最后一枪
*  哦哦 最后一枪
*  哦哦 最后一枪
*  哦哦 最后一枪
*  哦哦 最后一枪
*  哦哦 最后一枪
*  哦哦 最后一枪
*/

#ifndef SOARING_LIB_PER_DEFINE_H_
#define SOARING_LIB_PER_DEFINE_H_

//在WINDOWS下和POSIX标准兼容的宏，2003以下如何，我没有测试
#define _CRT_NONSTDC_NO_DEPRECATE 1
#define _CRT_SECURE_NO_WARNINGS   1

//---------------------------------------------------------------------------------------------

//ZCElib的库文件
#include <zce/predefine.h>
#include <zce/util/id_to_string.h>
#include <zce/time/time_value.h>
#include <zce/mysql/connect.h>
#include <zce/mysql/command.h>
#include <zce/mysql/execute.h>
#include <zce/lockfree/kfifo.h>
#include <zce/server/server_base.h>
#include <zce/shm_container/hash_table.h>
#include <zce/util/non_copyable.h>
#include <zce/container/lord_rings.h>
#include <zce/os_adapt/string.h>
#include <zce/os_adapt/thread.h>
#include <zce/os_adapt/socket.h>
#include <zce/os_adapt/thread.h>
#include <zce/os_adapt/error.h>
#include <zce/os_adapt/process.h>
#include <zce/os_adapt/socket.h>
#include <zce/os_adapt/getopt.h>
#include <zce/os_adapt/file.h>
#include <zce/os_adapt/dirent.h>
#include <zce/shared_mem/mmap.h>
#include <zce/shared_mem/posix.h>
#include <zce/logger/log_comm.h>
#include <zce/logger/logging.h>
#include <zce/lock/null_lock.h>
#include <zce/lock/thread_mutex.h>
#include <zce/config/property_tree.h>
#include <zce/config/file_implement.h>
#include <zce/thread/msgque_sema.h>
#include <zce/timer/timer_handler.h>
#include <zce/timer/queue_base.h>
#include <zce/timer/queue_wheel.h>
#include <zce/bus/twoway_pipe.h>
#include <zce/thread/thread_task.h>
#include <zce/socket/addr_base.h>
#include <zce/socket/addr_in.h>
#include <zce/socket/socket_base.h>
#include <zce/socket/stream.h>
#include <zce/socket/datagram.h>
#include <zce/socket/connector.h>
#include <zce/event/handle_base.h>
#include <zce/event/reactor_base.h>
#include <zce/event/reactor_select.h>
#include <zce/event/reactor_epoll.h>
#include <zce/thread/thread_task.h>
#include <zce/thread/thread_wait_mgr.h>
#include <zce/thread/msgque_condi.h>
#include <zce/server/mml_command.h>
#include <zce/server/get_option.h>
#include <zce/server/server_status.h>
#include <zce/bus/twoway_pipe.h>
#include <zce/lock/file_lock.h>
#include <zce/bytes/hash_value.h>
#include <zce/bytes/base_encode.h>
#include <zce/async/async_base.h>
#include <zce/async/fsm.h>

//
#if defined (ZCE_OS_WINDOWS) && defined (_MSC_VER)

#if !defined SOAR_LIB_LIBARY_NAME
#define SOAR_LIB_LIBARY_NAME   "soarlib.lib"
#endif

//自动包含的包含连接，简化你的操作
#pragma comment(lib, SOAR_LIB_LIBARY_NAME  )

#endif

//------------------------------------------------------------------------------------
//客户端服务器公用的头文件,服务器的头文件定义在下面

#if defined ZCE_OS_WINDOWS && defined TSSCOMM_HASDLL
#ifdef BUILD_SOARING_MODULE_DLL
#define  SOARING_EXPORT __declspec (dllexport)
#else
#define  SOARING_EXPORT __declspec (dllimport)
#endif
#else
#define SOARING_EXPORT
#endif

#endif //SOARING_LIB_PER_DEFINE_H_
