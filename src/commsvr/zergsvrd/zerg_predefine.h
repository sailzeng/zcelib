
#ifndef ZERG_SERVER_PRE_DEFINE_H_
#define ZERG_SERVER_PRE_DEFINE_H_

#include <zce_predefine.h>
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
#include <zce_os_adapt_error.h>
#include <zce_os_adapt_string.h>
#include <zce_os_adapt_file.h>
#include <zce_os_adapt_thread.h>
#include <zce_os_adapt_socket.h>
#include <zce_os_adapt_thread.h>
#include <zce_os_adapt_process.h>
#include <zce_os_adapt_getopt.h>
#include <zce_os_adapt_backtrace.h>
#include <zce_log_logging.h>
#include <zce_lock_null_lock.h>
#include <zce_lock_thread_mutex.h>
#include <zce_config_property_tree.h>
#include <zce_config_file_implement.h>
#include <zce_thread_msgque_sema.h>
#include <zce_thread_msgque_nonlock.h>
#include <zce_timer_handler_base.h>
#include <zce_timer_queue_wheel.h>
#include <zce_share_mem_mmap.h>
#include <zce_bus_mmap_pipe.h>
#include <zce_bus_two_way.h>
#include <zce_thread_task.h>
#include <zce_event_handle_base.h>
#include <zce_event_reactor_base.h>
#include <zce_config_getopt.h>
#include <zce_socket_base.h>
#include <zce_socket_acceptor.h>
#include <zce_socket_connector.h>
#include <zce_socket_stream.h>
#include <zce_mml_command.h>

#include <soar_predefine.h>
#include <soar_error_code.h>
#include <soar_svrd_cfg_base.h>
#include <soar_svrd_app_main.h>
#include <soar_zerg_frame.h>
#include <soar_mmap_buspipe.h>
#include <soar_svrd_app_base.h>
#include <soar_svrd_app_main.h>
#include <soar_stat_monitor.h>
#include <soar_svrd_timer_base.h>




//那天无法调试，google找到的解决方案。
//there is no source code available for the current location 解决方案
//
//1.首先试最常规的方法：Clean and then rebuild solution，但是没有解决
//2.进入Tools>Options,选择Debugging>General 却掉 Enable address-level debugging 选项，在去掉 Require source files to exactly match the original version.
//Okay，解决问题。

//好几个地方都用这个常量，统一设置，以后好改
static const socklen_t ZERG_SND_RCV_BUF_OPVAL = 320 * 1024;

#endif //_ZERG_SERVER_PRE_DEFINE_H_

