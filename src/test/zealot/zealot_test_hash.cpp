


#include "zealot_predefine.h"
#include "zealot_test_function.h"

int test_lru_hashtable(int /*argc*/ , char * /*argv*/ [])
{

    size_t node_num = 100,real_num = 0;
    size_t sz_del = 0;

    size_t szalloc = ZCE_LIB::shm_hashtable_expire <int, int >::getallocsize(node_num, real_num);

    std::cout << "need mem: " << (int)szalloc << " num of node:" << (int)node_num << std::endl;
    std::cout << "sizeof :" << sizeof(ZCE_LIB::shm_hashtable_expire <int, int >) << std::endl;
    char *tmproom = new char [szalloc + 4];
    memset(tmproom, 0, szalloc + 4);

    //
    ZCE_LIB::shm_hashtable_expire<int, int >* pmmap = ZCE_LIB::shm_hashtable_expire<int, int >::initialize(node_num, real_num, tmproom);
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




    ZCE_LIB::shm_hashtable_expire<int, int >::iterator it;
    //
    it = pmmap->find(1001);

    ZCE_LIB::shm_hashtable_expire<int, int >::iterator it_end = pmmap->end();
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

    std::pair<ZCE_LIB::shm_hashtable_expire <int, int >::iterator, bool> iter_bool = pmmap->insert_unique(100022, static_cast<unsigned int>(time(NULL)));
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



int test_lru_hashtable2(int /*argc*/ , char * /*argv*/ [])
{
    size_t numnode = 100, real_num = 0;
    size_t num_count = 0;

    size_t szalloc = ZCE_LIB::shm_hashtable_expire <int, int >::getallocsize(numnode, real_num);

    std::cout << "need mem: " << (int)szalloc << " num of node:" << (int)numnode << std::endl;
    std::cout << "sizeof :" << sizeof(ZCE_LIB::shm_hashtable_expire <int, int >) << std::endl;
    char *tmproom = new char [szalloc + 4];
    memset(tmproom, 0, szalloc + 4);

    //
    ZCE_LIB::shm_hashtable_expire<int, int >* pmmap = ZCE_LIB::shm_hashtable_expire<int, int >::initialize(numnode, real_num, tmproom);
    pmmap->insert_unique(1001, static_cast<unsigned int>(time(NULL)));
    pmmap->insert_unique(38636, static_cast<unsigned int>(time(NULL)));
    pmmap->insert_unique(36384378, static_cast<unsigned int>(time(NULL)));
    pmmap->insert_unique(65231237, static_cast<unsigned int>(time(NULL)));
    num_count  = pmmap->count(1001);

    ZCE_LIB::shm_hashtable_expire<int, int >::iterator it_tmp = pmmap->begin();
    ZCE_LIB::shm_hashtable_expire<int, int >::iterator it_end = pmmap->end();

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

int test_lru_hashtable3(int /*argc*/ , char * /*argv*/ [])
{

    size_t numnode = 100, real_num = 0;

    size_t count = 0;

    size_t szalloc = ZCE_LIB::shm_hashtable_expire <int, int >::getallocsize(numnode, real_num);

    std::cout << "need mem: " << (int)szalloc << " num of node:" << (int)numnode << std::endl;
    std::cout << "sizeof :" << sizeof(ZCE_LIB::shm_hashtable_expire <int, int >) << std::endl;
    char *tmproom = new char [szalloc + 4];
    memset(tmproom, 0, szalloc + 4);

    ZCE_LIB::shm_hashtable_expire<int, int >* pmmap = ZCE_LIB::shm_hashtable_expire<int, int >::initialize(numnode, real_num, tmproom);

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
    int operator ()(const MY_DATA &my_data)
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
class Lux_Data_Manager : public  ZCE_NON_Copyable
{

protected:
    //
    typedef ZCE_LIB::shm_hashtable_expire<MY_DATA, int, ZCE_LIB::smem_hash<int>, EXTRACT_INT_MY_DATA> HASH_TABLE_MY_DATA;
    //
    size_t              data_number_;
    //
    HASH_TABLE_MY_DATA *hash_my_data_;

    ZCE_ShareMem_Mmap   mmap_file_;

protected:
    //
    static Lux_Data_Manager   *instance_;
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
            (char *)(mmap_file_.addr()),
            if_restore);
        if (hash_my_data_)
        {
            return -1;
        }
        ZCE_UNUSED_ARG(ret);
        return 0;
    }

    int insert(const MY_DATA &my_data, unsigned int priority = time(NULL) )
    {
        std::pair<HASH_TABLE_MY_DATA::iterator, bool > iter_pair =
            hash_my_data_->insert_unique(my_data, priority);
        if ( false == iter_pair.second )
        {
            return -1;
        }
        return 0;
    }

    int insert_equal(const MY_DATA &my_data, unsigned int priority = time(NULL) )
    {
        std::pair<HASH_TABLE_MY_DATA::iterator, bool > iter_pair =
            hash_my_data_->insert_equal(my_data, priority);
        if ( false == iter_pair.second )
        {
            return -1;
        }
        return 0;
    }

    int erase(const MY_DATA &my_data)
    {
        bool bool_ret = hash_my_data_->erase_unique_value(my_data);
        if (!bool_ret)
        {
            return -1;
        }
        return 0;
    }

    int getmydata(MY_DATA &my_data)
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
    static Lux_Data_Manager *instance()
    {
        if (instance_ == NULL)
        {
            instance_  = new Lux_Data_Manager();
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
Lux_Data_Manager *Lux_Data_Manager::instance_ = NULL;

int test_lux_data_manager(int argc , char * /*argv*/ [])
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
    ZCE_Trace_LogMsg::instance()->init_time_log(LOGDEVIDE_BY_DAY, "C:\\123");

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


typedef ZCE_LIB::shm_hash_rehash<int, int> SMEM_HASH_SAFE_INT;

typedef ZCE_LIB::shm_hashtable<int, int> SMEM_HASH_TABLE_INT;

int test_hash_match(int /*argc*/ , char * /*argv*/ [])
{

    //测试的数量
    const size_t req_node_number = 50000000;


    SMEM_HASH_SAFE_INT    *hash_safe_p = NULL;
    size_t safe_node_number = req_node_number;
    size_t real_node_number = 0;
    size_t prime_ary[ZCE_LIB::MAX_PRIMES_LIST_NUM];
    size_t safe_len_smem = SMEM_HASH_SAFE_INT::getallocsize(safe_node_number, real_node_number, prime_ary, false);

    std::cout << "Hash safe table nobe nubmer " << real_node_number << std::endl;
    std::cout << "Hash safe table room size " << safe_len_smem << std::endl;

    char *safe_smem = new char [safe_len_smem];
    int  *insert_node = new int[req_node_number];

    hash_safe_p = SMEM_HASH_SAFE_INT::initialize(safe_node_number, real_node_number, safe_smem, -1, false);


    ZCE_LIB::mt19937_instance::instance()->srand((uint32_t)time(NULL));
    ZCE_LIB::random_mt19937 *mt19937_ptr = ZCE_LIB::mt19937_instance::instance();

    size_t insert_count = 0;
    int  rand_num = mt19937_ptr->rand();

    for (; insert_count < req_node_number; rand_num += 3)
    {

        std::pair<SMEM_HASH_SAFE_INT::iterator, bool> piar_iter =  hash_safe_p->insert(rand_num);

        //
        if ( piar_iter.second == true)
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
    ZCE_Progress_Timer progress_timer;
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

    SMEM_HASH_TABLE_INT *hash_table_p = NULL;
    size_t table_node_number = req_node_number,real_table_number = 0;
    size_t table_len_smem = SMEM_HASH_TABLE_INT::getallocsize(table_node_number, real_table_number);

    std::cout << "Hash link table nobe nubmer " << real_table_number << std::endl;
    std::cout << "Hash link table room size " << table_len_smem << std::endl;

    char *table_smem = new char [table_len_smem];

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


typedef ZCE_LIB::shm_hash_rehash<int, int> SMEM_HASH_SAFE_INT;

int test_hash_safe(int /*argc*/ , char * /*argv*/ [])
{
    //计时器，不是定时器呀
    ZCE_Auto_Progress_Timer auto_timer;

    SMEM_HASH_SAFE_INT    *abc = NULL;

    size_t node_req_number = 1000000;
    size_t node_number = node_req_number;
    size_t node_real_number = 0;

    size_t prime_ary[ZCE_LIB::MAX_PRIMES_LIST_NUM];
    size_t len_smem = SMEM_HASH_SAFE_INT::getallocsize(node_number, node_real_number, prime_ary, false);

    std::cout << "Hash table nobe nubmer " << node_real_number << std::endl;
    std::cout << "Hash table room size " << len_smem << std::endl;

    char *p_smem = new char [len_smem];

    abc = SMEM_HASH_SAFE_INT::initialize(node_number, node_real_number,p_smem, -1, false, false);

    bool exit_for = false;

    ZCE_LIB::mt19937_instance::instance()->srand((uint32_t)time(NULL));
    ZCE_LIB::random_mt19937 *mt19937_ptr = ZCE_LIB::mt19937_instance::instance();

    for (; exit_for == false;)
    {
        int  rand_num = mt19937_ptr->rand();

        for (;;)
        {
            std::pair<SMEM_HASH_SAFE_INT::iterator, bool> piar_iter =  abc->insert(rand_num);

            if ( piar_iter.second == false)
            {
                if (piar_iter.first == abc->end() )
                {
                    exit_for = true;
                    break;
                }
                else
                {
                    rand_num += static_cast<int>( node_number);
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


int test_hash_safe2(int /*argc*/ , char * /*argv*/ [])
{

    size_t node_req_number = 1000000;
    size_t node_number = node_req_number;
    size_t node_real_number = 0;
    size_t prime_ary[ZCE_LIB::MAX_PRIMES_LIST_NUM];
    size_t len_smem = SMEM_HASH_SAFE_INT::getallocsize(node_number, node_real_number, prime_ary, false);

    std::cout << "Hash table nobe nubmer " << node_real_number << std::endl;
    std::cout << "Hash table room size " << len_smem << std::endl;

    char *p_smem = new char [len_smem];
    SMEM_HASH_SAFE_INT *hash_safe_int = SMEM_HASH_SAFE_INT::initialize(node_number, node_real_number, p_smem, -1, false);

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



int test_hashtable(int /*argc*/ , char * /*argv*/ [])
{
    size_t numnode = 100,real_num=0;

    size_t szalloc = ZCE_LIB::shm_hashtable <int, int >::getallocsize(numnode, real_num);

    std::cout << "need mem: " << (int)szalloc << " num of node:" << (int)real_num << std::endl;
    std::cout << "sizeof smem_hashtable:" << sizeof(ZCE_LIB::shm_hashtable <int, int >) << std::endl;
    char *tmproom = new char [szalloc + 4];
    memset(tmproom, 0, szalloc + 4);

    ZCE_LIB::shm_hashtable<int, int >* pmmap = ZCE_LIB::shm_hashtable<int, int >::initialize(numnode, real_num,tmproom);
    pmmap->insert_unique(1001);
    ZCE_LIB::shm_hashtable<int, int >::iterator it = pmmap->find_value(1001);


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












