
#ifndef OGRE_SERVER_PRE_DEFINE_H_
#define OGRE_SERVER_PRE_DEFINE_H_

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
#include <zce_os_adapt_error.h>
#include <zce_os_adapt_string.h>
#include <zce_os_adapt_file.h>
#include <zce_os_adapt_thread.h>
#include <zce_os_adapt_socket.h>
#include <zce_os_adapt_thread.h>
#include <zce_os_adapt_process.h>
#include <zce_os_adapt_getopt.h>
#include <zce_os_adapt_dlfcn.h>
#include <zce_trace_log_debug.h>
#include <zce_trace_log_msg.h>
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

