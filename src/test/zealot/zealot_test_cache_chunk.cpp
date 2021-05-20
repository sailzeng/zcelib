#include "zealot_predefine.h"
#include "zealot_test_function.h"

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
    pmmap->set_node(68, tmpbuf1, usenode);
    size_t szdatalen;
    pmmap->get_node(usenode, szdatalen, tmpbuf);
    char tmpbuf2[32] = { "12345678901234567890" };
    pmmap->set_node(32, tmpbuf2, usenode);
    pmmap->get_node(usenode, szdatalen, tmpbuf);
    usenode = 0;
    pmmap->freenode(usenode);

    char tmpbuf3[168] = { "12345678901234567890123456789012345678901234567890123456789012345678901234567890" };
    bret = pmmap->set_node(168, tmpbuf3, usenode);
    pmmap->get_node(usenode, szdatalen, tmpbuf);
    pmmap->freenode(usenode);

    bret = pmmap->set_node(168, tmpbuf3, usenode);
    pmmap->get_node(usenode, szdatalen, tmpbuf);
    bret = pmmap->set_node(32, tmpbuf2, usenode);
    bret = pmmap->set_node(32, tmpbuf2, usenode);
    bret = pmmap->set_node(32, tmpbuf2, usenode);
    usenode = 3;
    pmmap->freenode(usenode);
    char tmpbuf4[2048] = { ":(---)" };
    bret = pmmap->set_node(2048, tmpbuf4, usenode);
    bret = pmmap->set_node(704, tmpbuf4, usenode);
    bret = pmmap->set_node(672, tmpbuf4, usenode);

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
    testchunk->free_size(testfreenode, testfreechunk, testfreeroom);
    std::cout << "free chunk:" << testfreechunk << std::endl;

    testchunk->set_node(9, testdata, testindex);
    testchunk->free_size(testfreenode, testfreechunk, testfreeroom);

    char tmpbuf[128];
    size_t szdatalen = 0;
    testchunk->get_node(testindex, szdatalen, tmpbuf);

    std::cout << "index:" << testindex << " " << "free chunk:" << testfreechunk << std::endl;
    testchunk->freenode(testindex);
    testchunk->free_size(testfreenode, testfreechunk, testfreeroom);
    std::cout << "free chunk:" << testfreechunk << std::endl;

    return 0;
}