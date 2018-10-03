#ifndef ZEALOT_PRE_DEFINE_H_
#define ZEALOT_PRE_DEFINE_H_



//ZCELIB的代码

#include <zce_predefine.h>
#include <zce_trace_debugging.h>
#include <zce_trace_log_plus.h>
#include <zce_trace_log_msg.h>
#include <zce_trace_aid_debug.h>

#include <zce_os_adapt_predefine.h>
#include <zce_os_adapt_file.h>
#include <zce_os_adapt_process.h>
#include <zce_os_adapt_sysinfo.h>
#include <zce_os_adapt_error.h>
#include <zce_os_adapt_socket.h>
#include <zce_os_adapt_rwlock.h>
#include <zce_os_adapt_dirent.h>
#include <zce_os_adapt_math.h>
#include <zce_os_adapt_stdlib.h>
#include <zce_os_adapt_coroutine.h>
#include <zce_os_adapt_flock.h>

#include <zce_share_mem_mmap.h>
#include <zce_share_mem_posix.h>
#include <zce_share_mem_systemv.h>

#include <zce_shm_predefine.h>
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
#include <zce_lock_guard.h>
#include <zce_thread_msgque_condi.h>
#include <zce_thread_msgque_sema.h>
#include <zce_thread_task.h>


#include <zce_timer_handler_base.h>
#include <zce_timer_queue_heap.h>
#include <zce_timer_queue_wheel.h>


#include <zce_fmtstr_helper.h>
#include <zce_fmtstr_format.h>
#include <zce_fmtstr_snprintf.h>
#include <zce_fmtstr_strnsplice.h>


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

#include <zce_async_fw_base.h>
#include <zce_async_fw_fsm.h>
#include <zce_async_fw_coroutine.h>

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