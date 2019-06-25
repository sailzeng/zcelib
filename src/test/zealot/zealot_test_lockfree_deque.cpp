


#include "zealot_predefine.h"
#include "zealot_test_function.h"

int test_dequechunk(int /*argc*/, char * /*argv*/ [])
{
    size_t dequesize = 1026;
    size_t szalloc = zce::shm_dequechunk::getallocsize(dequesize);
    std::cout << "need mem: " << (int)szalloc << std::endl;



    char *tmproom = new char [szalloc + 4];
    memset(tmproom, 0, szalloc + 4);

    zce::shm_dequechunk *pmmap =  zce::shm_dequechunk::initialize(dequesize, 2048, tmproom, false);

    if (pmmap->empty())
    {
        std::cout << "empty" << std::endl;
    }

    struct abc
    {
        unsigned int sz;
        char   data[252];
    } test_abc;

    test_abc.sz = 256;
    memset(test_abc.data, '1', 251);
    test_abc.data[250] = 'A';
    test_abc.data[251] = 0;

    zce::dequechunk_node *pnode = reinterpret_cast<zce::dequechunk_node *>(&test_abc);
    pmmap->push_end(pnode);
    std::cout << "freesize:" << (int)pmmap->free_size() << std::endl;
    pmmap->push_end(pnode);
    pmmap->push_end(pnode);
    pmmap->push_end(pnode);

    std::cout << "freesize:" << (int)pmmap->free_size() << std::endl;

    zce::dequechunk_node *pnode1 = NULL;
    pmmap->pop_front(pnode1);
    std::cout << "pnode1 sz:" << (int)pnode1->size_of_node_ << std::endl;
    std::cout << "pnode1 data:" << pnode1->chunk_data_ << std::endl;
    std::cout << "freesize:" << (int)pmmap->free_size() << std::endl;
    delete pnode1;



    struct def
    {
        unsigned int sz;
        char   data[4];
    } test_def;
    test_def.sz = 8;
    memset(test_def.data, '2', 2);
    test_def.data[2] = 'B';
    test_def.data[3] = 0;
    std::cout << "freesize:" << (int)pmmap->free_size() << std::endl;
    pnode = reinterpret_cast<zce::dequechunk_node *>(&test_def);
    pmmap->push_end(pnode);
    std::cout << "freesize:" << (int)pmmap->free_size() << std::endl;
    pmmap->push_end(pnode);
    std::cout << "freesize:" << (int)pmmap->free_size() << std::endl;

    pnode1 = NULL;
    pmmap->pop_front(pnode1);
    delete pnode1;
    pnode1 = NULL;
    pmmap->pop_front(pnode1);
    delete pnode1;
    //
    pnode1 = new (1024)zce::dequechunk_node;
    pmmap->pop_front(pnode1);
    delete pnode1;
    std::cout << "freesize:" << (int)pmmap->free_size() << std::endl;
    pnode1 = new (1024)zce::dequechunk_node;
    pmmap->pop_front(pnode1);
    delete pnode1;
    std::cout << "freesize:" << (int)pmmap->free_size() << std::endl;

    pnode1 = NULL;
    pmmap->pop_front(pnode1);
    std::cout << "pnode1 sz:" << (int)pnode1->size_of_node_ << std::endl;
    std::cout << "pnode1 data:" << pnode1->chunk_data_ << std::endl;
    std::cout << "freesize:" << (int)pmmap->free_size() << std::endl;
    delete pnode1;

    std::cout << "freesize:" << (int)pmmap->free_size() << std::endl;

    pnode1 = NULL;
    bool bsucc = pmmap->pop_front(pnode1);

    if (bsucc == false  )
    {
        std::cout << "pop_frond return false!" << std::endl;
    }

    //pnode1 =NULL;
    //pmmap->pop_frond(pnode1);
    //std::cout<<"pnode1 sz:"<<(int)pnode1->sizeofnode<<std::endl;
    //std::cout<<"freesize:"<<(int)pmmap->freesize()<<std::endl;
    //delete pnode1;


    return 0;
}







