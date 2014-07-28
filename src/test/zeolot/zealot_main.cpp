
//#define _STLP_USE_STATIC_LIB


#include "zealot_predefine.h"
#include "zealot_test_function.h"


int main(int argc , char *argv[])
{



    ZCE_Trace_LogMsg::instance()->init_stdout();

    //test_lua_script1(argc, argv);
    //test_lua_script3(argc, argv);

    printf("%s", "----------------------------------------------------------------------------\n");
    test_lua_script9(argc, argv);
    printf("%s", "----------------------------------------------------------------------------\n");

    system("pause");

#if 0
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

    return 0;
}


