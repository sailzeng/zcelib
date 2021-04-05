#ifndef _WORMHOLE_SERVER_PRE_DEFINE_H_
#define _WORMHOLE_SERVER_PRE_DEFINE_H_

//ZENLIB
#include <zce/predefine.h>
#include <zce_id_to_string.h>
#include <zce/time/time_value.h>
#include <zce_mysql_connect.h>
#include <zce_mysql_command.h>
#include <zce_mysql_process.h>
#include <zce_shm_cache_chunk.h>
#include <zce_lockfree_kfifo.h>
#include <zce_server_base.h>
#include <zce_shm_hash_table.h>
#include <zce/util/non_copyable.h>
#include <zce_boost_lord_rings.h>
#include <zce/os_adapt/error.h>
#include <zce/os_adapt/string.h>
#include <zce/os_adapt/file.h>
#include <zce/os_adapt/thread.h>
#include <zce/os_adapt/socket.h>
#include <zce/os_adapt/thread.h>
#include <zce/os_adapt/process.h>
#include <zce/os_adapt/getopt.h>
#include <zce/log/logging.h>
#include <zce_log_msg.h>
#include <zce_lock_null_lock.h>
#include <zce_lock_thread_mutex.h>
#include <zce_config_property_tree.h>
#include <zce_config_file_implement.h>
#include <zce_thread_msgque_sema.h>
#include <zce_thread_msgque_nonlock.h>
#include <zce/timer/handler_base.h>
#include <zce_timer_queue_wheel.h>
#include <zce_share_mem_mmap.h>
#include <zce/bus/mmap_pipe.h>
#include <zce/bus/twoway_pipe.h>
#include <zce/thread/thread_task.h>
#include <zce_server_status.h>
#include <zce_event_handle_base.h>
#include <zce_event_reactor_base.h>
#include <zce_config_getopt.h>
#include <zce/socket/socket_base.h>
#include <zce/socket/acceptor.h>
#include <zce_socket_connector.h>
#include <zce/socket/stream.h>
#include <zce_bytes_hash.h>

#include <soar_predefine.h>
#include <soar_error_code.h>
#include <soar_svrd_cfg_base.h>
#include <soar_zerg_frame.h>
#include <soar_mmap_buspipe.h>
#include <soar_svrd_app_bus.h>
#include <soar_svrd_app_main.h>
#include <soar_enum_define.h>
#include <soar_stat_monitor.h>
#include <soar_svrd_timer_base.h>





#endif //_WORMHOLE_SERVER_PRE_DEFINE_H_

