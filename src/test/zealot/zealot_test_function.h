#ifndef ZEALOT_TEST_FUNCTION_H_
#define ZEALOT_TEST_FUNCTION_H_



//²âÊÔËæ»úÊý
int test_matchboost_random(int argc, char *argv[]);
int test_template_random(int argc, char *argv[]);
int test_oo_random(int argc, char *argv[]);

//²âÊÔSHM¹²ÏíÄÚ´æµÄÈÝÆ÷
int test_lru_hashtable(int argc, char *argv[]);
int test_lru_hashtable2(int argc, char *argv[]);
int test_lru_hashtable3(int argc, char *argv[]);

int test_log_trace(int argc, char *argv[]);
int test_cachechunk(int argc, char *argv[]);
int test_cache_chunk2(int argc, char *argv[]);

int test_extendstl_cycdeque(int argc, char *argv[]);
int test_extendstl_cycdeque2(int argc, char *argv[]);


int test_hash_safe(int argc, char *argv[]);
int test_hash_safe2(int argc, char *argv[]);
int test_hash_match(int argc, char *argv[]);


int test_osadapt_file(int argc, char *argv[]);
int test_osadapt_perf(int argc, char *argv[]);


int test_nonr_thread_mutex(int argc, char *argv[]);
int test_msgqueue_condi(int argc, char *argv[]);


int test_host_name_fun(int, char * []);

int test_rw_lock1 (int argc, char *argv[]);
int test_rw_lock2 (int argc, char *argv[]);


int test_fmt_strncpy(int argc, char *argv[]);
int test_cpp_log_out(int argc, char *argv[]);


int test_bytes_hash(int argc, char *argv[]);
int test_bytes_encode(int argc, char *argv[]);
int test_bytes_crc32(int argc, char *argv[]);
int test_bytes_compress(int argc, char *argv[]);
int test_bytes_compress2(int argc, char *argv[]);
int test_bytes_compress3(int argc, char *argv[]);
int test_bytes_data_represent(int argc, char *argv[]);

int bytes_encrypt_right(int argc, char *argv[]);
int bytes_encrypt_perf(int argc, char *argv[]);



int test_scandir(int argc, char *argv[]);

//int test_inotify_reactor(int argc, char *argv[]);
int test_inotify_reactor(int argc, char *argv[]);
int test_inotify_reactor2(int /*argc*/, char * /*argv*/[]);

int test_template_random_2(int argc, char *argv[]);
int test_random_elapsed_time(int argc, char *argv[]);
int test_random_repeat_number(int argc, char *argv[]);
int test_random_var_obj_speed(int argc, char *argv[]);

int test_perf_memcpy(int argc, char *argv[]);
int test_perf_nonalignmemcpy(int argc, char *argv[]);

int benchmark_compress(int /*argc*/, char * /*argv*/[]);

int test_out_buffer(int argc, char *argv[]);
int test_log_debug(int /*argc*/, char * /*argv*/[]);

int test_timer_expire(int /*argc*/, char * /*argv*/[]);
int test_timer_expire2(int /*argc*/, char * /*argv*/[]);
int test_os_time(int /*argc*/, char * /*argv*/[]);

int test_server_status(int /*argc*/, char * /*argv*/[]);


int test_coroutine1(int /*argc*/, char * /*argv*/[]);
int test_coroutine2(int /*argc*/, char * /*argv*/[]);


int test_mmap_rbtree1(int /*argc*/, char * /*argv*/[]);
int test_mmap_rbtree2(int /*argc*/, char * /*argv*/[]);
int test_mmap_rbtree3(int /*argc*/, char * /*argv*/[]);

int test_mmap_avltree1(int /*argc*/, char * /*argv*/[]);
int test_mmap_avltree2(int /*argc*/, char * /*argv*/[]);
int test_mmap_avltree3(int /*argc*/, char * /*argv*/[]);
int test_mmap_avltree4(int /*argc*/, char * /*argv*/[]);
int test_mmap_avltree5(int /*argc*/, char * /*argv*/[]);


//Lua º¯Êý²âÊÔ¡£
int test_lua_script1(int /*argc*/, char * /*argv*/[]);
int test_lua_script2(int /*argc*/, char * /*argv*/[]);
int test_lua_script3(int /*argc*/, char * /*argv*/[]);
int test_lua_script4(int /*argc*/, char * /*argv*/[]);
int test_lua_script5(int /*argc*/, char * /*argv*/[]);
int test_lua_script6(int /*argc*/, char * /*argv*/[]);
int test_lua_script7(int /*argc*/, char * /*argv*/[]);
int test_lua_script8(int /*argc*/, char * /*argv*/[]);
int test_lua_script9(int /*argc*/, char * /*argv*/[]);

//
int test_async_fsm(int  /*argc*/, char * /*argv*/[]);
int test_async_coroutine(int  /*argc*/, char * /*argv*/[]);

//
int test_conf_xml(int  /*argc*/, char * /*argv*/[]);
int test_conf_ini(int  /*argc*/, char * /*argv*/[]);


int test_file(int /*argc*/, char * /*argv*/[]);
int test_pid_file(int /*argc*/, char * /*argv*/[]);

int test_container_performance(int  /*argc*/, char * /*argv*/[]);


int test_net_getaddrinfo(int argc, char *argv[]);

#endif //


