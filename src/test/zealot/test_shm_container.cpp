#include "predefine.h"

int test_cachechunk(int /*argc*/, char* /*argv*/[])
{
    size_t szalloc = zce::shm_cachechunk::getallocsize(4, 32, 32);
    bool bret = false;
    std::cout << "need mem: " << (int)szalloc << std::endl;
    std::cout << "sizeof(smem_cachechunk)" << sizeof(zce::shm_cachechunk) << std::endl;
    char* tmproom = new char[szalloc + 4];
    memset(tmproom, 0, szalloc + 4);

    zce::shm_cachechunk* pmmap = zce::shm_cachechunk::initialize(4, 32, 32, tmproom, false);
    char tmpbuf[512];
    char tmpbuf1[68] = { "1234567890123456789012345678901234567890" };
    tmpbuf1[67] = '\0';
    size_t usenode;
    pmmap->push_node(68, tmpbuf1, usenode);
    size_t szdatalen;
    pmmap->pull_node(usenode, szdatalen, tmpbuf);
    char tmpbuf2[32] = { "12345678901234567890" };
    pmmap->push_node(32, tmpbuf2, usenode);
    pmmap->pull_node(usenode, szdatalen, tmpbuf);
    usenode = 0;
    pmmap->free_node(usenode);

    char tmpbuf3[168] = { "12345678901234567890123456789012345678901234567890123456789012345678901234567890" };
    bret = pmmap->push_node(168, tmpbuf3, usenode);
    pmmap->pull_node(usenode, szdatalen, tmpbuf);
    pmmap->free_node(usenode);

    bret = pmmap->push_node(168, tmpbuf3, usenode);
    pmmap->pull_node(usenode, szdatalen, tmpbuf);
    bret = pmmap->push_node(32, tmpbuf2, usenode);
    bret = pmmap->push_node(32, tmpbuf2, usenode);
    bret = pmmap->push_node(32, tmpbuf2, usenode);
    usenode = 3;
    pmmap->free_node(usenode);
    char tmpbuf4[2048] = { ":(---)" };
    bret = pmmap->push_node(2048, tmpbuf4, usenode);
    bret = pmmap->push_node(704, tmpbuf4, usenode);
    bret = pmmap->push_node(672, tmpbuf4, usenode);

    ZCE_UNUSED_ARG(bret);
    return 0;
}

int test_cache_chunk2()
{
    char cachebuf[2000];

    char testdata[] = "12345678";
    zce::shm_cachechunk* testchunk = NULL;
    size_t testindex, testfreenode, testfreechunk, testfreeroom;
    testchunk = zce::shm_cachechunk::initialize(10, 100, 10, cachebuf);
    testchunk->free(testfreenode, testfreechunk, testfreeroom);
    std::cout << "free chunk:" << testfreechunk << std::endl;

    testchunk->push_node(9, testdata, testindex);
    testchunk->free(testfreenode, testfreechunk, testfreeroom);

    char tmpbuf[128];
    size_t szdatalen = 0;
    testchunk->pull_node(testindex, szdatalen, tmpbuf);

    std::cout << "index:" << testindex << " " << "free chunk:" << testfreechunk << std::endl;
    testchunk->free_node(testindex);
    testchunk->free(testfreenode, testfreechunk, testfreeroom);
    std::cout << "free chunk:" << testfreechunk << std::endl;

    return 0;
}

int test_list(int /*argc*/, char* /*argv*/[])
{
    const size_t numnode = 100;

    //size_t szalloc1 = shm_list<void *>::getallocsize(numnode,40);

    size_t szalloc = zce::shm_list<int>::getallocsize(numnode);
    std::cout << "need mem: " << (int)szalloc << std::endl;
    std::cout << "sizeof(shm_list<int >)" << sizeof(zce::shm_list<int >) << std::endl;
    char* tmproom = new char[szalloc + 4];
    memset(tmproom, 0, szalloc + 4);

    zce::shm_list<int>* pmmap = zce::shm_list<int>::initialize(numnode, tmproom, false);

    std::cout << "capacity:" << (int)pmmap->capacity() << std::endl;

    int tmpx = 0;

    for (tmpx = 0; tmpx < 100; tmpx++)
    {
        pmmap->push_front(tmpx);
    }

    for (zce::shm_list<int>::iterator it = pmmap->begin(); it != pmmap->end(); ++it)
    {
        std::cout << " " << *it;
    }

    std::cout << std::endl;
    std::cout << "size:" << (int)pmmap->size() << std::endl;
    std::cout << "sizefreenode:" << (int)pmmap->sizefreenode() << std::endl;

    for (tmpx = 0; tmpx < 10; tmpx++)
    {
        pmmap->pop_back();
    }

    for (tmpx = 0; tmpx < 10; tmpx++)
    {
        pmmap->push_back(tmpx);
    }

    for (zce::shm_list<int>::iterator it = pmmap->begin(); it != pmmap->end(); ++it)
    {
        std::cout << " " << *it;
    }

    std::cout << std::endl;
    std::cout << "size:" << (int)pmmap->size() << std::endl;
    std::cout << "sizefreenode:" << (int)pmmap->sizefreenode() << std::endl;

    for (tmpx = 0; tmpx < 100; tmpx++)
    {
        pmmap->pop_back();
    }

    for (tmpx = 0; tmpx < 10; tmpx++)
    {
        pmmap->push_back(tmpx);
    }

    for (tmpx = 0; tmpx < 10; tmpx++)
    {
        pmmap->push_front(tmpx);
    }

    for (zce::shm_list<int>::iterator it = pmmap->begin(); it != pmmap->end(); ++it)
    {
        std::cout << " " << *it;
    }

    std::cout << std::endl;
    std::cout << "size:" << (int)pmmap->size() << std::endl;
    std::cout << "sizefreenode:" << (int)pmmap->sizefreenode() << std::endl;

    //少打一个Begin
    for (zce::shm_list<int>::iterator it = --pmmap->end(); it != pmmap->begin(); --it)
    {
        std::cout << " " << *it;
    }

    std::cout << std::endl;

    for (tmpx = 0; tmpx < 5; tmpx++)
    {
        pmmap->pop_back();
    }

    for (tmpx = 0; tmpx < 5; tmpx++)
    {
        pmmap->pop_front();
    }

    for (zce::shm_list<int>::iterator it = pmmap->begin(); it != pmmap->end(); ++it)
    {
        std::cout << " " << *it;
    }

    std::cout << std::endl;
    std::cout << "size:" << (int)pmmap->size() << std::endl;
    std::cout << "sizefreenode:" << (int)pmmap->sizefreenode() << std::endl;

    zce::shm_list<int>::iterator ittmp = pmmap->end();
    ittmp--;
    ittmp--;
    ittmp--;
    pmmap->move_begin(ittmp, pmmap->end());

    for (zce::shm_list<int>::iterator it = pmmap->begin(); it != pmmap->end(); ++it)
    {
        std::cout << " " << *it;
    }

    std::cout << std::endl;
    std::cout << "size:" << (int)pmmap->size() << std::endl;
    std::cout << "sizefreenode:" << (int)pmmap->sizefreenode() << std::endl;

    return 0;
}

//用于测试RB Tree 或者 AVL Tree

int test_mmap_rbtree1(int /*argc*/, char* /*argv*/[])
{
    typedef zce::shm_rb_tree< int, int >  TEST_RB_TREE;
    TEST_RB_TREE* test_rb_tree;
    size_t  size_of_tree = 16;

    size_t sz_malloc = TEST_RB_TREE::getallocsize(size_of_tree);

    char* pt_rb_tree = new char[sz_malloc];
    memset(pt_rb_tree, 0, sz_malloc);

    //初始化
    test_rb_tree = TEST_RB_TREE::initialize(size_of_tree, pt_rb_tree, false);
    if (NULL == test_rb_tree)
    {
        return 0;
    }

    test_rb_tree->clear();
    //bool b_flag = test_rb_tree->empty();
    //b_flag = test_rb_tree->full();
    printf("Before insert:size=%zu, capacity=%zu, empty=%u, full=%u\n",
           test_rb_tree->size(), test_rb_tree->capacity(), test_rb_tree->empty(), test_rb_tree->full());

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

    printf("\nAfter insert:size=%zu, capacity=%zu, empty=%u, full=%u\n",
           test_rb_tree->size(), test_rb_tree->capacity(), test_rb_tree->empty(), test_rb_tree->full());

    printf("After insert Tree_node: ");
    TEST_RB_TREE::iterator  tree_iter = test_rb_tree->begin();
    TEST_RB_TREE::iterator  tree_iter_end = test_rb_tree->end();
    for (; tree_iter != tree_iter_end; ++tree_iter)
    {
        printf("%d ", (*tree_iter));
    }

    TEST_RB_TREE::iterator iter = test_rb_tree->find(12);
    printf("\nfind : %d \n", (*iter));

    size_t erase_count = 0;

    erase_count = test_rb_tree->erase_unique(5);

    printf("\nAfter erase:size=%zu, capacity=%zu, empty=%u, full=%u erase count =%zu\n",
           test_rb_tree->size(), test_rb_tree->capacity(), test_rb_tree->empty(), test_rb_tree->full(),
           erase_count);

    printf("After erase Tree_node: ");
    tree_iter = test_rb_tree->begin();
    tree_iter_end = test_rb_tree->end();
    for (; tree_iter != tree_iter_end; ++tree_iter)
    {
        printf("%d ", (*tree_iter));
    }

    erase_count = test_rb_tree->erase_equal(12);

    printf("\nAfter erase:size=%zu, capacity=%zu, empty=%u, full=%u erase count =%zu\n",
           test_rb_tree->size(), test_rb_tree->capacity(), test_rb_tree->empty(), test_rb_tree->full(),
           erase_count);

    printf("After erase Tree_node: ");
    tree_iter = test_rb_tree->begin();
    tree_iter_end = test_rb_tree->end();
    for (; tree_iter != tree_iter_end; ++tree_iter)
    {
        printf("%d ", (*tree_iter));
    }

    iter = test_rb_tree->find(12);
    printf("\nfind : %d \n", (*iter));

    erase_count = test_rb_tree->erase_unique(15);

    printf("\nAfter erase:size=%zu, capacity=%zu, empty=%u, full=%u erase count =%zu\n",
           test_rb_tree->size(), test_rb_tree->capacity(), test_rb_tree->empty(), test_rb_tree->full(),
           erase_count);

    printf("After erase Tree_node: ");
    tree_iter = test_rb_tree->begin();
    tree_iter_end = test_rb_tree->end();
    for (; tree_iter != tree_iter_end; ++tree_iter)
    {
        printf("%d ", (*tree_iter));
    }

    return 0;
}

//使用随机数测试，RB Tree
int test_mmap_rbtree2(int /*argc*/, char* /*argv*/[])
{
    typedef zce::shm_rb_tree< int, int >  TEST_RB_TREE;
    TEST_RB_TREE* test_rb_tree;
    const size_t  SIZE_OF_TREE = 2000;
    TEST_RB_TREE::iterator  tree_iter, tree_iter_end;
    size_t sz_malloc = TEST_RB_TREE::getallocsize(SIZE_OF_TREE);

    char* pt_avl_tree = new char[sz_malloc];
    memset(pt_avl_tree, 0, sz_malloc);

    //初始化
    test_rb_tree = TEST_RB_TREE::initialize(SIZE_OF_TREE, pt_avl_tree, false);
    if (NULL == test_rb_tree)
    {
        return 0;
    }

    //
    const uint32_t TEST_SEED = 1010123;
    zce::random_mt11213b  mt11231b_gen(TEST_SEED);
    int ins = 0;
    size_t ins_count = 0;
    for (ins_count = 0; ins_count < SIZE_OF_TREE;)
    {
        ins = mt11231b_gen.rand() % SIZE_OF_TREE;
        auto ins_iter = test_rb_tree->insert_unique(ins);
        if (ins_iter.second == true)
        {
            ++ins_count;
        }
    }

    auto f_iter = test_rb_tree->insert_unique(5);
    if (f_iter.second == true)
    {
        ZCE_ASSERT_ALL(false);
    }
    printf("%s", "\n==========================================================\n");
    printf("After insert:size=%zu, capacity=%zu, empty=%u, full=%u\n",
           test_rb_tree->size(),
           test_rb_tree->capacity(),
           test_rb_tree->empty(),
           test_rb_tree->full());

    tree_iter = test_rb_tree->begin();
    tree_iter_end = test_rb_tree->end();
    int a = 0, b = 0;
    for (; tree_iter != tree_iter_end;)
    {
        printf("%d ", (*tree_iter));

        a = (*tree_iter);
        ++tree_iter;
        if (tree_iter != tree_iter_end)
        {
            b = (*tree_iter);
            if (a >= b)
            {
                ZCE_ASSERT_ALL(false);
            }
        }
    }

    //printf("%s", "\n-----------------------------------------------------\n");
    //for (size_t u = 0; u <= SIZE_OF_TREE; ++u)
    //{
    //    test_avl_tree->debug_note(u);
    //}
    //printf("%s", "\n-----------------------------------------------------\n");

    int del = 0;
    size_t erase_count = 0;
    size_t del_count = 0;
    printf("%s", "\n");
    for (erase_count = 0; erase_count < SIZE_OF_TREE;)
    {
        del = mt11231b_gen.rand() % SIZE_OF_TREE;
        del_count = test_rb_tree->erase_unique(del);
        if (del_count > 0)
        {
            erase_count += del_count;
            printf("del %d .After del list:", del);
            tree_iter = test_rb_tree->begin();
            tree_iter_end = test_rb_tree->end();
            for (; tree_iter != tree_iter_end; ++tree_iter)
            {
                printf("%d ", (*tree_iter));
            }
            printf("%s", "\n");

            a = (*tree_iter);
            ++tree_iter;
            if (tree_iter != tree_iter_end)
            {
                b = (*tree_iter);
                if (a >= b)
                {
                    ZCE_ASSERT_ALL(false);
                }
            }

            //printf("%s", "\n-----------------------------------------------------\n");
            //for (size_t u = 0; u <= SIZE_OF_TREE; ++u)
            //{
            //    test_avl_tree->debug_note(u);
            //}
            //printf("%s", "\n-----------------------------------------------------\n");
        }
    }
    printf("%s", "\n---------------------------------------------\n");
    printf("\nAfter erase :size=%zu, capacity=%zu, empty=%u, full=%u\n",
           test_rb_tree->size(),
           test_rb_tree->capacity(),
           test_rb_tree->empty(),
           test_rb_tree->full());

    delete[] pt_avl_tree;
    pt_avl_tree = NULL;

    return 0;
}

//测试，AVL Tree
int test_mmap_avltree1(int /*argc*/, char* /*argv*/[])
{
    typedef zce::shmc::avl_tree< int, int >  TEST_AVL_TREE;
    TEST_AVL_TREE* test_avl_tree;
    const size_t  SIZE_OF_TREE = 500;
    TEST_AVL_TREE::iterator  tree_iter, tree_iter_end;
    size_t sz_malloc = TEST_AVL_TREE::getallocsize(SIZE_OF_TREE);

    char* pt_avl_tree = new char[sz_malloc];
    memset(pt_avl_tree, 0, sz_malloc);

    //初始化
    test_avl_tree = TEST_AVL_TREE::initialize(SIZE_OF_TREE, pt_avl_tree, false);
    if (NULL == test_avl_tree)
    {
        return 0;
    }

    //
    const uint32_t TEST_SEED = 1010123;
    zce::random_mt11213b  mt11231b_gen(TEST_SEED);

    size_t ins_count = 0;
    for (ins_count = 0; ins_count < SIZE_OF_TREE;)
    {
        int ins = mt11231b_gen.rand() % SIZE_OF_TREE;
        auto iter = test_avl_tree->insert_unique(ins);
        if (iter.second == true)
        {
            ++ins_count;
        }
    }

    auto f_iter = test_avl_tree->insert_unique(1);
    if (f_iter.second == true)
    {
        ZCE_ASSERT_ALL(false);
    }

    printf("Before insert:size=%zu, capacity=%zu, empty=%u, full=%u\n",
           test_avl_tree->size(),
           test_avl_tree->capacity(),
           test_avl_tree->empty(),
           test_avl_tree->full());

    tree_iter = test_avl_tree->begin();
    tree_iter_end = test_avl_tree->end();
    int a = 0, b = 0;
    for (; tree_iter != tree_iter_end;)
    {
        printf("%d ", (*tree_iter));
        a = (*tree_iter);
        ++tree_iter;
        if (tree_iter != tree_iter_end)
        {
            b = (*tree_iter);
            if (a >= b)
            {
                ZCE_ASSERT_ALL(false);
            }
        }
    }

    delete[] pt_avl_tree;
    pt_avl_tree = NULL;

    return 0;
}

//使用随机数测试，AVL Tree
int test_mmap_avltree2(int /*argc*/, char* /*argv*/[])
{
    typedef zce::shmc::avl_tree< int, int >  TEST_AVL_TREE;
    TEST_AVL_TREE* test_avl_tree;
    const size_t  SIZE_OF_TREE = 2000;
    TEST_AVL_TREE::iterator  tree_iter, tree_iter_end;
    size_t sz_malloc = TEST_AVL_TREE::getallocsize(SIZE_OF_TREE);

    char* pt_avl_tree = new char[sz_malloc];
    memset(pt_avl_tree, 0, sz_malloc);

    //初始化
    test_avl_tree = TEST_AVL_TREE::initialize(SIZE_OF_TREE, pt_avl_tree, false);
    if (NULL == test_avl_tree)
    {
        return 0;
    }

    //
    const uint32_t TEST_SEED = 1010123;
    zce::random_mt11213b  mt11231b_gen(TEST_SEED);
    int ins = 0;
    size_t ins_count = 0;
    for (ins_count = 0; ins_count < SIZE_OF_TREE;)
    {
        ins = mt11231b_gen.rand() % SIZE_OF_TREE;
        auto ins_iter = test_avl_tree->insert_unique(ins);
        if (ins_iter.second == true)
        {
            ++ins_count;
        }
    }

    auto f_iter = test_avl_tree->insert_unique(5);
    if (f_iter.second == true)
    {
        ZCE_ASSERT_ALL(false);
    }
    printf("%s", "\n==========================================================\n");
    printf("After insert:size=%zu, capacity=%zu, empty=%u, full=%u\n",
           test_avl_tree->size(),
           test_avl_tree->capacity(),
           test_avl_tree->empty(),
           test_avl_tree->full());

    tree_iter = test_avl_tree->begin();
    tree_iter_end = test_avl_tree->end();
    int a = 0, b = 0;
    for (; tree_iter != tree_iter_end; )
    {
        printf("%d ", (*tree_iter));

        bool ok = test_avl_tree->check_balanced(tree_iter.getserial());
        if (!ok)
        {
            printf("(No OK)");
        }

        a = (*tree_iter);
        ++tree_iter;
        if (tree_iter != tree_iter_end)
        {
            b = (*tree_iter);
            if (a >= b)
            {
                ZCE_ASSERT_ALL(false);
            }
        }
    }

    //printf("%s", "\n-----------------------------------------------------\n");
    //for (size_t u = 0; u <= SIZE_OF_TREE; ++u)
    //{
    //    test_avl_tree->debug_note(u);
    //}
    //printf("%s", "\n-----------------------------------------------------\n");

    int del = 0;
    size_t erase_count = 0;
    size_t del_count = 0;
    for (erase_count = 0; erase_count < SIZE_OF_TREE;)
    {
        del = mt11231b_gen.rand() % SIZE_OF_TREE;
        del_count = test_avl_tree->erase_unique(del);
        if (del_count > 0)
        {
            erase_count += del_count;
            printf("del %d .After del list:", del);
            tree_iter = test_avl_tree->begin();
            tree_iter_end = test_avl_tree->end();
            for (; tree_iter != tree_iter_end; ++tree_iter)
            {
                printf("%d ", (*tree_iter));

                bool ok = test_avl_tree->check_balanced(tree_iter.getserial());
                if (!ok)
                {
                    printf("(No OK)");
                }
            }
            printf("%s", "\n");

            a = (*tree_iter);
            ++tree_iter;
            if (tree_iter != tree_iter_end)
            {
                b = (*tree_iter);
                if (a >= b)
                {
                    ZCE_ASSERT_ALL(false);
                }
            }

            //printf("%s", "\n-----------------------------------------------------\n");
            //for (size_t u = 0; u <= SIZE_OF_TREE; ++u)
            //{
            //    test_avl_tree->debug_note(u);
            //}
            //printf("%s", "\n-----------------------------------------------------\n");
        }
    }
    printf("%s", "\n---------------------------------------------\n");
    printf("\nAfter erase :size=%zu, capacity=%zu, empty=%u, full=%u\n",
           test_avl_tree->size(),
           test_avl_tree->capacity(),
           test_avl_tree->empty(),
           test_avl_tree->full());

    delete[] pt_avl_tree;
    pt_avl_tree = NULL;

    return 0;
}

int test_mmap_avltree3(int /*argc*/, char* /*argv*/[])
{
    size_t erase_count = 0;
    typedef zce::shmc::avl_tree< int, int >  TEST_AVL_TREE;
    TEST_AVL_TREE* test_avl_tree;
    size_t  SIZE_OF_TREE = 16;

    size_t sz_malloc = TEST_AVL_TREE::getallocsize(SIZE_OF_TREE);

    char* pt_avl_tree = new char[sz_malloc];
    memset(pt_avl_tree, 0, sz_malloc);

    //初始化
    test_avl_tree = TEST_AVL_TREE::initialize(SIZE_OF_TREE, pt_avl_tree, false);
    if (NULL == test_avl_tree)
    {
        return 0;
    }

    test_avl_tree->clear();
    //bool b_flag = test_avl_tree->empty();
    //b_flag = test_avl_tree->full();
    printf("Before insert:size=%zu, capacity=%zu, empty=%u, full=%u\n",
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

    printf("\nAfter insert:size=%zu, capacity=%zu, empty=%u, full=%u\n",
           test_avl_tree->size(),
           test_avl_tree->capacity(),
           test_avl_tree->empty(),
           test_avl_tree->full());

    printf("After insert Tree_node: \n");

    TEST_AVL_TREE::iterator  tree_iter = test_avl_tree->begin();
    TEST_AVL_TREE::iterator  tree_iter_end = test_avl_tree->end();
    for (; tree_iter != tree_iter_end; ++tree_iter)
    {
        printf("%d ", (*tree_iter));
    }

    TEST_AVL_TREE::iterator iter = test_avl_tree->find(12);
    printf("\nfind : %d \n", (*iter));

    erase_count = test_avl_tree->erase_unique(12);
    printf("\nAfter erase:size=%zu, capacity=%zu, empty=%u, full=%u erase count =%zu\n",
           test_avl_tree->size(),
           test_avl_tree->capacity(),
           test_avl_tree->empty(),
           test_avl_tree->full(),
           erase_count);
    printf("\nAfter erase Tree_node: ");
    tree_iter = test_avl_tree->begin();
    tree_iter_end = test_avl_tree->end();
    for (; tree_iter != tree_iter_end; ++tree_iter)
    {
        printf("%d ", (*tree_iter));
    }

    test_avl_tree->clear();
    test_avl_tree->insert_equal(10);
    erase_count = test_avl_tree->erase_unique(10);
    printf("\nAfter erase Tree_node: ");
    tree_iter = test_avl_tree->begin();
    tree_iter_end = test_avl_tree->end();
    for (; tree_iter != tree_iter_end; ++tree_iter)
    {
        printf("%d ", (*tree_iter));
    }

    test_avl_tree->clear();
    test_avl_tree->insert_equal(7);
    test_avl_tree->insert_equal(8);
    test_avl_tree->insert_equal(15);
    erase_count = test_avl_tree->erase_unique(8);

    printf("\nAfter erase:size=%zu, capacity=%zu, empty=%u, full=%u erase count =%zu\n",
           test_avl_tree->size(),
           test_avl_tree->capacity(),
           test_avl_tree->empty(),
           test_avl_tree->full(),
           erase_count);
    printf("\nAfter erase Tree_node: ");
    tree_iter = test_avl_tree->begin();
    tree_iter_end = test_avl_tree->end();
    for (; tree_iter != tree_iter_end; ++tree_iter)
    {
        printf("%d ", (*tree_iter));
    }

    erase_count = test_avl_tree->erase_unique(7);
    tree_iter = test_avl_tree->begin();
    tree_iter_end = test_avl_tree->end();
    for (; tree_iter != tree_iter_end; ++tree_iter)
    {
        printf("%d ", (*tree_iter));
    }

    erase_count = test_avl_tree->erase_unique(10);
    tree_iter = test_avl_tree->begin();
    tree_iter_end = test_avl_tree->end();
    for (; tree_iter != tree_iter_end; ++tree_iter)
    {
        printf("%d ", (*tree_iter));
    }

    erase_count = test_avl_tree->erase_unique(15);
    tree_iter = test_avl_tree->begin();
    tree_iter_end = test_avl_tree->end();
    for (; tree_iter != tree_iter_end; ++tree_iter)
    {
        printf("%d ", (*tree_iter));
    }

    iter = test_avl_tree->find(12);
    printf("\nfind : %d \n", (*iter));

    delete[] pt_avl_tree;
    pt_avl_tree = NULL;

    return 0;
}

//测试4种插入情况的平衡旋转
int test_mmap_avltree4(int /*argc*/, char* /*argv*/[])
{
    typedef zce::shmc::avl_tree< int, int >  TEST_AVL_TREE;
    TEST_AVL_TREE* test_avl_tree;
    const size_t  SIZE_OF_TREE = 500;
    TEST_AVL_TREE::iterator  tree_iter, tree_iter_end;
    size_t sz_malloc = TEST_AVL_TREE::getallocsize(SIZE_OF_TREE);

    char* pt_avl_tree = new char[sz_malloc];
    memset(pt_avl_tree, 0, sz_malloc);

    //初始化
    test_avl_tree = TEST_AVL_TREE::initialize(SIZE_OF_TREE, pt_avl_tree, false);
    if (NULL == test_avl_tree)
    {
        return 0;
    }

    test_avl_tree->clear();
    test_avl_tree->insert_unique(15);
    test_avl_tree->insert_unique(10);
    test_avl_tree->insert_unique(5);

    printf("Before insert:size=%zu, capacity=%zu, empty=%u, full=%u\n",
           test_avl_tree->size(),
           test_avl_tree->capacity(),
           test_avl_tree->empty(),
           test_avl_tree->full());

    tree_iter = test_avl_tree->begin();
    tree_iter_end = test_avl_tree->end();
    printf("data list: \n");
    for (; tree_iter != tree_iter_end; ++tree_iter)
    {
        printf("%d ", (*tree_iter));
    }
    printf("%s", "\n");

    //
    test_avl_tree->clear();
    test_avl_tree->insert_unique(5);
    test_avl_tree->insert_unique(10);
    test_avl_tree->insert_unique(15);

    printf("Before insert:size=%zu, capacity=%zu, empty=%u, full=%u\n",
           test_avl_tree->size(),
           test_avl_tree->capacity(),
           test_avl_tree->empty(),
           test_avl_tree->full());

    tree_iter = test_avl_tree->begin();
    tree_iter_end = test_avl_tree->end();
    printf("data list: \n");
    for (; tree_iter != tree_iter_end; ++tree_iter)
    {
        printf("%d ", (*tree_iter));
    }
    printf("%s", "\n");

    //
    test_avl_tree->clear();
    test_avl_tree->insert_unique(15);
    test_avl_tree->insert_unique(10);
    test_avl_tree->insert_unique(12);

    printf("Before insert:size=%zu, capacity=%zu, empty=%u, full=%u\n",
           test_avl_tree->size(),
           test_avl_tree->capacity(),
           test_avl_tree->empty(),
           test_avl_tree->full());

    tree_iter = test_avl_tree->begin();
    tree_iter_end = test_avl_tree->end();
    printf("data list: \n");
    for (; tree_iter != tree_iter_end; ++tree_iter)
    {
        printf("%d ", (*tree_iter));
    }
    printf("%s", "\n");

    test_avl_tree->clear();
    test_avl_tree->insert_unique(15);
    test_avl_tree->insert_unique(20);
    test_avl_tree->insert_unique(18);

    printf("Before insert:size=%zu, capacity=%zu, empty=%u, full=%u\n",
           test_avl_tree->size(),
           test_avl_tree->capacity(),
           test_avl_tree->empty(),
           test_avl_tree->full());

    tree_iter = test_avl_tree->begin();
    tree_iter_end = test_avl_tree->end();
    printf("data list: \n");
    for (; tree_iter != tree_iter_end; ++tree_iter)
    {
        printf("%d ", (*tree_iter));
    }
    printf("%s", "\n");

    delete[] pt_avl_tree;
    pt_avl_tree = NULL;

    return 0;
}

//使用随机数测试，测试equal的情况,AVL Tree
int test_mmap_avltree5(int /*argc*/, char* /*argv*/[])
{
    typedef zce::shmc::avl_tree< int, int >  TEST_AVL_TREE;
    TEST_AVL_TREE* test_avl_tree;
    const size_t  SIZE_OF_TREE = 2000;
    TEST_AVL_TREE::iterator  tree_iter, tree_iter_end;
    size_t sz_malloc = TEST_AVL_TREE::getallocsize(SIZE_OF_TREE);

    char* pt_avl_tree = new char[sz_malloc];
    memset(pt_avl_tree, 0, sz_malloc);

    //初始化
    test_avl_tree = TEST_AVL_TREE::initialize(SIZE_OF_TREE, pt_avl_tree, false);
    if (NULL == test_avl_tree)
    {
        return 0;
    }

    //
    const uint32_t TEST_SEED = 1010123;
    zce::random_mt11213b  mt11231b_gen(TEST_SEED);
    int ins = 0;
    size_t ins_count = 0;
    //插入相同的数值
    for (ins_count = 0; ins_count < SIZE_OF_TREE;)
    {
        ins = mt11231b_gen.rand() % SIZE_OF_TREE;
        auto ins_iter = test_avl_tree->insert_equal(ins);
        if (ins_iter.second == true)
        {
            ++ins_count;
        }
        else
        {
            ZCE_ASSERT_ALL(false);
        }
    }

    auto f_iter = test_avl_tree->insert_unique(5);
    if (f_iter.second == true)
    {
        ZCE_ASSERT_ALL(false);
    }
    printf("%s", "\n==========================================================\n");
    printf("After insert:size=%zu, capacity=%zu, empty=%u, full=%u\n",
           test_avl_tree->size(),
           test_avl_tree->capacity(),
           test_avl_tree->empty(),
           test_avl_tree->full());

    tree_iter = test_avl_tree->begin();
    tree_iter_end = test_avl_tree->end();
    int a = 0, b = 0;
    for (; tree_iter != tree_iter_end;)
    {
        printf("%d ", (*tree_iter));

        bool ok = test_avl_tree->check_balanced(tree_iter.getserial());
        if (!ok)
        {
            printf("(No OK)");
        }

        a = (*tree_iter);
        ++tree_iter;
        if (tree_iter != tree_iter_end)
        {
            b = (*tree_iter);
            if (a > b)
            {
                ZCE_ASSERT_ALL(false);
            }
        }
    }

    //printf("%s", "\n-----------------------------------------------------\n");
    //for (size_t u = 0; u <= SIZE_OF_TREE; ++u)
    //{
    //    test_avl_tree->debug_note(u);
    //}
    //printf("%s", "\n-----------------------------------------------------\n");

    int del = 0;
    size_t erase_count = 0;
    size_t del_count = 0;
    for (erase_count = 0; erase_count < SIZE_OF_TREE;)
    {
        del = mt11231b_gen.rand() % SIZE_OF_TREE;
        del_count = test_avl_tree->erase_equal(del);
        if (del_count > 0)
        {
            erase_count += del_count;
            printf("del %d count.After del list: %zu", del, del_count);
            tree_iter = test_avl_tree->begin();
            tree_iter_end = test_avl_tree->end();
            for (; tree_iter != tree_iter_end; ++tree_iter)
            {
                printf("%d ", (*tree_iter));

                bool ok = test_avl_tree->check_balanced(tree_iter.getserial());
                if (!ok)
                {
                    printf("(No OK)");
                }
            }
            printf("%s", "\n");

            a = (*tree_iter);
            ++tree_iter;
            if (tree_iter != tree_iter_end)
            {
                b = (*tree_iter);
                if (a >= b)
                {
                    ZCE_ASSERT_ALL(false);
                }
            }

            //printf("%s", "\n-----------------------------------------------------\n");
            //for (size_t u = 0; u <= SIZE_OF_TREE; ++u)
            //{
            //    test_avl_tree->debug_note(u);
            //}
            //printf("%s", "\n-----------------------------------------------------\n");
        }
    }
    printf("%s", "\n---------------------------------------------\n");
    printf("\nAfter erase :size=%zu, capacity=%zu, empty=%u, full=%u\n",
           test_avl_tree->size(),
           test_avl_tree->capacity(),
           test_avl_tree->empty(),
           test_avl_tree->full());

    delete[] pt_avl_tree;
    pt_avl_tree = NULL;

    return 0;
}

int test_lru_hashtable(int /*argc*/, char* /*argv*/[])
{
    size_t node_num = 100, real_num = 0;
    size_t sz_del = 0;

    size_t szalloc = zce::shm_hashtable_expire <int, int >::getallocsize(node_num, real_num);

    std::cout << "need mem: " << (int)szalloc << " num of node:" << (int)node_num << std::endl;
    std::cout << "sizeof :" << sizeof(zce::shm_hashtable_expire <int, int >) << std::endl;
    char* tmproom = new char[szalloc + 4];
    memset(tmproom, 0, szalloc + 4);

    //
    zce::shm_hashtable_expire<int, int >* pmmap = zce::shm_hashtable_expire<int, int >::initialize(node_num, real_num, tmproom);
    pmmap->insert_unique(1001, static_cast<unsigned int>(time(NULL)));

    pmmap->insert_equal(1001, static_cast<unsigned int>(time(NULL)));
    pmmap->insert_equal(1001, static_cast<unsigned int>(time(NULL)));
    pmmap->insert_equal(1001, static_cast<unsigned int>(time(NULL)));

    pmmap->insert_unique(38637, static_cast<unsigned int>(time(NULL)));

    size_t count = pmmap->count(1001);
    std::cout << "count:" << (unsigned int)(count) << std::endl;

    bool bdel = pmmap->erase_unique(38637);
    assert(bdel == true);

    bdel = pmmap->erase_unique(1001);
    assert(bdel == true);

    count = pmmap->count(1001);
    std::cout << "count:" << (unsigned int)(count) << std::endl;

    pmmap->insert_unique(1001, static_cast<unsigned int>(time(NULL)));
    pmmap->insert_equal(1001, static_cast<unsigned int>(time(NULL)));
    pmmap->insert_equal(1001, static_cast<unsigned int>(time(NULL)));
    pmmap->insert_equal(1001, static_cast<unsigned int>(time(NULL)));

    pmmap->insert_unique(38637, static_cast<unsigned int>(time(NULL)));
    count = pmmap->count(1001);

    std::cout << "count:" << (unsigned int)(count) << std::endl;

    zce::shm_hashtable_expire<int, int >::iterator it;
    //
    it = pmmap->find(1001);

    zce::shm_hashtable_expire<int, int >::iterator it_end = pmmap->end();
    std::cout << "it serial: " << (int)(it.getserial()) << std::endl;

    if (it == it_end)
    {
        assert(false);
    }

    for (; it != it_end; ++it)
    {
        std::cout << "*it :" << *it << std::endl;
    }

    it = pmmap->find(1002);
    std::cout << "it serial: " << (int)(it.getserial()) << std::endl;

    if (it == pmmap->end())
    {
        std::cout << "Not Fount." << std::endl;
    }
    else
    {
        assert(false);
    }

    bdel = true;

    bdel = pmmap->erase_unique(38637);
    assert(bdel == true);

    sz_del = pmmap->erase_equal(1001);
    assert(sz_del == 4);

    std::cout << "size of :" << (unsigned int)(pmmap->size()) << std::endl;

    pmmap->insert_unique(1001, 1);
    pmmap->insert_unique(1002, 2);
    pmmap->insert_unique(1003, 3);
    pmmap->insert_unique(1004, 4);
    pmmap->insert_unique(38637, 5);
    std::cout << "size of :" << (unsigned int)(pmmap->size()) << std::endl;

    //pmmap->active(1004,1000);

    pmmap->expire(3);

    std::cout << "size of :" << (unsigned int)(pmmap->size()) << std::endl;
    it = pmmap->begin();
    it_end = pmmap->end();

    for (int i = 0; it != it_end; ++it, ++i)
    {
        std::cout << "idx of :" << i << " " << *it << std::endl;
    }

    pmmap->clear();

    for (size_t i = 0; i < node_num; ++i)
    {
        pmmap->insert_unique(static_cast<int>(1000 + i), static_cast<unsigned int>(i + 1950));
    }

    std::pair<zce::shm_hashtable_expire <int, int >::iterator, bool> iter_bool = pmmap->insert_unique(100022, static_cast<unsigned int>(time(NULL)));
    assert(iter_bool.second == false);

    it = pmmap->begin();
    it_end = pmmap->end();

    for (int i = 0; it != it_end; ++it, ++i)
    {
        std::cout << "idx of :" << i << " " << *it << std::endl;
    }

    pmmap->expire(2000);
    std::cout << "size of :" << (unsigned int)(pmmap->size()) << std::endl;
    return 0;
}

int test_lru_hashtable2(int /*argc*/, char* /*argv*/[])
{
    size_t numnode = 100, real_num = 0;
    size_t num_count = 0;

    size_t szalloc = zce::shm_hashtable_expire <int, int >::getallocsize(numnode, real_num);

    std::cout << "need mem: " << (int)szalloc << " num of node:" << (int)numnode << std::endl;
    std::cout << "sizeof :" << sizeof(zce::shm_hashtable_expire <int, int >) << std::endl;
    char* tmproom = new char[szalloc + 4];
    memset(tmproom, 0, szalloc + 4);

    //
    zce::shm_hashtable_expire<int, int >* pmmap = zce::shm_hashtable_expire<int, int >::initialize(numnode, real_num, tmproom);
    pmmap->insert_unique(1001, static_cast<unsigned int>(time(NULL)));
    pmmap->insert_unique(38636, static_cast<unsigned int>(time(NULL)));
    pmmap->insert_unique(36384378, static_cast<unsigned int>(time(NULL)));
    pmmap->insert_unique(65231237, static_cast<unsigned int>(time(NULL)));
    num_count = pmmap->count(1001);

    zce::shm_hashtable_expire<int, int >::iterator it_tmp = pmmap->begin();
    zce::shm_hashtable_expire<int, int >::iterator it_end = pmmap->end();

    for (; it_tmp != it_end; ++it_tmp)
    {
        std::cout << "it_tmp value: " << (*it_tmp) << std::endl;
    }

    pmmap->active_unique(1001, static_cast<unsigned int>(time(NULL)));
    it_tmp = pmmap->begin();

    for (; it_tmp != it_end; ++it_tmp)
    {
        std::cout << "it_tmp value: " << (*it_tmp) << std::endl;
    }
    ZCE_UNUSED_ARG(num_count);

    return 0;
}

int test_lru_hashtable3(int /*argc*/, char* /*argv*/[])
{
    size_t numnode = 100, real_num = 0;

    size_t count = 0;

    size_t szalloc = zce::shm_hashtable_expire <int, int >::getallocsize(numnode, real_num);

    std::cout << "need mem: " << (int)szalloc << " num of node:" << (int)numnode << std::endl;
    std::cout << "sizeof :" << sizeof(zce::shm_hashtable_expire <int, int >) << std::endl;
    char* tmproom = new char[szalloc + 4];
    memset(tmproom, 0, szalloc + 4);

    zce::shm_hashtable_expire<int, int >* pmmap = zce::shm_hashtable_expire<int, int >::initialize(numnode, real_num, tmproom);

    pmmap->insert_equal(1001, static_cast<unsigned int>(time(NULL)));
    pmmap->insert_equal(1001, static_cast<unsigned int>(time(NULL)));
    pmmap->insert_equal(1001, static_cast<unsigned int>(time(NULL)));
    pmmap->count(1001);

    pmmap->erase_equal(1001);

    pmmap->insert_equal(1001, static_cast<unsigned int>(time(NULL)));
    pmmap->insert_equal(1001, static_cast<unsigned int>(time(NULL)));
    pmmap->insert_equal(1001, static_cast<unsigned int>(time(NULL)));
    size_t sz_count = pmmap->count(1001);
    std::cout << sz_count << std::endl;

    pmmap->erase_equal(1001);

    pmmap->insert_equal(1001, static_cast<unsigned int>(time(NULL)));
    pmmap->insert_equal(1001, static_cast<unsigned int>(time(NULL)));
    pmmap->insert_equal(1001, static_cast<unsigned int>(time(NULL)));
    pmmap->erase_equal(1001);

    count = pmmap->count(1001);
    std::cout << "count:" << (unsigned int)(count) << std::endl;

    return 0;
}

struct MY_DATA
{
    MY_DATA()
    {
        abc_ = 0;
        def_ = 0.0;
    }
    int    abc_;
    double def_;
};

struct EXTRACT_INT_MY_DATA
{
    int operator ()(const MY_DATA& my_data)
    {
        return my_data.abc_;
    }
};

//struct EQUAL_MY_DATA
//{
//  bool operator ()(const MY_DATA&my_data1,const MY_DATA&my_data2)
//  {
//      return my_data1.abc_ == my_data2.abc_;
//  }
//};

//
class Lux_Data_Manager : public  zce::NON_Copyable
{
protected:
    //
    typedef zce::shm_hashtable_expire<MY_DATA, int, zce::smem_hash<int>, EXTRACT_INT_MY_DATA> HASH_TABLE_MY_DATA;
    //
    size_t              data_number_;
    //
    HASH_TABLE_MY_DATA* hash_my_data_;

    zce::SHM_Mmap   mmap_file_;

protected:
    //
    static Lux_Data_Manager* instance_;
protected:

    Lux_Data_Manager()
    {
    };
    ~Lux_Data_Manager()
    {
        mmap_file_.flush();
        mmap_file_.close();
    }

public:
    int initialize(size_t data_number, bool if_restore)
    {
        int ret = 0;

        data_number_ = data_number;
        size_t malloc_size = HASH_TABLE_MY_DATA::getallocsize(data_number, data_number_);

        ret = mmap_file_.open("./LUX_DATA_.MMAP", malloc_size);

        hash_my_data_ = HASH_TABLE_MY_DATA::initialize(data_number,
                                                       data_number_,
                                                       (char*)(mmap_file_.addr()),
                                                       if_restore);
        if (hash_my_data_)
        {
            return -1;
        }
        ZCE_UNUSED_ARG(ret);
        return 0;
    }

    int insert(const MY_DATA& my_data, unsigned int priority = time(NULL))
    {
        std::pair<HASH_TABLE_MY_DATA::iterator, bool > iter_pair =
            hash_my_data_->insert_unique(my_data, priority);
        if (false == iter_pair.second)
        {
            return -1;
        }
        return 0;
    }

    int insert_equal(const MY_DATA& my_data, unsigned int priority = time(NULL))
    {
        std::pair<HASH_TABLE_MY_DATA::iterator, bool > iter_pair =
            hash_my_data_->insert_equal(my_data, priority);
        if (false == iter_pair.second)
        {
            return -1;
        }
        return 0;
    }

    int erase(const MY_DATA& my_data)
    {
        bool bool_ret = hash_my_data_->erase_unique_value(my_data);
        if (!bool_ret)
        {
            return -1;
        }
        return 0;
    }

    int getmydata(MY_DATA& my_data)
    {
        HASH_TABLE_MY_DATA::iterator iter_tmp = hash_my_data_->find(my_data.abc_);
        if (iter_tmp == hash_my_data_->end())
        {
            return -1;
        }
        my_data = *iter_tmp;
        return 0;
    }

    void dump()
    {
        HASH_TABLE_MY_DATA::iterator iter_tmp = hash_my_data_->begin();
        HASH_TABLE_MY_DATA::iterator iter_end = hash_my_data_->end();
        for (; iter_tmp != iter_end; ++iter_tmp)
        {
            std::cout << (*iter_tmp).abc_ << " ";
        }
        std::cout << std::endl;
    }

public:
    //
    static Lux_Data_Manager* instance()
    {
        if (instance_ == NULL)
        {
            instance_ = new Lux_Data_Manager();
        }
        return instance_;
    }
    //
    static void clean_instance()
    {
        if (instance_)
        {
            delete instance_;
        }
        instance_ = NULL;
    }
};

//
Lux_Data_Manager* Lux_Data_Manager::instance_ = NULL;

int test_lux_data_manager(int argc, char* /*argv*/[])
{
    int ret = 0;
    size_t initialize_len = 1024;
    bool if_restore = false;
    if (argc > 2)
    {
        if_restore = true;
    }
    Lux_Data_Manager::instance()->initialize(initialize_len, if_restore);
    std::cout << " initialize_len " << initialize_len << std::endl;

    MY_DATA my_data;

    my_data.abc_ = 1000001;
    Lux_Data_Manager::instance()->insert(my_data);

    my_data.abc_ = 1000002;
    Lux_Data_Manager::instance()->insert(my_data);

    my_data.abc_ = 1000003;
    Lux_Data_Manager::instance()->insert(my_data);

    my_data.abc_ = 1000004;
    Lux_Data_Manager::instance()->insert(my_data);

    my_data.abc_ = 1000005;
    Lux_Data_Manager::instance()->insert(my_data);

    my_data.abc_ = 1000006;
    Lux_Data_Manager::instance()->insert(my_data);

    my_data.abc_ = 1000007;
    Lux_Data_Manager::instance()->insert(my_data);

    my_data.abc_ = 1000008;
    Lux_Data_Manager::instance()->insert(my_data);

    my_data.abc_ = 3;
    Lux_Data_Manager::instance()->insert(my_data);

    Lux_Data_Manager::instance()->dump();

    my_data.abc_ = 3;
    ret = Lux_Data_Manager::instance()->insert(my_data);
    std::cout << "repeat insert 3 ret =" << ret << std::endl;

    Lux_Data_Manager::instance()->dump();

    my_data.abc_ = 3;
    ret = Lux_Data_Manager::instance()->insert_equal(my_data);
    std::cout << "repeat insert_equal 3 ret =" << ret << std::endl;

    Lux_Data_Manager::instance()->dump();

    MY_DATA get_data;
    get_data.abc_ = 1000004;
    ret = Lux_Data_Manager::instance()->getmydata(get_data);
    std::cout << "get mydata 1000004 ret =" << ret << std::endl;

    get_data.abc_ = 1000005;
    ret = Lux_Data_Manager::instance()->getmydata(get_data);
    std::cout << "get mydata 1000005 ret =" << ret << std::endl;

    get_data.abc_ = 888888;
    ret = Lux_Data_Manager::instance()->getmydata(get_data);
    std::cout << "get mydata 888888 ret =" << ret << std::endl;

    my_data.abc_ = 3;
    ret = Lux_Data_Manager::instance()->erase(my_data);
    std::cout << "1.erase mydata 3 ret =" << ret << std::endl;

    my_data.abc_ = 3;
    ret = Lux_Data_Manager::instance()->erase(my_data);
    std::cout << "2.erase mydata 3 ret =" << ret << std::endl;

    my_data.abc_ = 3;
    ret = Lux_Data_Manager::instance()->erase(my_data);
    std::cout << "3.erase mydata 3 ret =" << ret << std::endl;

    Lux_Data_Manager::instance()->clean_instance();

    //
    zce::log_msg::instance()->init_time_log(LOGFILE_DEVIDE::BY_TIME_DAY, "C:\\123");

    ZCE_LOG(RS_ERROR, "ni shi yi zhi da wu gui.\n");
    ZCE_LOG(RS_ERROR, "ni shi yi zhi da wu gui.\n");
    ZCE_LOG(RS_ERROR, "ni shi yi zhi da wu gui.\n");
    ZCE_LOG(RS_ERROR, "ni shi yi zhi da wu gui.\n");
    ZCE_LOG(RS_ERROR, "ni shi yi zhi da wu gui.\n");
    ZCE_LOG(RS_ERROR, "ni shi yi zhi da wu gui.\n");
    ZCE_LOG(RS_ERROR, "ni shi yi zhi da wu gui.\n");
    ZCE_LOG(RS_ERROR, "ni shi yi zhi da wu gui.\n");
    ZCE_LOG(RS_ERROR, "ni shi yi zhi da wu gui.\n");
    ZCE_LOG(RS_ERROR, "ni shi yi zhi da wu gui.\n");

    return 0;
}

typedef zce::shm_hash_rehash<int, int> SMEM_HASH_SAFE_INT;

typedef zce::shm_hashtable<int, int> SMEM_HASH_TABLE_INT;

int test_hash_match(int /*argc*/, char* /*argv*/[])
{
    //测试的数量
    const size_t req_node_number = 50000000;

    SMEM_HASH_SAFE_INT* hash_safe_p = NULL;
    size_t safe_node_number = req_node_number;
    size_t real_node_number = 0;
    size_t prime_ary[zce::MAX_PRIMES_LIST_NUM];
    size_t safe_len_smem = SMEM_HASH_SAFE_INT::getallocsize(safe_node_number, real_node_number, prime_ary, false);

    std::cout << "Hash safe table nobe nubmer " << real_node_number << std::endl;
    std::cout << "Hash safe table room size " << safe_len_smem << std::endl;

    char* safe_smem = new char[safe_len_smem];
    int* insert_node = new int[req_node_number];

    hash_safe_p = SMEM_HASH_SAFE_INT::initialize(safe_node_number, real_node_number, safe_smem, -1, false);

    zce::mt19937_instance::instance()->srand((uint32_t)time(NULL));
    zce::random_mt19937* mt19937_ptr = zce::mt19937_instance::instance();

    size_t insert_count = 0;
    int  rand_num = mt19937_ptr->rand();

    for (; insert_count < req_node_number; rand_num += 3)
    {
        std::pair<SMEM_HASH_SAFE_INT::iterator, bool> piar_iter = hash_safe_p->insert(rand_num);

        //
        if (piar_iter.second == true)
        {
            insert_node[insert_count] = rand_num;
            ++insert_count;
        }
        else
        {
            rand_num = mt19937_ptr->rand();
        }
    }

    std::cout << "Hash safe table size " << hash_safe_p->size() << std::endl;
    std::cout << "Hash safe table real use ratio :" << double(hash_safe_p->size()) / hash_safe_p->capacity() << std::endl;
    std::cout << "Hash safe table expect use ratio : " << double(hash_safe_p->size()) / req_node_number << std::endl;

    //
    zce::Progress_Timer progress_timer;
    progress_timer.restart();

    //
    for (size_t x = 0; x < 3; ++x)
    {
        for (size_t y = 0; y < req_node_number; ++y)
        {
            hash_safe_p->find(insert_node[y]);
        }
    }

    //
    progress_timer.end();

    std::cout << "Hash safe table expect use micro seconds :" << progress_timer.elapsed_sec() << std::endl;

    //
    delete safe_smem;
    safe_smem = NULL;
    delete hash_safe_p;
    hash_safe_p = NULL;

    std::cout << std::endl;
    std::cout << std::endl;

    SMEM_HASH_TABLE_INT* hash_table_p = NULL;
    size_t table_node_number = req_node_number, real_table_number = 0;
    size_t table_len_smem = SMEM_HASH_TABLE_INT::getallocsize(table_node_number, real_table_number);

    std::cout << "Hash link table nobe nubmer " << real_table_number << std::endl;
    std::cout << "Hash link table room size " << table_len_smem << std::endl;

    char* table_smem = new char[table_len_smem];

    hash_table_p = SMEM_HASH_TABLE_INT::initialize(table_node_number, real_table_number, table_smem, false);

    for (size_t y = 0; y < req_node_number; ++y)
    {
        hash_table_p->insert_unique(insert_node[y]);
    }

    std::cout << "Hash link size " << hash_table_p->size() << std::endl;
    std::cout << "Hash link real use ratio :" << double(hash_table_p->size()) / hash_table_p->capacity() << std::endl;
    std::cout << "Hash link expect use ratio : " << double(hash_table_p->size()) / req_node_number << std::endl;
    std::cout << "Hash link index use ratio : " << double(hash_table_p->sizeuseindex()) / hash_table_p->capacity() << std::endl;

    //
    progress_timer.restart();

    //
    for (size_t x = 0; x < 3; ++x)
    {
        for (size_t y = 0; y < req_node_number; ++y)
        {
            hash_table_p->find(insert_node[y]);
        }
    }

    //
    progress_timer.end();

    std::cout << "Hash link table expect use micro seconds :" << progress_timer.elapsed_sec() << std::endl;
    //
    delete table_smem;
    table_smem = NULL;
    delete insert_node;
    insert_node = NULL;
    delete hash_table_p;
    hash_table_p = NULL;

    return 0;
}

typedef zce::shm_hash_rehash<int, int> SMEM_HASH_SAFE_INT;

int test_hash_safe(int /*argc*/, char* /*argv*/[])
{
    //计时器，不是定时器呀
    zce::HR_Progress_Timer auto_timer;

    SMEM_HASH_SAFE_INT* abc = NULL;

    size_t node_req_number = 1000000;
    size_t node_number = node_req_number;
    size_t node_real_number = 0;

    size_t prime_ary[zce::MAX_PRIMES_LIST_NUM];
    size_t len_smem = SMEM_HASH_SAFE_INT::getallocsize(node_number, node_real_number, prime_ary, false);

    std::cout << "Hash table nobe nubmer " << node_real_number << std::endl;
    std::cout << "Hash table room size " << len_smem << std::endl;

    char* p_smem = new char[len_smem];

    abc = SMEM_HASH_SAFE_INT::initialize(node_number, node_real_number, p_smem, -1, false, false);

    bool exit_for = false;

    zce::mt19937_instance::instance()->srand((uint32_t)time(NULL));
    zce::random_mt19937* mt19937_ptr = zce::mt19937_instance::instance();

    for (; exit_for == false;)
    {
        int  rand_num = mt19937_ptr->rand();

        for (;;)
        {
            std::pair<SMEM_HASH_SAFE_INT::iterator, bool> piar_iter = abc->insert(rand_num);

            if (piar_iter.second == false)
            {
                if (piar_iter.first == abc->end())
                {
                    exit_for = true;
                    break;
                }
                else
                {
                    rand_num += static_cast<int>(node_number);
                }
            }
            else
            {
                break;
            }
        }
    }

    std::cout << "Hash table size " << abc->size() << std::endl;
    std::cout << "Hash table real use ratio :" << double(abc->size()) / abc->capacity() << std::endl;
    std::cout << "Hash table expect use ratio : " << double(abc->size()) / node_req_number << std::endl;

    //
    delete p_smem;
    delete abc;

    return 0;
}

int test_hash_safe2(int /*argc*/, char* /*argv*/[])
{
    size_t node_req_number = 1000000;
    size_t node_number = node_req_number;
    size_t node_real_number = 0;
    size_t prime_ary[zce::MAX_PRIMES_LIST_NUM];
    size_t len_smem = SMEM_HASH_SAFE_INT::getallocsize(node_number, node_real_number, prime_ary, false);

    std::cout << "Hash table nobe nubmer " << node_real_number << std::endl;
    std::cout << "Hash table room size " << len_smem << std::endl;

    char* p_smem = new char[len_smem];
    SMEM_HASH_SAFE_INT* hash_safe_int = SMEM_HASH_SAFE_INT::initialize(node_number, node_real_number, p_smem, -1, false);

    hash_safe_int->insert(100101);
    hash_safe_int->insert(5004102);
    hash_safe_int->insert(9013019);
    hash_safe_int->insert(19033017);
    hash_safe_int->insert(89033017);

    hash_safe_int->erase(19033017);

    SMEM_HASH_SAFE_INT::iterator iter_tmp = hash_safe_int->begin();

    for (; iter_tmp != hash_safe_int->end(); ++iter_tmp)
    {
        std::cout << "Hash table node " << *iter_tmp << std::endl;
    }

    //
    delete p_smem;
    delete hash_safe_int;

    return 0;
}

int test_hashtable(int /*argc*/, char* /*argv*/[])
{
    size_t numnode = 100, real_num = 0;

    size_t szalloc = zce::shm_hashtable <int, int >::getallocsize(numnode, real_num);

    std::cout << "need mem: " << (int)szalloc << " num of node:" << (int)real_num << std::endl;
    std::cout << "sizeof smem_hashtable:" << sizeof(zce::shm_hashtable <int, int >) << std::endl;
    char* tmproom = new char[szalloc + 4];
    memset(tmproom, 0, szalloc + 4);

    zce::shm_hashtable<int, int >* pmmap = zce::shm_hashtable<int, int >::initialize(numnode, real_num, tmproom);
    pmmap->insert_unique(1001);
    zce::shm_hashtable<int, int >::iterator it = pmmap->find_value(1001);

    std::cout << "it serial: " << (int)(it.getserial()) << std::endl;

    bool bdel = pmmap->erase_unique(1001);

    it = pmmap->find_value(1002);

    bdel = pmmap->erase_unique(1001);

    std::cout << "it serial: " << (int)(it.getserial()) << std::endl;

    if (it == pmmap->end())
    {
        std::cout << "Not Fount." << std::endl;
    }
    ZCE_UNUSED_ARG(bdel);

    return 0;
}