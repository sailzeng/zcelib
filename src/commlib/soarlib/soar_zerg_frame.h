
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
struct ZERG_FRAME_OPTION
{
public:

    //frame_option_的头24个BIT作为选项字段,,
    uint32_t     option_ : 20 = 0;

    uint32_t     user_define_:4;

    //protocol_的头24个BIT作为选项字段,,
    uint32_t     protocol_:4;

    //后4BIT作为版本标识
    uint32_t     version_:4;


    //帧的描述,在frame_option_字段使用
    enum FRAME_OPTION
    {
        //-----------------------------------------------------------------
        //低16位用于内部的描述

        //
        DESC_DEFUALT = 0x0,

        //高优先级别，没有使用，
        DESC_HIGH_PRIORITY = 0x1,

        //某个命令帧发送失败,通知后面的服务
        DESC_SEND_ERROR = 0x2,

        //如果发送失败,重复尝试发送
        DESC_SEND_FAIL_RECORD = 0x4,
        //如果发送失败,通知后面的应用进程
        DESC_SNDPRC_NOTIFY_APP = 0x8,

        //如果发送成功后,直接断开连接，用于部分TCP的短连接
        DESC_SNDPRC_CLOSE_PEER = 0x10,

        //在包体追加登录KEY
        DESC_APPEND_SESSION_KEY = 0x20,

        //在包体追加登录ip
        DESC_APPEND_LOGIN_IP = 0x40,


        //-----------------------------------------------------------------
        //高16位用于外部的描述,8位描述，
        DESC_MONITOR_TRACK = 0x10000,

        //FRAME数据区的有用户签名
        DESC_HEAD_WITH_SIGNATURE = 0x20000,

        //帧的数据采用加密
        DESC_SESSION_ENCRYPT = 0x40000,

        //特殊的某些命令不用加密进行处理，用于加密情况某些命令无须加密的特殊情况
        DESC_SPECIAL_NO_ENCRYPT = 0x80000,

    };


    enum FRAME_PROTOCOL
    {
        //如果是TCP的帧,其实默认是TCP的帧,所以其实没有使用
        PROTOCOL_TCP = 0x00,

        //默认的通讯帧都是TCP的，这个描述字表示这个帧是UDP的
        PROTOCOL_UDP = 0x01,

        //如果有时间，为可靠UDP搞一个
        PROTOCOL_RUDP = 0x03,
    };

    enum FRAME_VERSION
    {
        //APPFram的版本V1,对外使用
        VERSION_V1 = 0x1,
        //APPFram的版本V2，对内使用
        VERSION_V2 = 0x2,
        //APPFram的版本V3，
        VERSION_V2 = 0x3,

    };
};



/*!
* @brief      Zerg服务器间传递消息的通用帧头
*
*/
class SOARING_EXPORT ZERG_FRAME_HEAD
{

protected:
    //构造函数，禁止大家都可以用的.
    ZERG_FRAME_HEAD();
public:
    //析构函数
    ~ZERG_FRAME_HEAD();

    //Assign =运算符号
    ZERG_FRAME_HEAD &operator = (const ZERG_FRAME_HEAD &other);

    ///初始化V1版本的包头,所有数据清0
    void init_framehead(uint32_t lenframe = LEN_OF_APPFRAME_HEAD,
                        uint32_t cmd = CMD_INVALID_CMD,
                        uint32_t frameoption = DESC_V1_VERSION);

    //
    inline void clear();

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
    void exchange_rcvsnd_svcid(ZERG_FRAME_HEAD &exframe );
    //回填返回包头
    void fillback_appframe_head(ZERG_FRAME_HEAD &exframe );

    //Clone一个APP FRAME
    ZERG_FRAME_HEAD *clone() const;
    //
    void clone(ZERG_FRAME_HEAD * dst_frame) const;
    //
    void clone_head(ZERG_FRAME_HEAD * dst_frame) const;

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

    template<typename info_type>
    int appdata_encode(size_t szframe_appdata, const info_type &info);

    template<typename info_type>
    int appdata_decode(size_t szframe_appdata, info_type &info);

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

    //取得IP地址信息
    uint32_t get_send_ip() const;



protected:



public:
    // explicit is better than implicit
    //经过很多次和VC编译器反复的摧残，我决定不再对外暴漏这个函数，外部请使用new_frame and delete_frame
    //为什么呢，其实本来我认为只使用类内部的Placement new and delete 也算是explicit的，
    //但被VC这么反复折腾，我前面先也用 ifdef对付过去了。但升级到VS2017发现左也不是，右也不是。不如不把new暴漏出去
    //http://www.cnblogs.com/fullsail/p/4292214.html
    //不placement new和delete了。听人劝吃饱饭。VS总是变化。搞的总是要折腾

    ///创建一个Frame
    static ZERG_FRAME_HEAD *new_frame(std::size_t lenframe);
    ///销毁一个frame
    static void delete_frame(ZERG_FRAME_HEAD *frame);


    ///输出APPFRAME的头部信息
    static void dumpoutput_framehead(ZCE_LOG_PRIORITY log_priority,
                                     const char *outstr,
                                     const ZERG_FRAME_HEAD *frame);
    ///输出APPFRAME的尾部信息
    static void dumpoutput_frameinfo(ZCE_LOG_PRIORITY log_priority,
                                     const char *outstr,
                                     const ZERG_FRAME_HEAD *frame);

public:

    //包头都尺寸,
    static const size_t LEN_OF_APPFRAME_HEAD = 50;

    //APPFAME版本V1
    static const unsigned char TSS_APPFRAME_V1 = 1;

    //---------------------------------------------------------------------------
    //FRAME的一些长度参数,

    //默认的最大长度是64K
    //为什么采用64K的原因是我们的UPD的最大长度是这个，而且这个缓冲区的长度比较适中.
    static const size_t MAX_LEN_OF_APPFRAME = 64 * 1024;



    //FRAME的最大长度,根据各个地方的长度而得到
    static const size_t MAX_LEN_OF_APPFRAME_DATA = MAX_LEN_OF_APPFRAME - LEN_OF_APPFRAME_HEAD;

    //
    static const size_t MAX_LEN_OF_TEA_APPDATA = MAX_LEN_OF_APPFRAME_DATA ;

public:

    ///整个通讯包长度,留足空间,包括帧头的长度.
    uint32_t               frame_length_;


    ZERG_FRAME_OPTION      frame_option_;


    ///命令字 命令字还是放在包头比较好,
    uint32_t               frame_command_;

    ///UID
    uint32_t               frame_userid_;

    ///事务ID,可以用作服务发起端作为一个标示，后面的服务器回填backfill_trans_id_字段返回,
    uint32_t               transaction_id_;
    ///回填的请求者的事务ID,
    uint32_t               backfill_trans_id_;

    union
    {
        ///发送和接收的服务器应用也要填写
        ///发送序列号，计划只在通讯层用,暂时没用用
        uint32_t           serial_number_;
        ///发送者的IP地址，内部使用
        uint32_t           send_ip_address_;

    };
    
    ///发送服务,包括发送服务器类型，发送服务器编号,没有编号，或者不是服务填写0
    SERVICES_ID            send_service_;
    ///接受服务器
    SERVICES_ID            recv_service_;
    ///代理服务器
    SERVICES_ID            proxy_service_;

    ///业务ID，GAMEID，用于标识游戏内部ID
    uint32_t               business_id_;




    ///frame_appdata_ 是一个变长度的字符串序列标示,
#ifdef ZCE_OS_WINDOWS
#pragma warning ( disable : 4200)
#endif
    char                 frame_appdata_[];
#ifdef ZCE_OS_WINDOWS
#pragma warning ( default : 4200)
#endif

};

#pragma pack ()


//初始化，ZERG服务器间传递消息的通用帧头
inline void ZERG_FRAME_HEAD::init_framehead(uint32_t lenframe,
                                           uint32_t cmd,
                                           uint32_t frameoption)
{
    frame_length_ = lenframe;
    frame_option_ = frameoption;
    frame_command_ = cmd;

    frame_userid_ = 0;
    business_id_ = 0;

    send_service_.set_svcid(0, 0);
    recv_service_.set_svcid(0, 0);
    proxy_service_.set_svcid(0, 0);
    send_serial_number_ = 0;
    transaction_id_ = 0;
    backfill_trans_id_ = 0;

}

//清理
inline void ZERG_FRAME_HEAD::clear()
{
    frame_length_ = CMD_INVALID_CMD;
    frame_option_ = DESC_V1_VERSION;
    frame_command_ = LEN_OF_APPFRAME_HEAD;
    frame_userid_ = 0;
    business_id_ = 0;
    send_service_.services_type_ = 0;
    send_service_.services_id_ = 0;
    recv_service_.services_type_ = 0;
    recv_service_.services_id_ = 0;
    proxy_service_.services_type_ = 0;
    proxy_service_.services_id_ = 0;
    transaction_id_ = 0;
    backfill_trans_id_ = 0;
    send_ip_address_ = 0;
}

//清理内部的选项信息
inline void ZERG_FRAME_HEAD::clear_inner_option()
{
    frame_option_ &=  OUTER_OPTION_MASK;
}
//清理所有的选项心想
inline void ZERG_FRAME_HEAD::clear_all_option()
{
    frame_option_ = 0;
}

//得到帧的总长度
inline size_t ZERG_FRAME_HEAD::get_appframe_len() const
{
    return frame_length_;
}
//得到帧头总长度
inline  size_t ZERG_FRAME_HEAD::get_frame_datalen() const
{
    return frame_length_ - LEN_OF_APPFRAME_HEAD;
}

//得到发送者的IP地址,网络字节序的，
inline uint32_t ZERG_FRAME_HEAD::get_send_ip() const
{
    return send_ip_address_;
}

//是否事通信服务器处理的命理
inline bool ZERG_FRAME_HEAD::is_zerg_processcmd()
{
    if (frame_command_ >= ZERG_COMMAND_BEGIN && frame_command_ <= ZERG_COMMAND_END)
    {
        return true;
    }

    return false;
}

//检查命令是否是内部处理命令
inline bool  ZERG_FRAME_HEAD::is_internal_process(bool &bsenderr)
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


//很耗时的操作，注意使用频度
#define DEBUGDUMP_FRAME_HEAD(x,y,z)    ZERG_FRAME_HEAD::dumpoutput_framehead(x,y,z);
#define DEBUGDUMP_FRAME_INFO(x,y,z)    ZERG_FRAME_HEAD::dumpoutput_frameinfo(x,y,z);


//非DEBUG版本会优化掉的宏
#if defined _DEBUG || defined DEBUG
#define DEBUGDUMP_FRAME_HEAD_DBG(x,y,z)      ZERG_FRAME_HEAD::dumpoutput_framehead(x,y,z);
#define DEBUGDUMP_FRAME_INFO_DBG(x,y,z)      ZERG_FRAME_HEAD::dumpoutput_frameinfo(x,y,z)
#else
#define DEBUGDUMP_FRAME_HEAD_DBG(x,y,z)
#define DEBUGDUMP_FRAME_INFO_DBG(x,y,z)
#endif

#endif //SOARING_LIB_SERVER_APP_FRAME_H_

