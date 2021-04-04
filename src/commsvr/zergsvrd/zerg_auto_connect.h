
#ifndef ZERG_SERVER_AUTO_CONNECT_H_
#define ZERG_SERVER_AUTO_CONNECT_H_



class Zerg_Server_Config;
class TCP_Svc_Handler;


/****************************************************************************************************
class Zerg_Auto_Connector
****************************************************************************************************/
class Zerg_Auto_Connector
{

public:

    //���캯��
    Zerg_Auto_Connector();
    ~Zerg_Auto_Connector();

    ///��ȡ����
    int get_config(const Zerg_Server_Config *config);

    // ���¼���������������
    int reload_cfg(const Zerg_Server_Config *config);


    /*!
    * @brief      �������еķ�����,����Ѿ������ӣ�������,
    * @return     void
    * @param      szvalid  ��Ȼ��Ч�ĵ�����
    * @param      sz_succ  �ɹ���ʼ���ӵķ���������
    * @param      szfail   ����ʧ�ܵķ����������������������첽���ӣ�����ط�����һ�����ǽ���������
    * @note
    */
    void reconnect_allserver(size_t &szvalid, size_t &sz_succ, size_t &szfail);


    /*!
    * @brief      ����SVC ID,����Ƿ����������ӵķ���.,
    * @return     int
    * @param      reconnect_svcid Ҫ������������·����Ϣ
    * @note       Ϊʲô����һ��TCP_Svc_Handler��Ϊ��������,��Ϊ�ڷ���Connect������,Ҳ����handle_close.
    */
    int connect_server_bysvcid(const SERVICES_ID &reconnect_svcid);


    /*!
    * @brief      ����services_type��ѯ��Ӧ�����������������б����� MS��������
    * @return     int == 0 ��ʾ�ɹ�
    * @param[in]  services_type
    * @param[out] ms_svcid_ary   ���õ������������б�����
    */
    int find_conf_ms_svcid_ary(uint16_t services_type,
                               std::vector<uint32_t> *&ms_svcid_ary);


    /*!
    * @brief      ������SVC ID�Ƿ����������ӵķ�����
    * @return     bool
    * @param      svc_id SVC ID
    */
    bool is_auto_connect_svcid(const SERVICES_ID &svc_id);

protected:

    //
    /*!
    * @brief      ����SVC ID+IP,����Ƿ����������ӵķ���.����������
    * @return     int
    * @param      svc_id       Ҫ�������ӵ�SVC ID
    * @param      inet_addr    ��ַ
    * @param      svc_handle , ����Ѿ�����Ӧ�����ӣ�������ط����ض��õ�Handle
    */
    int connect_one_server(const SERVICES_ID &svc_id,
                           const ZCE_Sockaddr_In &inet_addr,
                           TCP_Svc_Handler *&svc_handle);

protected:

    //
    typedef std::unordered_set<SERVICES_INFO, HASH_OF_SVCINFO, EQUAL_OF_SVCINFO> SET_OF_SVC_INFO;

    ///���Ͷ�Ӧ��SERVICES ID �����MAP������,
    typedef std::unordered_map<uint16_t, std::vector<uint32_t> > MAP_OF_TYPE_TO_IDARY;

protected:

    //������
    ZCE_Socket_Connector zerg_connector_;

    //����ʵ��ָ��
    const Zerg_Server_Config *zerg_svr_cfg_ = NULL;

    ///�������ӵ�
    size_t size_of_autoconnect_ = 0;

    ///SVC ID ��Ӧary_auto_connect_�������±꣬����ʹ��SVCID�Ĳ�ѯ����ز�ѯ����
    SET_OF_SVC_INFO autocnt_svcinfo_set_;

    //���Ͷ�Ӧ��SERVICES ID ���� ��MAP�����������id��˳����ʵ������˳��
    MAP_OF_TYPE_TO_IDARY type_to_idary_map_;

};


#endif //_ZERG_SERVER_AUTO_CONNECT_H_


