
#ifndef OGRE_SERVER_PRE_DEFINE_H_
#define OGRE_SERVER_PRE_DEFINE_H_

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
#include <zce/os_adapt/dlfcn.h>
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
#include <zce/event/handle_base.h>
#include <zce/event/reactor_base.h>
#include <zce/config/getopt.h>
#include <zce/socket/socket_base.h>
#include <zce/socket/acceptor.h>
#include <zce/socket/connector.h>
#include <zce/socket/stream.h>

#include <soar_predefine.h>
#include <soar_error_code.h>
#include <soar_svrd_cfg_base.h>
#include <soar_ogre_frame.h>
#include <soar_mmap_buspipe.h>
#include <soar_svrd_app_bus.h>
#include <soar_svrd_app_main.h>
#include <soar_enum_define.h>
#include <soar_stat_monitor.h>
#include <soar_svrd_timer_base.h>

//--------------------------------------------------------------------
typedef std::unordered_set<unsigned int> SetOfIPAddress;

////统计计数器,方便查询SVR的状态

//服务统计计数器
//enum ZERGLING_STAT
//{
//    ACCEPT_PEER_NUMBER,                  //正在Accept的PEER数量
//    ACCEPT_PEER_COUNTER,                 //已经Accept的PEER计数
//    CONNECT_PEER_NUMBER,                 //正在Connect的PEER数量
//    CONNECT_PEER_COUNTER,                //已经Connect的PEER计数
//    SEND_ERROR_COUNTER,                  //错误发送的计数器
//    RETRY_SEND_COUNTER,                  //重新发送的计数器
//    RETRY_SEND_SUCCESS,                  //重新发送成功的计数器
//    BUFFER_STORAGE_NUMBER,               //BUFFER缓冲区的CHUNK个数,
//    NUMBER_OF_COUNTER,                   //
//};

#endif //OGRE_SERVER_PRE_DEFINE_H_

