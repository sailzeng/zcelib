
#ifndef SOARING_LIB_SERVER_APP_FRAME_H_
#define SOARING_LIB_SERVER_APP_FRAME_H_

#include "soar_enum_define.h"
#include "soar_services_info.h"
#include "soar_frame_command.h"
#include "soar_error_code.h"

#pragma pack (1)


/*!
* @brief      FRAME的选项字节的UNION结构
*
*/
struct _ZERG_FRAME_OPTION
{
public:

    //frame_option_的头24个BIT作为选项字段,,
    uint32_t     inner_option_ : 24;

    //后4BIT作为版本标识
    uint32_t     frame_version_ : 8;
};



/*!
* @brief      Zerg服务器间传递消息的通用帧头
*
*/
class SOARING_EXPORT Zerg_App_Frame
{
public:

    //帧的描述,在frame_option_字段使用
    enum PET_APPFRAME_OPTION
    {
        //-----------------------------------------------------------------
        //低16位用于内部的描述

        //
        DESC_DEFUALT             = 0x0,

        //高优先级别，没有使用，
        DESC_HIGH_PRIORITY       = 0x1,

        //某个命令帧发送失败,通知后面的服务
        DESC_SEND_ERROR          = 0x2,

        //如果发送失败,重复尝试发送
        DESC_SEND_FAIL_RECORD    = 0x4,
        //如果发送失败,通知后面的应用进程
        DESC_SNDPRC_NOTIFY_APP   = 0x8,

        //如果发送成功后,直接断开连接，用于部分TCP的短连接
        DESC_SNDPRC_CLOSE_PEER   = 0x10,

        //在包体追加登录KEY
        DESC_APPEND_SESSION_KEY  = 0x20,

        //在包体追加登录ip
        DESC_APPEND_LOGIN_IP     = 0x40,

        //默认的通讯帧都是TCP的，这个描述字表示这个帧是UDP的
        DESC_UDP_FRAME           = 0x80,

        //如果是TCP的帧,其实默认是TCP的帧,所以其实没有使用
        DESC_TCP_FRAME           = 0x100,

        //-----------------------------------------------------------------
        //高16位用于外部的描述,8位描述，
        DESC_MONITOR_TRACK       = 0x10000,

        //FRAME数据区的有用户签名
        DESC_HEAD_WITH_SIGNATURE = 0x20000,

        //帧的数据采用加密
        DESC_SESSION_ENCRYPT     = 0x40000,

        //特殊的某些命令不用加密进行处理，用于加密情况某些命令无须加密的特殊情况
        DESC_SPECIAL_NO_ENCRYPT  = 0x80000,

        //APPFram的版本V1
        DESC_V1_VERSION          = 0x1000000,
        //APPFram的版本V2，终于还是升级了一次
        DESC_V2_VERSION          = 0x2000000,
    };

    //内部选项描述的掩码,内部选项在网络间看不见，在通讯服务器和业务服务器前可见。
    static const uint32_t INNER_OPTION_MASK = 0xFFFF;
    //外部选项描述+版本号的掩码
    static const uint32_t OUTER_OPTION_MASK = 0xFFFF0000;

public:

    //包头都尺寸,
    static const size_t LEN_OF_APPFRAME_HEAD = 50;

    //APPFAME版本V1
    static const unsigned char TSS_APPFRAME_V1 = 1;

    //---------------------------------------------------------------------------
    //FRAME的一些长度参数,

    //默认的最大长度是64K
    //为什么采用64K的原因是我们的UPD的最大长度是这个，而且这个缓冲区的长度比较适中.
    static const size_t MAX_LEN_OF_APPFRAME  = 64 * 1024;


    //TEA加密后增加的长度,UPD的数据区麻烦自己搞掂长度限制等问题
    static const size_t LEN_OF_TEA_REMAIN_ROOM = 17;

    //FRAME的最大长度,根据各个地方的长度而得到
    static const size_t MAX_LEN_OF_APPFRAME_DATA = MAX_LEN_OF_APPFRAME - LEN_OF_APPFRAME_HEAD - LEN_OF_TEA_REMAIN_ROOM;

    //
    static const size_t MAX_LEN_OF_TEA_APPDATA = MAX_LEN_OF_APPFRAME_DATA + LEN_OF_TEA_REMAIN_ROOM;

public:

    //整个通讯包长度,留足空间,包括帧头的长度.
    uint32_t               frame_length_;

    union
    {
        uint32_t           frame_option_;
        //
        _ZERG_FRAME_OPTION inner_option_;
    };

    //命令字 命令字还是放在包头比较好,
    uint32_t               frame_command_;

    //UID
    uint32_t               frame_uid_;

    //发送和接收的服务器应用也要填写

    //发送服务,包括发送服务器类型，发送服务器编号,没有编号，或者不是服务填写0
    SERVICES_ID            send_service_;
    //接受服务器
    SERVICES_ID            recv_service_;
    //代理服务器
    SERVICES_ID            proxy_service_;

    //事务ID,可以用作服务发起端作为一个标示，后面的服务器回填backfill_trans_id_字段返回,
    uint32_t               transaction_id_;
    //回填的请求者的事务ID,
    uint32_t               backfill_trans_id_;

    //业务ID，GAMEID，用于标识游戏内部ID
    uint32_t               app_id_;

    union
    {
        //发送序列号，计划只在通讯层用,暂时没用用
        uint32_t           send_serial_number_;
        //发送者的IP地址，内部使用
        uint32_t           send_ip_address_;

    };

    //frame_appdata_ 是一个变长度的字符串序列标示,
#ifdef ZCE_OS_WINDOWS
#pragma warning ( disable : 4200)
#endif
    char                 frame_appdata_[];
#ifdef ZCE_OS_WINDOWS
#pragma warning ( default : 4200)
#endif

public:
    //构造函数，大家都可以用的.
    Zerg_App_Frame(uint32_t cmd = CMD_INVALID_CMD,
                   uint32_t lenframe = LEN_OF_APPFRAME_HEAD,
                   uint32_t frameoption = DESC_V1_VERSION);

    //构造函数,用于,客户端初始化,
    Zerg_App_Frame(uint32_t cmd,
                   uint32_t lenframe,
                   uint32_t uin,
                   uint16_t sndsvrtype,
                   uint16_t rcvsvctype,
                   uint32_t frameoption = DESC_V1_VERSION);

    //构造函数,用于发送给一个非代理服务器
    Zerg_App_Frame(uint32_t cmd,
                   uint32_t lenframe,
                   uint32_t uin,
                   const SERVICES_ID &sndsvr,
                   const SERVICES_ID &rcvsvc,
                   uint32_t frameoption = DESC_V1_VERSION);

    //构造函数,用于发送给一个代理服务器
    Zerg_App_Frame(uint32_t cmd,
                   uint32_t lenframe,
                   uint32_t uin,
                   const SERVICES_ID &sndsvr,
                   const SERVICES_ID &proxy,
                   uint16_t rcvsvc,
                   uint32_t frameoption = DESC_V1_VERSION);

    //析构函数
    ~Zerg_App_Frame();

    //Assign =运算符号
    Zerg_App_Frame &operator = (const Zerg_App_Frame &other);

    //是否是内部处理的命令
    inline bool is_internal_process(bool &bsenderr);
    //是否事通信服务器处理的命理
    inline bool is_zerg_processcmd();

    //清理内部的选项信息
    inline void clear_inner_option();
    //清理所有的选项心想
    inline void clear_all_option();

    //将帧头的所有的uint16_t,uint32_t转换为网络序
    void framehead_encode();
    //将帧头的所有的uint16_t,uint32_t转换为本地序
    void framehead_decode();

    //初始化V1版本的包头,所有数据清0
    void init_framehead(uint32_t lenframe,
                        uint32_t option = 0,
                        uint32_t cmd = 0);

    //填充发送SVR信息
    void set_send_svcid(uint16_t svrtype, uint32_t svrid);
    //填充发送SVR信息
    void set_recv_svcid(uint16_t svrtype, uint32_t svrid);
    //填充代理SVR信息
    void set_proxy_svcid(uint16_t svrtype, uint32_t svrid);

    //填写所有的服务信息,
    void set_all_svcid(const SERVICES_ID &rcvinfo, const SERVICES_ID &sndinfo, const SERVICES_ID &proxyinfo);

    //填充AppData数据到APPFrame
    int fill_appdata(const size_t szdata, const char *vardata);

    //交换Rcv ,Snd SvrInfo
    void exchange_rcvsnd_svcid();
    //交换Rcv ,Snd SvrInfo,prochandle
    void exchange_rcvsnd_svcid(Zerg_App_Frame &exframe );
    //回填返回包头
    void fillback_appframe_head(Zerg_App_Frame &exframe );

    //Clone一个APP FRAME
    Zerg_App_Frame *clone() const;
    //
    void clone(Zerg_App_Frame *clone_frame) const;
    //
    void clone_head(Zerg_App_Frame *clone_frame) const;

    //Dump头部和DATA区的数据
    void dump_appframe_info(std::ostringstream &strstream) const;
    //Dump所有的数据信息,一个字节字节的告诉你,
    void dump_appframe_data(std::ostringstream &strstream) const;
    //Dump数据包头中的重要信息给你,
    void dump_appframe_head(std::ostringstream &strstream) const;

    //输出APPFRAME的头部信息
    void dumpoutput_framehead(const char *outstr,
                              ZCE_LOG_PRIORITY log_priority) const;
    //输出APPFRAME的尾部信息
    void dumpoutput_frameinfo(const char *outstr,
                              ZCE_LOG_PRIORITY log_priority)  const;

    //取得帧的长度
    inline size_t get_appframe_len() const;
    //取得帧数据的长度
    inline size_t get_frame_datalen() const;

    //TDR AppCode编码,szframe_appdata为frame_appdata_的buffer长度,如果frame_appdata_为一个缓冲,使用此函数
    template<class T> int appdata_encode(
        size_t szframe_appdata,
        const T &info,
        size_t data_start = 0,
        size_t *sz_code = NULL);

    //TDR AppCode解码,
    template<class T> int appdata_decode(
        T &info,
        size_t data_start = 0,
        size_t *sz_code = NULL) const;


#if defined ZCE_USE_PROTOBUF && ZCE_USE_PROTOBUF == 1

    ///将一个结构进行编码
    int protobuf_encode(size_t szframe_appdata,
                        const google::protobuf::MessageLite *msg,
                        size_t data_start = 0,
                        size_t *sz_code = NULL
                       );

    ///将一个结构进行解码
    int protobuf_decode(google::protobuf::MessageLite *msg,
                        size_t data_start = 0,
                        size_t *sz_code = NULL);

#endif


    //拷贝Output CDR中的Msg,Block数据,szframe_appdata为frame_appdata_的buffer长度,如果frame_appdata_为一个缓冲,使用此函数
    //ssize_t CopyCDRMsgBlock(size_t szframe_appdata,const ACE_OutputCDR& outcdr );
    //如果AppFrame为一个恰好长度的Frame,长度已经填写,使用此函数,
    //ssize_t CopyCDRMsgBlock(const ACE_OutputCDR& outcdr);

    //取得IP地址信息
    uint32_t get_send_ip() const;

    //-----------------------------------------------------------------------------------
    //FRAME的数据进行TEA算法加密解密的函数，不知道Jovi当年为啥要写成STATIC的，呵呵
    //APPDATA加密数据
    int appframe_encrypt(const char *session_key,
                         size_t data_start = 0);
    //APPDATA数据解密,
    int appframe_decrypt(const char *session_key,
                         size_t data_start = 0);

    //将APPDATA加密数据,加密数据保存到另外Zerg_App_Frame，
    int appframe_encrypt(const char *session_key,
                         Zerg_App_Frame *dest_frame,
                         size_t data_start = 0);
    //将APPDATA数据解密,解密数据保存在另外一个Zerg_App_Frame，
    int appframe_decrypt(const char *session_key,
                         Zerg_App_Frame *dest_frame,
                         size_t data_start = 0);

public:


    //重载New函数
    static void   *operator new (size_t , size_t lenframe = LEN_OF_APPFRAME_HEAD);
    //不重载delte与情理不通，但是其实没有什么问题,
    static void operator delete(void *ptrframe, size_t );


    //输出APPFRAME的头部信息
    static void dumpoutput_framehead(ZCE_LOG_PRIORITY log_priority,
                                     const char *outstr,
                                     const Zerg_App_Frame *frame);
    //输出APPFRAME的尾部信息
    static void dumpoutput_frameinfo(ZCE_LOG_PRIORITY log_priority,
                                     const char *outstr,
                                     const Zerg_App_Frame *frame);

    //--------------------------------------------------------------------------
    //FRAME的数据进行TEA算法加密解密的函数，STATIC函数，不知道Jovi当年为啥要写成STATIC的，呵呵
    //构造签名包
    static void signature_construct(Zerg_App_Frame *&proc_frame, uint32_t uin, const char *pSignature, size_t len);

};

#pragma pack ()

//清理内部的选项信息
inline void Zerg_App_Frame::clear_inner_option()
{
    frame_option_ &=  OUTER_OPTION_MASK;
}
//清理所有的选项心想
inline void Zerg_App_Frame::clear_all_option()
{
    frame_option_ = 0;
}

//得到帧的总长度
inline size_t Zerg_App_Frame::get_appframe_len() const
{
    return frame_length_;
}
//得到帧头总长度
inline  size_t Zerg_App_Frame::get_frame_datalen() const
{
    return frame_length_ - LEN_OF_APPFRAME_HEAD;
}

//得到发送者的IP地址,网络字节序的，
inline uint32_t Zerg_App_Frame::get_send_ip() const
{
    return send_ip_address_;
}

//是否事通信服务器处理的命理
inline bool Zerg_App_Frame::is_zerg_processcmd()
{
    if (frame_command_ >= ZERG_COMMAND_BEGIN && frame_command_ <= ZERG_COMMAND_END)
    {
        return true;
    }

    return false;
}

//检查命令是否是内部处理命令
inline bool  Zerg_App_Frame::is_internal_process(bool &bsenderr)
{
    bsenderr = false;

    if (frame_command_ >= INTERNAL_COMMAND_BEGIN && frame_command_ <= INTERNAL_COMMAND_END)
    {
        return true;
    }

    if (frame_option_ & DESC_SEND_ERROR)
    {
        bsenderr = true;
        return true;
    }

    return false;
}

/******************************************************************************************
Author          : Yunfeiyang  Date Of Creation: 2007年3月12日
Function        : Zerg_App_Frame::appdata_decode
Return          : template<class T> int
Parameter List  :
  Param1: size_t szframe_appdata FRAME的APPDATA的数据长度，注意不是整个APPFRMAE的长度
  Param2: const T& info          编码的数据
  Param3: size_t data_start      从DATA区的第几个位置开始填充数据，默认为0
  Param4: size_t *sz_code        如果关系编码的数据长度，传递一个指针得到长度，默认为NULL,表示不关心
Description     : frame_appdata_的编码函数
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
template<class T>
int Zerg_App_Frame::appdata_encode(size_t szframe_appdata,
                                   const T &info,
                                   size_t data_start,
                                   size_t *sz_code)
{
    size_t use_len = 0;
    int ret = info.pack(frame_appdata_ + data_start,
                        szframe_appdata - data_start, &use_len);

    if (ret == 0 && sz_code != NULL)
    {
        *sz_code = use_len;
    }

    if (ret != 0)
    {
        return SOAR_RET::ERROR_CDR_ENCODE_FAIL;
    }

    // 调整frame的长度
    frame_length_
        = (uint32_t)(data_start + use_len + LEN_OF_APPFRAME_HEAD);
    return 0;
}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2007年3月12日
Function        : Zerg_App_Frame::appdata_decode
Return          : template<class T> int
Parameter List  :
  Param1: T& info           解码转换的结构
  Param2: size_t data_start 从DATA区的第几个位置开始解码数据，默认为0
  Param3: size_t *sz_code   如果关系编码的数据长度，传递一个指针得到长度，默认为NULL,表示不关心
Description     : frame_appdata_的解码函数
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
template<class T>
int Zerg_App_Frame::appdata_decode(  T &info,
                                     size_t data_start,
                                     size_t *sz_code) const
{
    size_t use_len = 0;

    // frame_length_包含了frame头部数据的，所以要跳过
    int ret = info.unpack(frame_appdata_ + data_start,
                          frame_length_ - data_start - LEN_OF_APPFRAME_HEAD, &use_len);

    if (ret == 0 && sz_code != NULL)
    {
        //如果要得到编码数据的长度
        *sz_code = use_len;
    }

    if (ret != 0)
    {
        return SOAR_RET::ERROR_CDR_ENCODE_FAIL;
    }

    return 0;
}


//很耗时的操作，注意使用频度
#define DEBUGDUMP_FRAME_HEAD(x,y,z)    Zerg_App_Frame::dumpoutput_framehead(x,y,z);
#define DEBUGDUMP_FRAME_INFO(x,y,z)    Zerg_App_Frame::dumpoutput_frameinfo(x,y,z);


//非DEBUG版本会优化掉的宏
#if defined _DEBUG || defined DEBUG
#define DEBUGDUMP_FRAME_HEAD_DBG(x,y,z)      Zerg_App_Frame::dumpoutput_framehead(x,y,z);
#define DEBUGDUMP_FRAME_INFO_DBG(x,y,z)      Zerg_App_Frame::dumpoutput_frameinfo(x,y,z)
#else
#define DEBUGDUMP_FRAME_HEAD_DBG(x,y,z)
#define DEBUGDUMP_FRAME_INFO_DBG(x,y,z)
#endif

#endif //SOARING_LIB_SERVER_APP_FRAME_H_

