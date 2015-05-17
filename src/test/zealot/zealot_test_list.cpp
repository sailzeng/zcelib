


#include "zealot_predefine.h"
#include "zealot_test_function.h"



int test_list(int /*argc*/ , char * /*argv*/ [])
{
    const size_t numnode = 100;

    //size_t szalloc1 = smem_list<void *>::getallocsize(numnode,40);

    size_t szalloc = ZCE_LIB::smem_list<int>::getallocsize(numnode);
    std::cout << "need mem: " << (int)szalloc << std::endl;
    std::cout << "sizeof(smem_list<int >)" << sizeof(ZCE_LIB::smem_list<int >) << std::endl;
    char *tmproom = new char [szalloc + 4];
    memset(tmproom, 0, szalloc + 4);

    ZCE_LIB::smem_list<int> *pmmap = ZCE_LIB::smem_list<int>::initialize(numnode, tmproom, false);

    std::cout << "capacity:" << (int)pmmap->capacity() << std::endl;

    int tmpx = 0;

    for (tmpx = 0; tmpx < 100; tmpx++)
    {
        pmmap->push_front(tmpx);
    }

    for (ZCE_LIB::smem_list<int>::iterator it = pmmap->begin(); it != pmmap->end()  ; ++it)
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

    for (ZCE_LIB::smem_list<int>::iterator it = pmmap->begin(); it != pmmap->end()  ; ++it)
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

    for (ZCE_LIB::smem_list<int>::iterator it = pmmap->begin(); it != pmmap->end()  ; ++it)
    {
        std::cout << " " << *it;
    }

    std::cout << std::endl;
    std::cout << "size:" << (int)pmmap->size() << std::endl;
    std::cout << "sizefreenode:" << (int)pmmap->sizefreenode() << std::endl;


    //少打一个Begin
    for (ZCE_LIB::smem_list<int>::iterator it = --pmmap->end(); it != pmmap->begin(); --it)
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

    for (ZCE_LIB::smem_list<int>::iterator it = pmmap->begin(); it != pmmap->end()  ; ++it)
    {
        std::cout << " " << *it;
    }

    std::cout << std::endl;
    std::cout << "size:" << (int)pmmap->size() << std::endl;
    std::cout << "sizefreenode:" << (int)pmmap->sizefreenode() << std::endl;

    ZCE_LIB::smem_list<int>::iterator ittmp = pmmap->end();
    ittmp --;
    ittmp --;
    ittmp --;
    pmmap->move_begin(ittmp, pmmap->end());

    for (ZCE_LIB::smem_list<int>::iterator it = pmmap->begin(); it != pmmap->end()  ; ++it)
    {
        std::cout << " " << *it;
    }

    std::cout << std::endl;
    std::cout << "size:" << (int)pmmap->size() << std::endl;
    std::cout << "sizefreenode:" << (int)pmmap->sizefreenode() << std::endl;

    return 0;
}
