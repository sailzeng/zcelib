#ifndef _WORMHOLE_SERVER_PRE_DEFINE_H_
#define _WORMHOLE_SERVER_PRE_DEFINE_H_

//ZENLIB
#include <zce/predefine.h>
#include <zce_id_to_string.h>
#include <zce/time/time_value.h>
#include <zce/mysql/connect.h>
#include <zce/mysql/command.h>
#include <zce/mysql/process.h>
#include <zce/shm_container/cache_chunk.h>
#include <zce/lockfree/kfifo.h>
#include <zce/server/base.h>
#include <zce/shm_container/hash_table.h>
#include <zce/util/non_copyable.h>
#include <zce/util/lord_rings.h>
#include <zce/os_adapt/error.h>
#include <zce/os_adapt/string.h>
#include <zce/os_adapt/file.h>
#include <zce/os_adapt/thread.h>
#include <zce/os_adapt/socket.h>
#include <zce/os_adapt/thread.h>
#include <zce/os_adapt/process.h>
#include <zce/os_adapt/getopt.h>
#include <zce/logger/logging.h>
#include <zce_log_msg.h>
#include <zce/lock/null_lock.h>
#include <zce/lock/thread_mutex.h>
#include <zce/config/property_tree.h>
#include <zce/config/file_implement.h>
#include <zce/thread/msgque_sema.h>
#include <zce_thread_msgque_nonlock.h>
#include <zce/timer/handler_base.h>
#include <zce/timer/queue_wheel.h>
#include <zce_share_mem_mmap.h>
#include <zce/bus/mmap_pipe.h>
#include <zce/bus/twoway_pipe.h>
#include <zce/thread/thread_task.h>
#include <zce/server/status.h>
#include <zce/event/handle_base.h>
#include <zce/event/reactor_base.h>
#include <zce/config/getopt.h>
#include <zce/socket/socket_base.h>
#include <zce/socket/acceptor.h>
#include <zce/socket/connector.h>
#include <zce/socket/stream.h>
#include <zce/bytes/hash_value.h>

#include <soar/predefine.h>
#include <soar/enum/error_code.h>
#include <soar/svrd/cfg_base.h>
#include <soar/zerg/frame_zerg.h>
#include <soar/svrd/mmap_buspipe.h>
#include <soar/svrd/app_bus.h>
#include <soar/svrd/app_main.h>
#include <soar/enum/enum_define.h>
#include <soar/stat/monitor.h>
#include <soar/svrd/timer_base.h>





#endif //_WORMHOLE_SERVER_PRE_DEFINE_H_

