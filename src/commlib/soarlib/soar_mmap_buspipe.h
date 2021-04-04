
#ifndef SOARING_LIB_ZERG_MMAP_PIPE_H_
#define SOARING_LIB_ZERG_MMAP_PIPE_H_

#include "soar_zerg_frame.h"
#include "soar_error_code.h"
#include "soar_stat_monitor.h"
#include "soar_stat_define.h"
#include "soar_svrd_cfg_fsm.h"

class Zerg_App_Frame;

class SOARING_EXPORT Soar_MMAP_BusPipe : public ZCE_BusPipe_TwoWay
{

public:
    //���캯��,
    Soar_MMAP_BusPipe();
    //��������
    ~Soar_MMAP_BusPipe();

public:

    //��ʼ�����ֲ���,
    int initialize(SERVICES_ID &svrinfo,
                   size_t size_recv_pipe,
                   size_t size_send_pipe,
                   size_t max_frame_len,
                   bool if_restore);



    //����MMAP�������ļ�����
    void get_mmapfile_name(char *mmapfile, size_t buflen);

    //-----------------------------------------------------------------
    //��RECV�ܵ���ȡ֡
    inline int pop_front_recvpipe(Zerg_App_Frame *&proc_frame);
    //��SEND�ܵ�д��֡
    inline int push_back_sendpipe(Zerg_App_Frame *proc_frame);

    //ԭ�������ֶ���send_msg_to��һ��ʼ��Ϊ����ͦ�õģ��������𲽵ĸо������޸��滻��ʱ��ʹ�಻�١�
    //��ȷ�ϸ�Ϊpipe_sendmsg_to_service�ǲ���һ�������⣬��������һ��?

    //����һ����Ϣ����������ָ��������������ַ�ʽ
    template< class T>
    int pipe_sendmsg_to_service(unsigned int cmd,
                                unsigned int qquin,
                                unsigned int transaction_id,
                                unsigned int backfill_trans_id,
                                const SERVICES_ID &rcvsvc,
                                const SERVICES_ID &sendsvc,
                                const T &info,
                                unsigned int app_id = 0,
                                unsigned int option = 0);

    //�������ݵ����������,ָ��������������Լ����ܵķ���������
    template< class T>
    int pipe_sendmsg_to_proxy(unsigned int cmd,
                              unsigned int qquin,
                              unsigned int transaction_id,
                              unsigned int backfill_trans_id,
                              unsigned short rcvtype,
                              const SERVICES_ID &proxysvc,
                              const SERVICES_ID &sendsvc,
                              const T &info,
                              unsigned int app_id = 0,
                              unsigned int option = 0);

    //�������ݵ�������,����ָ������Ĵ�����������Լ����ܵķ�����ID
    template< class T>
    int pipe_sendmsg_to_service(unsigned int cmd,
                                unsigned int qquin,
                                unsigned int transaction_id,
                                unsigned int backfill_trans_id,
                                const SERVICES_ID &rcvsvc,
                                const SERVICES_ID &proxysvc,
                                const SERVICES_ID &sendsvc,
                                const T &info,
                                unsigned int app_id = 0,
                                unsigned int option = 0);

    // ����һ��buf��ָ���ķ�����
    int pipe_sendbuf_to_service(unsigned int cmd,
                                unsigned int qquin,
                                unsigned int transaction_id,
                                unsigned int backfill_trans_id,
                                const SERVICES_ID &rcvsvc,
                                const SERVICES_ID &proxysvc,
                                const SERVICES_ID &sendsvc,
                                const unsigned char *buf,
                                size_t buf_len,
                                unsigned int app_id = 0,
                                unsigned int option = 0);

    //-----------------------------------------------------------------
public:

    //Ϊ��SingleTon��׼��
    //ʵ���ĸ�ֵ
    static void instance(Soar_MMAP_BusPipe *);
    //ʵ���Ļ��
    static Soar_MMAP_BusPipe *instance();
    //���ʵ��
    static void clean_instance();

protected:

    //ZERG�ܵ�������,�����������Ϊ��Ҷ���
    static const char    ZERG_PIPENAME[NUM_OF_PIPE][16];

protected:
    //instance����ʹ�õĶ���
    static Soar_MMAP_BusPipe *zerg_bus_instance_;

protected:

    ///�����������������Ϣ.
    SERVICES_ID         zerg_svr_info_ = SERVICES_ID(0, 0);
    ///���͵Ļ�����
    static char         send_buffer_[Zerg_App_Frame::MAX_LEN_OF_APPFRAME];

    ///��ض���
    Soar_Stat_Monitor   *monitor_ = NULL;

};

//��RECV�ܵ���ȡ֡��
inline int Soar_MMAP_BusPipe::pop_front_recvpipe(Zerg_App_Frame *&proc_frame)
{
    int ret = pop_front_bus(RECV_PIPE_ID, reinterpret_cast<zce::lockfree::dequechunk_node *&>(proc_frame));

    // �Ӽ������
    if (ret == 0)
    {
        monitor_->increase_once(COMM_STAT_RECV_PKG_COUNT,
                                proc_frame->app_id_,
                                proc_frame->frame_command_);
        monitor_->increase_by_statid(COMM_STAT_RECV_PKG_BYTES,
                                     proc_frame->app_id_,
                                     proc_frame->frame_command_,
                                     proc_frame->frame_length_);


        //����Ǹ�����������ݰ���ӡ��������ǳ���ʱ������
        if (proc_frame->frame_option_ & Zerg_App_Frame::DESC_MONITOR_TRACK)
        {
            Zerg_App_Frame::dumpoutput_frameinfo(RS_ERROR, "[TRACK MONITOR][Recv]", proc_frame);
        }
    }

    return ret;
}

//��SEND�ܵ�д��֡��
inline int Soar_MMAP_BusPipe::push_back_sendpipe(Zerg_App_Frame *proc_frame)
{
    DEBUGDUMP_FRAME_HEAD_DBG(RS_DEBUG, "TO SEND PIPE FRAME:", proc_frame);

    if (proc_frame->frame_length_ > Zerg_App_Frame::MAX_LEN_OF_APPFRAME || proc_frame->frame_length_ < Zerg_App_Frame::LEN_OF_APPFRAME_HEAD)
    {
        ZCE_LOG(RS_ERROR, "[framework] Frame Len is error ,frame length :%u ,Please check your code.",
                proc_frame->frame_length_);
        return SOAR_RET::ERROR_FRAME_DATA_IS_ERROR;
    }

    //����Ǹ�����������ݰ���ӡ��������ǳ���ʱ������
    if (proc_frame->frame_option_ & Zerg_App_Frame::DESC_MONITOR_TRACK)
    {
        Zerg_App_Frame::dumpoutput_frameinfo(RS_ERROR, "[TRACK MONITOR][Send]", proc_frame);
    }

    int ret = push_back_bus(SEND_PIPE_ID,
                            reinterpret_cast<const zce::lockfree::dequechunk_node *>(proc_frame));

    if (ret != 0)
    {
        // ����ʧ��, �ܵ�����
        monitor_->increase_once(COMM_STAT_SEND_PKG_FAIL,
                                proc_frame->app_id_,
                                proc_frame->frame_command_);
        return SOAR_RET::ERROR_PIPE_IS_FULL;
    }
    else
    {
        monitor_->increase_once(COMM_STAT_SEND_PKG_SUCC,
                                proc_frame->app_id_,
                                proc_frame->frame_command_);
        monitor_->increase_by_statid(COMM_STAT_SEND_PKG_BYTES,
                                     proc_frame->app_id_,
                                     proc_frame->frame_command_,
                                     proc_frame->frame_length_);
    }

    // ���سɹ�
    return 0;
}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2007��3��24��
Function        : PipeRequstMsgToService
Return          : int
Parameter List  :
  Param1: unsigned int cmd  ���͵�����
  Param2: unsigned int qquin ���͵�UIN
  Param3: unsigned int transaction_id,����ID
  Param4: unsigned int backfill_trans_id,���������ID
  Param5: const SERVICES_ID& rcvsvc  ��������˭
  Param6: const SERVICES_ID& sendsvc ��������˭
  Param7: const T& info ���͵�����
  Param8: unsigned int option=0 ���͵�ѡ��
Description     : �ƶ�һ�����ܷ�������������,������PROXY������
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
template< class T>
int Soar_MMAP_BusPipe::pipe_sendmsg_to_service(unsigned int cmd,
                                               unsigned int qquin,
                                               unsigned int transaction_id,
                                               unsigned int backfill_trans_id,
                                               const SERVICES_ID &rcvsvc,
                                               const SERVICES_ID &sendsvc,
                                               const T &info,
                                               unsigned int app_id,
                                               unsigned int option)
{
    const SERVICES_ID proxysvc(0, 0);
    return pipe_sendmsg_to_service(cmd,
                                   qquin,
                                   transaction_id,
                                   backfill_trans_id,
                                   rcvsvc,
                                   proxysvc,
                                   sendsvc,
                                   info,
                                   app_id,
                                   option);
}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2007��3��24��
Function        : pipe_sendmsg_to_proxy
Return          : template< class T> int
Parameter List  :
  Param1: unsigned int cmd   ���͵�����
  Param2: unsigned int qquin ���͵�UIN
  Param3: unsigned int transaction_id,����ID
  Param4: unsigned int backfill_trans_id,���������ID
  Param5: unsigned short rcvtype ���յĽ����ߵ�������ʲô
  Param7: const SERVICES_ID& proxysvc PROXY��������˭
  Param6: const SERVICES_ID& sendsvc  ��������˭
  Param8: const T& info ���͵�����
  Param9: unsigned int option=0 ���͵�ѡ��
Description     : �ƶ�һ��PROXY���������н�������
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
template< class T>
int Soar_MMAP_BusPipe::pipe_sendmsg_to_proxy(unsigned int cmd,
                                             unsigned int qquin,
                                             unsigned int transaction_id,
                                             unsigned int backfill_trans_id,
                                             unsigned short rcvtype,
                                             const SERVICES_ID &proxysvc,
                                             const SERVICES_ID &sendsvc,
                                             const T &info,
                                             unsigned int app_id,
                                             unsigned int option)
{
    const SERVICES_ID rcvsvc(rcvtype, 0);
    return pipe_sendmsg_to_service(cmd,
                                   qquin,
                                   transaction_id,
                                   backfill_trans_id,
                                   rcvsvc,
                                   proxysvc,
                                   sendsvc,
                                   info,
                                   app_id,
                                   option);
}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2007��3��24��
Function        : pipe_sendmsg_to_service
Return          : template< class T>
Parameter List  :
  Param1: unsigned int cmd   ���͵�����
  Param2: unsigned int qquin ���͵�UIN
  Param3: unsigned int transaction_id,����ID,û��Ҫ��Է����ؾͲ�����д����д0
  Param4: unsigned int backfill_trans_id,���������ID,û��Ҫ��Է����ؾͲ�����д����д0
  Param5: const SERVICES_ID& rcvsvc ���յĽ����ߵ�������˭
  Param6: const SERVICES_ID& proxysvc PROXY��������˭
  Param7: const SERVICES_ID& sendsvc ��������˭
  Param8: const T& info ���͵�����
  Param9: unsigned int option=0 ���͵�ѡ��
Description     : ��Ӳ����ķ��ͺ�����
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
template< class T>
int Soar_MMAP_BusPipe::pipe_sendmsg_to_service(unsigned int cmd,
                                               unsigned int qquin,
                                               unsigned int transaction_id,
                                               unsigned int backfill_trans_id,
                                               const SERVICES_ID &rcvsvc,
                                               const SERVICES_ID &proxysvc,
                                               const SERVICES_ID &sendsvc,
                                               const T &info,
                                               unsigned int app_id,
                                               unsigned int option)
{
    int ret = 0;

    Zerg_App_Frame *send_frame = reinterpret_cast<Zerg_App_Frame *>(send_buffer_);

    send_frame->init_framehead(Zerg_App_Frame::MAX_LEN_OF_APPFRAME, option, cmd);
    send_frame->frame_uid_ = qquin;
    send_frame->app_id_ = app_id;

    send_frame->send_service_ = sendsvc;
    send_frame->proxy_service_ = proxysvc;
    send_frame->recv_service_ = rcvsvc;

    //��д����ID�ͻ�������ID
    send_frame->transaction_id_ = transaction_id;
    send_frame->backfill_trans_id_ = backfill_trans_id;

    ret = send_frame->appdata_encode(Zerg_App_Frame::MAX_LEN_OF_APPFRAME_DATA, info);

    if (ret != 0 )
    {
        ZCE_LOG(RS_ERROR, "[framework] [%s]TDR encode fail.ret =%d,Please check your code and buffer len.",
                __ZCE_FUNC__,
                ret);
        return ret;
    }

    //
    return push_back_sendpipe(send_frame);

}

#endif //SOARING_LIB_ZERG_MMAP_PIPE_H_

