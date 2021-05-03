
#ifndef ZERG_SERVER_PRE_DEFINE_H_
#define ZERG_SERVER_PRE_DEFINE_H_

#include <zce/predefine.h>
#include <zce/time/time_value.h>
#include <zce/mysql/connect.h>
#include <zce/mysql/command.h>
#include <zce/mysql/execute.h>
#include <zce/shm_container/cache_chunk.h>
#include <zce/lockfree/kfifo.h>
#include <zce/server/server_base.h>
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
#include <zce/os_adapt/backtrace.h>
#include <zce/logger/logging.h>
#include <zce/lock/null_lock.h>
#include <zce/lock/thread_mutex.h>
#include <zce/config/property_tree.h>
#include <zce/config/file_implement.h>
#include <zce/thread/msgque_sema.h>
#include <zce/thread/msgque_nonlock.h>
#include <zce/timer/handler_base.h>
#include <zce/timer/queue_wheel.h>
#include <zce/os_adapt/shm.h>
#include <zce/bus/mmap_pipe.h>
#include <zce/bus/twoway_pipe.h>
#include <zce/thread/thread_task.h>
#include <zce/event/handle_base.h>
#include <zce/event/reactor_base.h>
#include <zce/os_adapt/getopt.h>
#include <zce/socket/socket_base.h>
#include <zce/socket/acceptor.h>
#include <zce/socket/connector.h>
#include <zce/socket/stream.h>
#include <zce/server/mml_command.h>

#include <soar/predefine.h>
#include <soar/enum/error_code.h>
#include <soar/svrd/cfg_base.h>
#include <soar/svrd/app_main.h>
#include <soar/zerg/frame_zerg.h>
#include <soar/svrd/mmap_buspipe.h>
#include <soar/svrd/app_base.h>
#include <soar/svrd/app_main.h>
#include <soar/stat/monitor.h>
#include <soar/svrd/timer_base.h>




//那天无法调试，google找到的解决方案。
//there is no source code available for the current location 解决方案
//
//1.首先试最常规的方法：Clean and then rebuild solution，但是没有解决
//2.进入Tools>Options,选择Debugging>General 却掉 Enable address-level debugging 选项，在去掉 Require source files to exactly match the original version.
//Okay，解决问题。

//好几个地方都用这个常量，统一设置，以后好改
static const socklen_t ZERG_SND_RCV_BUF_OPVAL = 320 * 1024;

#endif //_ZERG_SERVER_PRE_DEFINE_H_

