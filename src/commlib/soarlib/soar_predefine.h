/*!
* @copyright  2004-2014  Apache License, Version 2.0 FULLSAIL
* @filename   soar_predefine.h
* @author     Sailzeng <sailerzeng@gmail.com>
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
#include <zce_predefine.h>
#include <zce_id_to_string.h>
#include <zce_time_value.h>
#include <zce_mysql_connect.h>
#include <zce_mysql_command.h>
#include <zce_mysql_process.h>
#include <zce_shm_cache_chunk.h>
#include <zce_lockfree_kfifo.h>
#include <zce_server_base.h>
#include <zce_shm_hash_table.h>
#include <zce_boost_non_copyable.h>
#include <zce_boost_lord_rings.h>
#include <zce_os_adapt_string.h>
#include <zce_os_adapt_thread.h>
#include <zce_os_adapt_socket.h>
#include <zce_os_adapt_thread.h>
#include <zce_os_adapt_error.h>
#include <zce_os_adapt_process.h>
#include <zce_os_adapt_socket.h>
#include <zce_os_adapt_getopt.h>
#include <zce_os_adapt_file.h>
#include <zce_os_adapt_dirent.h>
#include <zce_share_mem_mmap.h>
#include <zce_share_mem_posix.h>
#include <zce_trace_debugging.h>
#include <zce_trace_log_msg.h>
#include <zce_lock_null_lock.h>
#include <zce_lock_thread_mutex.h>
#include <zce_config_property_tree.h>
#include <zce_config_file_implement.h>
#include <zce_thread_msgque_sema.h>
#include <zce_thread_msgque_nonlock.h>
#include <zce_timer_handler_base.h>
#include <zce_timer_queue_base.h>
#include <zce_timer_queue_wheel.h>
#include <zce_bus_two_way.h>
#include <zce_thread_task.h>
#include <zce_socket_addr_base.h>
#include <zce_socket_addr_in.h>
#include <zce_socket_base.h>
#include <zce_socket_stream.h>
#include <zce_socket_datagram.h>
#include <zce_socket_connector.h>
#include <zce_event_handle_base.h>
#include <zce_event_reactor_base.h>
#include <zce_event_reactor_select.h>
#include <zce_event_reactor_epoll.h>
#include <zce_thread_task.h>
#include <zce_thread_wait_mgr.h>
#include <zce_config_getopt.h>
#include <zce_server_status.h>
#include <zce_thread_bus_pipe.h>
#include <zce_lock_file_lock.h>
#include <zce_bytes_hash.h>
#include <zce_bytes_encode.h>
#include <zce_async_base.h>
#include <zce_async_fsm.h>

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
#define SOARING_EXPORT __declspec (dllexport)
#else
#define SOARING_EXPORT __declspec (dllimport)
#endif
#else
#define SOARING_EXPORT
#endif


#endif //SOARING_LIB_PER_DEFINE_H_

