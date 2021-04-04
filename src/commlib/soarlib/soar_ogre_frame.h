
#ifndef SOARING_LIB_OGRE_SERVER_INTERFACE_HEAD_H_
#define SOARING_LIB_OGRE_SERVER_INTERFACE_HEAD_H_

#include "soar_ogre_peer_id.h"

/******************************************************************************************
struct  Ogre4a_App_Frame OGRE ���������ڲ�����֡ͷ
******************************************************************************************/
#pragma pack (1)

class Ogre4a_App_Frame
{


public:

    //֡������,��m_Option�ֶ�ʹ��
    enum OGRE_APPFRAME_OPTION
    {

        //����PEERΪUDP
        OGREDESC_PEER_UDP = 0x1,
        //����PEERΪTCP
        OGREDESC_PEER_TCP = 0x2,

        //PEER���ر�
        OGREDESC_PEER_CLOSED = 0x4,
        //ǿ�ƹر�PEER,���ܷ��Ͷ����м��Ƿ�������,ע���DESC_SNDPRC_CLOSE_PEER������
        OGREDESC_CLOSE_PEER = 0x8,

        //ĳ������֡����ʧ��,֪ͨ����ķ���
        OGREDESC_SEND_ERROR = 0x10,
        //�������ʧ��,��¼����ʧ�ܵ�����
        OGREDESC_SEND_FAIL_RECORD = 0x20,
        //�������ʧ��,֪ͨ�����Ӧ�ý���
        OGREDESC_SNDPRC_NOTIFY_APP = 0x40,
        //������ͳɹ���,ֱ�ӶϿ����ӣ����ڲ���TCP�Ķ�����
        OGREDESC_SNDPRC_CLOSE_PEER = 0x80,
    };

public:

    //
    Ogre4a_App_Frame();
protected:
    //
    ~Ogre4a_App_Frame();

public:
    //����FRAME DATA������
    inline void reset_framehead();
    //�������
    inline void fill_write_data(const uint32_t size_data, const char *src_data);
    //��ȡ����
    inline void get_data(uint32_t &size_data, char *dest_data) const;

    //����DATA����
    int add_data(uint32_t add_size, char *add_data);

    //�����Լ�Rcv ,Snd Peer Info
    void exchange_rcvsnd_peerInfo();
    //�������˽���Rcv ,Snd Peer Info,
    void exchange_rcvsnd_peerInfo(const Ogre4a_App_Frame *exframe );

    //DUMP OGRE FRAME��ͷ����Ϣ
    void dump_ogre_framehead(const char *outstr,
                             ZCE_LOG_PRIORITY log_priority) const;

public:

    //��placement new��delete�ˡ�����Ȱ�Ա�����VS���Ǳ仯���������Ҫ����

    ///����һ��Frame
    static Ogre4a_App_Frame *new_ogre(std::size_t lenframe);
    ///����һ��frame
    static void delete_ogre(Ogre4a_App_Frame *frame) noexcept;

public:

    //��������DATA����������,
    static void set_max_framedata_len(unsigned int  max_framedata);

    //DUMP OGRE FRAME��ͷ����Ϣ
    static void dump_ogre_framehead(const Ogre4a_App_Frame *proc_frame,
                                    const char *outstr,
                                    ZCE_LOG_PRIORITY log_priority);


public:

    //FRAME��ͷ���ߴ�,
    static const size_t  LEN_OF_OGRE_FRAME_HEAD = 20;

    //����DATA����������,��������
    static size_t        MAX_OF_OGRE_DATA_LEN;
    //����FRAME�ĳ���,ΪMAX_OF_OGRE_DATA_LEN �� LEN_OF_OGRE_FRAME_HEAD
    static size_t        MAX_OF_OGRE_FRAME_LEN;



public:
    //֡�ĳ��ȣ�����֡ͷ�Ľṹ
    uint32_t ogre_frame_len_;

    //�Զ���Ϣ
    OGRE_PEER_ID snd_peer_info_;
    //�Զ���Ϣ
    OGRE_PEER_ID rcv_peer_info_;

    //֡��ѡ���
    unsigned int ogre_frame_option_ : 28;
    //�ٺ�4BIT��Ϊ���ʹ������
    unsigned int ogre_send_errnum_ : 4;

    //Appdata ��һ���䳤�ȵ��ַ������б�ʾ,
#ifdef ZCE_OS_WINDOWS
#pragma warning ( disable : 4200)
#endif
    char  frame_data_[];
#ifdef ZCE_OS_WINDOWS
#pragma warning ( default : 4200)
#endif

};

#pragma pack ()

//---------------------------------------------------------------------------------------------
//��ӡ���ͷ����Ϣ�Ŀ��ƺ�
#if defined _DEBUG || defined DEBUG
#define DEBUGDUMP_OGRE_HEAD(x,y,z)      Ogre4a_App_Frame::dump_ogre_framehead(x,y,z);
#else
#define DEBUGDUMP_OGRE_HEAD(x,y,z)
#endif

//---------------------------------------------------------------------------------------------
//��������

//����FRAME DATA������
inline void Ogre4a_App_Frame::reset_framehead()
{
    ogre_frame_len_ = LEN_OF_OGRE_FRAME_HEAD;
    ogre_frame_option_ = 0;
    ogre_send_errnum_ = 0;
}

//�������
inline void Ogre4a_App_Frame::fill_write_data(const unsigned int size_data, const char *src_data)
{
    memcpy(frame_data_ + ogre_frame_len_ - LEN_OF_OGRE_FRAME_HEAD, src_data, size_data);
    ogre_frame_len_ += size_data;
}

//��ȡ����
inline void Ogre4a_App_Frame::get_data(unsigned int &size_data, char *dest_data) const
{
    size_data = ogre_frame_len_ - LEN_OF_OGRE_FRAME_HEAD;
    memcpy(dest_data, frame_data_ + LEN_OF_OGRE_FRAME_HEAD, size_data);
}




/******************************************************************************************
�ӿڶ���
******************************************************************************************/
//
//��.so���غ�������
static const char STR_JUDGE_RECV_WHOLEFRAME[] = "JudgeRecvWholeFrame";
//
typedef int ( *FP_JudgeRecv_WholeFrame)(const char *recv_buf,
                                        unsigned int buf_len,
                                        unsigned int max_buf_len,
                                        bool &if_whole,
                                        unsigned int &frame_len);

#endif //#ifndef SOARING_LIB_OGRE_SERVER_INTERFACE_HEAD_H_

