/*!
* @copyright  2004-2014  Apache License, Version 2.0 FULLSAIL
* @filename   soar_sndrcv_base.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2011年11月30日
* @brief      对应ZERG的，测试，或者客户端接口，
*             用于将TCP，UDP的发送代码部分做一个整合，搞出一个base类，处理一些共有事情
*
* @details
*
*
*
* @note
*
*/


#ifndef SOARING_LIB_SND_RCV_BASE_H_
#define SOARING_LIB_SND_RCV_BASE_H_

/******************************************************************************************
class SendRecv_Msg_Base base 类
******************************************************************************************/
class SendRecv_Msg_Base
{

public:
    //
    SendRecv_Msg_Base();
    virtual ~SendRecv_Msg_Base();

public:

    //
    /*!
    * @brief      设置相应的SVC INFO,FRAME 长度
    * @param      recv_service  接收的服务器ID
    * @param      send_service  发送的服务器ID
    * @param      proxy_service PROXY的服务器ID
    * @param      frame_len     准备的FRAME长度
    * @note       
    */
    void set_services_id(const soar::SERVICES_ID &recv_service,
                         const soar::SERVICES_ID &send_service,
                         const soar::SERVICES_ID &proxy_service,
                         size_t frame_len = soar::Zerg_Frame::MAX_LEN_OF_APPFRAME);

    //取得收到的事务ID
    void get_recv_transid(unsigned int &trans_id);

    unsigned int get_send_transid();
    unsigned int get_backfill_transid();

    //取得测试的APPFRAME
    soar::Zerg_Frame *get_send_appframe();
    //取得接收的APPFRAME
    soar::Zerg_Frame *get_recv_appframe();

protected:

    //接受者的soar::SERVICES_ID
    soar::SERVICES_ID         msg_recv_service_;
    //发送者的tibetan_send_service_，
    soar::SERVICES_ID         msg_send_service_;
    //代理服务器的信息
    soar::SERVICES_ID         msg_proxy_service_;

    //缓冲区的帧的长度,你可以设置接受数据的大小，如果都是64K,太多可能影响你最后的总容量,但目前感觉问题不大
    size_t                    test_frame_len_;

    //事务ID发生器，就是一个计数器
    unsigned int              trans_id_builder_;

    //为了多线程将原来的test_appframe_改为了2个，一个接受一个发送，这样在多线程处理的情况下，
    //就不会出现共用一个BUFFER的事情

    //发送缓冲区的帧
    soar::Zerg_Frame         *msg_send_frame_;
    //接收缓冲区
    soar::Zerg_Frame         *msg_recv_frame_;

    //收到的事务ID
    unsigned int              recv_trans_id_;

    //回填的事务ID
    unsigned int              backfill_trans_id_;

};

#endif //SOARING_LIB_SND_RCV_BASE_H_

