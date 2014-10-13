
#ifndef SOARING_LIB_OGRE_SERVER_INTERFACE_HEAD_H_
#define SOARING_LIB_OGRE_SERVER_INTERFACE_HEAD_H_

#include "soar_ogre_svc_peer.h"

/******************************************************************************************
struct  Ogre4a_App_Frame OGRE 服务器的内部命令帧头
******************************************************************************************/
#pragma pack (1)

class Ogre4a_App_Frame
{
public:
    //FRAME包头都尺寸,
    static const size_t  LEN_OF_OGRE_FRAME_HEAD = 20;

    //最大的DATA数据区长度,可以配置
    static size_t        MAX_OF_OGRE_DATA_LEN;
    //最大的FRAME的长度,为MAX_OF_OGRE_DATA_LEN ＋ LEN_OF_OGRE_FRAME_HEAD
    static size_t        MAX_OF_OGRE_FRAME_LEN;

    //帧的描述,在m_Option字段使用
    enum OGRE_APPFRAME_OPTION
    {

        //描述PEER为UDP
        OGREDESC_PEER_UDP            = 0x1,
        //描述PEER为TCP
        OGREDESC_PEER_TCP            = 0x2,

        //PEER被关闭
        OGREDESC_PEER_CLOSED         = 0x4,
        //强制关闭PEER,不管发送队列中间是否有数据,注意和DESC_SNDPRC_CLOSE_PEER的区别
        OGREDESC_CLOSE_PEER          = 0x8,

        //某个命令帧发送失败,通知后面的服务
        OGREDESC_SEND_ERROR          = 0x10,
        //如果发送失败,记录发送失败的事情
        OGREDESC_SEND_FAIL_RECORD     = 0x20,
        //如果发送失败,通知后面的应用进程
        OGREDESC_SNDPRC_NOTIFY_APP   = 0x40,
        //如果发送成功后,直接断开连接，用于部分TCP的短连接
        OGREDESC_SNDPRC_CLOSE_PEER   = 0x80,
    };

public:
    //帧的长度，包括帧头的结构
    unsigned int                   ogre_frame_len_;

    //对端信息
    SOCKET_PERR_ID               snd_peer_info_;
    //对端信息
    SOCKET_PERR_ID               rcv_peer_info_;

    //帧的选项，见
    unsigned int                   ogre_frame_option_: 28;
    //再后4BIT作为发送错误计数
    unsigned int                   ogre_send_errnum_: 4;

    //Appdata 是一个变长度的字符串序列标示,
#ifdef ZCE_OS_WINDOWS
#pragma warning ( disable : 4200)
#endif
    char                           frame_data_[];
#ifdef ZCE_OS_WINDOWS
#pragma warning ( default : 4200)
#endif

public:

    //
    Ogre4a_App_Frame();
    //
    Ogre4a_App_Frame(const SOCKET_PERR_ID);
    //
    ~Ogre4a_App_Frame();

public:
    //

public:
    //重置FRAME DATA的数据
    inline void ResetFrameHead();
    //填充数据
    inline void fill_write_data(const unsigned int size_data, const char *src_data);
    //读取数据
    inline void GetReadData(unsigned int &size_data, char *dest_data);
    //得到使用点的指针
    inline char *GetUsePoint();
    //增加FRAME的长度
    inline void AddFrameLen(unsigned int add_frame_len);

    //交换自己Rcv ,Snd Peer Info
    void ExchangeRcvSndPeerInfo();
    //和其他人交换Rcv ,Snd Peer Info,
    void ExchangeRcvSndPeerInfo(const Ogre4a_App_Frame *exframe );

    //DUMP OGRE FRAME的头部信息
    void DumOgreFrameHead(const char *outstr,
                          ZCE_LOG_PRIORITY log_priority) const;

public:
    //重载New函数
    static void   *operator new (size_t , size_t lenframe);
    //不重载delte与情理不通，但是其实没有什么问题,
    static void operator delete(void *ptrframe, size_t );

public:

    //配置最大的DATA数据区长度,
    static void SetMaxFrameDataLen(unsigned int  max_framedata);
    //DUMP OGRE FRAME的头部信息
    static void DumOgreFrameHead(const Ogre4a_App_Frame *proc_frame,
                                 const char *outstr,
                                 ZCE_LOG_PRIORITY log_priority);

};

#pragma pack ()

//---------------------------------------------------------------------------------------------
//打印输出头部信息的控制宏
#if defined _DEBUG || defined DEBUG
#define DEBUGDUMP_OGRE_HEAD(x,y,z)      Ogre4a_App_Frame::DumOgreFrameHead(x,y,z);
#else
#define DEBUGDUMP_OGRE_HEAD(x,y,z)
#endif

//---------------------------------------------------------------------------------------------
//内联函数

//重置FRAME DATA的数据
inline void Ogre4a_App_Frame::ResetFrameHead()
{
    ogre_frame_len_ = LEN_OF_OGRE_FRAME_HEAD;
    ogre_frame_option_ = 0;
    ogre_send_errnum_ = 0;
}
//填充数据
inline void Ogre4a_App_Frame::fill_write_data(const unsigned int size_data, const char *src_data)
{
    memcpy(frame_data_ + ogre_frame_len_ - LEN_OF_OGRE_FRAME_HEAD, src_data, size_data);
    ogre_frame_len_ += size_data;
}
//读取数据
inline void Ogre4a_App_Frame::GetReadData(unsigned int &size_data, char *dest_data)
{
    size_data = ogre_frame_len_ - LEN_OF_OGRE_FRAME_HEAD;
    memcpy(dest_data, frame_data_ + LEN_OF_OGRE_FRAME_HEAD, size_data);
}
//得到使用点的指针
inline char *Ogre4a_App_Frame::GetUsePoint()
{
    return frame_data_ + ogre_frame_len_ - LEN_OF_OGRE_FRAME_HEAD;
}
//增加FRAME的长度
inline void Ogre4a_App_Frame::AddFrameLen(unsigned int add_frame_len)
{
    ogre_frame_len_ += add_frame_len;
}

/******************************************************************************************
接口定义
******************************************************************************************/
//
//从.so加载函数名称
static const char STR_JUDGE_RECV_WHOLEFRAME[] = "JudgeRecvWholeFrame";
//
typedef int ( *FP_JudgeRecv_WholeFrame)(const char *recv_buf,
                                      unsigned int buf_len,
                                      unsigned int max_buf_len,
                                      bool &if_whole,
                                      unsigned int &frame_len);

#endif //#ifndef SOARING_LIB_OGRE_SERVER_INTERFACE_HEAD_H_

