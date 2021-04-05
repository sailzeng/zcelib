#ifndef ZEALOT_PRE_DEFINE_H_
#define ZEALOT_PRE_DEFINE_H_



//ZCELIB的代码

#include <zce/predefine.h>
#include <zce/log/logging.h>
#include <zce/log/plus.h>
#include <zce/log/msg.h>

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

#include <zce/shared_mem/mmap.h>
#include <zce/shared_mem/posix.h>
#include <zce/shared_mem/systemv.h>

#include <zce/shm_container/common.h>
#include <zce/shm_container/hash_rehash.h>
#include <zce/shm_container/hash_expire.h>
#include <zce/shm_container/hash_table.h>
#include <zce/shm_container/rbtree.h>
#include <zce/shm_container/array.h>
#include <zce/shm_container/list.h>
#include <zce/shm_container/cache_chunk.h>
#include <zce/lockfree/kfifo.h>
#include <zce/shm_container/avltree.h>

#include <zce/time/progress_timer.h>


#include <zce/util/random.h>
#include <zce/lock/thread_mutex.h>
#include <zce/lock/thread_rw_mutex.h>
#include <zce/lock/lock_guard.h>
#include <zce/thread/msgque_condi.h>
#include <zce/thread/msgque_sema.h>
#include <zce/thread/thread_task.h>


#include <zce/timer/handler_base.h>
#include <zce/timer/queue_heap.h>
#include <zce/timer/queue_wheel.h>


#include <zce/string/helper.h>
#include <zce/string/format.h>
#include <zce/string/extend.h>


#include <zce/mysql/connect.h>
#include <zce/mysql/command.h>
#include <zce/util/id_to_string.h>

#include <zce/server/get_option.h>


#include <zce/bytes/hash_value.h>
#include <zce/bytes/encrypt.h>
#include <zce/bytes/base_encode.h>
#include <zce/bytes/encrypt.h>
#include <zce/bytes/compress.h>
#include <zce/bytes/serialize.h>


#include <zce/event/handle_base.h>
#include <zce/event/handle_inotify.h>
#include <zce/event/reactor_select.h>
#include <zce/event/reactor_wfmo.h>

#include <zce/async/async_base.h>
#include <zce/async/fsm.h>
#include <zce/async/coroutine.h>

#include <zce/script/lua_tie.h>

#include <zce/server/server_base.h>
#include <zce/server/server_status.h>
#include <zce/server/get_option.h>

#include <zce/config/property_tree.h>
#include <zce/config/file_implement.h>



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