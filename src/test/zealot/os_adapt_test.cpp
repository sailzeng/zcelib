#include "predefine.h"

namespace zeal
{
int test_windows_handle(int  /*argc*/, char* /*argv*/[])
{
#if defined ZCE_OS_WINDOWS

    int file_desc = open("C:\\123.txt", O_CREAT | O_APPEND);

    if (file_desc == 0)
    {
        return 0;
    }

    //fh_1 == fh_2 内核句柄一致
    HANDLE fh_1 = (HANDLE)_get_osfhandle(file_desc);
    HANDLE fh_2 = (HANDLE)_get_osfhandle(file_desc);

    std::cout << fh_1 << std::endl;
    std::cout << fh_2 << std::endl;

    //file_desc != filedesc_1 != filedesc_2,3个文件描述符不一样
    int filedesc_1 = _open_osfhandle((intptr_t)fh_1, O_RDONLY);
    int filedesc_2 = _open_osfhandle((intptr_t)fh_1, O_RDONLY);

    std::cout << (int)filedesc_1 << std::endl;
    std::cout << (int)filedesc_2 << std::endl;

    //fh_1 == fh_2 == fh_3,内核句柄一致一致
    HANDLE fh_3 = (HANDLE)_get_osfhandle(filedesc_1);
    std::cout << fh_3 << std::endl;

#endif

    return 0;
}

int test_osadapt_file(int  /*argc*/, char* /*argv*/[])
{
    size_t file_len = 0;
    auto pair = zce::read_file("C:\\123.txt", &file_len);
    if (pair.first != 0)
    {
        return pair.first;
    }
    std::cout << pair.second << std::endl;
    return 0;
}

int test_osadapt_perf()
{
    int ret = 0;
    ZCE_PROCESS_PERFORM prc_perf_info;
    ret = zce::get_self_perf(&prc_perf_info);

    if (ret != 0)
    {
        return ret;
    }

    ZCE_SYSTEM_INFO zce_system_info;
    ret = zce::get_system_info(&zce_system_info);

    if (ret != 0)
    {
        return ret;
    }

    ZCE_SYSTEM_PERFORMANCE zce_system_perf;
    ret = zce::get_system_perf(&zce_system_perf);

    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

//选取所有的.h .c .cpp .hpp .cxx文件
int hfile_selector(const struct dirent* dir_info)
{
    if (dir_info->d_type != DT_REG)
    {
        return 0;
    }
    size_t name_len = strlen(dir_info->d_name);
    if (name_len <= 2)
    {
        return 0;
    }
    if (zce::strcasecmp(dir_info->d_name + name_len - 2, ".h") == 0)
    {
        return 1;
    }
    if (zce::strcasecmp(dir_info->d_name + name_len - 2, ".c") == 0)
    {
        return 1;
    }
    if (name_len <= 4)
    {
        return 0;
    }
    if (zce::strcasecmp(dir_info->d_name + name_len - 4, ".cpp") == 0)
    {
        return 1;
    }
    if (zce::strcasecmp(dir_info->d_name + name_len - 4, ".cxx") == 0)
    {
        return 1;
    }
    if (zce::strcasecmp(dir_info->d_name + name_len - 4, ".hpp") == 0)
    {
        return 1;
    }
    return 0;
}

int test_scandir(int /*argc*/, char /*argv*/* [])
{
    const char* TEST_PATH = "D:\\Courage\\v8\\v8\\include";
    zce::clear_last_error();
    struct  dirent** namelist = nullptr;
    int number_file = zce::scandir(TEST_PATH,
                                   &namelist,
                                   hfile_selector,
                                   zce::scandir_namesort);

    if (number_file <= 0)
    {
        std::cout << "error." << std::endl;
        return -1;
    }

    for (int i = 0; i < number_file; i++)
    {
        std::cout << "file name " << i << ":" << namelist[i]->d_name << std::endl;
    }

    zce::free_scandir_result(number_file,
                             namelist);

    std::vector<dirent> dirent_ary;
    //selector没起作用
    //std::function<bool(const dirent&)> selector;
    int ret = 0;
    ret = zce::readdir_direntary(TEST_PATH,
                                 nullptr,
                                 dirent_ary);
    for (int i = 0; i < dirent_ary.size(); i++)
    {
        std::cout << "file name " << i << ":" << dirent_ary[i].d_name << std::endl;
    }

    dirent_ary.clear();
    ret = zce::readdir_direntary(TEST_PATH,
                                 nullptr,
                                 nullptr,
                                 true,
                                 false,
                                 true,
                                 dirent_ary);
    for (int i = 0; i < dirent_ary.size(); i++)
    {
        std::cout << "file name " << i << ":" << dirent_ary[i].d_name << std::endl;
    }
    return 0;
}

struct Zealot_SVC : public zce::server_base
{
};

Zealot_SVC svc;
int test_pid_file(int /*argc*/, char /*argv*/* [])
{
    svc.out_pid_file("C:\\1");
    return 0;
}

const size_t TEST_NUMBER = 100000 * 10;

void test_findwith_container(size_t container_len)
{
    std::vector<int>          int_vector;
    std::map<int, int>         int_map;
    std::unordered_map<int, int>    int_hash;

    int_vector.resize(container_len);
    int_hash.rehash(container_len);

    //
    for (size_t i = 0; i < container_len; i++)
    {
        int_vector[i] = (int)i;
        int_map[(int)i] = (int)i;
        int_hash[(int)i] = (int)i;
    }

    zce::hr_progress_timer test_timer;

    test_timer.restart();
    for (size_t i = 0; i < TEST_NUMBER; ++i)
    {
        int find_number = (int)i % container_len;
        //
        for (size_t j = 0; j < container_len; j++)
        {
            if (int_vector[j] == find_number)
            {
                break;
            }
        }
    }

    test_timer.end();

    std::cout << "test vector gettimeofday :" << test_timer.elapsed_usec() << " " << std::endl;

#ifdef ZCE_OS_WINDOWS
#pragma warning ( push )
#pragma warning ( disable : 4834)
#endif
    test_timer.restart();
    for (size_t i = 0; i < TEST_NUMBER; ++i)
    {
        int find_number = (int)i % container_len;
        int_map.find(find_number);
    }

    test_timer.end();

    std::cout << "test map gettimeofday :" << test_timer.elapsed_usec() << " " << std::endl;

    test_timer.restart();
    for (size_t i = 0; i < TEST_NUMBER; ++i)
    {
        int find_number = (int)i % container_len;
        int_hash.find(find_number);
    }

    test_timer.end();

    std::cout << "test hash gettimeofday :" << test_timer.elapsed_usec() << " " << std::endl;
}

#ifdef ZCE_OS_WINDOWS
#pragma warning ( pop )
#endif

//

int test_container_performance(int  /*argc*/, char* /*argv*/[])
{
    for (int j = 0; j < 3; ++j)
    {
        std::cout << "container length = 10 " << std::endl;
        test_findwith_container(10);

        std::cout << "container length = 20 " << std::endl;
        test_findwith_container(20);

        std::cout << "container length = 50 " << std::endl;
        test_findwith_container(50);

        std::cout << "container length = 100 " << std::endl;
        test_findwith_container(100);

        std::cout << "container length = 200 " << std::endl;
        test_findwith_container(200);

        std::cout << "container length = 500 " << std::endl;
        test_findwith_container(500);

        std::cout << "container length = 1000 " << std::endl;
        test_findwith_container(1000);
    }

    return 0;
}

int test_progress_timer()
{
    zce::chrono_hr_timer hr_timer;
    zce::time_value sleep_len(2, 5000);

    hr_timer.restart();
    zce::sleep(sleep_len);
    hr_timer.end();
    std::cout << "ZCE_Chrono_HR_Timer :" << hr_timer.elapsed_usec() << " " << std::endl;

    return 0;
}

static int test_stack5()
{
    zce::backtrace_stack(5, stdout);
    return 0;
}

static int test_stack4()
{
    test_stack5();
    return 0;
}

static int test_stack3()
{
    test_stack4();
    return 0;
}

static int test_stack2()
{
    test_stack3();
    return 0;
}

static int test_stack1()
{
    test_stack2();
    return 0;
}

int test_back_stack()
{
    return test_stack1();
}

int task_fcntl_unlock_1(zce::file_lock_t* lock,
                        int  whence,
                        ssize_t start,
                        ssize_t len)
{
    zce::sleep(zce::time_value(0, 1000));
    int ret = zce::fcntl_unlock(lock, whence, start, len);
    EXPECT_EQ(ret == 0, true);
    if (ret != 0)
    {
        return -1;
    }
    return 0;
}

int test_flock()
{
    static const unsigned char BASE64_CHAR[65] =
    {
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/"
    };

    const size_t FILE_SIZE = 640 * 1024;
    std::unique_ptr<char[] > buff(new char[FILE_SIZE]);
    std::cout << zce::getcwd(buff.get(), FILE_SIZE - 1) << std::endl;
    memset(buff.get(), 0, FILE_SIZE);
    for (size_t i = 0; i < FILE_SIZE / 64; ++i)
    {
        memcpy(buff.get() + i * 64, BASE64_CHAR, 64);
    }

    ZCE_HANDLE fh = zce::open("../testdir/flock.lock", O_CREAT | O_RDWR);
    EXPECT_EQ(fh != ZCE_INVALID_HANDLE, true);
    if (fh == ZCE_INVALID_HANDLE)
    {
        return -1;
    }
    zce::safe_handle safe_fh(fh);
    ssize_t ssz = zce::write(fh, buff.get(), FILE_SIZE);
    if (ssz != FILE_SIZE)
    {
        return -1;
    }
    zce::file_lock_t lock;
    int ret = zce::file_lock_init(&lock, fh);
    EXPECT_EQ(ret == 0, true);
    if (ret != 0)
    {
        return -1;
    }
    ret = zce::fcntl_rdlock(&lock, SEEK_SET, 0, 0);
    EXPECT_EQ(ret == 0, true);
    if (ret != 0)
    {
        return -1;
    }
    ret = zce::fcntl_unlock(&lock, SEEK_SET, 0, 0);
    EXPECT_EQ(ret == 0, true);
    if (ret != 0)
    {
        return -1;
    }

    ret = zce::fcntl_rdlock(&lock, SEEK_SET, 10, 20);
    EXPECT_EQ(ret == 0, true);
    if (ret != 0)
    {
        return -1;
    }
    ret = zce::fcntl_rdlock(&lock, SEEK_SET, 15, 20);
    EXPECT_EQ(ret == 0, true);
    if (ret != 0)
    {
        return -1;
    }
    ret = zce::fcntl_trywrlock(&lock, SEEK_SET, 15, 30);
    EXPECT_EQ(ret != 0, true);
    if (ret == 0)
    {
        return -1;
    }
    else
    {
        std::cout << "fcntl_trywrlock fail,SEEK_SET,15,30." << std::endl;
    }

    ret = zce::fcntl_unlock(&lock, SEEK_SET, 15, 20);
    EXPECT_EQ(ret == 0, true);
    if (ret != 0)
    {
        return -1;
    }

    ZCE_THREAD_ID task1;
    ret = zce::pthread_createex(&task1,
                                PTHREAD_CREATE_DETACHED,
                                8192 * 10,
                                task_fcntl_unlock_1,
                                &lock,
                                SEEK_SET,
                                10,
                                20);
    EXPECT_EQ(ret, 0);
    if (ret != 0)
    {
        return -1;
    }
    ret = zce::fcntl_wrlock(&lock, SEEK_SET, 15, 20);
    EXPECT_EQ(ret == 0, true);
    if (ret != 0)
    {
        return -1;
    }
    ret = zce::fcntl_trywrlock(&lock, SEEK_SET, 20, 30);
    EXPECT_EQ(ret != 0, true);

    //! Windows 允许“锁类型不同”的重复加锁（句柄维度的锁重入）
    // ret = zce::fcntl_tryrdlock(&lock, SEEK_SET, 20, 30);
    // EXPECT_EQ(ret != 0, true);

    ZCE_THREAD_ID task2;
    ret = zce::pthread_createex(&task2,
                                PTHREAD_CREATE_DETACHED,
                                8192 * 10,
                                task_fcntl_unlock_1,
                                &lock,
                                SEEK_SET,
                                15,
                                20);
    EXPECT_EQ(ret, 0);
    if (ret != 0)
    {
        return -1;
    }
    ret = zce::fcntl_wrlock(&lock, SEEK_SET, 20, 30);
    EXPECT_EQ(ret == 0, true);
    if (ret != 0)
    {
        return -1;
    }

    return 0;
}

TEST(OsadaptTestSuite, TestAPI)
{
    EXPECT_EQ(test_back_stack(), 0);
    EXPECT_EQ(test_progress_timer(), 0);
    EXPECT_EQ(test_osadapt_perf(), 0);
    EXPECT_EQ(test_flock(), 0);
}
}
