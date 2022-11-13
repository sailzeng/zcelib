#pragma once

#include <zce/predefine.h>
#include <zce/time/time_value.h>
#include <zce/logger/logging.h>
#include <zce/logger/log_msg.h>
#include <zce/mysql/connect.h>
#include <zce/mysql/command.h>
#include <zce/mysql/execute.h>
#include <zce/lockfree/kfifo.h>
#include <zce/server/server_base.h>
#include <zce/shm_container/hash_table.h>
#include <zce/util/non_copyable.h>
#include <zce/container/lord_rings.h>
#include <zce/os_adapt/error.h>
#include <zce/os_adapt/string.h>
#include <zce/os_adapt/file.h>
#include <zce/os_adapt/thread.h>
#include <zce/os_adapt/socket.h>
#include <zce/os_adapt/thread.h>
#include <zce/os_adapt/process.h>
#include <zce/os_adapt/getopt.h>
#include <zce/os_adapt/dlfcn.h>
#include <zce/lock/null_lock.h>
#include <zce/lock/thread_mutex.h>
#include <zce/config/property_tree.h>
#include <zce/config/file_implement.h>
#include <zce/thread/msgque_sema.h>
#include <zce/thread/msgque_condi.h>
#include <zce/timer/timer_handler.h>
#include <zce/timer/queue_wheel.h>
#include <zce/shared_mem/mmap.h>
#include <zce/bus/mmap_pipe.h>
#include <zce/bus/twoway_pipe.h>
#include <zce/event/handle_base.h>
#include <zce/event/reactor_base.h>
#include <zce/server/get_option.h>
#include <zce/socket/socket_base.h>
#include <zce/socket/acceptor.h>
#include <zce/socket/connector.h>
#include <zce/socket/stream.h>

#include <soar/predefine.h>
#include <soar/enum/error_code.h>
#include <soar/svrd/cfg_base.h>
#include <soar/ogre/frame_ogre.h>
#include <soar/svrd/app_bus.h>
#include <soar/svrd/svrd_buspipe.h>
#include <soar/svrd/app_main.h>
#include <soar/enum/enum_define.h>
#include <soar/stat/monitor.h>
#include <soar/svrd/timer_base.h>

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
