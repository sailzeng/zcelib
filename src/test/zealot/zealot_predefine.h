#ifndef ZEALOT_PRE_DEFINE_H_
#define ZEALOT_PRE_DEFINE_H_



//ZCELIB的代码

#include <zce/predefine.h>
#include <zce/log/logging.h>
#include <zce_log_plus.h>
#include <zce_log_msg.h>

#include <zce/os_adapt/common.h>
#include <zce/os_adapt/file.h>
#include <zce/os_adapt/process.h>
#include <zce/os_adapt/sysinfo.h>
#include <zce/os_adapt/error.h>
#include <zce/os_adapt/socket.h>
#include <zce/os_adapt/rwlock.h>
#include <zce/os_adapt/dirent.h>
#include <zce/os_adapt/math.h>
#include <zce/os_adapt/stdlib.h>
#include <zce/os_adapt/coroutine.h>
#include <zce/os_adapt/flock.h>
#include <zce/os_adapt/backtrace.h>

#include <zce_share_mem_mmap.h>
#include <zce_share_mem_posix.h>
#include <zce_share_mem_systemv.h>

#include <zce/shm_container/common.h>
#include <zce_shm_hash_rehash.h>
#include <zce_shm_hash_expire.h>
#include <zce_shm_hash_table.h>
#include <zce_shm_rbtree.h>
#include <zce_shm_array.h>
#include <zce_shm_list.h>
#include <zce_shm_cache_chunk.h>
#include <zce_lockfree_kfifo.h>
#include <zce_shm_avltree.h>

#include <zce_time_progress_timer.h>


#include <zce_boost_random.h>
#include <zce_lock_thread_mutex.h>
#include <zce_lock_thread_rw_mutex.h>
#include <zce/lock/lock_guard.h>
#include <zce_thread_msgque_condi.h>
#include <zce_thread_msgque_sema.h>
#include <zce/thread/thread_task.h>


#include <zce/timer/handler_base.h>
#include <zce_timer_queue_heap.h>
#include <zce_timer_queue_wheel.h>


#include <zce/string/helper.h>
#include <zce_string_format.h>
#include <zce_string_extend.h>


#include <zce_mysql_connect.h>
#include <zce_mysql_command.h>
#include <zce_id_to_string.h>

#include <zce_config_getopt.h>


#include <zce_bytes_hash.h>
#include <zce_bytes_encrypt.h>
#include <zce_bytes_encode.h>
#include <zce_bytes_encrypt.h>
#include <zce_bytes_compress.h>
#include <zce_bytes_serialize.h>


#include <zce_event_handle_base.h>
#include <zce_event_handle_inotify.h>
#include <zce_event_reactor_select.h>
#include <zce_event_reactor_wfmo.h>

#include <zce/async/async_base.h>
#include <zce_async_fsm.h>
#include <zce_async_coroutine.h>

#include <zce_script_lua_tie.h>

#include <zce_server_base.h>
#include <zce_server_status.h>


#include <zce_config_property_tree.h>
#include <zce_config_file_implement.h>
#include <zce_config_getopt.h>


//BOOST的代码，用于一些对比测试
#if defined ZCE_OS_WINDOWS
#pragma warning ( disable : 4819)
#endif


//#include <boost/cstdint.hpp>
//#include <boost/random.hpp>
//#include <boost/date_time/posix_time/posix_time.hpp>
//#include <boost/date_time/posix_time/posix_time_types.hpp>

#if defined ZCE_OS_WINDOWS
#pragma warning ( default : 4819)
#endif



#endif //