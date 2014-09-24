
#ifndef OGRE_SERVER_PRE_DEFINE_H_
#define OGRE_SERVER_PRE_DEFINE_H_

#include <zen_predefine.h>
#include <zen_id_to_string.h>
#include <zen_time_value.h>
#include <zen_mysql_connect.h>
#include <zen_mysql_command.h>
#include <zen_mysql_process.h>
#include <zen_shm_cache_chunk.h>
#include <zen_shm_lockfree_deque.h>
#include <zen_server_toolkit.h>
#include <zen_shm_hash_table.h>
#include <zen_boost_non_copyable.h>
#include <zen_boost_lord_rings.h>
#include <zen_os_adapt_error.h>
#include <zen_os_adapt_string.h>
#include <zen_os_adapt_file.h>
#include <zen_os_adapt_thread.h>
#include <zen_os_adapt_socket.h>
#include <zen_os_adapt_thread.h>
#include <zen_os_adapt_process.h>
#include <zen_os_adapt_getopt.h>
#include <zen_os_adapt_dlfcn.h>
#include <zen_trace_log_debug.h>
#include <zen_trace_log_msg.h>
#include <zen_lock_null_lock.h>
#include <zen_lock_thread_mutex.h>
#include <zen_config_property_tree.h>
#include <zen_config_ini_implement.h>
#include <zen_thread_msgque_sema.h>
#include <zen_thread_msgque_nonlock.h>
#include <zen_timer_handler_base.h>
#include <zen_timer_queue_wheel.h>
#include <zen_share_mem_mmap.h>
#include <zen_bus_mmap_pipe.h>
#include <zen_bus_two_way.h>
#include <zen_thread_task.h>
#include <zen_event_handle_base.h>
#include <zen_event_reactor_base.h>
#include <zen_config_getopt.h>
#include <zen_config_ini_implement.h>
#include <zen_socket_base.h>
#include <zen_socket_acceptor.h>
#include <zen_socket_connector.h>
#include <zen_socket_stream.h>

#include <comm_predefine.h>
#include <comm_error_code.h>
#include <comm_random_number.h>
#include <comm_service_config.h>
#include <comm_app_frame.h>
#include <comm_zerg_mmappipe.h>
#include <comm_mml_command.h>
#include <comm_mml_console_handler.h>
#include <comm_cfgsvr_sdk.h>
#include <comm_encrypt_arithmetic.h>
#include <comm_cmd_statistic.h>
#include <comm_service_config.h>
#include <comm_svrd_config.h>
#include <comm_svrd_application.h>
#include <comm_svrd_app_main.h>
#include <comm_bill_record.h>
#include <comm_stat_monitor.h>
#include <comm_xml_config.h>
#include <comm_ogre_interface.h>

//--------------------------------------------------------------------
typedef hash_set<unsigned int> SetOfIPAddress;

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

#endif //_OGRE_SERVER_PRE_DEFINE_H_

