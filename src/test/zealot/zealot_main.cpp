//#define _STLP_USE_STATIC_LIB

#include "predefine.h"
#include "zealot_main.h"


namespace zeal {

    // 用于收集多个测试过滤项
    static std::vector<std::string>& GetFilterList() {
        static std::vector<std::string> filters;
        return filters;
    }

    // 添加一个过滤项，如 "Suite.Test" 或 "Suite.*"
    void AddTestFilter(const std::string& filter) {
        GetFilterList().push_back(filter);
    }

    // 应用过滤项：构造并设置 ::testing::GTEST_FLAG(filter)
    void ApplyTestFilters() {
        const auto& filters = GetFilterList();
        std::string final_filter;
        for (const auto& f : filters) {
            if (!final_filter.empty()) {
                final_filter += ":";
            }
            final_filter += f;
        }
        if (!final_filter.empty()) {
            ::testing::GTEST_FLAG(filter) = final_filter;
        }
    }

}


int main(int argc, char* argv[])
{
#if defined ZCE_OS_WINDOWS
    SetConsoleOutputCP(65001);
#endif

    ::testing::InitGoogleTest(&argc, argv);

    zce::log_msg::instance()->open_time_log(LOGFILE_DEVIDE::BY_TIME_DAY,
                                            "D:\\My.Log\\TEST_2",
                                            zce::log_file::DEFAULT_LOG_SIZE,
                                            true,
                                            false,
                                            true,
                                            false,
                                            ZCE_U32_OR_2(zce::LOG_OUTPUT::LOGFILE, zce::LOG_OUTPUT::ERROUT),
                                            static_cast<int>(zce::LOG_HEAD::LOGLEVEL));
    int ret = 0;
    ZCE_TRACE_FILELINE(RS_DEBUG);
    ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::main", ret);

    zeal::AddTestFilter("SQLiteTestSuite.TestAPI");
    //zeal::AddTestFilter("UtilTestSuite.TestAPI");
    //zeal::AddTestFilter("CoroutineTestSuite.TestAPI");
    //zeal::AddTestFilter("SerializeTestSuite.TestAPI");
    //zeal::AddTestFilter("OsadaptTestSuite.TestAPI");
    zeal::ApplyTestFilters();
    ret = RUN_ALL_TESTS();

    //!需要关闭日志
    zce::log_msg::instance()->close();
    zce::log_msg::clear_inst();
    return 0;
#if 0
    ret = test_big_uint64(argc, argv);
    test_dns_resolve(argc, argv);
    test_rudp(argc, argv);
    return 0;


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
