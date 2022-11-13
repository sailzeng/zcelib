#ifndef ZEALOT_PRE_DEFINE_H_
#define ZEALOT_PRE_DEFINE_H_

//ZCELIB的代码

#include <zce/predefine.h>
#include <zce/logger/logging.h>
#include <zce/logger/log_file.h>
#include <zce/logger/log_msg.h>
#include <zce/logger/log_print.h>

#include <zce/os_adapt/define.h>
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

#include <zce/lockfree/kfifo.h>
#include <zce/shm_container/avltree.h>

#include <zce/time/time_value.h>
#include <zce/time/progress_timer.h>

#include <zce/util/random.h>
#include <zce/lock/thread_mutex.h>
#include <zce/lock/thread_rw_mutex.h>
#include <zce/lock/lock_guard.h>
#include <zce/thread/msgque_condi.h>
#include <zce/thread/msgque_sema.h>
#include <zce/thread/thread_task.h>

#include <zce/timer/timer_handler.h>
#include <zce/timer/queue_heap.h>
#include <zce/timer/queue_wheel.h>

#include <zce/string/to_string.h>
#include <zce/string/format.h>
#include <zce/string/extend.h>

#include <zce/mysql/connect.h>
#include <zce/mysql/command.h>
#include <zce/util/id_to_string.h>
#include <zce/util/mpl.h>

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

#include <zce/sqlite/sqlite_hdl.h>
#include <zce/sqlite/sqlite_stmt.h>
#include <zce/sqlite/sqlite_result.h>

#include <zce/net/dns_resolve.h>
#include <zce/net/ping.h>

#include <zce/buffer/cycle_buffer.h>
#include <zce/buffer/queue_buffer.h>

#include <zce/pool/object_pool.h>
#include <zce/pool/buffer_pool.h>
#include <zce/pool/dataptr_pool.h>
#include <zce/pool/shareptr_pool.h>
#include <zce/pool/chunk_pool.h>

#include <zce/aio/caller.h>
#include <zce/aio/worker.h>
#include <zce/aio/awaiter.h>

#include <thread>
#include <mutex>

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

#ifndef ZEALOT_TEST_FUNCTION_H_
#define ZEALOT_TEST_FUNCTION_H_

//测试随机数
int test_matchboost_random(int argc, char* argv[]);
int test_template_random(int argc, char* argv[]);
int test_oo_random(int argc, char* argv[]);

//测试SHM共享内存的容器
int test_lru_hashtable(int argc, char* argv[]);
int test_lru_hashtable2(int argc, char* argv[]);
int test_lru_hashtable3(int argc, char* argv[]);

int test_log_trace(int argc, char* argv[]);
int test_cachechunk(int argc, char* argv[]);
int test_cache_chunk2(int argc, char* argv[]);

int test_hash_safe(int argc, char* argv[]);
int test_hash_safe2(int argc, char* argv[]);
int test_hash_match(int argc, char* argv[]);

int test_osadapt_file(int argc, char* argv[]);
int test_osadapt_perf(int argc, char* argv[]);

int test_nonr_thread_mutex(int argc, char* argv[]);
int test_msgqueue_condi(int argc, char* argv[]);

int test_host_name_fun(int, char*[]);

int test_rw_lock1(int argc, char* argv[]);
int test_rw_lock2(int argc, char* argv[]);

int test_fmt_splice(int argc, char* argv[]);
int test_cpp_log_out(int argc, char* argv[]);
int test_foo_snprintf(int argc, char* argv[]);

int test_bytes_hash(int argc, char* argv[]);
int test_bytes_encode(int argc, char* argv[]);
int test_bytes_crc32(int argc, char* argv[]);
int test_bytes_compress(int argc, char* argv[]);
int test_bytes_compress2(int argc, char* argv[]);
int test_bytes_compress3(int argc, char* argv[]);
int test_bytes_data_represent(int argc, char* argv[]);

int bytes_encrypt_right(int argc, char* argv[]);
int bytes_encrypt_perf(int argc, char* argv[]);

int test_scandir(int argc, char* argv[]);

int test_inotify_reactor(int argc, char* argv[]);
int test_inotify_reactor2(int /*argc*/, char* /*argv*/[]);

int test_template_random_2(int argc, char* argv[]);
int test_random_elapsed_time(int argc, char* argv[]);
int test_random_repeat_number(int argc, char* argv[]);
int test_random_var_obj_speed(int argc, char* argv[]);

int test_perf_memcpy(int argc, char* argv[]);
int test_perf_nonalignmemcpy(int argc, char* argv[]);

int benchmark_compress(const char* file_name);

int test_out_buffer(int argc, char* argv[]);
int test_log_debug(int /*argc*/, char* /*argv*/[]);

int test_timer_expire(int /*argc*/, char* /*argv*/[]);
int test_timer_expire2(int /*argc*/, char* /*argv*/[]);
int test_os_time(int /*argc*/, char* /*argv*/[]);

int test_server_status(int /*argc*/, char* /*argv*/[]);

int test_coroutine1(int /*argc*/, char* /*argv*/[]);
int test_coroutine2(int /*argc*/, char* /*argv*/[]);

int test_mmap_rbtree1(int /*argc*/, char* /*argv*/[]);
int test_mmap_rbtree2(int /*argc*/, char* /*argv*/[]);
int test_mmap_rbtree3(int /*argc*/, char* /*argv*/[]);

int test_mmap_avltree1(int /*argc*/, char* /*argv*/[]);
int test_mmap_avltree2(int /*argc*/, char* /*argv*/[]);
int test_mmap_avltree3(int /*argc*/, char* /*argv*/[]);
int test_mmap_avltree4(int /*argc*/, char* /*argv*/[]);
int test_mmap_avltree5(int /*argc*/, char* /*argv*/[]);

//Lua 函数测试。
int test_lua_script1(int /*argc*/, char* /*argv*/[]);
int test_lua_script2(int /*argc*/, char* /*argv*/[]);
int test_lua_script3(int /*argc*/, char* /*argv*/[]);
int test_lua_script4(int /*argc*/, char* /*argv*/[]);
int test_lua_script5(int /*argc*/, char* /*argv*/[]);
int test_lua_script6(int /*argc*/, char* /*argv*/[]);
int test_lua_script7(int /*argc*/, char* /*argv*/[]);
int test_lua_script8(int /*argc*/, char* /*argv*/[]);
int test_lua_script9(int /*argc*/, char* /*argv*/[]);

//
int test_async_fsm(int  /*argc*/, char* /*argv*/[]);
int test_async_coroutine(int  /*argc*/, char* /*argv*/[]);

//
int test_conf_xml(int  /*argc*/, char* /*argv*/[]);
int test_conf_ini(int  /*argc*/, char* /*argv*/[]);

int test_pid_file(int /*argc*/, char* /*argv*/[]);

int test_container_performance(int  /*argc*/, char* /*argv*/[]);

int test_progress_timer(int  /*argc*/, char* /*argv*/[]);

int test_net_getaddrinfo(int argc, char* argv[]);

int test_memory_debug(int /*argc*/, char* /*argv */[]);
int test_back_stack(int /*argc*/, char* /*argv */[]);

int test_sqlite_handle(int /*argc*/, char* /*argv */[]);
int test_sqlite_stmt(int /*argc*/, char* /*argv */[]);
int test_sqlite_configtable(int /*argc*/, char* /*argv */[]);

int test_fifo_cycbuf1(int /*argc*/, char* /*argv*/[]);
int test_lockfree_ring_ptr1(int /*argc*/, char* /*argv*/[]);
int test_lockfree_ring_ptr2(int /*argc*/, char* /*argv*/[]);
int test_lockfree_ring1(int /*argc*/, char* /*argv*/[]);
int test_lockfree_ring2(int /*argc*/, char* /*argv*/[]);
int test_pool(int /*argc*/, char* /*argv*/[]);
int test_multiobj_pool(int /*argc*/, char* /*argv*/[]);

int test_rudp(int /*argc*/, char* /*argv*/[]);
int test_rudp_core(int /*argc*/, char* /*argv*/[]);
int test_rudp_client(int /*argc*/, char* /*argv*/[]);

int test_dns_resolve(int argc, char* argv[]);
int test_ping(int argc, char* argv[]);

//
int test_aio1(int argc, char* argv[]);
int test_aio3(int argc, char* argv[]);
int test_msgring_condi(int argc, char* argv[]);
int test_aio4(int argc, char* argv[]);
#endif //

#endif //