#include "predefine.h"

#include <zce/util/random.h>
#include <zce/rudp/rudp.h>

bool core_run = true;
bool peer_run = true;

BOOL win_exit_signal(DWORD)
{
    core_run = false;
    peer_run = false;
    return TRUE;
}

void exit_signal(int)
{
    core_run = false;
    peer_run = false;
    return;
}

int test_rudp(int argc, char* argv[])
{
#ifdef ZCE_OS_WINDOWS
    //Windows下设置退出处理函数，可以用Ctrl + C 退出
    BOOL bret = ::SetConsoleCtrlHandler((PHANDLER_ROUTINE)win_exit_signal, TRUE);
    if (!bret)
    {
        ZCE_LOG(RS_ERROR, "SetConsoleCtrlHandler fail");
    }
#endif
    //这个几个信号被认可为退出信号
    signal(SIGINT, exit_signal);
    signal(SIGQUIT, exit_signal);
    signal(SIGTERM, exit_signal);

    if (argc > 1)
    {
        return test_rudp_core(argc, argv);
    }
    return test_rudp_client(argc, argv);
}

ssize_t core_recv(zce::rudp::PEER *peer)
{
    ZCE_LOG(RS_DEBUG, "[CORE recv] session id[%u] recv data len [%u]",
            peer->session_id(),
            peer->recv_wnd_size());
    return 0;
}

ssize_t peer_recv(zce::rudp::PEER *peer)
{
    ZCE_LOG(RS_DEBUG, "[PEER recv] session id[%u] recv data len [%u]",
            peer->session_id(),
            peer->recv_wnd_size());
    return 0;
}

int test_rudp_core(int /*argc*/, char* /*argv*/[])
{
    int ret = 0;
    zce::rudp::CORE core;
    sockaddr_in core_addr;
    std::function<ssize_t(zce::rudp::PEER *)> callbak_fun(core_recv);
    zce::set_sockaddr_in(&core_addr, "0.0.0.0", 888);
    ret = core.open((sockaddr *)&core_addr,
                    1024,
                    1024,
                    64 * 1024,
                    64 * 1024,
                    callbak_fun);
    if (ret != 0)
    {
        return ret;
    }

    for (; core_run;)
    {
        size_t size_recv = 0;
        zce::Time_Value tv(0, 100000);
        core.receive_timeout_i(&tv, &size_recv);
    }
    core.close();
    return 0;
}

int test_rudp_client(int /*argc*/, char* /*argv*/[])
{
    int ret = 0;
    zce::rudp::CLIENT client;
    sockaddr_in reomote_addr;
    std::function<ssize_t(zce::rudp::PEER *)> callbak_fun(peer_recv);
    zce::set_sockaddr_in(&reomote_addr, "127.0.0.1", 888);
    ret = client.open((sockaddr *)&reomote_addr,
                      1024,
                      64 * 1024,
                      64 * 1024,
                      callbak_fun);

    if (ret != 0)
    {
        return ret;
    }
    zce::Time_Value tv(3, 0);
    client.connect_timeout(&tv);
    //打开文件
    zce::AUTO_HANDLE fd(zce::open("E:\\1.pdf", O_RDONLY));
    if (ZCE_INVALID_HANDLE == fd.get())
    {
        return -1;
    }

    //获取文件尺寸，有长度可以避免有时候如果读取的文件长度和缓冲相等，要读取一次的麻烦，
    size_t file_size = 0;
    ret = zce::filesize(fd.get(), &file_size);
    if (0 != ret)
    {
        return -1;
    }
    size_t remain_file_len = file_size;
    //每次尽力读取4K数据，
    const size_t READ_LEN = 4 * 1024;
    std::unique_ptr<char[]> read_buf(new char[READ_LEN]);
    ssize_t read_len = 0;
    size_t send_len = 0, remain_send_len = 0;
    for (; core_run;)
    {
        tv.set(0, 10000);
        size_t sz_recv = 0;
        client.receive_timeout_i(&tv, &sz_recv);
        //读取内容
        if (remain_file_len > 0 && remain_send_len == 0)
        {
            read_len = zce::read(fd.get(), read_buf.get(), READ_LEN);
            if (read_len < 0)
            {
                return -1;
            }
            remain_file_len -= read_len;
            remain_send_len = read_len;
            send_len = read_len;
            ret = client.send(read_buf.get(), send_len);
            if (ret != 0)
            {
                return ret;
            }
        }
        if (remain_send_len > 0)
        {
            send_len = remain_send_len;
            ret = client.send(read_buf.get(), send_len);
            remain_send_len -= send_len;
        }
    }
    client.close();
    return 0;
}