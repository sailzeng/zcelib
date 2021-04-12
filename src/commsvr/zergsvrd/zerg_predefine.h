
#ifndef ZERG_SERVER_PRE_DEFINE_H_
#define ZERG_SERVER_PRE_DEFINE_H_

#include <zce/predefine.h>
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
#include <zce/os_adapt/backtrace.h>
#include <zce/logger/logging.h>
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
#include <zce_mml_command.h>

#include <soar_predefine.h>
#include <soar_error_code.h>
#include <soar_svrd_cfg_base.h>
#include <soar_svrd_app_main.h>
#include <soar_zerg_frame.h>
#include <soar_mmap_buspipe.h>
#include <soar_svrd_app_base.h>
#include <soar_svrd_app_main.h>
#include <soar_stat_monitor.h>
#include <soar_svrd_timer_base.h>




//�����޷����ԣ�google�ҵ��Ľ��������
//there is no source code available for the current location �������
//
//1.���������ķ�����Clean and then rebuild solution������û�н��
//2.����Tools>Options,ѡ��Debugging>General ȴ�� Enable address-level debugging ѡ���ȥ�� Require source files to exactly match the original version.
//Okay��������⡣

//�ü����ط��������������ͳһ���ã��Ժ�ø�
static const socklen_t ZERG_SND_RCV_BUF_OPVAL = 320 * 1024;

#endif //_ZERG_SERVER_PRE_DEFINE_H_

