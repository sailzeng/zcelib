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