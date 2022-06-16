#include "predefine.h"

#include <zce/util/random.h>
#include <zce/rudp/rudp.h>

const char TEST_FILE[4][256] =
{
    "E:\\TEST_1_SRC.TXT",
    "E:\\TEST_1_DST.TXT",
    "E:\\TEST_2_SRC.TXT",
    "E:\\TEST_2_DST.TXT",
};

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
        zce::Log_Msg::instance()->init_time_log(LOGFILE_DEVIDE::BY_TIME_DAY,
                                                "E:\\My.Log\\CORE",
                                                0,
                                                true,
                                                false,
                                                true,
                                                (int)LOG_OUTPUT::LOGFILE | (int)LOG_OUTPUT::ERROUT,
                                                (int)(LOG_HEAD::LOGLEVEL));
        ZCE_TRACE_FILELINE(RS_DEBUG);
        return test_rudp_core(argc, argv);
    }
    else
    {
        zce::Log_Msg::instance()->init_time_log(LOGFILE_DEVIDE::BY_TIME_DAY,
                                                "E:\\My.Log\\CLIENT",
                                                0,
                                                true,
                                                false,
                                                true,
                                                (int)LOG_OUTPUT::LOGFILE | (int)LOG_OUTPUT::ERROUT,
                                                (int)(LOG_HEAD::LOGLEVEL));
        ZCE_TRACE_FILELINE(RS_DEBUG);
        test_rudp_client(argc, argv);
    }
    return 0;
}

ssize_t core_recv(zce::rudp::ACCEPT* peer)
{
    ZCE_LOG(RS_DEBUG, "[CORE recv] session id[%u] recv data len [%u]",
            peer->session_id(),
            peer->recv_bytes());

    zce::AUTO_HANDLE fd(zce::open(TEST_FILE[3], O_CREAT | O_APPEND | O_WRONLY));
    if (ZCE_INVALID_HANDLE == fd.get())
    {
        ZCE_LOG(RS_ERROR, "");
        return -1;
    }
    size_t file_size = 0;
    int ret = zce::filesize(fd.get(), &file_size);
    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR, "");
        return ret;
    }
    std::unique_ptr<char[]> write_buf(new char[peer->recv_bytes()]);
    size_t recv_size = peer->recv_bytes();
    peer->recv(write_buf.get(),
               &recv_size);
    ssize_t write_len = zce::write(fd.get(), write_buf.get(), recv_size);
    if (recv_size != (size_t)write_len)
    {
        ZCE_LOG(RS_DEBUG, "");
    }

    ZCE_LOG(RS_DEBUG, "[CORE recv] session id[%u] recv wnd len [%u] file size[%u] write size[%u].",
            peer->session_id(),
            peer->recv_bytes(),
            file_size,
            recv_size);
    return 0;
}

zce::rudp::ACCEPT* g_accpet_rudp = nullptr;

int core_accept(zce::rudp::ACCEPT* peer)
{
    g_accpet_rudp = peer;
    return 0;
}

ssize_t client_recv(zce::rudp::CLIENT* peer)
{
    ZCE_LOG(RS_DEBUG, "[CLIENT recv] session id[%u] recv data len [%u]",
            peer->session_id(),
            peer->recv_bytes());
    zce::AUTO_HANDLE fd(zce::open(TEST_FILE[3], O_CREAT | O_APPEND | O_WRONLY));
    if (ZCE_INVALID_HANDLE == fd.get())
    {
        ZCE_LOG(RS_ERROR, "");
        return -1;
    }
    size_t file_size = 0;
    int ret = zce::filesize(fd.get(), &file_size);
    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR, "");
        return ret;
    }
    std::unique_ptr<char[]> write_buf(new char[peer->recv_bytes()]);
    size_t recv_size = peer->recv_bytes();
    peer->recv(write_buf.get(),
               &recv_size);
    ssize_t write_len = zce::write(fd.get(), write_buf.get(), recv_size);
    if (recv_size != (size_t)write_len)
    {
        ZCE_LOG(RS_DEBUG, "");
    }
    ZCE_LOG(RS_DEBUG, "[CLIENT recv] session id[%u] recv wnd len [%u] file size[%u] write size[%u].",
            peer->session_id(),
            peer->recv_bytes(),
            file_size,
            recv_size);

    return 0;
}

int test_rudp_core(int /*argc*/, char* /*argv*/[])
{
    int ret = 0;
    ret = zce::unlink(TEST_FILE[3]);
    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR, "delete file fail.");
    }
    /*
    //打开文件
    zce::AUTO_HANDLE fd(zce::open(TEST_FILE[0], O_RDONLY));
    if (ZCE_INVALID_HANDLE == fd.get())
    {
        ZCE_LOG(RS_ERROR, "open file fail.");
        return -1;
    }
    size_t file_size = 0;
    ret = zce::filesize(fd.get(), &file_size);
    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR, "filesize fail.");
        return ret;
    }
    */
    zce::rudp::CORE core;
    sockaddr_in core_addr;
    std::function<ssize_t(zce::rudp::ACCEPT*)> callbak_recv(core_recv);
    std::function<int(zce::rudp::ACCEPT*)> callbak_accept(core_accept);
    zce::set_sockaddr_in(&core_addr, "0.0.0.0", 888);
    ret = core.open((sockaddr*)&core_addr,
                    1024,
                    10 * 1024 * 1024,
                    10 * 1024 * 1024,
                    &callbak_recv,
                    &callbak_accept);
    if (ret != 0)
    {
        return ret;
    }

    zce::Time_Value tv(0, 100000);
    //size_t remain_file_len = file_size;
    //每次尽力读取4K数据，
    const size_t READ_LEN = 4 * 1024;
    std::unique_ptr<char[]> read_buf(new char[READ_LEN]);
    //ssize_t read_len = 0;
    //size_t send_len = 0, remain_send_len = 0, once_process_len = 0, all_proces_len = 0;
    for (; core_run;)
    {
        tv.set(0, 10000);
        size_t recv_bytes = 0, recv_num = 0, accept_num = 0;

        core.receive_timeout(&tv, &recv_num, &accept_num, &recv_bytes);
        core.time_out();
        //读取文件内容
        /*
        if (remain_file_len > 0 && remain_send_len == 0 && g_accpet_rudp)
        {
            read_len = zce::read(fd.get(), read_buf.get(), READ_LEN);
            if (read_len < 0)
            {
                ZCE_LOG(RS_ERROR,
                        "read file ret =%d errno =%d",
                        ret,
                        zce::last_error());
                return -1;
            }
            remain_file_len -= read_len;
            remain_send_len = read_len;
            send_len = read_len;
            once_process_len = 0;
        }
        if (remain_send_len > 0 && g_accpet_rudp)
        {
            send_len = remain_send_len;
            ret = g_accpet_rudp->send(read_buf.get() + once_process_len, &send_len);
            if (ret != 0)
            {
                if (zce::last_error() != EWOULDBLOCK)
                {
                    ZCE_LOG(RS_ERROR,
                            "send ret =%d errno =%d",
                            ret,
                            zce::last_error());
                    return ret;
                }
            }
            remain_send_len -= send_len;
            once_process_len += send_len;
            all_proces_len += send_len;
        }

        if (once_process_len > 0)
        {
            ZCE_LOG(RS_INFO,
                    "[TEST]remain_file_len[%u] remain_send_len[%u] "
                    "process_len[%u] all_proces_len [%u]",
                    remain_file_len,
                    remain_send_len,
                    once_process_len,
                    all_proces_len);
        }
        */
    }

    core.close();
    return 0;
}

int test_rudp_client(int /*argc*/, char* /*argv*/[])
{
    int ret = 0;
    ret = zce::unlink(TEST_FILE[1]);
    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR, "delete file fail.");
    }
    zce::rudp::CLIENT client;
    sockaddr_in reomote_addr;
    std::function<ssize_t(zce::rudp::CLIENT*)> callbak_fun(client_recv);
    zce::set_sockaddr_in(&reomote_addr, "127.0.0.1", 888);
    ret = client.open((sockaddr*)&reomote_addr,
                      10 * 1024 * 1024,
                      10 * 1024 * 1024,
                      &callbak_fun);
    if (ret != 0)
    {
        return ret;
    }
    zce::Time_Value tv(3, 0);
    ret = client.connect_timeout(&tv);
    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR, "connect fail.");
        return ret;
    }
    //打开文件
    zce::AUTO_HANDLE fd(zce::open(TEST_FILE[2], O_RDONLY));
    if (ZCE_INVALID_HANDLE == fd.get())
    {
        ZCE_LOG(RS_ERROR, "open file fail.");
        return -1;
    }

    //获取文件尺寸，有长度可以避免有时候如果读取的文件长度和缓冲相等，要读取一次的麻烦，
    size_t file_size = 0;
    ret = zce::filesize(fd.get(), &file_size);
    if (0 != ret)
    {
        ZCE_LOG(RS_ERROR, "filesize fail.");
        return -1;
    }
    size_t remain_file_len = file_size;
    //每次尽力读取4K数据，
    const size_t READ_LEN = 4 * 1024;
    std::unique_ptr<char[]> read_buf(new char[READ_LEN]);
    ssize_t read_len = 0;
    size_t send_len = 0, remain_send_len = 0, once_process_len = 0, all_proces_len = 0;
    for (; core_run;)
    {
        tv.set(0, 10000);
        size_t sz_recv = 0;
        client.receive_timeout(&tv, &sz_recv);
        client.time_out();
        //读取内容
        if (remain_file_len > 0 && remain_send_len == 0)
        {
            read_len = zce::read(fd.get(), read_buf.get(), READ_LEN);
            if (read_len < 0)
            {
                ZCE_LOG(RS_ERROR,
                        "read file ret =%d errno =%d",
                        ret,
                        zce::last_error());
                return -1;
            }
            remain_file_len -= read_len;
            remain_send_len = read_len;
            send_len = read_len;
            once_process_len = 0;
        }
        if (remain_send_len > 0)
        {
            send_len = remain_send_len;
            ret = client.send(read_buf.get() + once_process_len, &send_len);
            if (ret != 0)
            {
                if (zce::last_error() != EWOULDBLOCK)
                {
                    ZCE_LOG(RS_ERROR,
                            "send ret =%d errno =%d",
                            ret,
                            zce::last_error());
                    return ret;
                }
            }
            remain_send_len -= send_len;
            once_process_len += send_len;
            all_proces_len += send_len;
        }
        if (remain_file_len > 0 || remain_send_len > 0)
        {
            ZCE_LOG(RS_INFO,
                    "[TEST]remain_file_len[%u] remain_send_len[%u] "
                    "process_len[%u] all_proces_len [%u]",
                    remain_file_len,
                    remain_send_len,
                    once_process_len,
                    all_proces_len);
        }
    }

    client.close();
    return 0;
}