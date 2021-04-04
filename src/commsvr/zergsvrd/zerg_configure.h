#ifndef ZERG_SERVER_CONFIG_H_
#define ZERG_SERVER_CONFIG_H_



//===================================================================================

//��������
struct SERVICES_INFO_TABLE
{

    typedef std::unordered_set<SERVICES_INFO, HASH_OF_SVCINFO, EQUAL_OF_SVCINFO> SET_OF_SVCINFO;

public:
    //���캯��,
    SERVICES_INFO_TABLE(size_t sz_table = INIT_NUM_OF_SVR_CONFG);
    ~SERVICES_INFO_TABLE();


    /*!
    * @brief      ����SvrInfo��Ϣ��ѯIP������Ϣ
    * @return     int
    * @param      svc_id      �������,��������ϢSVCINFO
    * @param      ip_address  �������,��������ϢSVCINFO
    * @param      idc_no      �������,IDC��Ϣ
    * @param      business_id �������,���������
    * @note
    */
    int find_svcinfo(const SERVICES_ID &svc_id,
                     ZCE_Sockaddr_In &ip_address,
                     unsigned int &idc_no,
                     unsigned int &business_id) const;

    //����SvrInfo��Ϣ��ѯIP������Ϣ
    int find_svcinfo(const SERVICES_ID &svc_id,
                     SERVICES_INFO &svc_info) const;

    //����Ƿ�ӵ����Ӧ��Services Info
    bool hash_svcinfo(const SERVICES_ID &svc_id) const;

    //����������Ϣ
    int add_svcinfo(const SERVICES_INFO &svc_info);


    //����SVR������Ϣ.
    void clear();


protected:
    //���õĸ���
    static const int INIT_NUM_OF_SVR_CONFG = 2048;

protected:
    //
    SET_OF_SVCINFO    services_table_;

};

//===================================================================================

/*!
* @brief     С�����������
*
* @note
*/
struct ZERG_CONFIG_DATA
{

public:

    ///���ĵİ󶨵�SVC ID������Ҳ���Ǽ�������
    static const size_t MAX_BIND_SERVICES_ID = 4;
    ///
    static const size_t MAX_SLAVE_SERVICES_ID = MAX_BIND_SERVICES_ID - 1;

    ///���ļ�ص�FRAME������,��ϣ��̫��,��������Ӱ��Ч��
    static const size_t MAX_MONITOR_FRAME_NUMBER = 16;

    ///�����������ӷ���������
    static const size_t MAX_AUTO_CONNECT_SVRS = 512;

    ///���ĸ��ټ�¼�������������ͣ������Զ�ѡ��·�ɣ��㲥��
    static const size_t MAX_RECORD_SERVICES_TYPE = 16;

    ///Ĭ�ϵ�ACCEPT�˿ڵ�backlog��LightHTTP���ֵ��������1024�������Ҫ��Ȼ���½�ܶ��ˣ����Ե���������
    static const int DEFUALT_ZERG_BACKLOG = 128;


public:

    /// #������ӵķ��������� ##ǰ��128000�����1024
    size_t max_accept_svr_ = 1024;

    ///ZERG�ı����Ƿ�ʹ��
    bool zerg_insurance_ = true;

    ///
    int accept_backlog_ = DEFUALT_ZERG_BACKLOG;



    /// �������ӵķ���BUFFER���������FRAME�ĸ��� �������ٶ������ϴ�ĺ�˷���������Ĵ�һЩ,
    uint32_t acpt_send_deque_size_ = 32;
    /// ÿ��connect ��ȥ�ģ�tcp���ӵķ��Ͷ��г���
    uint32_t cnnt_send_deque_size_ = 128;


    ///������֧�ֵ�SVC ID������������>=1
    size_t bind_svcid_num_ = 0;
    ///������֧�ֵ�SVC ID��ע�⣬self_svc_id_��Զ�����ڵ�һ��
    SERVICES_ID  bind_svcid_ary_[MAX_BIND_SERVICES_ID];

    ///�������ӵķ���������
    size_t auto_connect_num_ = 0;
    ///�������ӵķ���������
    SERVICES_ID  auto_connect_svrs_[MAX_AUTO_CONNECT_SVRS];


    /// #��CONNECT���յ�����,��Сʱ��,0-50��������������>0,����15-60������
    uint32_t accepted_timeout_ = 60;
    /// RECEIVEһ�����ݵĳ�ʱʱ��,Ϊ0��ʾ������,�������ҵ�����߼��ж�һ��
    uint32_t receive_timeout_ = 0;

    /// �Ƿ���Ϊ���������
    bool is_proxy_ = false;


    /// #��һ�����������ظ����Է��͵Ĵ���,Ŀǰ���ֵû���ô��ˣ�
    uint32_t retry_error_ = 3;



    ///�ܾ����������ӵ�IP��ַ�б��ÿո�ֿ�
    std::string reject_ip_;
    ///�������ӵ�IP��ַ�б��ÿո�ֿ�
    std::string allow_ip_;

    ///��ص����������
    size_t monitor_cmd_count_ = 0;
    ///��ص�����
    uint32_t monitor_cmd_list_[MAX_MONITOR_FRAME_NUMBER];
};


//===================================================================================

/*!
* @brief      С������ô�����
*
* @note
*/
class Zerg_Server_Config : public Server_Config_Base
{
public:
    //
    Zerg_Server_Config();
    virtual ~Zerg_Server_Config();

    //
public:


    ///��ȡ�����ļ����õ��ļ����ò���
    virtual int read_cfgfile();


    /*!
    * @brief      ��ȡ������־�������һЩ������Ϣ���Ա���ٻ���
    * @param      out_lvl �������
    */
    virtual void dump_cfg_info(ZCE_LOG_PRIORITY out_lvl);


    //����SVCID�õ�SVC INFO��ַ��Ϣ
    int get_svcinfo_by_svcid(const SERVICES_ID &svc_id, SERVICES_INFO  &svc_info) const;


    ///�������ж�ȡZERG������
    int get_zerg_cfg(const ZCE_Conf_PropertyTree *conf_tree);


    ///�õ�ĳ�������ļ���������Ϣ,�����ļ�������[SERVICES_TABLE]�ֶ�
    int get_svcidtable_cfg(const ZCE_Conf_PropertyTree *conf_tree);


public:
    ///ZERG�������ļ�
    std::string zerg_cfg_file_;

    ///ZERG����������
    ZERG_CONFIG_DATA  zerg_cfg_data_;

    ///��������ü��ϱ�
    SERVICES_INFO_TABLE  services_info_table_;
};

#endif //_ZERG_SERVER_CONFIG_H_

