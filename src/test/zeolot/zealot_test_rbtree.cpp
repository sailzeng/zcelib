


#include "zealot_predefine.h"
#include "zealot_test_function.h"

/*

int test_mmap_rbtree()
{
    typedef mmap_rb_tree<int,int>  TEST_RB_TREE;
    TEST_RB_TREE*            test_rb_tree;
    size_t  size_of_tree = 16;

    //MMAP内存文件，
    ACE_Mem_Map         mmap_file_;
    char mmap_file[64]={0};
    snprintf(mmap_file,sizeof(mmap_file)-1,"./TEST_RB_TREE.MMAP");
    ACE_OS::unlink(mmap_file);

    size_t sz_malloc = TEST_RB_TREE::getallocsize(size_of_tree);

    //处理共享内存的操作方式
    int prot = PROT_RDWR;
    int flags = O_RDWR|O_CREAT;
    int share = ACE_MAP_SHARED;

    //MAP一个文件
    if (mmap_file_.map(mmap_file,static_cast<int>(sz_malloc),
        flags, ACE_DEFAULT_FILE_PERMS, prot, share) != 0)
    {
        return 0;
    }

    char *pt_rb_tree = static_cast<char *>( mmap_file_.addr());

    //初始化
    test_rb_tree = TEST_RB_TREE::initialize(size_of_tree,pt_rb_tree, false);
    if ( NULL == test_rb_tree)
    {
        return 0;
    }

    _mmap_rb_tree_index *index_base = test_rb_tree->getindexbase();
    memset(index_base, 0, sizeof(_mmap_rb_tree_index) * (size_of_tree+2));

    test_rb_tree->clear();
    bool b_flag = test_rb_tree->empty();
    b_flag = test_rb_tree->full();
    printf("Before insert:size=%u, capacity=%u, empty=%u, full=%u\n",
        test_rb_tree->size(), test_rb_tree->capacity(), test_rb_tree->empty(), test_rb_tree->full() );

    test_rb_tree->insert_equal(10);
    test_rb_tree->insert_equal(7);
    test_rb_tree->insert_equal(8);
    test_rb_tree->insert_equal(15);
    test_rb_tree->insert_equal(5);
    test_rb_tree->insert_equal(12);
    test_rb_tree->insert_equal(6);
    test_rb_tree->insert_equal(11);
    test_rb_tree->insert_equal(12);

    printf("\nAfter insert:size=%u, capacity=%u, empty=%u, full=%u\n",
        test_rb_tree->size(), test_rb_tree->capacity(), test_rb_tree->empty(), test_rb_tree->full() );

    printf("After insert Tree_node: ");
    TEST_RB_TREE::iterator  rb_tree_iter = test_rb_tree->begin();
    TEST_RB_TREE::iterator  rb_tree_iter_end = test_rb_tree->end();
    for (; rb_tree_iter != rb_tree_iter_end; ++rb_tree_iter )
    {
        printf("%d ", (*rb_tree_iter) );
    }

    TEST_RB_TREE::iterator iter = test_rb_tree->find(12);
    printf("\nfind : %d \n", (*iter));

    int aa = test_rb_tree->erase_equal(12);

    printf("\nAfter erase:size=%u, capacity=%u, empty=%u, full=%u\n",
        test_rb_tree->size(), test_rb_tree->capacity(), test_rb_tree->empty(), test_rb_tree->full() );

    printf("After erase Tree_node: ");
    rb_tree_iter = test_rb_tree->begin();
    rb_tree_iter_end = test_rb_tree->end();
    for (; rb_tree_iter != rb_tree_iter_end; ++rb_tree_iter )
    {
        printf("%d ", (*rb_tree_iter) );
    }

    iter = test_rb_tree->find(12);
    printf("\nfind : %d \n", (*iter));
    return 0;
}
*/




