
#ifndef OGRE4A_IP_RESTRICT_H_
#define OGRE4A_IP_RESTRICT_H_

/****************************************************************************************************
class  Ogre4aIPRestrictMgr
****************************************************************************************************/
class Ogre_IPRestrict_Mgr
{

    typedef std::unordered_set<unsigned int> SET_OF_IPADDRESS;

protected:

    Ogre_IPRestrict_Mgr();
    ~Ogre_IPRestrict_Mgr();

public:

    ///�������ļ��еõ���ص�����
    int get_config(const Ogre_Server_Config *config);
    //���IP����
    int check_ip_restrict(const ZCE_Sockaddr_In &remoteaddress);

public:

    //����ʵ������
    static Ogre_IPRestrict_Mgr *instance();
    //������ʵ��
    static void clean_instance();

protected:

    //����������ӵ�IP��ַ
    SET_OF_IPADDRESS                  allow_ip_set_;
    //�ܾ��������ӵ�IP��ַ
    SET_OF_IPADDRESS                  reject_ip_set_;

protected:
    //����ʵ��
    static Ogre_IPRestrict_Mgr     *instance_;
};

#endif //OGRE4A_IP_RESTRICT_H_

