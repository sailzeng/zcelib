#include "predefine.h"

ZCE_HANDLE file_handle = ZCE_INVALID_HANDLE;

void on_writefile(zce::aio::AIO_Handle* ahdl)
{
    auto fhdl = (zce::aio::FS_Handle*)(ahdl);
    std::cout << fhdl->result_ << " " << fhdl->result_count_;
}

void on_readfile(zce::aio::AIO_Handle* ahdl)
{
    auto fhdl = (zce::aio::FS_Handle*)(ahdl);
    std::cout << fhdl->result_ << " " << fhdl->result_count_;
}

int test_aio1(int argc, char* argv[])
{
    int ret = 0;
    zce::aio::Worker aio_worker;
    ret = aio_worker.initialize(5, 2048);
    if (ret)
    {
        return ret;
    }
    ret = zce::aio::caller::fs_write_file(&aio_worker,
                                          "E://aio_test_001.txt",
                                          "01234567890123456789",
                                          20,
                                          on_writefile);
    if (ret)
    {
        return ret;
    }
    char read_buf[1024];
    ret = zce::aio::caller::fs_read_file(&aio_worker,
                                         "E://aio_test_002.txt",
                                         read_buf,
                                         1024,
                                         on_readfile);
    if (ret)
    {
        return ret;
    }
    size_t count = 0;
    do
    {
        size_t num = 0;
        zce::Time_Value tv(1, 0);
        aio_worker.process_response(num, &tv);
        if (num > 0)
        {
            count += num;
        }

    } while (count >= 2);

    return 0;
}