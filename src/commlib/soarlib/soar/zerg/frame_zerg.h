
/*!
* @copyright  2004-2021  Apache License, Version 2.0 FULLSAIL
* @filename   soar/zerg/frame_zerg.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version    
* @date       2021年4月17日
* @brief      很无奈，我还是讲所有的类，整理成了struct，因为如果要兼容多个版本，
*             类实在不是一个好的写法。
*             
* @details    
*             
*             
*             
* @note       
*             
*/

#ifndef SOARING_LIB_SERVER_APP_FRAME_H_
#define SOARING_LIB_SERVER_APP_FRAME_H_

#include "soar/enum/enum_define.h"
#include "soar/zerg/services_info.h"
#include "soar/zerg/frame_command.h"
#include "soar/enum/error_code.h"

namespace soar
{

#pragma pack (1)

/*!
* @brief      FRAME的选项字节的UNION结构
*
*/
struct _ZERG_FRAME_OPTION
{
public:

#if ZCE_BYTES_ORDER == ZCE_LITTLE_ENDIAN
    //frame_option_的头24个BIT作为选项字段,
    uint32_t     option_:20;
    //保留4个bit，
    uint32_t     reserve_:4;
    //
    uint32_t     protocol_:4;
    //后4BIT作为版本标识
    uint32_t     version_:4;
#else
    //后4BIT作为版本标识
    uint32_t     version_:4;
    //
    uint32_t     protocol_:4;
    //保留4个bit，
    uint32_t     reserve_:4;
    //frame_option_的头24个BIT作为选项字段,
    uint32_t     option_:20;
#endif
};

struct _ZERG_CLIENT_USE
{
    uint16_t game_id_ = 0;

    uint16_t reserve_use_ = 0;
};


class Zerg_Head
{
public:

    //将帧头的所有的uint16_t,uint32_t转换为网络序
    void hton();
    //将帧头的所有的uint16_t,uint32_t转换为本地序
    void ntoh();

public:

    ///整个通讯包长度,留足空间,包括帧头的长度.
    uint32_t               length_ = sizeof(Zerg_Head);

    union
    {
        ///frame 选项
        _ZERG_FRAME_OPTION frame_option_;
        /// 辅助编大小端转换
        uint32_t           u32_option_ = 0;
    };

    ///命令字 命令字还是放在包头比较好,
    uint32_t               command_ = 0;

    ///用户ID，可以是一个ID也可以是一个名字HASH映射的ID
    uint32_t               user_id_ = 0;

    union
    {
        ///发送序列号，计划只在通讯层用,暂时没用用
        uint32_t           serial_number_ = 0;
        ///发送者的IP地址，内部使用,
        uint32_t           send_ip_address_;
    };

    ///事务FSM ID,可以用作服务发起端作为一个标示，后面的服务器回填backfill_trans_id_字段返回,
    uint32_t               fsm_id_ = 0;
    ///回填的请求者的事务ID,
    uint32_t               backfill_fsm_id_ = 0;

    ///发送和接收的服务器应用也要填写
    ///接受服务器
    soar::SERVICES_ID      recv_service_;

    ///发送服务,包括发送服务器类型，发送服务器编号,没有编号，或者不是服务填写0
    soar::SERVICES_ID      send_service_;

    union
    {
        ///代理服务器
        soar::SERVICES_ID  proxy_service_;
        /// 
        _ZERG_CLIENT_USE   client_use_;
    };
};

/*!
* @brief      Zerg服务器间传递消息的通用帧头
*
*/
class  Zerg_Frame : public Zerg_Head
{
public:

    //帧的描述,在frame_option_字段使用
    enum FRAME_OPTION
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

        //
        DESC_MONITOR_TRACK       = 0x100,

        //FRAME数据区的有用户签名
        DESC_HEAD_WITH_SIGNATURE = 0x200,

        //帧的数据采用加密
        DESC_SESSION_ENCRYPT     = 0x400,

        //特殊的某些命令不用加密进行处理，用于加密情况某些命令无须加密的特殊情况
        DESC_SPECIAL_NO_ENCRYPT  = 0x800,


        //如果是TCP的帧,其实默认是TCP的帧,所以其实没有使用
        DESC_TCP_FRAME           = 0x0,
        //默认的通讯帧都是TCP的，这个描述字表示这个帧是UDP的
        DESC_UDP_FRAME           = 0x2,
        //
        DESC_RUDP_FRAME          = 0x3,


        //APPFram的版本V1
        DESC_V1_VERSION = 0x1,
        //APPFram的版本V2，终于还是升级了一次
        DESC_V2_VERSION = 0x2,
    };


protected:
    //构造函数，禁止大家都可以用的.
    Zerg_Frame() = delete;
    //析构函数
    ~Zerg_Frame() = delete;
    //Assign =运算符号
    Zerg_Frame &operator = (const Zerg_Frame &other)=delete;

public:
    ///初始化V1版本的包头,所有数据清0
    inline void init_head(uint32_t len,
                          uint32_t option = OPTION_V1_TCP,
                          uint32_t cmd = 0);

    //
    inline void clear();

    //是否事通信服务器处理的命理
    inline bool is_zerg_processcmd();


    //填充AppData数据到APPFrame
    int fill_appdata(const size_t szdata, const char *vardata);

    //交换Rcv ,Snd SvrInfo
    void exchange_rcvsnd_svcid();
    //交换Rcv ,Snd SvrInfo,prochandle
    void exchange_rcvsnd_svcid(Zerg_Frame &exframe );
    //回填返回包头
    void fillback_appframe_head(Zerg_Frame &exframe );


    //给dst_frame克隆一个自己
    void clone(Zerg_Frame * dst_frame) const;
    //给dst_frame复制一个头部数据
    void clone_head(Zerg_Frame * dst_frame) const;

    //取得帧的长度
    inline size_t get_frame_len() const;
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

public:
    // explicit is better than implicit
    //经过很多次和VC编译器反复的摧残，我决定不再对外暴漏new这个函数，外部请使用new_frame and delete_frame
    //为什么呢，其实本来我认为只使用类内部的Placement new and delete 也算是explicit的，
    //但被VC这么反复折腾，我前面先也用 ifdef对付过去了。但升级到VS2017发现左也不是，右也不是。不如不把new暴漏出去
    //http://www.cnblogs.com/fullsail/p/4292214.html
    //不placement new和delete了。听人劝吃饱饭。VS总是变化。搞的总是要折腾

    ///创建一个Frame
    static Zerg_Frame *new_frame(std::size_t lenframe);
    ///销毁一个frame
    static void delete_frame(Zerg_Frame *frame);


    /*!
    * @brief      输出ZERG FRAME的头部信息
    * @return     void
    * @param      log_priority 输出的日志优先级
    * @param      outer_str    你自己要添加的文本信息（用于区别是什么地方输出的）
    * @param      frame        要输出的frame     
    */
    static void dump_frame_head(zce::LOG_PRIORITY log_priority,
                                const char *outer_str,
                                const Zerg_Frame *frame);
    ///输出APPFRAME的所有信息
    static void dump_frame_all(zce::LOG_PRIORITY log_priority,
                               const char *outer_str,
                               const Zerg_Frame *frame);

public:

    //---------------------------------------------------------------------------
    //选项，
    static const uint32_t OPTION_V1_TCP = ((DESC_V1_VERSION > 28) | (DESC_TCP_FRAME > 24));
    static const uint32_t OPTION_V1_UDP = ((DESC_V1_VERSION > 28) | (DESC_UDP_FRAME > 24));
    //---------------------------------------------------------------------------
    //包头都尺寸,40
    static const size_t LEN_OF_APPFRAME_HEAD = sizeof(Zerg_Head);
    
    //FRAME的一些长度参数,默认的最大长度是64K
    //为什么采用64K的原因是我们的UPD的最大长度是这个，而且这个缓冲区的长度比较适中.
    static const size_t MAX_LEN_OF_APPFRAME = 64 * 1024;

    //FRAME的最大长度,根据各个地方的长度而得到
    static const size_t MAX_LEN_OF_APPFRAME_DATA = MAX_LEN_OF_APPFRAME - LEN_OF_APPFRAME_HEAD;

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
inline void Zerg_Frame::init_head(uint32_t len,
                                  uint32_t option,
                                  uint32_t cmd)
{
    length_ = len;
    u32_option_ = option;
    command_ = cmd;
    user_id_ = 0;
    send_service_.set_svcid(0, 0);
    recv_service_.set_svcid(0, 0);
    proxy_service_.set_svcid(0, 0);
    serial_number_ = 0;
    fsm_id_ = 0;
    backfill_fsm_id_ = 0;
}

//清理
inline void Zerg_Frame::clear()
{
    length_ = CMD_INVALID_CMD;
    frame_option_.version_ = DESC_V1_VERSION;
    command_ = LEN_OF_APPFRAME_HEAD;
    user_id_ = 0;
    send_service_.services_type_ = 0;
    send_service_.services_id_ = 0;
    recv_service_.services_type_ = 0;
    recv_service_.services_id_ = 0;
    proxy_service_.services_type_ = 0;
    proxy_service_.services_id_ = 0;
    fsm_id_ = 0;
    backfill_fsm_id_ = 0;
    send_ip_address_ = 0;
}



//得到帧的总长度
inline size_t Zerg_Frame::get_frame_len() const
{
    return length_;
}
//得到帧头总长度
inline  size_t Zerg_Frame::get_frame_datalen() const
{
    return length_ - LEN_OF_APPFRAME_HEAD;
}


//是否事通信服务器处理的命理
inline bool Zerg_Frame::is_zerg_processcmd()
{
    if (command_ >= ZERG_COMMAND_BEGIN && command_ <= ZERG_COMMAND_END)
    {
        return true;
    }

    return false;
}


}

//很耗时的操作，注意使用频度
#define DUMP_ZERG_FRAME_HEAD(x,y,z)    soar::Zerg_Frame::dump_frame_head(x,y,z)
#define DUMP_ZERG_FRAME_ALL(x,y,z)     soar::Zerg_Frame::dump_frame_all(x,y,z)


//非DEBUG版本会优化掉的宏
#if defined _DEBUG || defined DEBUG
#define DEBUG_DUMP_ZERG_FRAME_HEAD(x,y,z)     soar::Zerg_Frame::dump_frame_head(x,y,z)
#define DEBUG_DUMP_ZERG_FRAME_ALL(x,y,z)      soar::Zerg_Frame::dump_frame_all(x,y,z)
#else
#define DEBUG_DUMP_ZERG_FRAME_HEAD(x,y,z)
#define DEBUG_DUMP_ZERG_FRAME_ALL(x,y,z)
#endif




#endif //SOARING_LIB_SERVER_APP_FRAME_H_
