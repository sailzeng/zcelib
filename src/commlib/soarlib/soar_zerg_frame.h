
#ifndef SOARING_LIB_SERVER_APP_FRAME_H_
#define SOARING_LIB_SERVER_APP_FRAME_H_

#include "soar_enum_define.h"
#include "soar_services_info.h"
#include "soar_frame_command.h"
#include "soar_error_code.h"

#pragma pack (1)


/*!
* @brief      FRAME��ѡ���ֽڵ�UNION�ṹ
*
*/
struct _ZERG_FRAME_OPTION
{
public:

    //frame_option_��ͷ24��BIT��Ϊѡ���ֶ�,,
    uint32_t     inner_option_ : 24;

    //��4BIT��Ϊ�汾��ʶ
    uint32_t     frame_version_ : 8;
};



/*!
* @brief      Zerg�������䴫����Ϣ��ͨ��֡ͷ
*
*/
class SOARING_EXPORT Zerg_App_Frame
{
public:

    //֡������,��frame_option_�ֶ�ʹ��
    enum PET_APPFRAME_OPTION
    {
        //-----------------------------------------------------------------
        //��16λ�����ڲ�������

        //
        DESC_DEFUALT             = 0x0,

        //�����ȼ���û��ʹ�ã�
        DESC_HIGH_PRIORITY       = 0x1,

        //ĳ������֡����ʧ��,֪ͨ����ķ���
        DESC_SEND_ERROR          = 0x2,

        //�������ʧ��,�ظ����Է���
        DESC_SEND_FAIL_RECORD    = 0x4,
        //�������ʧ��,֪ͨ�����Ӧ�ý���
        DESC_SNDPRC_NOTIFY_APP   = 0x8,

        //������ͳɹ���,ֱ�ӶϿ����ӣ����ڲ���TCP�Ķ�����
        DESC_SNDPRC_CLOSE_PEER   = 0x10,

        //�ڰ���׷�ӵ�¼KEY
        DESC_APPEND_SESSION_KEY  = 0x20,

        //�ڰ���׷�ӵ�¼ip
        DESC_APPEND_LOGIN_IP     = 0x40,

        //Ĭ�ϵ�ͨѶ֡����TCP�ģ���������ֱ�ʾ���֡��UDP��
        DESC_UDP_FRAME           = 0x80,

        //�����TCP��֡,��ʵĬ����TCP��֡,������ʵû��ʹ��
        DESC_TCP_FRAME           = 0x100,

        //-----------------------------------------------------------------
        //��16λ�����ⲿ������,8λ������
        DESC_MONITOR_TRACK       = 0x10000,

        //FRAME�����������û�ǩ��
        DESC_HEAD_WITH_SIGNATURE = 0x20000,

        //֡�����ݲ��ü���
        DESC_SESSION_ENCRYPT     = 0x40000,

        //�����ĳЩ����ü��ܽ��д������ڼ������ĳЩ����������ܵ��������
        DESC_SPECIAL_NO_ENCRYPT  = 0x80000,

        //APPFram�İ汾V1
        DESC_V1_VERSION          = 0x1000000,
        //APPFram�İ汾V2�����ڻ���������һ��
        DESC_V2_VERSION          = 0x2000000,
    };


protected:
    //���캯������ֹ��Ҷ������õ�.
    Zerg_App_Frame();
public:
    //��������
    ~Zerg_App_Frame();

    //Assign =�������
    Zerg_App_Frame &operator = (const Zerg_App_Frame &other);

    ///��ʼ��V1�汾�İ�ͷ,����������0
    void init_framehead(uint32_t lenframe = LEN_OF_APPFRAME_HEAD,
                        uint32_t cmd = CMD_INVALID_CMD,
                        uint32_t frameoption = DESC_V1_VERSION);

    //
    inline void clear();

    //�Ƿ����ڲ����������
    inline bool is_internal_process(bool &bsenderr);
    //�Ƿ���ͨ�ŷ��������������
    inline bool is_zerg_processcmd();

    //�����ڲ���ѡ����Ϣ
    inline void clear_inner_option();
    //�������е�ѡ������
    inline void clear_all_option();

    //��֡ͷ�����е�uint16_t,uint32_tת��Ϊ������
    void framehead_encode();
    //��֡ͷ�����е�uint16_t,uint32_tת��Ϊ������
    void framehead_decode();


    //��䷢��SVR��Ϣ
    void set_send_svcid(uint16_t svrtype, uint32_t svrid);
    //��䷢��SVR��Ϣ
    void set_recv_svcid(uint16_t svrtype, uint32_t svrid);
    //������SVR��Ϣ
    void set_proxy_svcid(uint16_t svrtype, uint32_t svrid);

    //��д���еķ�����Ϣ,
    void set_all_svcid(const SERVICES_ID &rcvinfo, const SERVICES_ID &sndinfo, const SERVICES_ID &proxyinfo);

    //���AppData���ݵ�APPFrame
    int fill_appdata(const size_t szdata, const char *vardata);

    //����Rcv ,Snd SvrInfo
    void exchange_rcvsnd_svcid();
    //����Rcv ,Snd SvrInfo,prochandle
    void exchange_rcvsnd_svcid(Zerg_App_Frame &exframe );
    //����ذ�ͷ
    void fillback_appframe_head(Zerg_App_Frame &exframe );

    //Cloneһ��APP FRAME
    Zerg_App_Frame *clone() const;
    //
    void clone(Zerg_App_Frame * dst_frame) const;
    //
    void clone_head(Zerg_App_Frame * dst_frame) const;

    //Dumpͷ����DATA��������
    void dump_appframe_info(std::ostringstream &strstream) const;
    //Dump���е�������Ϣ,һ���ֽ��ֽڵĸ�����,
    void dump_appframe_data(std::ostringstream &strstream) const;
    //Dump���ݰ�ͷ�е���Ҫ��Ϣ����,
    void dump_appframe_head(std::ostringstream &strstream) const;

    //���APPFRAME��ͷ����Ϣ
    void dumpoutput_framehead(const char *outstr,
                              ZCE_LOG_PRIORITY log_priority) const;
    //���APPFRAME��β����Ϣ
    void dumpoutput_frameinfo(const char *outstr,
                              ZCE_LOG_PRIORITY log_priority)  const;

    //ȡ��֡�ĳ���
    inline size_t get_appframe_len() const;
    //ȡ��֡���ݵĳ���
    inline size_t get_frame_datalen() const;

    template<typename info_type>
    int appdata_encode(size_t szframe_appdata, const info_type &info);

    template<typename info_type>
    int appdata_decode(size_t szframe_appdata, info_type &info);

#if defined ZCE_USE_PROTOBUF && ZCE_USE_PROTOBUF == 1

    ///��һ���ṹ���б���
    int protobuf_encode(size_t szframe_appdata,
                        const google::protobuf::MessageLite *msg,
                        size_t data_start = 0,
                        size_t *sz_code = NULL
                       );

    ///��һ���ṹ���н���
    int protobuf_decode(google::protobuf::MessageLite *msg,
                        size_t data_start = 0,
                        size_t *sz_code = NULL);

#endif

    //ȡ��IP��ַ��Ϣ
    uint32_t get_send_ip() const;



protected:



public:
    // explicit is better than implicit
    //�����ܶ�κ�VC�����������ĴݲУ��Ҿ������ٶ��Ⱪ©����������ⲿ��ʹ��new_frame and delete_frame
    //Ϊʲô�أ���ʵ��������Ϊֻʹ�����ڲ���Placement new and delete Ҳ����explicit�ģ�
    //����VC��ô�������ڣ���ǰ����Ҳ�� ifdef�Ը���ȥ�ˡ���������VS2017������Ҳ���ǣ���Ҳ���ǡ����粻��new��©��ȥ
    //http://www.cnblogs.com/fullsail/p/4292214.html
    //��placement new��delete�ˡ�����Ȱ�Ա�����VS���Ǳ仯���������Ҫ����

    ///����һ��Frame
    static Zerg_App_Frame *new_frame(std::size_t lenframe);
    ///����һ��frame
    static void delete_frame(Zerg_App_Frame *frame);


    ///���APPFRAME��ͷ����Ϣ
    static void dumpoutput_framehead(ZCE_LOG_PRIORITY log_priority,
                                     const char *outstr,
                                     const Zerg_App_Frame *frame);
    ///���APPFRAME��β����Ϣ
    static void dumpoutput_frameinfo(ZCE_LOG_PRIORITY log_priority,
                                     const char *outstr,
                                     const Zerg_App_Frame *frame);

public:

    //�ڲ�ѡ������������,�ڲ�ѡ��������俴��������ͨѶ��������ҵ�������ǰ�ɼ���
    static const uint32_t INNER_OPTION_MASK = 0xFFFF;
    //�ⲿѡ������+�汾�ŵ�����
    static const uint32_t OUTER_OPTION_MASK = 0xFFFF0000;

public:

    //��ͷ���ߴ�,
    static const size_t LEN_OF_APPFRAME_HEAD = 50;

    //APPFAME�汾V1
    static const unsigned char TSS_APPFRAME_V1 = 1;

    //---------------------------------------------------------------------------
    //FRAME��һЩ���Ȳ���,

    //Ĭ�ϵ���󳤶���64K
    //Ϊʲô����64K��ԭ�������ǵ�UPD����󳤶����������������������ĳ��ȱȽ�����.
    static const size_t MAX_LEN_OF_APPFRAME = 64 * 1024;


    //TEA���ܺ����ӵĳ���,UPD���������鷳�Լ���೤�����Ƶ�����
    static const size_t LEN_OF_TEA_REMAIN_ROOM = 17;

    //FRAME����󳤶�,���ݸ����ط��ĳ��ȶ��õ�
    static const size_t MAX_LEN_OF_APPFRAME_DATA = MAX_LEN_OF_APPFRAME - LEN_OF_APPFRAME_HEAD - LEN_OF_TEA_REMAIN_ROOM;

    //
    static const size_t MAX_LEN_OF_TEA_APPDATA = MAX_LEN_OF_APPFRAME_DATA + LEN_OF_TEA_REMAIN_ROOM;

public:

    ///����ͨѶ������,����ռ�,����֡ͷ�ĳ���.
    uint32_t               frame_length_;

    union
    {
        uint32_t           frame_option_;
        ///
        _ZERG_FRAME_OPTION inner_option_;
    };

    ///������ �����ֻ��Ƿ��ڰ�ͷ�ȽϺ�,
    uint32_t               frame_command_;

    ///UID
    uint32_t               frame_uid_;
    ///ҵ��ID��GAMEID�����ڱ�ʶ��Ϸ�ڲ�ID
    uint32_t               app_id_;

    ///���ͺͽ��յķ�����Ӧ��ҲҪ��д

    ///���ͷ���,�������ͷ��������ͣ����ͷ��������,û�б�ţ����߲��Ƿ�����д0
    SERVICES_ID            send_service_;
    ///���ܷ�����
    SERVICES_ID            recv_service_;
    ///���������
    SERVICES_ID            proxy_service_;

    ///����ID,�����������������Ϊһ����ʾ������ķ���������backfill_trans_id_�ֶη���,
    uint32_t               transaction_id_;
    ///����������ߵ�����ID,
    uint32_t               backfill_trans_id_;


    union
    {
        ///�������кţ��ƻ�ֻ��ͨѶ����,��ʱû����
        uint32_t           send_serial_number_;
        ///�����ߵ�IP��ַ���ڲ�ʹ��
        uint32_t           send_ip_address_;

    };

    ///frame_appdata_ ��һ���䳤�ȵ��ַ������б�ʾ,
#ifdef ZCE_OS_WINDOWS
#pragma warning ( disable : 4200)
#endif
    char                 frame_appdata_[];
#ifdef ZCE_OS_WINDOWS
#pragma warning ( default : 4200)
#endif

};

#pragma pack ()


//��ʼ����ZERG�������䴫����Ϣ��ͨ��֡ͷ
inline void Zerg_App_Frame::init_framehead(uint32_t lenframe,
                                           uint32_t cmd,
                                           uint32_t frameoption)
{
    frame_length_ = lenframe;
    frame_option_ = frameoption;
    frame_command_ = cmd;

    frame_uid_ = 0;
    app_id_ = 0;

    send_service_.set_svcid(0, 0);
    recv_service_.set_svcid(0, 0);
    proxy_service_.set_svcid(0, 0);
    send_serial_number_ = 0;
    transaction_id_ = 0;
    backfill_trans_id_ = 0;

}

//����
inline void Zerg_App_Frame::clear()
{
    frame_length_ = CMD_INVALID_CMD;
    frame_option_ = DESC_V1_VERSION;
    frame_command_ = LEN_OF_APPFRAME_HEAD;
    frame_uid_ = 0;
    app_id_ = 0;
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

//�����ڲ���ѡ����Ϣ
inline void Zerg_App_Frame::clear_inner_option()
{
    frame_option_ &=  OUTER_OPTION_MASK;
}
//�������е�ѡ������
inline void Zerg_App_Frame::clear_all_option()
{
    frame_option_ = 0;
}

//�õ�֡���ܳ���
inline size_t Zerg_App_Frame::get_appframe_len() const
{
    return frame_length_;
}
//�õ�֡ͷ�ܳ���
inline  size_t Zerg_App_Frame::get_frame_datalen() const
{
    return frame_length_ - LEN_OF_APPFRAME_HEAD;
}

//�õ������ߵ�IP��ַ,�����ֽ���ģ�
inline uint32_t Zerg_App_Frame::get_send_ip() const
{
    return send_ip_address_;
}

//�Ƿ���ͨ�ŷ��������������
inline bool Zerg_App_Frame::is_zerg_processcmd()
{
    if (frame_command_ >= ZERG_COMMAND_BEGIN && frame_command_ <= ZERG_COMMAND_END)
    {
        return true;
    }

    return false;
}

//��������Ƿ����ڲ���������
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


//�ܺ�ʱ�Ĳ�����ע��ʹ��Ƶ��
#define DEBUGDUMP_FRAME_HEAD(x,y,z)    Zerg_App_Frame::dumpoutput_framehead(x,y,z);
#define DEBUGDUMP_FRAME_INFO(x,y,z)    Zerg_App_Frame::dumpoutput_frameinfo(x,y,z);


//��DEBUG�汾���Ż����ĺ�
#if defined _DEBUG || defined DEBUG
#define DEBUGDUMP_FRAME_HEAD_DBG(x,y,z)      Zerg_App_Frame::dumpoutput_framehead(x,y,z);
#define DEBUGDUMP_FRAME_INFO_DBG(x,y,z)      Zerg_App_Frame::dumpoutput_frameinfo(x,y,z)
#else
#define DEBUGDUMP_FRAME_HEAD_DBG(x,y,z)
#define DEBUGDUMP_FRAME_INFO_DBG(x,y,z)
#endif

#endif //SOARING_LIB_SERVER_APP_FRAME_H_

