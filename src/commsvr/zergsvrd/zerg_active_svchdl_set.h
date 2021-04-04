
#ifndef ZERG_TCP_SERVICES_INFO_SET_H_
#define ZERG_TCP_SERVICES_INFO_SET_H_



class TCP_Svc_Handler;




/*!
* @brief      SVC ID SET
*
* @note       ��˼һ��,�Ƿ�Ӧ���������װ����Ҳ��ì�ܣ������TCP_Svc_Handlerֱ��ʹ��STL�ķ�װ���ݣ�
*             ��ô���ǵķ�װ���پۺ͵ķ�ʽ������ô�ĺá���TCP_Svc_Handler��Ҫ�ٴη�װ��������
*/
class Active_SvcHandle_Set
{


public:
    ///���캯��,
    Active_SvcHandle_Set();
    ///��������
    ~Active_SvcHandle_Set();

    //��ʼ��
    void initialize(size_t sz_peer);


    /*!
    * @brief      ����SVCID����Active��Handle�����ѯ�Ƿ�����
    * @return     int ==0 ��ʾ��ѯ�ɹ���
    * @param[in]  svrinfo    ��ѯ��SERVICES_ID,
    * @param[out] svc_handle ���صĶ�Ӧ��handle
    */
    int find_handle_by_svcid(const SERVICES_ID &svc_id,
                             TCP_Svc_Handler *&svc_handle);

    /*!
    * @brief      �Ը��ؾ���ķ�ʽ������services type��ѯһ����SVC��������������˳����ѯ�ķ��أ�
    *             lbseq(Load Balance sequence)
    * @return     int  ==0��ʾ��ѯ�ɹ�
    * @param[in]  services_type ����������
    * @param[out] services_id   ��ѯ����SVC ID
    * @param      svc_handle    ���ض�Ӧ��Handle
    * @note       ������ѯ��֤���͵����ݾ������ؾ��⣬
    */
    int find_lbseqhdl_by_type(uint16_t services_type,
                              uint32_t &services_id,
                              TCP_Svc_Handler *&svc_handle);


    /*!
    * @brief
    * @return     int
    * @param      services_type
    * @param      lb_factor
    * @param      services_id
    * @param      svc_handle
    */
    int find_lbfactorhdl_by_type(uint16_t services_type,
                                 uint32_t lb_factor,
                                 uint32_t &services_id,
                                 TCP_Svc_Handler *&svc_handle);


    /*!
    * @brief      �������ķ�ʽ������services type������ѯ�õ�һ����SVC ID�Լ���Ӧ��Handle��
    *             ֻ��������AUTO CONNECT���õ����ӳ�ȥ�ķ�����������˳�����AUTO CONNECT����
    *             ˳�������������ǰ��ķ��������ȿ��� MS(Main Standby)
    * @return     int ==0��ʾ��ѯ�ɹ�
    * @param[in]  services_type ����������
    * @param[out] find_services_id  ��ѯ����SVC ID
    * @param[out] svc_handle    ���ض�Ӧ��Handle
    * @note       ���������������⣬ѡ�񱸷ݡ��������Ա�֤���͵����ݶ�����������һ��������
    *             ע������ģʽ�����õ�������������Ҫ��������(<=4)��
    */
    int find_mshdl_by_type(uint16_t services_type,
                           uint32_t &find_services_id,
                           TCP_Svc_Handler *&svc_handle);


    /*!
    * @brief      ��ѯ���Ͷ�Ӧ������active��SVC ID���飬���ڹ㲥��
    * @return     int       ==0��ʾ��ѯ�ɹ�
    * @param      services_type  ��ѯ�ķ���������
    * @param      id_ary    ������Ͷ�Ӧ������active��SVC ID��services_id��ֵ
    * @note       ���Զ�ĳ�����ͽ��й㲥
    */
    int find_hdlary_by_type(uint16_t services_type, std::vector<uint32_t> *&id_ary);

    /*!
    * @brief      ��������������Ϣ
    * @return     int
    * @param      svc_id      �����ӵķ�������SVC ID
    * @param      new_svchdl  �����ӵķ������ľ��
    */
    int add_services_peerinfo(const SERVICES_ID &svc_id,
                              TCP_Svc_Handler *new_svchdl);


    /*!
    * @brief
    * @return     int        ��������������Ϣ
    * @param      svc_id     �����ӵķ�������SVC ID
    * @param      new_svchdl �����ӵķ��������
    * @param      old_svchdl ���ԭ����һ��svrinfo��Ӧ��Hdler,����֪ͨ��,
    * @note       ���ص�old_svchdl,������������
    */
    int replace_services_peerInfo(const SERVICES_ID &svc_id,
                                  TCP_Svc_Handler *new_svchdl,
                                  TCP_Svc_Handler *&old_svchdl);


    /*!
    * @brief      ����SERVICES_ID,ɾ��PEER��Ϣ,
    * @return     int    ==0��ʾɾ���ɹ�
    * @param      svc_id   Ҫɾ����������SVC ID
    * @note
    */
    int del_services_peerInfo(const SERVICES_ID &svc_id);

    ///��ǰ������
    size_t get_services_peersize();

    ///
    void dump_svr_peerinfo(ZCE_LOG_PRIORITY out_lvl);

    ///�ر����е�PEER
    void clear_and_closeall();


protected:



    struct SERVICES_ID_TABLE
    {
        ///
        size_t orderid_use_id_ = 0;
        ///
        std::vector<uint32_t>  services_id_ary_;
    };

    ///
    typedef std::unordered_map<SERVICES_ID, TCP_Svc_Handler *, HASH_OF_SVCID> MAP_OF_SVCPEERINFO;

    ///���ڸ���TYPEѡ��һ����������������߸���TYPE�㲥������������͵ķ�����
    typedef std::unordered_map<uint16_t, SERVICES_ID_TABLE > MAP_OF_TYPE_TO_IDTABLE;


    ///
    size_t max_peer_size_ = 0;

    ///
    MAP_OF_SVCPEERINFO  svr_info_set_;
    ///
    MAP_OF_TYPE_TO_IDTABLE type_to_idtable_;
};



#endif //_ZERG_TCP_SERVICES_INFO_SET_H_


