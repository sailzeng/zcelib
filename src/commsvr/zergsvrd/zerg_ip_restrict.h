
#ifndef ZERG_IP_RESTRICT_H_
#define ZERG_IP_RESTRICT_H_

class Zerg_Server_Config;



/*!
* @brief
*
* @note
*/
class Zerg_IPRestrict_Mgr
{

    typedef std::unordered_set<unsigned int> Set_Of_IPAddress;

protected:
    //
    Zerg_IPRestrict_Mgr();
    ~Zerg_IPRestrict_Mgr();

public:

    ///�������ļ��еõ���ص�����
    int get_config(const Zerg_Server_Config *config);
    ///���IP����
    int check_iprestrict(const ZCE_Sockaddr_In &remoteaddress);

public:

    ///����ʵ������
    static Zerg_IPRestrict_Mgr *instance();
    ///������ʵ��
    static void clean_instance();

protected:
    ///����ʵ��
    static Zerg_IPRestrict_Mgr        *instance_;

protected:

    //����������ӵ�IP��ַ
    Set_Of_IPAddress           allow_ip_set_;
    //�ܾ��������ӵ�IP��ַ
    Set_Of_IPAddress           reject_ip_set_;

};

#endif //_ZERG_IP_RESTRICT_H_

