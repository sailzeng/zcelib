/*!
* @copyright  2004-2014  Apache License, Version 2.0 FULLSAIL
* @filename   soar_predefine.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2006��6��22��
* @brief      ���ģ�����ļ��Ļ���ͷ�ļ������һ�����������.
*
*
* @details
*
*
*
* @note       Ϊ�˱���һЩ���ֲ��ִ���Ĵ��࣬
*
*  ���һǹ
*  �޽�
*  һ����������������
*  ɲ�Ǽ�����ӿ��������
*  Ŷ
*  ֻ����ˮ
*  Ŷ
*  û�б���
*  �����������һǹ
*  ��Ը������Ī����ٹ�
*  Ŷ
*  ���һǹ
*  Ŷ
*  ���һǹ
*  ��֪���ж��� �ж��ٻ���û��
*  ��֪���ж��� �ж��ٻ���û��
*  ��֪���ж��� �����˺���һ��
*  ��֪���ж��� ���ٸ����һǹ
*  ��˯������ů��������
*  ��¶Ϧ����ľ�Էҷ�
*  ŶŶ ֻ��һ�仰 ����������
*  ��֪���ж��� �ж��ٻ���û��
*  ��֪���ж��� �ж��ٻ���û��
*  ��֪���ж��� �����˺���һ��
*  ��֪���ж��� ���ٸ����һǹ
*  һ�����������ҵ�����
*  ɲ�Ǽ�����ӿ���ҵ�����
*  ŶŶ ���һǹ
*  ŶŶ ���һǹ
*  ŶŶ ���һǹ
*  ŶŶ ���һǹ
*  ŶŶ ���һǹ
*  ŶŶ ���һǹ
*  ŶŶ ���һǹ
*  ŶŶ ���һǹ
*/


#ifndef SOARING_LIB_PER_DEFINE_H_
#define SOARING_LIB_PER_DEFINE_H_

//��WINDOWS�º�POSIX��׼���ݵĺ꣬2003������Σ���û�в���
#define _CRT_NONSTDC_NO_DEPRECATE 1
#define _CRT_SECURE_NO_WARNINGS   1



//---------------------------------------------------------------------------------------------

//ZCElib�Ŀ��ļ�
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
#include <zce/os_adapt/string.h>
#include <zce/os_adapt/thread.h>
#include <zce/os_adapt/socket.h>
#include <zce/os_adapt/thread.h>
#include <zce/os_adapt/error.h>
#include <zce/os_adapt/process.h>
#include <zce/os_adapt/socket.h>
#include <zce/os_adapt/getopt.h>
#include <zce/os_adapt/file.h>
#include <zce/os_adapt/dirent.h>
#include <zce_share_mem_mmap.h>
#include <zce_share_mem_posix.h>
#include <zce/log/logging.h>
#include <zce_log_msg.h>
#include <zce_lock_null_lock.h>
#include <zce_lock_thread_mutex.h>
#include <zce_config_property_tree.h>
#include <zce_config_file_implement.h>
#include <zce_thread_msgque_sema.h>
#include <zce_thread_msgque_nonlock.h>
#include <zce/timer/handler_base.h>
#include <zce/timer/queue_base.h>
#include <zce_timer_queue_wheel.h>
#include <zce/bus/twoway_pipe.h>
#include <zce/thread/thread_task.h>
#include <zce/socket/addr_base.h>
#include <zce/socket/addr_in.h>
#include <zce/socket/socket_base.h>
#include <zce/socket/stream.h>
#include <zce_socket_datagram.h>
#include <zce_socket_connector.h>
#include <zce_event_handle_base.h>
#include <zce_event_reactor_base.h>
#include <zce_event_reactor_select.h>
#include <zce_event_reactor_epoll.h>
#include <zce/thread/thread_task.h>
#include <zce_thread_wait_mgr.h>
#include <zce_config_getopt.h>
#include <zce_server_status.h>
#include <zce_thread_bus_pipe.h>
#include <zce_lock_file_lock.h>
#include <zce_bytes_hash.h>
#include <zce_bytes_encode.h>
#include <zce/async/async_base.h>
#include <zce_async_fsm.h>

//
#if defined (ZCE_OS_WINDOWS) && defined (_MSC_VER)

#if !defined SOAR_LIB_LIBARY_NAME
#define SOAR_LIB_LIBARY_NAME   "soarlib.lib"
#endif

//�Զ������İ������ӣ�����Ĳ���
#pragma comment(lib, SOAR_LIB_LIBARY_NAME  )

#endif


//------------------------------------------------------------------------------------
//�ͻ��˷��������õ�ͷ�ļ�,��������ͷ�ļ�����������

#if defined ZCE_OS_WINDOWS && defined TSSCOMM_HASDLL
#ifdef BUILD_SOARING_MODULE_DLL
#define SOARING_EXPORT __declspec (dllexport)
#else
#define SOARING_EXPORT __declspec (dllimport)
#endif
#else
#define SOARING_EXPORT
#endif


#endif //SOARING_LIB_PER_DEFINE_H_

