
#include "zerg_predefine.h"
#include "zerg_tcp_ctrl_handler.h"
#include "zerg_comm_manager.h"
#include "zerg_app_timer.h"
#include "zerg_stat_define.h"

/****************************************************************************************************
class  TCP_Svc_Handler
****************************************************************************************************/
//CONNECT鍚庣瓑寰呮暟鎹殑瓒呮椂鏃堕棿
unsigned int   TCP_Svc_Handler::accepted_timeout_ = 3;
//鎺ュ彈鏁版嵁鐨勮秴鏃舵椂闂�
unsigned int   TCP_Svc_Handler::receive_timeout_ = 5;

//TIME ID
const int      TCP_Svc_Handler::TCPCTRL_TIME_ID[] = { 1, 2 };


//
Active_SvcHandle_Set TCP_Svc_Handler::svr_peer_info_set_;



//杩欏効娌℃湁浣跨敤Singlton鐨勬柟寮忕殑鍘熷洜濡備笅锛�
//1.閫熷害绗竴鐨勬�濇兂
//2.鍘熸潵娌℃湁鐢╥nstance

//
ZBuffer_Storage  *TCP_Svc_Handler::zbuffer_storage_ = NULL;
//閫氫俊绠＄悊鍣�
Zerg_Comm_Manager *TCP_Svc_Handler::zerg_comm_mgr_ = NULL;
//
Soar_Stat_Monitor *TCP_Svc_Handler::server_status_ = NULL;

//鑷繁鏄惁鏄唬鐞�
bool           TCP_Svc_Handler::if_proxy_ = false;

//
size_t         TCP_Svc_Handler::num_accept_peer_ = 0;
//
size_t         TCP_Svc_Handler::num_connect_peer_ = 0;


//鏈�澶у彲浠ユ帴鍙楃殑鎺ュ彈鏁伴噺
size_t         TCP_Svc_Handler::max_accept_svr_ = 0;
//鏈�澶у彲浠ユ帴鍙楃殑杩炴帴鏁伴噺
size_t         TCP_Svc_Handler::max_connect_svr_ = 0;

//瀹归噺鍛婅闃堝��
size_t         TCP_Svc_Handler::accpet_threshold_warn_ = 0;
//宸茬粡瓒呰繃鍛婅闃堝�肩殑娆℃暟
size_t         TCP_Svc_Handler::threshold_warn_number_ = 0;

//
Zerg_Auto_Connector TCP_Svc_Handler::zerg_auto_connect_;


//svc handler鐨勬睜瀛�
TCP_Svc_Handler::POOL_OF_TCP_HANDLER TCP_Svc_Handler::pool_of_acpthdl_;
//svc handler鐨勬睜瀛�
TCP_Svc_Handler::POOL_OF_TCP_HANDLER TCP_Svc_Handler::pool_of_cnthdl_;

//鍙戦�佺紦鍐插尯鐨勬渶澶rame鏁帮紝浠庨厤缃鍙�
size_t         TCP_Svc_Handler::accept_send_deque_size_ = 0;

//涓诲姩杩炴帴鐨勫彂閫侀槦鍒楅暱搴�
size_t  TCP_Svc_Handler::connect_send_deque_size_ = 0;

unsigned int  TCP_Svc_Handler::handler_id_builder_ = 0;


//鏋勯�犲嚱鏁�
TCP_Svc_Handler::TCP_Svc_Handler(TCP_Svc_Handler::HANDLER_MODE hdl_mode) :
    ZCE_Event_Handler(ZCE_Reactor::instance()),
    ZCE_Timer_Handler(ZCE_Timer_Queue_Base::instance()),
    handler_mode_(hdl_mode),
    my_svc_id_(0, 0),
    peer_svr_id_(0, 0),
    rcv_buffer_(NULL),
    recieve_counter_(0),
    send_counter_(0),
    recieve_bytes_(0),
    send_bytes_(0),
    peer_status_(PEER_STATUS_NOACTIVE),
    timeout_time_id_(-1),
    receive_times_(0),
    if_force_close_(false),
    start_live_time_(0),
    tptoid_table_id_(size_t(-1))
{

    if (HANDLER_MODE_CONNECT == hdl_mode)
    {
        snd_buffer_deque_.initialize(connect_send_deque_size_);
    }
    else if (HANDLER_MODE_ACCEPTED == hdl_mode)
    {
        snd_buffer_deque_.initialize(accept_send_deque_size_);
    }
    else
    {
        ZCE_ASSERT(false);
    }
}


//鐢ㄤ簬Accept鐨勭鍙ｇ殑澶勭悊Event Handle鍒濆鍖栧鐞�.
void TCP_Svc_Handler::init_tcpsvr_handler(const SERVICES_ID &my_svcinfo,
                                          const ZCE_Socket_Stream &sockstream,
                                          const ZCE_Sockaddr_In     &socketaddr)
{
    handler_mode_ = HANDLER_MODE_ACCEPTED;
    my_svc_id_ = my_svcinfo;
    peer_svr_id_.set_svcid(0, 0);
    rcv_buffer_ = NULL;
    recieve_counter_ = 0;
    send_counter_ = 0;
    recieve_bytes_ = 0;
    send_bytes_ = 0;
    socket_peer_ = sockstream;
    peer_address_ = socketaddr;
    peer_status_ = PEER_STATUS_JUST_ACCEPT;
    timeout_time_id_ = -1;
    receive_times_ = 0;
    if_force_close_ = false;
    start_live_time_ = 0;
    tptoid_table_id_ = size_t(-1);

    ////璋冩暣Socket 涓篛_NONBLOCK
    int ret = socket_peer_.sock_enable(O_NONBLOCK);

    const size_t IP_ADDR_LEN = 32;
    char ip_addr_str[IP_ADDR_LEN + 1];
    size_t use_len = 0;

    ZCE_LOG(RS_INFO, "[zergsvr] Accept peer socket IP Address:[%s] Success. Set O_NONBLOCK ret =%d.",
            peer_address_.to_string(ip_addr_str,IP_ADDR_LEN,use_len),
            ret);

    //鍏堝鍔犺鏁板櫒銆傞伩鍏嶇瓑涓嬪叧闂殑鏃跺�欏張--浜嗐��
    ++num_accept_peer_;

    //妫�鏌ユ渶澶ч摼鎺ユ暟閲�,REACTOR鑷繁鍏跺疄鏈夋帶鍒�,浣嗘槸鎴戝張瑕佹帶鍒禔CCEPT鍙堣鎺у埗CONNECT.
    //鎵�浠ュ彧濂�,澶碭X澶�, 鍙﹀杩欎釜浜嬫儏鍏跺疄浠庣悊璁轰笂鏉ヨ,鍑犱箮涓嶄細鍙戠敓,
    if (num_accept_peer_ <= max_accept_svr_)
    {
        //缁х画妫�鏌ユ槸鍚︽湁鍛婅闃堝��
        if (num_accept_peer_ > accpet_threshold_warn_)
        {
            const size_t WARNNING_TIMES = 5;

            if ((threshold_warn_number_ % (WARNNING_TIMES)) == 0)
            {
                ZCE_LOG(RS_ALERT, "[zergsvr] Great than threshold_warn_number_ Reject! num_accept_peer_:%u,threshold_warn_number_:%u,accpet_threshold_warn_:%u,max_accept_svr_:%u .",
                        num_accept_peer_,
                        threshold_warn_number_,
                        accpet_threshold_warn_,
                        max_accept_svr_);
            }

            //璁板綍鍛婅鎬绘鏁扮殑澧炲姞
            ++threshold_warn_number_;
        }

        //娉ㄥ唽璇诲啓浜嬩欢
        ret = reactor()->register_handler(this,
                                          ZCE_Event_Handler::READ_MASK | ZCE_Event_Handler::WRITE_MASK);

        //
        if (ret != 0)
        {
            ZCE_LOG(RS_ERROR, "[zergsvr] Register accept [%s] handler fail! ret =%u  errno=%u|%s .",
                    peer_address_.to_string(ip_addr_str,IP_ADDR_LEN,use_len),
                    ret,
                    zce::last_error(),
                    strerror(zce::last_error()));

            handle_close();
            return;
        }

        reactor()->cancel_wakeup(this, ZCE_Event_Handler::WRITE_MASK);

        //缁熻
        server_status_->set_by_statid(ZERG_ACCEPT_PEER_NUMBER, 0, 0, static_cast<int>(num_accept_peer_));
        server_status_->increase_by_statid(ZERG_ACCEPT_PEER_COUNTER, 0, 0, 1);
    }
    //瑕佹祴璇曟鏌ヤ竴涓�,
    else
    {
        ZCE_LOG(RS_ERROR, "[zergsvr] Peer [%s] great than max_accept_svr_ Reject! num_accept_peer_:%u,max_accept_svr_:%u .",
                peer_address_.to_string(ip_addr_str,IP_ADDR_LEN,use_len),
                num_accept_peer_,
                max_accept_svr_);
        handle_close();
        return;
    }

    //濡傛灉閰嶇疆浜嗚秴鏃跺嚭鏉�,N绉掑繀椤绘敹鍒颁竴涓寘

    ZCE_Time_Value delay(0, 0);
    ZCE_Time_Value interval(0, 0);

    //
    (accepted_timeout_ > 0) ? delay.sec(accepted_timeout_) : delay.sec(STAT_TIMER_INTERVAL_SEC);
    (receive_timeout_ > 0) ? interval.sec(receive_timeout_) : interval.sec(STAT_TIMER_INTERVAL_SEC);

    timeout_time_id_ = timer_queue()->schedule_timer(this, &TCPCTRL_TIME_ID[0], delay, interval);


    //淇濇椿
    int keep_alive = 1;
    socklen_t opvallen = sizeof(int);
    socket_peer_.setsockopt(SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<void *>(&keep_alive), opvallen);

    //杩欏効鍦版柟瀵艰嚧杩囦竴涓狟UG锛屽氨鏄鎴风鏀朵笉鍒版暟鎹紝鎵�浠ヨ繖涓�夐」杩樹笉閫傚悎鍦ㄨ繖涓湇鍔″櫒浣跨敤銆傚懙鍛点��
    //杩欎釜閫夐」鏄繚璇佷富鍔ㄥ叧闂殑鏃跺�欙紝涓嶇敤绛夊緟灏嗘暟鎹彂閫佺粰瀵规柟,
    //杩欐涓滆タ寮�杩囦袱娆★紝浣嗕篃鍏抽棴浜嗕袱娆★紝鎴戣繕鏄及璁℃湁涓�浜涚粏鑺傛垜娌℃湁寮勬槑鐧姐��
    //struct linger sock_linger = {1, 30};
    //sock_linger.l_onoff = 1;
    //sock_linger.l_linger = 30;
    //opvallen = sizeof(linger);
    //socket_peer_.set_option(SOL_SOCKET,SO_LINGER,reinterpret_cast<void *>(&sock_linger),opvallen);

#if defined _DEBUG || defined DEBUG
    socklen_t sndbuflen, rcvbuflen;
    opvallen = sizeof(socklen_t);
    socket_peer_.getsockopt(SOL_SOCKET, SO_RCVBUF, reinterpret_cast<void *>(&rcvbuflen), &opvallen);
    socket_peer_.getsockopt(SOL_SOCKET, SO_SNDBUF, reinterpret_cast<void *>(&sndbuflen), &opvallen);
    ZCE_LOG(RS_DEBUG, "[zergsvr] Accept peer SO_RCVBUF:%u SO_SNDBUF %u.", rcvbuflen, sndbuflen);

#endif
}



//涓诲姩CONNET閾炬帴鍑哄幓鐨凥ANDLER锛屽搴擡vent Handle鐨勫垵濮嬪寲.
void TCP_Svc_Handler::init_tcpsvr_handler(const SERVICES_ID &my_svcinfo,
                                          const SERVICES_ID &peer_svrinfo,
                                          const ZCE_Socket_Stream &sockstream,
                                          const ZCE_Sockaddr_In     &socketaddr)
{
    handler_mode_ = HANDLER_MODE_CONNECT;
    my_svc_id_ = my_svcinfo;
    peer_svr_id_ = peer_svrinfo;
    rcv_buffer_ = NULL;
    recieve_counter_ = 0;
    send_counter_ = 0;
    recieve_bytes_ = 0;
    send_bytes_ = 0;
    socket_peer_ = sockstream;
    peer_address_ = socketaddr;
    peer_status_ = PEER_STATUS_NOACTIVE;
    timeout_time_id_ = -1;
    receive_times_ = 0;
    if_force_close_ = false;
    start_live_time_ = 0;
    tptoid_table_id_ = size_t(-1);

    //璋冩暣Socket 涓篈CE_NONBLOCK
    int ret = socket_peer_.sock_enable(O_NONBLOCK);
    const size_t IP_ADDR_LEN = 32;
    char ip_addr_str[IP_ADDR_LEN + 1];
    size_t use_len = 0;

    ZCE_LOG(RS_INFO, "[zergsvr] Connect peer socket Services ID[%u|%u] IP Address:[%s] Success. Set O_NONBLOCK ret =%d.",
            peer_svr_id_.services_type_,
            peer_svr_id_.services_id_,
            peer_address_.to_string(ip_addr_str,IP_ADDR_LEN,use_len),
            ret);

    snd_buffer_deque_.initialize(connect_send_deque_size_);

    //娉ㄥ唽鍒�
    ret = reactor()->register_handler(this, ZCE_Event_Handler::CONNECT_MASK);

    //鎴戝嚑涔庢病鏈夎杩噐egister_handler澶辫触,
    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR, "[zergsvr] Register services [%u|%u] IP[%s]  connect handler fail! ret =%d  errno=%d|%s .",
                peer_svr_id_.services_type_,
                peer_svr_id_.services_id_,
                peer_address_.to_string(ip_addr_str,IP_ADDR_LEN,use_len),
                ret,
                zce::last_error(),
                strerror(zce::last_error()));
        handle_close();
        return;
    }

    //鏀惧叆杩炴帴澶勭悊鐨凪AP
    ret = svr_peer_info_set_.add_services_peerinfo(peer_svr_id_, this);

    //鍦ㄨ繖鍎胯嚜鏉�鏄笉鏄嵄闄╀簡涓�鐐�
    if (ret != 0)
    {
        handle_close();
        return;
    }

    ++num_connect_peer_;

    ZCE_Time_Value delay(STAT_TIMER_INTERVAL_SEC, 0);
    ZCE_Time_Value interval(STAT_TIMER_INTERVAL_SEC, 0);

    timeout_time_id_ = timer_queue()->schedule_timer(this, &TCPCTRL_TIME_ID[0], delay, interval);

    //缁熻
    server_status_->set_by_statid(ZERG_CONNECT_PEER_NUMBER, 0, 0, num_connect_peer_);
    server_status_->increase_by_statid(ZERG_CONNECT_PEER_COUNTER, 0, 0, 1);

    //SO_RCVBUF锛孲O_SNDBUF锛屾寜鐓NPv1鐨勮瑙ｏ紝搴旇鍦╟onnect涔嬪墠璁剧疆锛岃櫧鐒舵垜鐨勬祴璇曡瘉鏄庯紝鏀惧湪杩欏効璁剧疆涔熷彲浠ャ��

    int keep_alive = 1;
    socklen_t opvallen = sizeof(int);
    socket_peer_.setsockopt(SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<void *>(&keep_alive), opvallen);

    //Win32涓嬫病鏈夎繖涓�夐」
#ifndef ZCE_OS_WINDOWS
    //閬垮厤DELAY鍙戦�佽繖绉嶆儏鍐�
    int NODELAY = 1;
    opvallen = sizeof(int);
    socket_peer_.setsockopt(SOL_TCP, TCP_NODELAY, reinterpret_cast<void *>(&NODELAY), opvallen);
#endif

#if defined _DEBUG || defined DEBUG
    socklen_t sndbuflen = 0, rcvbuflen = 0;
    opvallen = sizeof(socklen_t);
    socket_peer_.getsockopt(SOL_SOCKET, SO_RCVBUF, reinterpret_cast<void *>(&rcvbuflen), &opvallen);
    socket_peer_.getsockopt(SOL_SOCKET, SO_SNDBUF, reinterpret_cast<void *>(&sndbuflen), &opvallen);
    ZCE_LOG(RS_DEBUG, "[zergsvr] Set Connect Peer SO_RCVBUF:%u SO_SNDBUF %u.", rcvbuflen, sndbuflen);
#endif

}

TCP_Svc_Handler::~TCP_Svc_Handler()
{
}


//浠庨厤缃枃浠惰鍙栭厤缃俊鎭�
int TCP_Svc_Handler::get_config(const Zerg_Server_Config *config)
{
    int ret = 0;

    //unsigned int tmp_uint = 0 ;
    //浠嶤ONNECT鍒版敹鍒版暟鎹殑鏃堕暱
    accepted_timeout_ = config->zerg_cfg_data_.accepted_timeout_;


    //RECEIVE涓�涓暟鎹殑瓒呮椂鏃堕棿,涓�0琛ㄧず涓嶉檺鍒�
    receive_timeout_ = config->zerg_cfg_data_.receive_timeout_;


    //鏄惁鏄竴涓唬鐞嗘湇鍔�,浠ｇ悊鐨勫彂閫佽涓哄拰鏅�氭湇鍔″櫒涓嶄竴鏍�.
    if_proxy_ = config->zerg_cfg_data_.is_proxy_;


    //鏈�澶х殑閾炬帴鎴戠殑,鎴戦摼鎺ュ嚭鍘荤殑鏈嶅姟鍣ㄤ釜鏁�
    max_accept_svr_ = config->zerg_cfg_data_.max_accept_svr_;
    max_connect_svr_ = config->zerg_cfg_data_.auto_connect_num_;


    //瀹归噺鍛婅闃堝��
    accpet_threshold_warn_ = static_cast<size_t> (max_accept_svr_ * 0.8);
    ZCE_LOG(RS_INFO, "[zergsvr] Max accept svr number :%u,accept warn threshold number:%u. ",
            max_accept_svr_,
            accpet_threshold_warn_);


    //鍙戦�佺紦鍐插尯鐨勬渶澶rame鏁�
    accept_send_deque_size_ = config->zerg_cfg_data_.acpt_send_deque_size_;

    //涓诲姩杩炴帴鐨勫彂閫侀槦鍒楅暱搴�
    connect_send_deque_size_ = config->zerg_cfg_data_.cnnt_send_deque_size_;
    ZCE_LOG(RS_INFO, "[zergsvr] conncet send deque size :%u ,accept send deque size :%u",
            connect_send_deque_size_, accept_send_deque_size_);

    //寰楀埌杩炴帴鐨凷ERVER鐨勯厤缃�
    ret = zerg_auto_connect_.get_config(config);

    if (ret != 0)
    {
        return ret;
    }

    //
    return 0;
}



//灏嗛渶瑕佸垵濮嬪寲鐨勯潤鎬佸弬鏁板垵濮嬪寲
//涓�浜涘弬鏁颁粠閰嶇疆绫昏鍙�,閬垮厤鍚庨潰鐨勬搷浣滆繕瑕佽闂厤缃被
int TCP_Svc_Handler::init_all_static_data()
{
    //
    //int ret = 0;
    //
    zerg_comm_mgr_ = Zerg_Comm_Manager::instance();
    //鑷繁鐨勬湇鍔＄殑绫诲瀷,鏈嶅姟缂栧彿,APPID
    //
    zbuffer_storage_ = ZBuffer_Storage::instance();

    //鏈嶅姟鍣ㄧ殑缁熻鎿嶄綔瀹炰緥
    server_status_ = Soar_Stat_Monitor::instance();


    //鏈�澶ц閾炬帴鏁伴噺绛変簬鑷姩閾炬帴鏈嶅姟鐨勬暟閲�,澧炲姞16涓�

    ZCE_LOG(RS_INFO, "[zergsvr] MaxAcceptSvr:%u MaxConnectSvr:%u.", max_accept_svr_, max_connect_svr_);

    //涓篊ONNECT鐨凥DL棰勫厛鍒嗛厤鍐呭瓨锛屾垚涓轰竴涓睜瀛�
    ZCE_LOG(RS_INFO, "[zergsvr] Connet Hanlder:size of TCP_Svc_Handler [%u],one connect handler have deqeue length [%u],number of connect handler [%u]."
            "About need  memory [%u] bytes.",
            sizeof(TCP_Svc_Handler),
            MAX_OF_CONNECT_PEER_SEND_DEQUE,
            max_connect_svr_,
            (max_connect_svr_ * (sizeof(TCP_Svc_Handler) + MAX_OF_CONNECT_PEER_SEND_DEQUE * sizeof(size_t)))
           );
    pool_of_cnthdl_.initialize(max_connect_svr_);

    for (size_t i = 0; i < max_connect_svr_; ++i)
    {
        TCP_Svc_Handler *p_handler = new TCP_Svc_Handler(HANDLER_MODE_CONNECT);
        pool_of_cnthdl_.push_back(p_handler);
    }

    //涓篈CCEPT鐨凥DL棰勫厛鍒嗛厤鍐呭瓨锛屾垚涓轰竴涓睜瀛�
    ZCE_LOG(RS_INFO, "[zergsvr] Accept Hanlder:size of TCP_Svc_Handler [%u],one accept handler have deqeue length [%u],number of accept handler [%u]."
            "About need  memory [%u] bytes.",
            sizeof(TCP_Svc_Handler),
            accept_send_deque_size_,
            max_accept_svr_,
            (max_accept_svr_ * (sizeof(TCP_Svc_Handler) + accept_send_deque_size_ * sizeof(size_t)))
           );
    pool_of_acpthdl_.initialize(max_accept_svr_);

    for (size_t i = 0; i < max_accept_svr_; ++i)
    {
        TCP_Svc_Handler *p_handler = new TCP_Svc_Handler(HANDLER_MODE_ACCEPTED);
        pool_of_acpthdl_.push_back(p_handler);
    }

    //涔熻鎶婅繖涓墿灞曚竴浜涙瘮杈冩晥鐜囧ソ銆�
    svr_peer_info_set_.initialize(max_accept_svr_ + max_connect_svr_ + 1024);

    //杩炴帴鎵�鏈夌殑SERVER
    size_t szsucc = 0, szfail = 0, szvalid = 0;
    zerg_auto_connect_.reconnect_allserver(szvalid, szsucc, szfail);

    return 0;
}

//鍙栧緱鍙ユ焺
ZCE_HANDLE TCP_Svc_Handler::get_handle(void) const
{
    return (ZCE_HANDLE)socket_peer_.get_handle();
}

//鑾峰緱涓�涓暣鍨嬬殑handle
unsigned int TCP_Svc_Handler::get_handle_id()
{
#if defined ZCE_OS_WINDOWS

    ++handler_id_builder_;

    if (handler_id_builder_ == 0)
    {
        ++handler_id_builder_;
    }

    return  handler_id_builder_;

#elif defined ZCE_OS_LINUX
    return get_handle();
#endif
}


//璇诲彇,鏂繛鐨勪簨浠惰Е鍙戝鐞嗗嚱鏁�
int TCP_Svc_Handler::handle_input()
{
    //璇诲彇鏁版嵁
    size_t szrecv;
    int ret = read_data_from_peer(szrecv);

    const size_t IP_ADDR_LEN = 32;
    char ip_addr_str[IP_ADDR_LEN + 1];
    size_t use_len = 0;
    ZCE_LOGMSG_DEBUG(RS_DEBUG, "Read event ,svcinfo[%u|%u] IP[%s], handle input event triggered. ret:%d,szrecv:%u.",
                     peer_svr_id_.services_type_,
                     peer_svr_id_.services_id_,
                     peer_address_.to_string(ip_addr_str,IP_ADDR_LEN,use_len),
                     ret,
                     szrecv);
    ZCE_UNUSED_ARG(ip_addr_str);
    ZCE_UNUSED_ARG(use_len);

    //杩欏効浠讳綍閿欒閮藉叧闂�,
    if (ret != 0)
    {
        return -1;
    }

    //灏嗘暟鎹斁鍏ユ帴鏀剁殑绠￠亾,杩欏効杩斿洖鐨勯敊璇簲璇ラ兘鏄澶勭悊鐨勯敊璇�,鏀惧叆绠￠亾鐨勯敊璇簲璇ヤ笉浼氫笂鍗�
    ret = push_frame_to_comm_mgr();

    //杩欏効浠讳綍閿欒閮藉叧闂�,
    if (ret != 0)
    {
        return -1;
    }


    return 0;
}



//璇诲彇,鏂繛鐨勪簨浠惰Е鍙戝鐞嗗嚱鏁�
int TCP_Svc_Handler::handle_output()
{

    //濡傛灉NON BLOCK Connect鎴愬姛,涔熶細璋冪敤handle_output
    if (PEER_STATUS_NOACTIVE == peer_status_)
    {
        //澶勭悊杩炴帴鍚庣殑浜嬪疁
        process_connect_register();

        return 0;
    }

    int ret = 0;
    ret = write_all_data_to_peer();

    if (0 != ret)
    {
        //
        //涓轰粈涔堟垜涓嶅鐞嗛敊璇憿,涓峳eturn -1,鍥犱负濡傛灉閿欒浼氬叧闂璖ocket,handle_input灏嗚璋冪敤,杩欏効涓嶉噸澶嶅鐞�
        //濡傛灉鏄腑鏂瓑閿欒,绋嬪簭鍙互缁х画鐨�.
        //鍚庢潵涓哄暐鍙堟敼鎴愪簡return -1,鍔犲揩澶勭悊?蹇樿蹇樿浜嗐�傚簲璇ュ啓娉ㄩ噴鍛�銆�
        return -1;
    }

    return 0;
}



//瀹氭椂鍣ㄨЕ鍙�
int TCP_Svc_Handler::timer_timeout(const ZCE_Time_Value &now_time, const void *arg)
{
    const int timeid = *(static_cast<const int *>(arg));
    const size_t IP_ADDR_LEN = 32;
    char ip_addr_str[IP_ADDR_LEN + 1];
    size_t use_len = 0;
    //杩炴帴涓奛绉掞紝鎴栬�呮帴鍙椾簡M绉�
    if (TCPCTRL_TIME_ID[0] == timeid)
    {
        //濡傛灉鏈夊彈鍒版暟鎹紝閭ｄ箞浠�涔堜篃涓嶅仛
        if (receive_times_ > 0)
        {
            receive_times_ = 0;
        }
        //濡傛灉娌℃湁鏀跺埌鏁版嵁,璺虫ゼ鑷潃
        else
        {
            //濡傛灉鏄洃鍚殑绔彛锛岃�屼笖鏈夌浉搴旂殑瓒呮椂鍒ゆ柇
            if (HANDLER_MODE_ACCEPTED == handler_mode_ &&
                ((0 == start_live_time_ && 0 < accepted_timeout_) ||
                 (0 < start_live_time_ && 0 < receive_timeout_)))
            {

                ZCE_LOG(RS_ERROR, "[zergsvr] Connect or receive expire event,peer services [%u|%u] IP[%s]"
                        "want to close handle. live time %lu. recieve times=%u.",
                        peer_svr_id_.services_type_,
                        peer_svr_id_.services_id_,
                        peer_address_.to_string(ip_addr_str,IP_ADDR_LEN,use_len),
                        now_time.sec() - start_live_time_,
                        receive_times_);

                //鍦ㄨ繖鍎跨洿鎺ヨ皟鐢╤andle_close
                handle_close();
                return 0;
            }
        }

        //绗竴娆′娇鐢ㄧ殑鏄痑ccepted_timeout_
        if (0 == start_live_time_)
        {
            start_live_time_ = now_time.sec();
        }

        //鎵撳嵃涓�涓嬪悇涓鍙ｇ殑鐢熷瓨淇℃伅
        ZCE_LOG(RS_DEBUG, "[zergsvr] Connect or receive expire event,peer services [%u|%u] IP[%s] live "
                "time %lu. recieve times=%u.",
                peer_svr_id_.services_type_,
                peer_svr_id_.services_id_,
                peer_address_.to_string(ip_addr_str,IP_ADDR_LEN,use_len),
                now_time.sec() - start_live_time_,
                receive_times_);

        //杩欑被缁熻濡傛灉杩囦簬棰戠箒褰卞搷绋嬪簭鐨勮繍琛�,鎵�浠ユ斁鍏ュ畾鏃跺櫒浣�,铏界劧浼氭劅瑙変笉鏄お鍑嗙‘,浣嗘槸鎬ц兘浼樺厛
        server_status_->increase_by_statid(ZERG_RECV_SUCC_COUNTER, 0, 0, recieve_counter_);
        server_status_->increase_by_statid(ZERG_SEND_SUCC_COUNTER, 0, 0, send_counter_);
        server_status_->increase_by_statid(ZERG_SEND_BYTES_COUNTER, 0, 0, send_bytes_);
        server_status_->increase_by_statid(ZERG_RECV_BYTES_COUNTER, 0, 0, recieve_bytes_);

        recieve_counter_ = 0;
        recieve_bytes_ = 0;
        send_counter_ = 0;
        send_bytes_ = 0;
    }
    else if (TCPCTRL_TIME_ID[1] == timeid)
    {
    }

    return 0;
}


//PEER Event Handler鍏抽棴鐨勫鐞�
int TCP_Svc_Handler::handle_close()
{
    ZCE_LOG(RS_DEBUG, "[zergsvr] TCP_Svc_Handler::handle_close : %u.%u.",
            peer_svr_id_.services_type_, peer_svr_id_.services_id_);

    //涓嶈浣跨敤cancel_timer(this),鍏剁箒鐞�,鑰屼笖鎱�,濂借new,鑰屼笖鏈変竴涓笉鐭ュ悕鐨勬鏈�
    const size_t IP_ADDR_LEN = 32;
    char ip_addr_str[IP_ADDR_LEN + 1];
    size_t use_len = 0;
    //鍙栨秷姝vent Handler鐩稿叧鐨勫畾鏃跺櫒
    if (-1 != timeout_time_id_)
    {
        timer_queue()->cancel_timer(timeout_time_id_);
        timeout_time_id_ = -1;
    }


    //鍙栨秷MASK,鏈�鍚庨樁娈�,閬垮厤璋冪敤handle_close,
    //鍐呴儴浼氳繘琛宺emove_handler
    ZCE_Event_Handler::handle_close();

    //鍏抽棴绔彛,
    socket_peer_.close();

    //閲婃斁鎺ユ敹鏁版嵁缂撳啿鍖�
    if (rcv_buffer_)
    {
        zbuffer_storage_->free_byte_buffer(rcv_buffer_);
        rcv_buffer_ = NULL;
    }

    //澶勭悊鍙戦�佹暟鎹紦鍐插尯
    size_t sz_of_deque = snd_buffer_deque_.size();

    for (size_t i = 0; i < sz_of_deque; i++)
    {
        //澶勭悊鍙戦�侀敊璇槦鍒�,鍚屾椂杩涜鍥炴敹
        process_send_error(snd_buffer_deque_[i], true);
        snd_buffer_deque_[i] = NULL;
    }

    snd_buffer_deque_.clear();

    //濡傛灉鏈嶅姟鏄縺娲荤姸鎬侊紝鎴栬�呮槸涓诲姩杩炴帴鐨勬湇鍔�.
    if (peer_status_ == PEER_STATUS_ACTIVE || handler_mode_ == HANDLER_MODE_CONNECT)
    {
        //娉ㄩ攢杩欎簺淇℃伅
        svr_peer_info_set_.del_services_peerInfo(peer_svr_id_);

        //濡傛灉鏄潪鍚庣涓氬姟杩涚▼寮哄埗鍏抽棴锛屽垯鍙戦�佷竴涓�氱煡缁欎笟鍔¤繘绋嬶紝鍚﹀垯涓嶅仛閫氱煡
        if (false == if_force_close_)
        {
            //閫氱煡鍚庨潰鐨勬湇鍔″櫒

            Zerg_Buffer *close_buf = zbuffer_storage_->allocate_buffer();
            ZERG_FRAME_HEAD *proc_frame = reinterpret_cast<ZERG_FRAME_HEAD *>(close_buf->buffer_data_);

            proc_frame->init_framehead(ZERG_FRAME_HEAD::LEN_OF_APPFRAME_HEAD, 0, INNER_REG_SOCKET_CLOSED);
            proc_frame->send_service_ = peer_svr_id_;
            zerg_comm_mgr_->pushback_recvpipe(proc_frame);
            zbuffer_storage_->free_byte_buffer(close_buf);
        }
    }

    //杩欑被缁熻濡傛灉杩囦簬棰戠箒褰卞搷绋嬪簭鐨勮繍琛�,鎵�浠ユ斁鍏ユ渶鍚庝綔,铏界劧浼氭劅瑙変笉鏄お鍑嗙‘,浣嗘槸鎬ц兘浼樺厛
    server_status_->increase_by_statid(ZERG_RECV_SUCC_COUNTER, 0, 0, recieve_counter_);
    server_status_->increase_by_statid(ZERG_SEND_SUCC_COUNTER, 0, 0, send_counter_);
    server_status_->increase_by_statid(ZERG_SEND_BYTES_COUNTER, 0, 0, send_bytes_);
    server_status_->increase_by_statid(ZERG_RECV_BYTES_COUNTER, 0, 0, recieve_bytes_);

    recieve_counter_ = 0;
    recieve_bytes_ = 0;
    send_counter_ = 0;
    send_bytes_ = 0;


    peer_status_ = PEER_STATUS_NOACTIVE;

    //鏍规嵁涓嶅悓鐨勭被鍨嬭鏁�,

    //涓嶈繘琛屼富鍔ㄩ噸鏂拌繛鎺�,濡傛灉鏈変竴涓柊鐨勬暟鎹鍙戦�佹椂涓诲姩閲嶆柊杩炴帴
    if (handler_mode_ == HANDLER_MODE_CONNECT)
    {
        ZCE_LOG(RS_INFO, "[zergsvr] Connect peer close, services[%u|%u] socket IP|Port :[%s].",
                peer_svr_id_.services_type_,
                peer_svr_id_.services_id_,
                peer_address_.to_string(ip_addr_str,IP_ADDR_LEN,use_len)
               );

        --num_connect_peer_;
        server_status_->set_by_statid(ZERG_CONNECT_PEER_NUMBER, 0, 0,
                                      num_connect_peer_);
        //灏嗘寚閽堝綊杩樺埌姹犲瓙涓棿鍘�
        pool_of_cnthdl_.push_back(this);
    }
    else if (handler_mode_ == HANDLER_MODE_ACCEPTED)
    {
        ZCE_LOG(RS_INFO, "[zergsvr] Accept peer close, services[%u|%u] socket IP|Port :[%s.",
                peer_svr_id_.services_type_,
                peer_svr_id_.services_id_,
                peer_address_.to_string(ip_addr_str,IP_ADDR_LEN,use_len)
               );

        --num_accept_peer_;
        server_status_->set_by_statid(ZERG_ACCEPT_PEER_NUMBER, 0, 0, num_accept_peer_);
        //灏嗘寚閽堝綊杩樺埌姹犲瓙涓棿鍘�
        pool_of_acpthdl_.push_back(this);
    }

    return 0;
}



<<<<<<< HEAD
//收到一个完整的帧后的预处理工作
//合并发送队列
int TCP_Svc_Handler::preprocess_recvframe(ZERG_FRAME_HEAD *proc_frame)
=======
//鏀跺埌涓�涓畬鏁寸殑甯у悗鐨勯澶勭悊宸ヤ綔
//鍚堝苟鍙戦�侀槦鍒�
int TCP_Svc_Handler::preprocess_recvframe(Zerg_App_Frame *proc_frame)
>>>>>>> 822b6377f4cdc443cbb2f70750601ca28174e570
{

    //Zerg_App_Frame *proc_frame = reinterpret_cast<Zerg_App_Frame *>( rcv_buffer_->buffer_data_);
    const size_t IP_ADDR_LEN = 32;
    char ip_addr_str[IP_ADDR_LEN + 1];
    size_t use_len = 0;
    //蹇呴』瑙ｇ爜,鍚﹀垯鍚庨潰鐨勭閬撴棤娉曡瘑鍒�
    proc_frame->framehead_decode();
    DEBUGDUMP_FRAME_HEAD_DBG(RS_DEBUG, "preprocess_recvframe After framehead_decode:", proc_frame);

    //娓呯悊鍐呴儴閫夐」,閬垮厤琚敊璇暟鎹垨鑰呭叾浠栦汉鏁磋泭
    proc_frame->clear_inner_option();

    //妫�鏌ヨ繖涓抚鏄惁鏄彂閫佺粰杩欎釜SVR,

    //濡傛灉鏄唬鐞�,妫�鏌ュ抚鐨勪唬鐞嗛儴鍒嗘暟鎹�
    if (proc_frame->proxy_service_.services_type_ != SERVICES_ID::INVALID_SERVICES_TYPE && if_proxy_ == true)
    {

        if (my_svc_id_ != proc_frame->proxy_service_)
        {
            return SOAR_RET::ERR_ZERG_APPFRAME_ERROR;
        }
        //
        proc_frame->proxy_service_.services_id_ = my_svc_id_.services_id_;
    }
    //妫�鏌ュ抚鐨勬帴鍙楅儴鍒�
    else
    {
        if (my_svc_id_ != proc_frame->recv_service_)
        {
            return SOAR_RET::ERR_ZERG_APPFRAME_ERROR;
        }
        //瀵逛簬鏌愪簺鎯呭喌(涓昏鏄鎴风),瀵规柟涓嶇煡閬揝ervicesID
        else
        {
            proc_frame->recv_service_ = my_svc_id_;
        }

    }

    //濡傛灉绔彛浠呬粎鍒氬垰ACCEPT涓婃潵锛岃繕娌℃湁鏀跺埌鏁版嵁
    if (PEER_STATUS_JUST_ACCEPT == peer_status_)
    {
        //璁板綍Service Info,鐢ㄤ簬鍚庨潰鐨勫鐞�,(鍙戦�佺殑鏃跺��)
        if (proc_frame->proxy_service_.services_type_ != SERVICES_ID::INVALID_SERVICES_TYPE && if_proxy_ == false)
        {
            peer_svr_id_ = proc_frame->proxy_service_;
        }
        else
        {
            //濡傛灉鏄壒娈婄殑涓嶅姞瀵嗙殑瀹㈡埛绔紝閭ｄ箞鐩存帴鎷縣andle鐢熸垚ID锛屽洜涓鸿繖涓椂鍊欐病鏈塹q鍙枫��
            if (SERVICES_ID::DYNAMIC_ALLOC_SERVICES_ID == proc_frame->send_service_.services_id_)
            {
                //鍒嗛厤涓�涓狪D缁欎綘銆備絾浣犺璁板緱鍥炲～鍠�
                proc_frame->send_service_.services_id_ = get_handle_id();
                peer_svr_id_ = proc_frame->send_service_;

                //proc_frame->frame_uid_   = proc_frame->send_service_.services_id_;

                //杩欑鎯呭喌锛屽彲浠ュ厛杩斿洖涓�涓狪D缁欏鏂�
                //send_simple_zerg_cmd(,peer_svr_info_);
            }
            else
            {
                peer_svr_id_ = proc_frame->send_service_;
            }
        }

        //娉ㄥ唽,濡傛灉鍘熸潵鏈夊搷搴旂殑閾炬帴,浼氳繑鍥炲師鏈夌殑閾炬帴.replace_services_peerInfo,蹇呯劧鎴愬姛
        TCP_Svc_Handler *old_hdl = NULL;
        svr_peer_info_set_.replace_services_peerInfo(peer_svr_id_, this, old_hdl);

        //濡傛灉鏈夊師鏈夌殑閾炬帴,鍒欐壘鍒板師鏉ョ殑閭ｄ釜韪笅鍘�.
        if (old_hdl != NULL)
        {
            //鑰屼笖淇敼鍘熸湁閾炬帴鐨勭姸鎬�,閬垮厤閲嶅浠嶴ET鍒犻櫎
            old_hdl->peer_status_ = PEER_STATUS_JUST_ACCEPT;
            old_hdl->send_simple_zerg_cmd(ZERG_REPEAT_LOGIN_KICK_OLD_RSP,
                                          peer_svr_id_,
                                          ZERG_FRAME_HEAD::DESC_SNDPRC_CLOSE_PEER);

            //涓嶇洿鎺ュ叧闂簡锛岃�屾槸鍏堟妸鍛戒护鍙戦�佸畬鎴愪簡锛屽啀鍏抽棴
            //old_hdl->handle_close(ACE_INVALID_HANDLE, 0);
        }

        //鏈�鍚庤皟鏁磋嚜宸盤EER鐨勭姸鎬�
        peer_status_ = PEER_STATUS_ACTIVE;

        ZCE_LOG(RS_INFO, "[zergsvr] Accept peer services[%u|%u],IP|Prot[%s] regist success.",
                peer_svr_id_.services_type_,
                peer_svr_id_.services_id_,
                peer_address_.to_string(ip_addr_str,IP_ADDR_LEN,use_len)
               );
    }
    //濡傛灉绔彛浠呬粎鍒氬垰ACCEPT涓婂幓锛岃繕娌℃湁鏀跺埌鏁版嵁
    else if (PEER_STATUS_JUST_CONNECT == peer_status_)
    {
        //鏈�鍚庤皟鏁磋嚜宸盤EER鐨勭姸鎬�
        peer_status_ = PEER_STATUS_ACTIVE;

        ZCE_LOG(RS_INFO, "[zergsvr] Connect peer services[%u|%u],IP|Prot[%s] active success.",
                peer_svr_id_.services_type_,
                peer_svr_id_.services_id_,
                peer_address_.to_string(ip_addr_str,IP_ADDR_LEN,use_len)
               );

    }
    else
    {
        if (SERVICES_ID::DYNAMIC_ALLOC_SERVICES_ID != proc_frame->send_service_.services_id_)
        {
            //鍚﹀垯妫�娴嬪彂閫佽�呰繕鏄笉鏄師鏉ョ殑鍙戦�佽�咃紝鏄惁琚鏀�
            if ((peer_svr_id_ != proc_frame->send_service_) && (peer_svr_id_ != proc_frame->proxy_service_))
            {
                return SOAR_RET::ERR_ZERG_APPFRAME_ERROR;
            }
        }
    }

    //濡傛灉鏄摼鎺ユ敞鍐岀殑璇锋眰锛屽洖绛斾竴涓簲绛�
    //濡傛灉鎯冲仛澶嶆潅鐨勪氦浜掓帶鍒讹紝搴旇鍦ㄨ繖涓湴鏂瑰姞浠ｇ爜銆�
    if (ZERG_CONNECT_REGISTER_REQ == proc_frame->frame_command_)
    {
        send_simple_zerg_cmd(ZERG_CONNECT_REGISTER_RSP, peer_svr_id_);
    }

    //璁板綍鎺ュ彈浜嗗灏戞鏁版嵁
    receive_times_++;

    if (receive_times_ == 0)
    {
        ++receive_times_;
    }

    //
    //濉啓IP鍦板潃鍜岀鍙ｅ彿
    proc_frame->send_ip_address_ = peer_address_.get_ip_address();


    return 0;
}

//杩斿洖绔彛鐨勭姸鎬�,
TCP_Svc_Handler::PEER_STATUS  TCP_Svc_Handler::get_peer_status()
{
    return peer_status_;
}

/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2005骞�11鏈�27鏃�
Function        : TCP_Svc_Handler::process_connect_register
Return          : int
Parameter List  : NULL
Description     : 澶勭悊娉ㄥ唽鍙戦��,
Calls           :
Called By       :
Other           : 鍒氬垰杩炴帴涓婂鏂�,鍙戦�佷竴涓敞鍐屼俊鎭粰瀵规柟.濡傛灉鏈夊懡浠ゅ彂閫佸懡浠�
Modify Record   :
******************************************************************************************/
int TCP_Svc_Handler::process_connect_register()
{
    const size_t IP_ADDR_LEN = 32;
    char ip_addr_str[IP_ADDR_LEN + 1];
    size_t use_len = 0;
    peer_status_ = PEER_STATUS_JUST_CONNECT;

    //涓诲姩鍙戝皠涓�涓敞鍐孋MD锛屽鏋滄湁瑕佸彂灏勭殑鏁版嵁姝ｅソ.
    send_simple_zerg_cmd(ZERG_CONNECT_REGISTER_REQ, peer_svr_id_);

    //鍐嶆姌鑵句簡鎴戣嚦灏�3澶╀互鍚庯紝缁堜簬鍙戠幇浜咵POLL鍙嶅瑙﹀彂鍐欎簨浠剁殑鍘熷洜鏄病鏈夊彇娑圕ONNECT_MASK
    reactor()->cancel_wakeup(this, ZCE_Event_Handler::CONNECT_MASK);

    //娉ㄥ唽璇诲彇鐨凪ASK
    reactor()->schedule_wakeup(this, ZCE_Event_Handler::READ_MASK);


    //鎵撳嵃淇℃伅
    ZCE_Sockaddr_In      peeraddr;
    socket_peer_.getpeername(&peeraddr);
    ZCE_LOG(RS_INFO, "[zergsvr] Connect services[%u|%u] peer socket IP|Port :[%s] Success.",
            peer_svr_id_.services_type_,
            peer_svr_id_.services_id_,
            peeraddr.to_string(ip_addr_str,IP_ADDR_LEN,use_len));

    return 0;
}




//浠嶱EER璇诲彇鏁版嵁
int TCP_Svc_Handler::read_data_from_peer(size_t &szrevc)
{

    szrevc = 0;
    ssize_t recvret = 0;
    const size_t IP_ADDR_LEN = 32;
    char ip_addr_str[IP_ADDR_LEN + 1];
    size_t use_len = 0;
    //鐢宠鍒嗛厤涓�涓唴瀛�
    if (rcv_buffer_ == NULL)
    {
        rcv_buffer_ = zbuffer_storage_->allocate_buffer();
    }

    //ZCE_LOG(RS_INFO,"[zergsvr] read_data_from_peer %d .", get_handle());

    //鍏呭垎鍒╃敤缂撳啿鍖哄幓鎺ユ敹
    recvret = socket_peer_.recv(rcv_buffer_->buffer_data_ + rcv_buffer_->size_of_buffer_,
                                Zerg_Buffer::CAPACITY_OF_BUFFER - rcv_buffer_->size_of_buffer_,
                                0);

    //琛ㄧず琚叧闂垨鑰呭嚭鐜伴敊璇�
    if (recvret < 0)
    {
        //鎴戝彧浣跨敤EWOULDBLOCK 浣嗘槸瑕佹敞鎰廍AGAIN, zce::last_error() != EWOULDBLOCK && zce::last_error() != EAGAIN
        if (zce::last_error() != EWOULDBLOCK)
        {
            szrevc = 0;

            //閬囧埌涓柇,绛夊緟閲嶅叆
            if (zce::last_error() == EINTR)
            {
                return 0;
            }

            //缁熻鎺ユ敹閿欒
            server_status_->increase_by_statid(ZERG_RECV_FAIL_COUNTER, 0, 0, 1);

            //璁板綍閿欒,杩斿洖閿欒
            ZCE_LOG(RS_ERROR, "[zergsvr] Receive data error ,services[%u|%u],IP[%s] peer:%u,zce::last_error()=%d|%s.",
                    peer_svr_id_.services_type_,
                    peer_svr_id_.services_id_,
                    peer_address_.to_string(ip_addr_str,IP_ADDR_LEN,use_len),
                    socket_peer_.get_handle(),
                    zce::last_error(),
                    strerror(zce::last_error()));
            return SOAR_RET::ERR_ZERG_FAIL_SOCKET_OP_ERROR;
        }

        //缁熻鎺ユ敹闃诲鐨勯敊璇�
        server_status_->increase_by_statid(ZERG_RECV_BLOCK_COUNTER, 0, 0, 1);

        //濡傛灉閿欒鏄樆濉�,浠�涔堥兘涓嶄綔
        return 0;
    }

    //Socket琚叧闂紝涔熻繑鍥為敊璇爣绀�
    if (recvret == 0)
    {
        return SOAR_RET::ERR_ZERG_SOCKET_CLOSE;
    }

    //姝ゆ椂RET搴旇> 0
    szrevc = recvret;

    //鎺ユ敹浜哊涓瓧绗�
    rcv_buffer_->size_of_buffer_ += static_cast<size_t>(szrevc);
    recieve_bytes_ += static_cast<size_t>(szrevc);

    return 0;
}




//妫�鏌ユ槸鍚︽敹鍒颁簡涓�涓畬鏁寸殑甯�,
//杩欏効杩樺瓨鍦ㄤ竴绉嶅彲鑳�,涓�娆℃敹鍙栦簡澶氫釜甯х殑鍙兘,
int TCP_Svc_Handler::check_recv_full_frame(bool &bfull,
                                           unsigned int &whole_frame_len)
{
    whole_frame_len = 0;
    bfull = false;
    const size_t IP_ADDR_LEN = 32;
    char ip_addr_str[IP_ADDR_LEN + 1];
    size_t use_len = 0;
    //濡傛灉杩�4涓瓧鑺傞兘娌℃湁鏀堕泦榻�,涓嶇敤澶勭悊涓嬮潰杩欐
    //娉ㄦ剰杩欏効鏄湪32浣嶇幆澧冭�冭檻
    if (rcv_buffer_->size_of_buffer_ - rcv_buffer_->size_of_use_ >= sizeof(unsigned int))
    {
        //濡傛灉鏈�4涓瓧鑺�,妫�鏌ュ抚鐨勯暱搴�
        ZRD_U32_FROM_BYTES(whole_frame_len, (rcv_buffer_->buffer_data_ + rcv_buffer_->size_of_use_));
        whole_frame_len = ntohl(whole_frame_len);

<<<<<<< HEAD
        //如果包的长度大于定义的最大长度,小于最小长度,见鬼去,出现做个错误不是代码错误，就是被人整蛊
        if (whole_frame_len > ZERG_FRAME_HEAD::MAX_LEN_OF_APPFRAME || whole_frame_len < ZERG_FRAME_HEAD::LEN_OF_APPFRAME_HEAD)
=======
        //濡傛灉鍖呯殑闀垮害澶т簬瀹氫箟鐨勬渶澶ч暱搴�,灏忎簬鏈�灏忛暱搴�,瑙侀鍘�,鍑虹幇鍋氫釜閿欒涓嶆槸浠ｇ爜閿欒锛屽氨鏄浜烘暣铔�
        if (whole_frame_len > Zerg_App_Frame::MAX_LEN_OF_APPFRAME || whole_frame_len < Zerg_App_Frame::LEN_OF_APPFRAME_HEAD)
>>>>>>> 822b6377f4cdc443cbb2f70750601ca28174e570
        {
            ZCE_LOG(RS_ERROR, "[zergsvr] Recieve error frame,services[%u|%u],IP[%s], famelen %u , MAX_LEN_OF_APPFRAME:%u ,recv and use len:%u|%u.",
                    peer_svr_id_.services_type_,
                    peer_svr_id_.services_id_,
                    peer_address_.to_string(ip_addr_str,IP_ADDR_LEN,use_len),
                    whole_frame_len,
                    ZERG_FRAME_HEAD::MAX_LEN_OF_APPFRAME,
                    rcv_buffer_->size_of_buffer_,
                    rcv_buffer_->size_of_use_);
            //
            DEBUGDUMP_FRAME_HEAD_DBG(RS_DEBUG,
                                     "Error frame before framehead_decode,",
                                     reinterpret_cast<ZERG_FRAME_HEAD *>(rcv_buffer_->buffer_data_));
            return SOAR_RET::ERR_ZERG_GREATER_MAX_LEN_FRAME;
        }
    }


    //濡傛灉鎺ュ彈鐨勬暟鎹凡缁忓畬鏁�,(鑷冲皯鏈変竴涓畬鏁�)
    if (rcv_buffer_->size_of_buffer_ - rcv_buffer_->size_of_use_ >= whole_frame_len && whole_frame_len > 0)
    {
        bfull = true;
        ++recieve_counter_;
        ZCE_LOGMSG_DEBUG(RS_DEBUG, "Receive a whole frame from services[%u|%u] IP|Port [%s] FrameLen:%u.",
                         peer_svr_id_.services_type_,
                         peer_svr_id_.services_id_,
                         peer_address_.to_string(ip_addr_str,IP_ADDR_LEN,use_len),
                         whole_frame_len);
    }

    return 0;
}


//灏嗘暟鎹啓鍏EER锛屽悓鏃跺鐞嗗懆杈圭殑浜嬫儏锛屽寘鎷啓浜嬩欢娉ㄥ唽,濡傛灉鍙戦�侀槦鍒楄繕鏈夋暟鎹紝缁х画鍙戦�佺瓑
int TCP_Svc_Handler::write_all_data_to_peer()
{
    int ret = 0;
    const size_t IP_ADDR_LEN = 32;
    char ip_addr_str[IP_ADDR_LEN + 1];
    size_t use_len = 0;
    for (;;)
    {
        //鍙戦�佷竴涓暟鎹寘
        size_t szsend;
        bool   bfull = false;
        ret = write_data_to_peer(szsend, bfull);

        //鍑虹幇閿欒,
        if (ret != 0)
        {
            return ret;
        }

        //濡傛灉鏁版嵁鎶ュ凡缁忓畬鏁村彂閫�
        if (true == bfull)
        {
            //鎴愬姛锛岄噴鏀剧敵璇风殑绌洪棿
            zbuffer_storage_->free_byte_buffer(snd_buffer_deque_[0]);
            snd_buffer_deque_[0] = NULL;
            snd_buffer_deque_.pop_front();
        }
        //濡傛灉娌℃湁鍏ㄩ儴鍙戦�佸嚭鍘伙紝绛夊緟涓嬩竴娆″啓鏃堕棿鐨勮Е鍙�
        else
        {
            break;
        }

        //濡傛灉宸茬粡娌℃湁鏁版嵁鍙互鍙戦�佷簡
        if (snd_buffer_deque_.size() == 0)
        {
            break;
        }
    }

    //鍙栧緱褰撳墠鐨凪ASK鍊�
    int  handle_mask = get_mask();


    //濡傛灉闃熷垪涓病鏈夊彲浠ュ啓鐨勬暟鎹�
    if (snd_buffer_deque_.size() == 0)
    {
        //
        if (handle_mask & ZCE_Event_Handler::WRITE_MASK)
        {
            //鍙栨秷鍙啓鐨凪ASK鍊�,
            ret = reactor()->cancel_wakeup(this, ZCE_Event_Handler::WRITE_MASK);

            //return -1琛ㄧず閿欒锛屾纭繑鍥炵殑鏄痮ld mask鍊�
            if (-1 == ret)
            {
                ZCE_LOG(RS_ERROR, "[zergsvr] TNNND cancel_wakeup return(%d) == -1 errno=%d|%s. ",
                        ret,
                        zce::last_error(),
                        strerror(zce::last_error()));
            }

        }

        //濡傛灉灏嗚鍏抽棴
        if (true == if_force_close_)
        {
            ZCE_LOG(RS_INFO, "[zergsvr] Send to peer services [%u|%u] IP|Port :[%s] complete ,want to close peer on account of frame option.",
                    peer_svr_id_.services_type_,
                    peer_svr_id_.services_id_,
                    peer_address_.to_string(ip_addr_str,IP_ADDR_LEN,use_len));
            //璁╀笂灞傚幓鍏抽棴锛岃灏忓績锛屽皬蹇冿紝寰堥夯鐑︼紝寰堝鐢熷懡鍛ㄦ湡鐨勯棶棰�
            return SOAR_RET::ERR_ZERG_SOCKET_CLOSE;
        }
    }
    //濡傝繃娌℃湁鍙戦�佹垚鍔燂紝鍏ㄩ儴鍙戦�佸嚭鍘伙紝鍒欏噯澶囪繘琛屽啓浜嬩欢
    else
    {
        //娌℃湁WRITE MASK锛屽噯澶囧鍔犲啓鏍囧織
        if (!(handle_mask & ZCE_Event_Handler::WRITE_MASK))
        {
            ret = reactor()->schedule_wakeup(this, ZCE_Event_Handler::WRITE_MASK);

            //schedule_wakeup 杩斿洖return -1琛ㄧず閿欒锛屽啀娆S ACE涓�娆★紝姝ｇ‘杩斿洖鐨勬槸old mask鍊�
            if (-1 == ret)
            {
                ZCE_LOG(RS_ERROR, "[zergsvr] TNNND schedule_wakeup return (%d)== -1 errno=%d|%s. ",
                        ret,
                        zce::last_error(),
                        strerror(zce::last_error()));
            }
        }
    }

    return 0;
}



//灏嗘暟鎹啓鍏EER
int TCP_Svc_Handler::write_data_to_peer(size_t &szsend, bool &bfull)
{
    bfull = false;
    szsend = 0;
    const size_t IP_ADDR_LEN = 32;
    char ip_addr_str[IP_ADDR_LEN + 1];
    size_t use_len = 0;
    //濡傛灉娌℃湁鏁版嵁瑕佸彂閫�, 鍒拌繖鍎垮簲璇ユ槸鏈夐棶棰�
    //#if defined DEBUG || defined _DEBUG
    if (snd_buffer_deque_.empty() == true)
    {
        ZCE_LOG(RS_ERROR, "[zergsvr] Goto handle_output|write_data_to_peer ,but not data to send. Please check,buffer deque size=%u.",
                snd_buffer_deque_.size());
        ZCE_BACKTRACE_STACK(RS_ERROR);
        reactor()->cancel_wakeup(this, ZCE_Event_Handler::WRITE_MASK);
        ZCE_ASSERT(false);
        return 0;
    }


    //鍓嶉潰鏈夋鏌�,涓嶄細瓒婄晫
    Zerg_Buffer *sndbuffer = snd_buffer_deque_[0];

    ssize_t sendret = socket_peer_.send(sndbuffer->buffer_data_ + sndbuffer->size_of_buffer_,
                                        sndbuffer->size_of_use_ - sndbuffer->size_of_buffer_,
                                        0);

    if (sendret <= 0)
    {

        //閬囧埌涓柇,绛夊緟閲嶅叆鐨勫垽鏂槸if (zce::last_error() == EINVAL),浣嗚繖鍎夸笉浠旂粏妫�鏌ラ敊璇�,涓�瑙嗗悓浠�,涓婂眰鍥炲拷瑙嗘墍鏈夐敊璇�,濡傛灉閿欒鑷村懡,杩樹細鏈塰andle_input鍙嶅皠
        //鎴戝彧浣跨敤EWOULDBLOCK 浣嗘槸瑕佹敞鎰廍AGAIN zce::last_error() != EWOULDBLOCK && zce::last_error() != EAGAIN
        if (zce::last_error() != EWOULDBLOCK)
        {
            //鍚庨潰搴旇浼氭墦鍗版柟鐨処P锛岃繖鍎夸笉閲嶅
            ZCE_LOG(RS_ERROR, "[zergsvr] Send data error,services[%u|%u] IP|Port [%s],Peer:%d errno=%d|%s .",
                    peer_svr_id_.services_type_,
                    peer_svr_id_.services_id_,
                    peer_address_.to_string(ip_addr_str,IP_ADDR_LEN,use_len),
                    socket_peer_.get_handle(),
                    zce::last_error(),
                    strerror(zce::last_error()));
            server_status_->increase_by_statid(ZERG_SEND_FAIL_COUNTER, 0, 0, 1);

            return SOAR_RET::ERR_ZERG_FAIL_SOCKET_OP_ERROR;
        }

        //缁熻鍙戦�侀樆濉炵殑閿欒
        server_status_->increase_by_statid(ZERG_SEND_BLOCK_COUNTER, 0, 0, 1);

        //濡傛灉閿欒鏄樆濉�,浠�涔堥兘涓嶄綔
        return 0;
    }

    szsend = sendret;

    //鍙戦�佷簡N涓瓧绗�
    sndbuffer->size_of_buffer_ += static_cast<size_t>(szsend);
    send_bytes_ += static_cast<size_t>(szsend);

    //濡傛灉鏁版嵁宸茬粡鍏ㄩ儴鍙戦�佷簡
    if (sndbuffer->size_of_use_ == sndbuffer->size_of_buffer_)
    {
        bfull = true;
        ++send_counter_;
        //ZCE_LOGMSG_DEBUG(RS_DEBUG,"Send a few(n>=1) whole frame To  IP|Port :%s|%u FrameLen:%u.",
        //    peer_address_.get_host_addr(),
        //    peer_address_.get_port_number(),
        //    sndbuffer->size_of_buffer_);
    }

    return 0;
}

//澶勭悊鍙戦�侀敊璇�.
int TCP_Svc_Handler::process_send_error(Zerg_Buffer *tmpbuf, bool frame_encode)
{
    const size_t IP_ADDR_LEN = 32;
    char ip_addr_str[IP_ADDR_LEN + 1];
    size_t use_len = 0;
    //璁板綍宸茬粡浣跨敤鍒扮殑浣嶇疆
    size_t use_start = tmpbuf->size_of_buffer_;
    tmpbuf->size_of_buffer_ = 0;

    //涓�涓槦鍒椾腑闂村彲鑳芥湁澶氫釜FRAME锛岃瀵瑰ご閮ㄨ繘琛岃В鐮侊紝鎵�浠ュ繀椤讳竴涓釜寮勫嚭鏉�
    while (tmpbuf->size_of_buffer_ != tmpbuf->size_of_use_)
    {
        ZERG_FRAME_HEAD *proc_frame = reinterpret_cast<ZERG_FRAME_HEAD *>(tmpbuf->buffer_data_ +
                                                                        tmpbuf->size_of_buffer_);

        //濡傛灉FRAME宸茬粡缂栫爜
        if (frame_encode)
        {
            proc_frame->framehead_decode();
        }

        //妫�鏌ュ凡缁忎娇鐢ㄧ殑鍦板潃琛ㄧず杩欎釜甯ф槸鍚﹀彂閫佷簡,濡傛灉宸茬粡鍙戦�佷簡锛岃繖涓抚灏变笉瑕佸鐞�

        //濡傛灉娌℃湁鍙戦�佸畬鎴愶紝璁板綍涓嬫潵锛岃繘琛屽鐞�
        if (use_start < tmpbuf->size_of_buffer_ + proc_frame->frame_length_)
        {

<<<<<<< HEAD
            //如果是要记录的命令，记录下来，可以帮忙回溯一些问题
            if (proc_frame->frame_option_ & ZERG_FRAME_HEAD::DESC_SEND_FAIL_RECORD)
=======
            //濡傛灉鏄璁板綍鐨勫懡浠わ紝璁板綍涓嬫潵锛屽彲浠ュ府蹇欏洖婧竴浜涢棶棰�
            if (proc_frame->frame_option_ & Zerg_App_Frame::DESC_SEND_FAIL_RECORD)
>>>>>>> 822b6377f4cdc443cbb2f70750601ca28174e570
            {
                ZCE_LOG(RS_ERROR, "[zergsvr] Connect peer ,send frame fail.frame len[%u] frame command[%u] frame "
                        "uid[%u] snd svcid[%u|%u] proxy svc [%u|%u] recv[%u|%u] address[%s],peer status[%u]. ",
                        proc_frame->frame_length_,
                        proc_frame->frame_command_,
                        proc_frame->frame_userid_,
                        proc_frame->send_service_.services_type_,
                        proc_frame->send_service_.services_id_,
                        proc_frame->proxy_service_.services_type_,
                        proc_frame->proxy_service_.services_id_,
                        proc_frame->recv_service_.services_type_,
                        proc_frame->recv_service_.services_id_,
                        peer_address_.to_string(ip_addr_str,IP_ADDR_LEN,use_len),
                        peer_status_
                       );
            }
        }

        //澧炲姞閿欒鍙戦�佺殑澶勭悊
        server_status_->increase_by_statid(ZERG_SEND_FAIL_COUNTER, 0, 0, 1);

        //
        tmpbuf->size_of_buffer_ += proc_frame->frame_length_;
    }

    //褰掕繕鍒癙OOL涓棿銆�
    zbuffer_storage_->free_byte_buffer(tmpbuf);

    return 0;


}



// 浠庢睜瀛愰噷闈㈠緱鍒颁竴涓狧andler缁欏ぇ瀹朵娇鐢�
TCP_Svc_Handler *TCP_Svc_Handler::alloce_hdl_from_pool(HANDLER_MODE handler_mode)
{
    //
    if (handler_mode == HANDLER_MODE_ACCEPTED)
    {
        if (pool_of_acpthdl_.size() == 0)
        {
            ZCE_LOG(RS_INFO, "[zergsvr] Pool is too small to process accept handler,please notice."
                    "Pool size:%u,capacity:%u.",
                    pool_of_acpthdl_.size(),
                    pool_of_acpthdl_.capacity()
                   );
            return NULL;
        }

        TCP_Svc_Handler *p_handler = NULL;
        pool_of_acpthdl_.pop_front(p_handler);
        return p_handler;
    }
    //Connect鐨勭鍙ｅ簲璇ユ案杩滀笉鍙戠敓鍙栦笉鍒癏anler鐨勪簨鎯�
    else if (HANDLER_MODE_CONNECT == handler_mode)
    {
        ZCE_ASSERT(pool_of_cnthdl_.size() > 0);
        TCP_Svc_Handler *p_handler = NULL;
        pool_of_cnthdl_.pop_front(p_handler);
        return p_handler;
    }
    //Never go here.
    else
    {
        ZCE_ASSERT(false);
        return NULL;
    }
}


//閾炬帴鎵�鏈夌殑瑕佽嚜鍔ㄩ摼鎺ョ殑鏈嶅姟鍣�,杩欎釜浜嬮伩鍏嶆湇鍔″櫒鐨勯摼鎺ユ柇鍙ｅ悗銆傚張娌℃湁鏁版嵁鍙戦�佺殑鎯呭喌
void TCP_Svc_Handler::reconnect_allserver()
{
    //杩炴帴鎵�鏈夌殑SERVER
    size_t num_valid = 0, num_succ = 0, num_fail = 0;
    zerg_auto_connect_.reconnect_allserver(num_valid, num_succ, num_fail);
}
//
int TCP_Svc_Handler::uninit_all_staticdata()
{
    //
    svr_peer_info_set_.clear_and_closeall();
    //
    pool_of_acpthdl_.clear();

    return 0;
}

//澶勭悊鍙戦�佷竴涓暟鎹�
int TCP_Svc_Handler::process_send_data(Zerg_Buffer *tmpbuf)
{
    int ret = 0;
    server_status_->increase_by_statid(ZERG_SEND_FRAME_COUNTER, 0, 0, 1);
    //
    ZERG_FRAME_HEAD *proc_frame = reinterpret_cast<ZERG_FRAME_HEAD *>(tmpbuf->buffer_data_);
    DEBUGDUMP_FRAME_HEAD_DBG(RS_DEBUG, "process_send_data Before framehead_encode:", proc_frame);

    SERVICES_ID *p_sendto_svrinfo = NULL;

    //鍙戦�佺粰浠ｇ悊锛屽彂閫佺粰鎺ュ彈鑰�
    if (proc_frame->proxy_service_.services_type_ != SERVICES_ID::INVALID_SERVICES_TYPE && if_proxy_ == false)
    {
        p_sendto_svrinfo = &(proc_frame->proxy_service_);
    }
    else
    {
        p_sendto_svrinfo = &(proc_frame->recv_service_);
    }

    //骞挎挱
    if (p_sendto_svrinfo->services_id_ == SERVICES_ID::BROADCAST_SERVICES_ID)
    {
        std::vector<uint32_t> *id_ary;
        ret = svr_peer_info_set_.find_hdlary_by_type(p_sendto_svrinfo->services_type_, id_ary);
        if (0 != ret )
        {
            ZCE_LOG(RS_ERROR, "process_send_data: service_id==BROADCAST_SERVICES_ID but cant't find has service_type=%d svrinfo",
                    p_sendto_svrinfo->services_type_);
            return SOAR_RET::ERR_ZERG_SEND_FRAME_FAIL;
        }

        size_t ary_size = id_ary->size();
        TCP_Svc_Handler *svchanle = NULL;
        for (size_t i = 0; i < ary_size; ++i)
        {
            SERVICES_ID bc_svc_id(p_sendto_svrinfo->services_type_, (*id_ary)[i]);
            ret = svr_peer_info_set_.find_handle_by_svcid(bc_svc_id, svchanle);

            //鐞嗚涓婁笉鍙兘鎵句笉鍒�
            ZCE_ASSERT(ret == 0);
            if (ret != 0)
            {
            }

            p_sendto_svrinfo->services_id_ = bc_svc_id.services_id_;
            svchanle->put_frame_to_sendlist(tmpbuf);
        }

    }
    //缁欎竴涓汉
    else
    {
        uint32_t services_id = SERVICES_ID::INVALID_SERVICES_ID;
        TCP_Svc_Handler *svchanle = NULL;

        //瀵逛竴浜涘姩鎬佺殑SVC ID杩涜澶勭悊
        //璐熻浇鍧囪　鐨勬柟寮�
        if (p_sendto_svrinfo->services_id_ == SERVICES_ID::LOAD_BALANCE_DYNAMIC_ID)
        {

            ret = svr_peer_info_set_.find_lbseqhdl_by_type(p_sendto_svrinfo->services_type_, services_id, svchanle);
            if (ret != 0)
            {
                ZCE_LOG(RS_ERROR, "process_send_data: service_id==LOAD_BALANCE_DYNAMIC_ID but cant't find has service_type=%d svrinfo",
                        p_sendto_svrinfo->services_type_);
            }

            // 淇敼涓�涓嬭鍙戦�佺殑svrinfo鐨刬d
            p_sendto_svrinfo->services_id_ = services_id;
            ZCE_LOG(RS_DEBUG, "process_send_data: service_type=%d service_id= LOAD_BALANCE_DYNAMIC_ID,"
                    " change service id to %u",
                    p_sendto_svrinfo->services_type_,
                    p_sendto_svrinfo->services_id_);
        }
        //杩欑鎯呭喌锛岄厤缃殑鏈嶅姟鍣ㄦ暟閲忎笉鑳藉お澶�
        //璐熻浇鍧囪　鐨勬柟寮�
        else if (p_sendto_svrinfo->services_id_ == SERVICES_ID::MAIN_STANDBY_DYNAMIC_ID)
        {
            ret = svr_peer_info_set_.find_mshdl_by_type(p_sendto_svrinfo->services_type_, services_id, svchanle);
            if (ret != 0)
            {
                ZCE_LOG(RS_ERROR, "process_send_data: service_id==MAIN_STANDBY_DYNAMIC_ID but cant't find has service_type=%d svrinfo",
                        p_sendto_svrinfo->services_type_);
            }
            // 淇敼涓�涓嬭鍙戦�佺殑svrinfo鐨刬d
            p_sendto_svrinfo->services_id_ = services_id;
            ZCE_LOG(RS_DEBUG, "process_send_data: service_type=%d service_id= LOAD_BALANCE_DYNAMIC_ID,"
                    " change service id to %u",
                    p_sendto_svrinfo->services_type_,
                    p_sendto_svrinfo->services_id_);
            //鍒拌揪杩欏効搴旇鏈夊嚑绉嶆儏鍐�,
            //涓昏矾鐢監K锛屽浜嶢CTIVE鐘舵�侊紝浣跨敤涓昏矾鐢卞彂閫�
            //涓昏矾鐢变笉澶勪簬ACTIVE鐘舵�侊紝浣嗘槸澶囦唤璺敱澶勪簬ACTIVE鐘舵�侊紝浣跨敤澶囦唤璺敱鍙戦��
            //涓昏矾鐢卞瓨鍦紝浣嗘槸涓嶅浜嶢CTIVE鐘舵�侊紝澶囦唤璺敱涔熶笉澶勪簬ACTIVE鐘舵�侊紝灏嗘暟鎹氦缁欎竴涓汉锛岀紦鍐插埌鍙戦�侀槦鍒�
        }
        else
        {
            ret = svr_peer_info_set_.find_handle_by_svcid(*p_sendto_svrinfo, svchanle);
            //濡傛灉鏄涓诲姩杩炴帴鍑哄幓鐨勬湇鍔″櫒
            if (0 != ret )
            {
                ZCE_LOG(RS_ERROR, "process_send_data: but cant't find has svc id=%u.%u svrinfo",
                        p_sendto_svrinfo->services_type_,
                        p_sendto_svrinfo->services_id_);

                //if (zerg_auto_connect_.is_auto_connect_svcid(*p_sendto_svrinfo))
                //{
                //    //涓嶆鏌ユ槸鍚︽垚鍔燂紝寮傛杩炴帴锛�99.99999%鏄垚鍔熺殑,
                //    zerg_auto_connect_.reconnect_server(*p_sendto_svrinfo);
                //}
            }
        }

        //Double Check鏂规硶
        //濡傛灉SVCHANDLE涓虹┖,琛ㄧず娌℃湁鐩稿叧鐨勮繛鎺�,杩涜閿欒澶勭悊
        if (svchanle == NULL)
        {
            //杩欏効杩樻病鏈夌紪鐮�
            ZCE_LOG(RS_ERROR, "[zergsvr] [SEND TO NO EXIST HANDLE] ,send to a no exist handle[%u|%u],it could "
                    "have been existed. frame command[%u]. uid[%u] frame length[%u].",
                    p_sendto_svrinfo->services_type_,
                    p_sendto_svrinfo->services_id_,
                    proc_frame->frame_command_,
                    proc_frame->frame_userid_,
                    proc_frame->frame_length_
                   );
            DEBUGDUMP_FRAME_HEAD_DBG(RS_ERROR, "[SEND TO NO EXIST HANDLE]", proc_frame );
            return SOAR_RET::ERR_ZERG_SEND_FRAME_FAIL;
        }


        //灏嗗彂閫佺殑FRAME缁橦ANDLE瀵硅薄锛屽綋鐒惰繖涓湴鏂规湭蹇呬竴瀹氭斁鐨勮繘鍘伙紝鍥犱负鏈夊嚑绉嶆儏鍐�,
        //1.灏辨槸涓�涓叧闂寚浠�,
        //2.HANDLE鍐呴儴鐨勯槦鍒楁弧浜�,

        //杩欏効涓嶈繘琛岄敊璇鐞嗭紝鍥犱负put_frame_to_sendlist鍐呴儴杩涜浜嗛敊璇鐞嗭紝鍥炴敹绛夋搷浣�
        //鍒拌繖鍎夸负姝紝璁や负鎴愬姛
        svchanle->put_frame_to_sendlist(tmpbuf);
    }

    return 0;
}


///鏍规嵁services_type鏌ヨ瀵瑰簲鐨勯厤缃富澶囨湇鍔″櫒鍒楄〃鏁扮粍 MS锛堜富澶囷級,
///璇峰弬鑰� @ref Zerg_Auto_Connector
int TCP_Svc_Handler::find_conf_ms_svcid_ary(uint16_t services_type,
                                            std::vector<uint32_t> *&ms_svcid_ary)
{
    return zerg_auto_connect_.find_conf_ms_svcid_ary(services_type, ms_svcid_ary);
}


//鍙戦�佺畝鍗曠殑鐨刏ERG鍛戒护,鐢ㄤ簬鏌愪簺鐗规畩鍛戒护鐨勫鐞�
int TCP_Svc_Handler::send_simple_zerg_cmd(unsigned int cmd,
                                          const SERVICES_ID &recv_services_info,
                                          unsigned int option)
{
    //ZCE_LOGMSG_DEBUG(RS_DEBUG,"Send simple command to services[%u|%u] IP[%s|%u],Cmd %u.",
    //    peer_svr_info_.services_type_,
    //    peer_svr_info_.services_id_,
    //    peer_address_.get_host_addr(),
    //    peer_address_.get_port_number(),
    //    cmd);
    //鍚戝鏂瑰彂閫佷竴涓績璺冲寘
    Zerg_Buffer *tmpbuf = zbuffer_storage_->allocate_buffer();
    ZERG_FRAME_HEAD *proc_frame = reinterpret_cast<ZERG_FRAME_HEAD *>(tmpbuf->buffer_data_);

<<<<<<< HEAD
    proc_frame->init_framehead(ZERG_FRAME_HEAD::LEN_OF_APPFRAME_HEAD, option, cmd);
    //注册命令
=======
    proc_frame->init_framehead(Zerg_App_Frame::LEN_OF_APPFRAME_HEAD, option, cmd);
    //娉ㄥ唽鍛戒护
>>>>>>> 822b6377f4cdc443cbb2f70750601ca28174e570
    proc_frame->send_service_ = my_svc_id_;

    //濡傛灉鑷繁鏄唬鐞嗘湇鍔″櫒,濉啓浠ｇ悊鏈嶅姟鍣ㄤ俊鎭�,淇濊瘉鍙戦��,
    if (if_proxy_)
    {
        proc_frame->proxy_service_ = my_svc_id_;
    }

    //
    proc_frame->recv_service_ = recv_services_info;

    //
    tmpbuf->size_of_use_ = ZERG_FRAME_HEAD::LEN_OF_APPFRAME_HEAD;

    //
    return put_frame_to_sendlist(tmpbuf);
}

//鍙戦�佸績璺�
int TCP_Svc_Handler::send_zergheatbeat_reg()
{
    //
    return send_simple_zerg_cmd(ZERG_HEART_BEAT_REQ, peer_svr_id_);
}


//灏嗗彂閫佹暟鎹斁鍏ュ彂閫侀槦鍒椾腑
//濡傛灉涓�涓狿EER娌℃湁杩炴帴涓�,绛夊緟鍙戦�佺殑鏁版嵁涓嶈兘澶氫簬PEER_STATUS_NOACTIVE涓�
//put_frame_to_sendlist鍐呴儴杩涜浜嗛敊璇鐞嗭紝鍥炴敹绛夋搷浣�
int TCP_Svc_Handler::put_frame_to_sendlist(Zerg_Buffer *tmpbuf)
{
    int ret = 0;
    const size_t IP_ADDR_LEN = 32;
    char ip_addr_str[IP_ADDR_LEN + 1];
    size_t use_len = 0;
    ZERG_FRAME_HEAD *proc_frame = reinterpret_cast<ZERG_FRAME_HEAD *>(tmpbuf->buffer_data_);

    //濡傛灉鏄�氱煡鍏抽棴绔彛
    if (proc_frame->frame_command_ == INNER_RSP_CLOSE_SOCKET)
    {
        ZCE_LOG(RS_INFO, "[zergsvr] Recvice CMD_RSP_CLOSE_SOCKET,services[%u|%u] IP[%s] Svchanle will close.",
                peer_svr_id_.services_type_,
                peer_svr_id_.services_id_,
                peer_address_.to_string(ip_addr_str,IP_ADDR_LEN,use_len));
        if_force_close_ = true;
        //鍥炴敹甯�
        process_send_error(tmpbuf, false);
        //濡傛灉涓嶆槸UDP鐨勫鐞�,鍏抽棴绔彛,UDP鐨勪笢瑗挎病鏈夐摼鎺ョ殑姒傚康,
        handle_close();

        //杩斿洖涓�涓敊璇紝璁╀笂灞傚洖鏀�
        return SOAR_RET::ERR_ZERG_SOCKET_CLOSE;
    }

<<<<<<< HEAD
    //如果发送完成,并且后台业务要求关闭端口,注意必须转换网络序
    if (proc_frame->frame_option_ & ZERG_FRAME_HEAD::DESC_SNDPRC_CLOSE_PEER)
=======
    //濡傛灉鍙戦�佸畬鎴�,骞朵笖鍚庡彴涓氬姟瑕佹眰鍏抽棴绔彛,娉ㄦ剰蹇呴』杞崲缃戠粶搴�
    if (proc_frame->frame_option_ & Zerg_App_Frame::DESC_SNDPRC_CLOSE_PEER)
>>>>>>> 822b6377f4cdc443cbb2f70750601ca28174e570
    {
        ZCE_LOG(RS_INFO, "[zergsvr] This Peer Services[%u|%u] IP|Port :[%s] will close when all frame"
                " send complete ,because send frame has option Zerg_App_Frame::DESC_SNDPRC_CLOSE_PEER.",
                peer_svr_id_.services_type_,
                peer_svr_id_.services_id_,
                peer_address_.to_string(ip_addr_str,IP_ADDR_LEN,use_len));
        if_force_close_ = true;
    }

    //娉ㄦ剰杩欎釜鍦版柟锛屾垜浠細璋冩暣杩欎釜Services ID锛屼繚璇佸彂閫佸嚭鍘荤殑鏁版嵁閮芥槸鐢ㄨ嚜宸辩殑SVCID鏍囩ず鐨�.
    if (!if_proxy_)
    {
        proc_frame->send_service_ = my_svc_id_;
    }

    //瀵瑰ご閮ㄨ繘琛岀紪鐮�
    proc_frame->framehead_encode();

    //鏀惧叆鍙戦�侀槦鍒�,骞舵敞鍐屾爣蹇椾綅
    bool bret = snd_buffer_deque_.push_back(tmpbuf);

    if (!bret)
    {
        server_status_->increase_by_statid(ZERG_SEND_LIST_FULL_COUNTER, 0, 0, 1);
        //涓㈠純鎴栬�呴敊璇鐞嗛偅涓暟鎹瘮杈冨ソ鍛�?杩欏効鍊煎緱鍟嗘Ψ, 鎴戣繖鍎胯繘琛岄敊璇鐞�(鍙兘涓㈠純)鐨勬槸鏈�鏂扮殑.
        //鎴戠殑鑰冭檻鏄鏋滃懡浠ゆ湁鍏堝悗鎬�.鑰屼笖鍙互閬垮厤鍐呭瓨鎿嶄綔.
        ZCE_LOG(RS_ERROR, "[zergsvr] Services [%u|%u] IP|Port[%s] send buffer cycle deque is full,this data must throw away,Send deque capacity =%u,may be extend it.",
                peer_svr_id_.services_type_,
                peer_svr_id_.services_id_,
                peer_address_.to_string(ip_addr_str,IP_ADDR_LEN,use_len),
                snd_buffer_deque_.capacity());

        //鍥炴敹甯�
        process_send_error(tmpbuf, true);
        //杩斿洖涓�涓敊璇�
        return SOAR_RET::ERR_ZERG_SEND_FRAME_FAIL;
    }

    //------------------------------------------------------------------
    //杩欏効寮�濮嬶紝鏁版嵁宸茬粡鏀惧叆鍙戦�侀槦鍒楋紝鍥炴敹鍙互鍐峢andle_close鑷繁鍥炴敹浜�.

    if (peer_status_ != PEER_STATUS_NOACTIVE)
    {
        ret = write_all_data_to_peer();

        //鍑虹幇閿欒,
        if (ret != 0)
        {
            //涓轰粈涔堟垜涓嶅鐞嗛敊璇憿,涓峳eturn -1,鍥犱负濡傛灉閿欒浼氬叧闂璖ocket,handle_input灏嗚璋冪敤,杩欏効涓嶉噸澶嶅鐞�
            //濡傛灉鏄腑鏂瓑閿欒,绋嬪簭鍙互缁х画鐨�.
            handle_close();

            //鍙戦�佹暟鎹凡缁忔斁鍏ラ槦鍒楋紝杩斿洖OK
            return 0;
        }

        //鍚堝苟
        unite_frame_sendlist();
    }

    //鍙湁鏀惧叆鍙戦�侀槦鍒楁墠绠楁垚鍔�.
    return 0;
}


//鍚堝苟鍙戦�侀槦鍒�
void TCP_Svc_Handler::unite_frame_sendlist()
{
    //濡傛灉鏈�2涓互涓婄殑鐨勫彂閫侀槦鍒楋紝鍒欏彲浠ヨ�冭檻鍚堝苟澶勭悊
    size_t sz_deque = snd_buffer_deque_.size();

    if (sz_deque <= 1)
    {
        return;
    }

<<<<<<< HEAD
    //如果倒数第2个桶有能力放下倒数第1个桶的FRAME数据，则进行合并操作。
    if (ZERG_FRAME_HEAD::MAX_LEN_OF_APPFRAME - snd_buffer_deque_[sz_deque - 2]->size_of_use_ > snd_buffer_deque_[sz_deque - 1]->size_of_use_)
=======
    //濡傛灉鍊掓暟绗�2涓《鏈夎兘鍔涙斁涓嬪�掓暟绗�1涓《鐨凢RAME鏁版嵁锛屽垯杩涜鍚堝苟鎿嶄綔銆�
    if (Zerg_App_Frame::MAX_LEN_OF_APPFRAME - snd_buffer_deque_[sz_deque - 2]->size_of_use_ > snd_buffer_deque_[sz_deque - 1]->size_of_use_)
>>>>>>> 822b6377f4cdc443cbb2f70750601ca28174e570
    {
        //灏嗗�掓暟绗�1涓妭鐐圭殑鏁版嵁鏀惧叆鍊掓暟绗�2涓妭鐐逛腑闂淬�傛墍浠ュ疄闄呯殑Cache鑳藉姏鏄潪甯稿己鐨勶紝
        //绌洪棿鍒╃敤鐜囦篃寰堥珮銆傝秺鍙戜僵鏈嶆垜鑷繁浜嗐��
        memcpy(snd_buffer_deque_[sz_deque - 2]->buffer_data_ + snd_buffer_deque_[sz_deque - 2]->size_of_use_,
               snd_buffer_deque_[sz_deque - 1]->buffer_data_,
               snd_buffer_deque_[sz_deque - 1]->size_of_use_);
        snd_buffer_deque_[sz_deque - 2]->size_of_use_ += snd_buffer_deque_[sz_deque - 1]->size_of_use_;

        //灏嗗�掓暟绗竴涓柦鏀炬帀
        zbuffer_storage_->free_byte_buffer(snd_buffer_deque_[sz_deque - 1]);
        snd_buffer_deque_[sz_deque - 1] = NULL;
        snd_buffer_deque_.pop_back();
    }

    ////涓嬮潰鐨勪唬鐮佺敤浜庡悎骞剁殑娴嬭瘯锛屽钩甯镐細娉ㄩ噴鎺�
    //else
    //{
    //    ZCE_LOGMSG_DEBUG(RS_DEBUG,"Goto unite_frame_sendlist sz_deque=%u,Zerg_App_Frame::MAX_LEN_OF_APPFRAME=%u,"
    //        "snd_buffer_deque_[sz_deque-2]->size_of_use_=%u,"
    //        "snd_buffer_deque_[sz_deque-1]->size_of_use_=%u.",
    //        sz_deque,
    //        Zerg_App_Frame::MAX_LEN_OF_APPFRAME,
    //        snd_buffer_deque_[sz_deque-2]->size_of_use_,
    //        snd_buffer_deque_[sz_deque-1]->size_of_use_);
    //}

}



//灏嗘暟鎹抚浜ょ粰閫氫俊绠＄悊鍣紝鏀惧叆绠￠亾
int TCP_Svc_Handler::push_frame_to_comm_mgr()
{

    int ret = 0;
    const size_t IP_ADDR_LEN = 32;
    char ip_addr_str[IP_ADDR_LEN + 1];
    size_t use_len = 0;
    //
    rcv_buffer_->size_of_use_ = 0;

    //
    while (rcv_buffer_)
    {
        unsigned int whole_frame_len = 0;
        bool bfull = false;
        ret = check_recv_full_frame(bfull, whole_frame_len);

        if (ret != 0)
        {
            return -1;
        }

        //濡傛灉娌℃湁鍙楀埌
        if (false == bfull)
        {
            if (rcv_buffer_->size_of_use_ > 0)
            {
                //鎷疯礉鐨勫唴瀛樺彲鑳戒氦閿�,鎵�浠ヤ笉鐢╩emcpy
                memmove(rcv_buffer_->buffer_data_,
                        rcv_buffer_->buffer_data_ + rcv_buffer_->size_of_use_,
                        rcv_buffer_->size_of_buffer_ - rcv_buffer_->size_of_use_);

                //鏀瑰彉buffer闀垮害
                rcv_buffer_->size_of_buffer_ = rcv_buffer_->size_of_buffer_ - rcv_buffer_->size_of_use_;
                rcv_buffer_->size_of_use_ = 0;
            }

            break;
        }

        ZERG_FRAME_HEAD *proc_frame = reinterpret_cast<ZERG_FRAME_HEAD *>(rcv_buffer_->buffer_data_ + rcv_buffer_->size_of_use_);

        //濡傛灉宸茬粡鏀堕泦浜嗕竴涓暟鎹�
        ret = preprocess_recvframe(proc_frame);

        //宸茬粡鏈夌浉鍚岀殑ID杩炴帴浜�,鎴栬�呭抚妫�鏌ラ敊璇�
        if (ret != 0)
        {
            //杩笉寰楀凡,鎶� 閿欒绛斿簲鐨勫嚱鏁版斁鍒拌繖涓湴鏂�,鏀惧埌鍚庨潰鐨勫嚱鏁�,瑕佽緭鍑虹殑绉嶇被杩囧,杩濊儗鎴戠殑浠ｇ爜缇庡.
            if (SOAR_RET::ERR_ZERG_APPFRAME_ERROR == ret || SOAR_RET::ERR_ZERG_SERVER_ALREADY_LONGIN == ret)
            {
                //
                ZCE_LOG(RS_ERROR, "[zergsvr] Peer services[%u|%u] IP[%s] appFrame Error,Frame Len:%u,"
                        "Command:%u,Uin:%u "
                        "Peer SvrType|SvrID:%u|%u,"
                        "Self SvrType|SvrID:%u|%u,"
                        "Send SvrType|SvrID:%u|%u,"
                        "Recv SvrType|SvrID:%u|%u,"
                        "Proxy SvrType|SvrID:%u|%u.",
                        peer_svr_id_.services_type_,
                        peer_svr_id_.services_id_,
                        peer_address_.to_string(ip_addr_str,IP_ADDR_LEN,use_len),
                        proc_frame->frame_length_,
                        proc_frame->frame_command_,
                        proc_frame->frame_userid_,
                        peer_svr_id_.services_type_, peer_svr_id_.services_id_,
                        my_svc_id_.services_type_, my_svc_id_.services_id_,
                        proc_frame->send_service_.services_type_,
                        proc_frame->send_service_.services_id_,
                        proc_frame->recv_service_.services_type_,
                        proc_frame->recv_service_.services_id_,
                        proc_frame->proxy_service_.services_type_,
                        proc_frame->proxy_service_.services_id_
                       );
            }
            else
            {
                ZCE_LOG(RS_ERROR, "[zergsvr] Peer services [%u|%u] IP[%s] preprocess_recvframe Ret =%d.",
                        peer_svr_id_.services_type_,
                        peer_svr_id_.services_id_,
                        peer_address_.to_string(ip_addr_str,IP_ADDR_LEN,use_len),
                        ret);
            }

            //缁熻鎺ユ敹閿欒
            server_status_->increase_by_statid(ZERG_RECV_FAIL_COUNTER, 0, 0, 1);
            return -1;
        }

        //灏嗘暟鎹斁鍏ユ帴鏀剁殑绠￠亾,涓嶆娴嬮敊璇�,鍥犱负閿欒浼氳褰曟棩蹇�,鑰屼笖鏈夐敊璇紝涔熸棤娉曞鐞�

        zerg_comm_mgr_->pushback_recvpipe(proc_frame);

        //鎺ユ敹涓�涓畬鏁寸殑鏁版嵁
        rcv_buffer_->size_of_use_ += whole_frame_len;

        if (rcv_buffer_->size_of_buffer_ == rcv_buffer_->size_of_use_)
        {
            //鏃犺澶勭悊姝ｇ‘涓庡惁,閮介噴鏀剧紦鍐插尯鐨勭┖闂�
            zbuffer_storage_->free_byte_buffer(rcv_buffer_);
            rcv_buffer_ = NULL;
        }
        //濡傛灉绗竴涓寘鐨勬敹鍒版暟鎹凡缁忓ぇ浜庤繖涓暱搴�.閭ｄ箞灏变細鍑虹幇涓嬮潰鐨勬儏鍐碉紝
        //濡傛灉杩欏効鎯抽伩鍏嶅鏉傜殑鍒ゆ柇锛屽彲浠ラ檺瀹氭敹鍒扮殑绗竴涓暟鎹寘鐨勬渶澶ч暱搴︿负甯уご鐨勯暱搴︼紝浣嗘槸杩欐牱浼氶檷浣庢晥鐜囥��
        else if (rcv_buffer_->size_of_buffer_ > rcv_buffer_->size_of_use_)
        {

        }
    }

    return 0;
}


void TCP_Svc_Handler::get_max_peer_num(size_t &maxaccept, size_t &maxconnect)
{
    maxaccept = max_accept_svr_;
    maxconnect = max_connect_svr_;
}


//寰楀埌Handle瀵瑰簲PEER鐨処P鍦板潃#绔彛淇℃伅
const ZCE_Sockaddr_In & TCP_Svc_Handler::get_peer()
{
    return peer_address_;
}

void TCP_Svc_Handler::dump_status_staticinfo(ZCE_LOG_PRIORITY out_lvl)
{
    ZCE_LOG(out_lvl, "Dump TCP_Svc_Handler Static Info:");
    ZCE_LOG(out_lvl, "max_accept_svr_=%lu", max_accept_svr_);
    ZCE_LOG(out_lvl, "max_connect_svr_=%lu", max_connect_svr_);
    ZCE_LOG(out_lvl, "if_proxy_= %s", if_proxy_ ? "True" : "False");
    ZCE_LOG(out_lvl, "accepted_timeout_=%u:", accepted_timeout_);
    ZCE_LOG(out_lvl, "receive_timeout_=%u", receive_timeout_);
    ZCE_LOG(out_lvl, "num_accept_peer_=%u", num_accept_peer_);
    ZCE_LOG(out_lvl, "num_connect_peer_=%lu", num_connect_peer_);
    ZCE_LOG(out_lvl, "NUM CONNECT PEER=%lu", num_connect_peer_);
}



//
void TCP_Svc_Handler::dump_status_info(ZCE_LOG_PRIORITY out_lvl)
{
    const size_t OUT_BUF_LEN = 64;
    char out_buf[OUT_BUF_LEN+1];
    out_buf[OUT_BUF_LEN] = '\0';
    size_t use_buffer = 0;
    ZCE_LOG(out_lvl, "my_svc_id_=[%hu.%u]", my_svc_id_.services_type_, my_svc_id_.services_id_);
    ZCE_LOG(out_lvl, "peer_svr_id_=[%hu.%u]", peer_svr_id_.services_type_, peer_svr_id_.services_id_);
    ZCE_LOG(out_lvl, "peer_address_=%s", peer_address_.to_string(out_buf, OUT_BUF_LEN - 1,use_buffer));
    ZCE_LOG(out_lvl, "peer_status_=%d", peer_status_);
#if defined (ZCE_OS_WINDOWS)
    ZCE_LOG(out_lvl, "get_handle=%p", get_handle());
#elif defined (ZCE_OS_LINUX)
    ZCE_LOG(out_lvl, "get_handle=%d", get_handle());
#endif
    ZCE_LOG(out_lvl, "recieve_bytes_ =%lu,rcv_buffer_ =%d", recieve_bytes_, ((rcv_buffer_ != NULL) ? 1 : 0));
    ZCE_LOG(out_lvl, "send_bytes_=%lu snd_buffer_deque_.size=%lu", send_bytes_, snd_buffer_deque_.size());
}

//Dump 鎵�鏈夌殑PEER淇℃伅
void TCP_Svc_Handler::dump_svcpeer_info(ZCE_LOG_PRIORITY out_lvl)
{
    ZCE_LOG(out_lvl, "Services Peer Size =%lu", svr_peer_info_set_.get_services_peersize());
    svr_peer_info_set_.dump_svr_peerinfo(out_lvl);
}

//鍏抽棴鐩稿簲鐨勮繛鎺�
int TCP_Svc_Handler::close_services_peer(const SERVICES_ID &svr_info)
{
    int ret = 0;
    TCP_Svc_Handler *svchanle = NULL;
    ret = svr_peer_info_set_.find_handle_by_svcid(svr_info, svchanle);

    //濡傛灉鏄閲嶆柊杩涜杩炴帴鐨勬湇鍔″櫒涓诲姩涓诲姩杩炴帴,
    if (ret != 0)
    {
        return ret;
    }

    svchanle->handle_close();
    return 0;
}

//鏍规嵁鏈夌殑SVR INFO锛屾煡璇㈢浉搴旂殑HDL
int TCP_Svc_Handler::find_services_peer(const SERVICES_ID &svc_id, TCP_Svc_Handler *&svchanle)
{
    int ret = 0;
    ret = svr_peer_info_set_.find_handle_by_svcid(svc_id, svchanle);

    //濡傛灉鏄閲嶆柊杩涜杩炴帴鐨勬湇鍔″櫒涓诲姩涓诲姩杩炴帴,
    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

const ZCE_Sockaddr_In &TCP_Svc_Handler::get_peer_sockaddr() const
{
    return peer_address_;
}

///鍙栧緱tptoid_table_id_
size_t TCP_Svc_Handler::get_tptoid_table_id()
{
    return tptoid_table_id_;
}
///璁剧疆tptoid_table_id_
void TCP_Svc_Handler::set_tptoid_table_id(size_t ary_id)
{
    tptoid_table_id_ = ary_id;
}
