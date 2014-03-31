


#include "zealot_predefine.h"
#include "zealot_test_function.h"



int test_mmap_rbtree(int /*argc*/, char * /*argv*/[])
{
    typedef ZCE_LIB::shm_rb_tree< int, int >  TEST_RB_TREE;
    TEST_RB_TREE* test_rb_tree;
    size_t  size_of_tree = 16;


    size_t sz_malloc = TEST_RB_TREE::getallocsize(size_of_tree);
   

    char *pt_rb_tree = new char[sz_malloc];
    memset(pt_rb_tree, 0, sz_malloc);

    //初始化
    test_rb_tree = TEST_RB_TREE::initialize(size_of_tree,pt_rb_tree, false);
    if ( NULL == test_rb_tree)
    {
        return 0;
    }
  

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
    auto insert_iter = test_rb_tree->insert_unique(12);
    if (insert_iter.second == true)
    {
        abort();
    }
    else
    {
        printf("%s", "OK\n");
    }

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

    size_t aa = test_rb_tree->erase_equal(12);

    printf("\nAfter erase:size=%u, capacity=%u, empty=%u, full=%u erase count =%lu\n",
        test_rb_tree->size(), test_rb_tree->capacity(), test_rb_tree->empty(), test_rb_tree->full(),
        aa);

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

int test_mmap_avltree(int /*argc*/, char * /*argv*/[])
{
    size_t erase_count = 0;
    typedef ZCE_LIB::shm_avl_tree< int, int >  TEST_AVL_TREE;
    TEST_AVL_TREE* test_avl_tree;
    size_t  size_of_tree = 16;
    TEST_AVL_TREE::iterator  rb_tree_iter, rb_tree_iter_end;
    size_t sz_malloc = TEST_AVL_TREE::getallocsize(size_of_tree);

    char *pt_rb_tree = new char[sz_malloc];
    memset(pt_rb_tree, 0, sz_malloc);

    //初始化
    test_avl_tree = TEST_AVL_TREE::initialize(size_of_tree, pt_rb_tree, false);
    if (NULL == test_avl_tree)
    {
        return 0;
    }


    test_avl_tree->clear();
    bool b_flag = test_avl_tree->empty();
    b_flag = test_avl_tree->full();

    printf("Before insert:size=%u, capacity=%u, empty=%u, full=%u\n",
        test_avl_tree->size(),
        test_avl_tree->capacity(),
        test_avl_tree->empty(),
        test_avl_tree->full());

    test_avl_tree->insert_equal(10);
    test_avl_tree->insert_equal(7);
    test_avl_tree->insert_equal(8);
    test_avl_tree->insert_equal(15);
    test_avl_tree->insert_equal(5);
    test_avl_tree->insert_equal(12);
    test_avl_tree->insert_equal(6);
    test_avl_tree->insert_equal(11);
    test_avl_tree->insert_equal(12);

    printf("\nAfter insert:size=%u, capacity=%u, empty=%u, full=%u\n",
        test_avl_tree->size(),
        test_avl_tree->capacity(),
        test_avl_tree->empty(),
        test_avl_tree->full());
    test_avl_tree->debug_note(test_avl_tree->header());
    test_avl_tree->debug_note(0);
    test_avl_tree->debug_note(1);
    test_avl_tree->debug_note(2);
    test_avl_tree->debug_note(3);
    test_avl_tree->debug_note(4);
    test_avl_tree->debug_note(5);
    test_avl_tree->debug_note(6);
    test_avl_tree->debug_note(7);
    test_avl_tree->debug_note(8);
    test_avl_tree->debug_note(9);
    

    TEST_AVL_TREE::iterator iter = test_avl_tree->find(12);
    printf("\nfind : %d \n", (*iter));

    erase_count = test_avl_tree->erase_unique(12);

    printf("\nAfter erase 12:size=%u, capacity=%u, empty=%u, full=%u erase count =%lu\n",
        test_avl_tree->size(),
        test_avl_tree->capacity(),
        test_avl_tree->empty(),
        test_avl_tree->full(),
        erase_count);
    test_avl_tree->debug_note(test_avl_tree->header());
    test_avl_tree->debug_note(0);
    test_avl_tree->debug_note(1);
    test_avl_tree->debug_note(2);
    test_avl_tree->debug_note(3);
    test_avl_tree->debug_note(4);
    test_avl_tree->debug_note(5);
    test_avl_tree->debug_note(6);
    test_avl_tree->debug_note(7);
    test_avl_tree->debug_note(8);
    test_avl_tree->debug_note(9);

    printf("\nAfter erase Tree_node: ");
    rb_tree_iter = test_avl_tree->begin();
    rb_tree_iter_end = test_avl_tree->end();
    for (; rb_tree_iter != rb_tree_iter_end; ++rb_tree_iter)
    {
        printf("%d ", (*rb_tree_iter));
    }
    printf("%s", "\n");

    erase_count = test_avl_tree->erase_unique(11);
    printf("\nAfter erase 11:size=%u, capacity=%u, empty=%u, full=%u erase count =%lu\n",
        test_avl_tree->size(),
        test_avl_tree->capacity(),
        test_avl_tree->empty(),
        test_avl_tree->full(),
        erase_count);
    test_avl_tree->debug_note(test_avl_tree->header());
    test_avl_tree->debug_note(0);
    test_avl_tree->debug_note(1);
    test_avl_tree->debug_note(2);
    test_avl_tree->debug_note(3);
    test_avl_tree->debug_note(4);
    test_avl_tree->debug_note(5);
    test_avl_tree->debug_note(6);
    test_avl_tree->debug_note(7);
    test_avl_tree->debug_note(8);
    test_avl_tree->debug_note(9);

    printf("\nAfter erase Tree_node: ");
    rb_tree_iter = test_avl_tree->begin();
    rb_tree_iter_end = test_avl_tree->end();
    for (; rb_tree_iter != rb_tree_iter_end; ++rb_tree_iter)
    {
        printf("%d ", (*rb_tree_iter));
    }
    printf("%s", "\n");
    return 0;
}


int test_mmap_avltree1(int /*argc*/, char * /*argv*/[])
{
    size_t erase_count = 0;
    typedef ZCE_LIB::shm_avl_tree< int, int >  TEST_AVL_TREE;
    TEST_AVL_TREE* test_avl_tree;
    size_t  size_of_tree = 16;


    size_t sz_malloc = TEST_AVL_TREE::getallocsize(size_of_tree);


    char *pt_rb_tree = new char[sz_malloc];
    memset(pt_rb_tree, 0, sz_malloc);

    //初始化
    test_avl_tree = TEST_AVL_TREE::initialize(size_of_tree, pt_rb_tree, false);
    if (NULL == test_avl_tree)
    {
        return 0;
    }


    test_avl_tree->clear();
    bool b_flag = test_avl_tree->empty();
    b_flag = test_avl_tree->full();
    printf("Before insert:size=%u, capacity=%u, empty=%u, full=%u\n",
        test_avl_tree->size(), 
        test_avl_tree->capacity(),
        test_avl_tree->empty(),
        test_avl_tree->full());

    test_avl_tree->insert_equal(10);
    test_avl_tree->insert_equal(7);
    test_avl_tree->insert_equal(8);
    test_avl_tree->insert_equal(15);
    test_avl_tree->insert_equal(5);
    test_avl_tree->insert_equal(12);
    test_avl_tree->insert_equal(6);
    test_avl_tree->insert_equal(11);
    test_avl_tree->insert_equal(12);

    auto insert_iter = test_avl_tree->insert_unique(12);
    if (insert_iter.second == true)
    {
        abort();
    }
    else
    {
        printf("%s", "OK\n");
    }

    printf("\nAfter insert:size=%u, capacity=%u, empty=%u, full=%u\n",
        test_avl_tree->size(),
        test_avl_tree->capacity(),
        test_avl_tree->empty(),
        test_avl_tree->full());

    printf("After insert Tree_node: \n");

    TEST_AVL_TREE::iterator  rb_tree_iter = test_avl_tree->begin();
    TEST_AVL_TREE::iterator  rb_tree_iter_end = test_avl_tree->end();
    for (; rb_tree_iter != rb_tree_iter_end; ++rb_tree_iter)
    {
        printf("%d ", (*rb_tree_iter));
    }

    TEST_AVL_TREE::iterator iter = test_avl_tree->find(12);
    printf("\nfind : %d \n", (*iter));

    erase_count = test_avl_tree->erase_unique(12);
    printf("\nAfter erase:size=%u, capacity=%u, empty=%u, full=%u erase count =%lu\n",
        test_avl_tree->size(),
        test_avl_tree->capacity(),
        test_avl_tree->empty(),
        test_avl_tree->full(),
        erase_count);
    printf("\nAfter erase Tree_node: ");
    rb_tree_iter = test_avl_tree->begin();
    rb_tree_iter_end = test_avl_tree->end();
    for (; rb_tree_iter != rb_tree_iter_end; ++rb_tree_iter)
    {
        printf("%d ", (*rb_tree_iter));
    }

    test_avl_tree->clear();
    test_avl_tree->insert_equal(10);
    erase_count = test_avl_tree->erase_unique(10);
    test_avl_tree->debug_note(test_avl_tree->header());
    printf("\nAfter erase Tree_node: ");
    rb_tree_iter = test_avl_tree->begin();
    rb_tree_iter_end = test_avl_tree->end();
    for (; rb_tree_iter != rb_tree_iter_end; ++rb_tree_iter)
    {
        printf("%d ", (*rb_tree_iter));
    }

    test_avl_tree->clear();
    test_avl_tree->insert_equal(7);
    test_avl_tree->insert_equal(8);
    test_avl_tree->insert_equal(15);
    erase_count = test_avl_tree->erase_unique(8);

    test_avl_tree->debug_note(test_avl_tree->header());
    //test_avl_tree->debug_note(0);
    //test_avl_tree->debug_note(1);
    //test_avl_tree->debug_note(2);

    printf("\nAfter erase:size=%u, capacity=%u, empty=%u, full=%u erase count =%lu\n",
        test_avl_tree->size(), 
        test_avl_tree->capacity(),
        test_avl_tree->empty(), 
        test_avl_tree->full(),
        erase_count);
    printf("\nAfter erase Tree_node: ");
    rb_tree_iter = test_avl_tree->begin();
    rb_tree_iter_end = test_avl_tree->end();
    for (; rb_tree_iter != rb_tree_iter_end; ++rb_tree_iter)
    {
        printf("%d ", (*rb_tree_iter));
    }

    erase_count = test_avl_tree->erase_unique(7);
    test_avl_tree->debug_note(test_avl_tree->header());
    rb_tree_iter = test_avl_tree->begin();
    rb_tree_iter_end = test_avl_tree->end();
    for (; rb_tree_iter != rb_tree_iter_end; ++rb_tree_iter)
    {
        printf("%d ", (*rb_tree_iter));
    }

    erase_count = test_avl_tree->erase_unique(10);
    test_avl_tree->debug_note(test_avl_tree->header());
    rb_tree_iter = test_avl_tree->begin();
    rb_tree_iter_end = test_avl_tree->end();
    for (; rb_tree_iter != rb_tree_iter_end; ++rb_tree_iter)
    {
        printf("%d ", (*rb_tree_iter));
    }

    erase_count = test_avl_tree->erase_unique(15);
    test_avl_tree->debug_note(test_avl_tree->header());
    rb_tree_iter = test_avl_tree->begin();
    rb_tree_iter_end = test_avl_tree->end();
    for (; rb_tree_iter != rb_tree_iter_end; ++rb_tree_iter)
    {
        printf("%d ", (*rb_tree_iter));
    }

    iter = test_avl_tree->find(12);
    printf("\nfind : %d \n", (*iter));
    return 0;
}













