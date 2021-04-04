

#ifndef OGRE_SERVER_CONFIG_H_
#define OGRE_SERVER_CONFIG_H_

//======================================================================================



/*!
* @brief
*
* @note
*/
struct TCP_PEER_CONFIG_INFO
{
public:
    ///���ַ�����ת���õ�
    int from_str(const char *peer_info_str);

public:

    ///PEER �ĵ�ַ
    ZCE_Sockaddr_In       peer_socketin_;
    ///
    std::string          module_file_;
};


/*!
* @brief
*
* @note
*/
struct TCP_PEER_MODULE_INFO
{
public:

    TCP_PEER_MODULE_INFO();
    ~TCP_PEER_MODULE_INFO();

public:

    ///����ģ��
    int open_module();

    ///�ر�ģ��
    int close_module();

public:

    ///
    OGRE_PEER_ID  peer_id_;

    ///
    TCP_PEER_CONFIG_INFO peer_info_;

    ///TCP��ȡ���ݵ�ģ��HANDLER
    ZCE_SHLIB_HANDLE     recv_mod_handler_ = ZCE_SHLIB_INVALID_HANDLE;
    ///
    FP_JudgeRecv_WholeFrame   fp_judge_whole_frame_ = NULL;

};


//�õ�KEY��HASH����
struct HASH_OF_PEER_MODULE
{
public:
    size_t operator()(const TCP_PEER_MODULE_INFO &peer_module) const
    {
        return (size_t(peer_module.peer_id_.peer_port_) << 16) + peer_module.peer_id_.peer_ip_address_;
    }
};

struct EQUAL_OF_PEER_MODULE
{
public:
    //ע���ж������������еı���
    bool operator()(const TCP_PEER_MODULE_INFO &left, const TCP_PEER_MODULE_INFO &right) const
    {
        //���SVC INFO�����,����Ϊ���
        if (right.peer_id_ == left.peer_id_)
        {
            return true;
        }

        return false;
    }
};

//======================================================================================


/*!
* @brief
*
* @note
*/
struct OGRE_CONFIG_DATA
{

public:

    ///���ĵİ󶨵�SVC ID������Ҳ���Ǽ�������
    static const size_t MAX_TCPACCEPT_PEERID_NUM = 4;

    static const size_t MAX_UDP_PEERID_NUM = 4;

    ///�����������ӷ���������
    static const size_t MAX_AUTO_CONNECT_PEER_NUM = 512;

    ///���ĸ��ټ�¼�������������ͣ������Զ�ѡ��·�ɣ��㲥��
    static const size_t MAX_RECORD_SERVICES_TYPE = 16;

    ///Ĭ�ϵ�ACCEPT�˿ڵ�backlog��LightHTTP���ֵ��������1024�������Ҫ��Ȼ���½�ܶ��ˣ����Ե���������
    static const int DEFUALT_ZERG_BACKLOG = 128;

public:

    ///����֡�ĳ���
    unsigned int max_data_len_ = 32 * 1024;

    /// #������ӵķ��������� ##ǰ��128000�����1024
    size_t max_accept_svr_ = 1024;

    ///�����Ƿ�ʹ��
    bool ogre_insurance_ = true;

    ///
    int accept_backlog_ = DEFUALT_ZERG_BACKLOG;


    /// �������ӵķ���BUFFER���������FRAME�ĸ��� �������ٶ������ϴ�ĺ�˷���������Ĵ�һЩ,
    uint32_t acpt_send_deque_size_ = 32;
    /// ÿ��connect ��ȥ�ģ�tcp���ӵķ��Ͷ��г���
    uint32_t cnnt_send_deque_size_ = 128;

    /// #��CONNECT���յ�����,��Сʱ��,0-50��������������>0,����15-60������
    uint32_t accepted_timeout_ = 60;
    /// RECEIVEһ�����ݵĳ�ʱʱ��,Ϊ0��ʾ������,�������ҵ�����߼��ж�һ��
    uint32_t receive_timeout_ = 0;


    /// #��һ�����������ظ����Է��͵Ĵ���,Ŀǰ���ֵû���ô��ˣ�
    uint32_t retry_error_ = 3;

    ///�ܾ����������ӵ�IP��ַ�б��ÿո�ֿ�
    std::string reject_ip_;
    ///�������ӵ�IP��ַ�б��ÿո�ֿ�
    std::string allow_ip_;



    ///������֧�ֵ�Accept TCP PEER��������
    size_t accept_peer_num_ = 0;
    ///������֧��Accept TCP PEER
    TCP_PEER_CONFIG_INFO  accept_peer_ary_[MAX_TCPACCEPT_PEERID_NUM];

    ///������֧�ֵ�Connect TCP PEER���������������ӳ�ȥ����������
    size_t auto_connect_num_ = 0;
    ///�������ӵķ���������
    TCP_PEER_CONFIG_INFO  auto_cnt_peer_ary_[MAX_AUTO_CONNECT_PEER_NUM];


    ///������֧�ֵ�UDP PEER��������
    size_t udp_peer_num_ = 0;
    ///������֧��UDP PEER
    ZCE_Sockaddr_In udp_peer_ary_[MAX_UDP_PEERID_NUM];

};


//===================================================================================


class Ogre_Server_Config : public Server_Config_Base
{

public:

    Ogre_Server_Config();
    virtual ~Ogre_Server_Config();

public:

    ///��ȡ�����ļ����õ��ļ����ò���
    virtual int read_cfgfile();


    ///�������ж�ȡOGRE������
    int get_ogre_cfg(const ZCE_Conf_PropertyTree *conf_tree);

public:


    //OGRE�����ļ���·��
    std::string             ogre_cfg_file_;

    //OGRE����ģ���·��
    std::string             ogre_mod_path_;

    ///ZERG����������
    OGRE_CONFIG_DATA        ogre_cfg_data_;


};

#endif //OGRE_SERVER_CONFIG_H_

