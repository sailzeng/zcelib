//#define _STLP_USE_STATIC_LIB

#include "predefine.h"

int main(int argc, char* argv[])
{
    SetConsoleOutputCP(65001);
    ZCE_UNUSED_ARG(argc);
    ZCE_UNUSED_ARG(argv);

#define ZCE_USE_LOGMSG 1
#if 0
    zce::LogMsg::instance()->init_time_log(LOGFILE_DEVIDE::BY_TIME_DAY,
                                           "E:\\My.Log\\TEST_2",
                                           0,
                                           true,
                                           false,
                                           true,
                                           ZCE_U32_OR_2(LOG_OUTPUT::LOGFILE, LOG_OUTPUT::ERROUT),
                                           static_cast<int>(LOG_HEAD::LOGLEVEL));
    ZCE_TRACE_FILELINE(RS_DEBUG);
#endif
    test_ping(argc, argv);
#if 0
    test_dns_resolve(argc, argv);
    test_rudp(argc, argv);
    return 0;
    test_sqlite_handle(argc, argv);
    test_sqlite_stmt(argc, argv);

    test_memory_debug(argc, argv);
    test_back_stack(argc, argv);
    test_out_buffer(argc, argv);
    test_progress_timer(argc, argv);
    test_bytes_hash(argc, argv);

    benchmark_compress("D:\\TestDir\\compress\\txt\\05.txt");

    test_net_getaddrinfo(argc, argv);

    test_bytes_data_represent(argc, argv);

    test_inotify_reactor(argc, argv);
    test_conf_ini(argc, argv);
    test_conf_xml(argc, argv);

    test_async_coroutine(argc, argv);

    //bytes_encrypt_perf(argc, argv);
    //test_lua_script1(argc, argv);
    //test_lua_script3(argc, argv);

    printf("%s", "----------------------------------------------------------------------------\n");
    test_lua_script1(argc, argv);
    test_lua_script2(argc, argv);
    test_lua_script3(argc, argv);
    test_lua_script4(argc, argv);
    test_lua_script4(argc, argv);
    test_lua_script5(argc, argv);
    test_lua_script6(argc, argv);
    test_lua_script7(argc, argv);
    test_lua_script8(argc, argv);
    test_lua_script9(argc, argv);
    printf("%s", "----------------------------------------------------------------------------\n");

    //test_perf_memcpy(argc, argv);
    //test_perf_nonalignmemcpy(argc,argv);
    test_out_buffer(argc, argv);

    printf("%s", "----------------------------------------------------------------------------\n");
    test_mmap_rbtree1(argc, argv);
    test_mmap_rbtree2(argc, argv);

    printf("%s", "----------------------------------------------------------------------------\n");

    test_mmap_avltree5(argc, argv);
    //test_mmap_avltree2(argc, argv);

    //std::cin.ignore().get();

    //rc_stab();
    //bytes_encrypt_right(argc, argv);
    //bytes_encrypt_perf(argc, argv);
    //system("pause");
    //test_rw_lock2(argc, argv);
    //system("pause");
    //test_rw_lock1(argc, argv);
#endif
}