#include "zealot_predefine.h"
#include "zealot_test_function.h"


int test_osadapt_file(int  /*argc*/, char * /*argv*/[])
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


    std::cout << (int) fh_1 << std::endl;
    std::cout << (int) fh_2 << std::endl;

    //file_desc != filedesc_1 != filedesc_2,3个文件描述符不一样
    int filedesc_1 = _open_osfhandle((intptr_t)fh_1, O_RDONLY);
    int filedesc_2 = _open_osfhandle((intptr_t)fh_1, O_RDONLY);

    std::cout << (int) filedesc_1 << std::endl;
    std::cout << (int) filedesc_2 << std::endl;

    //fh_1 == fh_2 == fh_3,内核句柄一致一致
    HANDLE fh_3 = (HANDLE)_get_osfhandle(filedesc_1);
    std::cout << (int) fh_3 << std::endl;

#endif

    return 0;
}

int test_osadapt_perf(int  /*argc*/, char * /*argv*/[])
{
    int ret = 0;
    ZCE_PROCESS_PERFORM prc_perf_info;
    ret = ZCE_LIB::get_self_perf(&prc_perf_info);

    if (ret != 0)
    {
        return ret;
    }

    ZCE_SYSTEM_INFO zce_system_info;
    ret = ZCE_LIB::get_system_info(&zce_system_info);

    if (ret != 0)
    {
        return ret;
    }

    ZCE_SYSTEM_PERFORMANCE zce_system_perf;
    ret =  ZCE_LIB::get_system_perf(&zce_system_perf);

    if (ret != 0)
    {
        return ret;
    }


    return 0;
}




#include "zealot_predefine.h"
#include "zealot_test_function.h"

//选取所有的.h文件
int hfile_selector(const struct dirent *dir_info)
{
    size_t name_len = strlen (dir_info->d_name);
    if (name_len <= 2)
    {
        return 0;
    }
    if ((dir_info->d_name[name_len - 1] == 'h' || dir_info->d_name[name_len - 1] == 'H')
        && dir_info->d_name[name_len - 2] == '.')
    {
        return 1;
    }
    return 0;
}

int test_scandir(int /*argc*/, char /*argv*/ *[])
{
    ZCE_LIB::clear_last_error();
    struct  dirent  **namelist = NULL;
    int number_file =  ZCE_LIB::scandir("E:\\Courage\\readline-5.2",
                                       &namelist,
                                       hfile_selector,
                                       ZCE_LIB::alphasort);

    if (number_file <= 0)
    {
        std::cout << "error." << std::endl;
        return -1;
    }

    for (int i = 0; i < number_file; i++)
    {
        std::cout << "file name " << i << ":" << namelist[i]->d_name << std::endl;
    }


    ZCE_LIB::free_scandir_result(number_file, namelist);

    //for (int i = 0; i < number_file; ++i)
    //{
    //    ::free (namelist[i]);
    //}
    //::free (namelist);

    return 0;
}

int test_file(int /*argc*/, char * /*argv*/[])
{
    int ret = 0;
    ZCE_HANDLE open_file;
    open_file = ZCE_LIB::open("C:\\1.txt", O_CREAT | O_TRUNC | O_RDWR);
    if (open_file == ZCE_INVALID_HANDLE)
    {
        return -1;
    }

    const char *write_str = "11111111111111111112222222222222222233333333333333333\n";
    size_t write_len = strlen(write_str);
    ZCE_LIB::ftruncate(open_file, write_len);
    ZCE_LIB::lseek(open_file, SEEK_SET, 0);
    ret = ZCE_LIB::flock(open_file, LOCK_EX | LOCK_NB);
    if (0 != ret)
    {
        return -1;
    }

    ssize_t ok_len = ZCE_LIB::write(open_file, write_str, write_len);

    if (ok_len <= 0)
    {
        return -1;
    }

    ZCE_LIB::close(open_file);
    return 0;
}

struct Zealot_SVC :public ZCE_Server_Base
{

};

int test_pid_file(int /*argc*/, char /*argv*/ *[])
{
    Zealot_SVC svc;
    svc.out_pid_file("C:\\1",true);
    return 0;
}

