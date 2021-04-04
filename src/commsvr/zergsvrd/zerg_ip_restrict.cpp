
#include "zerg_predefine.h"
#include "zerg_configure.h"
#include "zerg_ip_restrict.h"


//ʵ��
Zerg_IPRestrict_Mgr *Zerg_IPRestrict_Mgr::instance_ = NULL;

/****************************************************************************************************
class  Zerg_IPRestrict_Mgr ����ͨѶ�м��IP��������
****************************************************************************************************/
Zerg_IPRestrict_Mgr::Zerg_IPRestrict_Mgr()
{
}
//�Լ���������ͣ�ͳһ�ر���handle_close,����ط����ùر�
Zerg_IPRestrict_Mgr::~Zerg_IPRestrict_Mgr()
{
}

//�������ļ��еõ���ص�����
int Zerg_IPRestrict_Mgr::get_config(const Zerg_Server_Config *config)
{
    int ret = 0;
    //unsigned int tmp_uint= 0;

    allow_ip_set_.clear();
    reject_ip_set_.clear();

    std::vector<std::string> v;
    zce::str_split(config->zerg_cfg_data_.allow_ip_.c_str(), "|", v);
    allow_ip_set_.rehash(v.size());

    //��ȡ�������ӵķ�����IP��ַ
    for (unsigned int i = 0; i < v.size(); ++i )
    {
        ZCE_Sockaddr_In     inetadd;
        //0��һ��Ĭ�϶˿�
        ret = inetadd.set(v[i].c_str(), 0);
        if (0 != ret)
        {
            return -1;
        }

        allow_ip_set_.insert(inetadd.get_ip_address());
    }

    //��ȡ�ܾ����ӵķ�����IP��ַ
    v.clear();
    zce::str_split(config->zerg_cfg_data_.reject_ip_.c_str(), "|", v);
    reject_ip_set_.rehash(v.size());

    for (unsigned int i = 0; i < v.size(); ++i)
    {
        ZCE_Sockaddr_In     inetadd;
        ret = inetadd.set(v[i].c_str(), 0);
        if (0 != ret)
        {
            return -1;
        }
        reject_ip_set_.insert(inetadd.get_ip_address());
    }

    return 0;
}

int Zerg_IPRestrict_Mgr::check_iprestrict(const ZCE_Sockaddr_In &remoteaddress)
{
    const size_t IP_ADDR_LEN = 32;
    char ip_addr_str[IP_ADDR_LEN + 1];
    size_t use_len = 0;
    //�����������ӵķ�������ַ�м�û��... kill
    if (allow_ip_set_.empty() == false )
    {
        Set_Of_IPAddress::iterator iter = allow_ip_set_.find(remoteaddress.get_ip_address());

        if ( iter == allow_ip_set_.end() )
        {
            ZCE_LOG(RS_INFO, "[zergsvr] A NO Allowed IP|Port : [%s] Connect me.",
                    remoteaddress.to_string(ip_addr_str,IP_ADDR_LEN,use_len));
            return SOAR_RET::ERR_ZERG_IP_RESTRICT_CHECK_FAIL;
        }
    }

    //�����Ҫ���ܾ����ӵ�IP��ַ...kill
    if (reject_ip_set_.empty() == false )
    {
        Set_Of_IPAddress::iterator iter = reject_ip_set_.find(remoteaddress.get_ip_address());

        if ( iter != reject_ip_set_.end() )
        {
            ZCE_LOG(RS_INFO, "[zergsvr] Reject IP|Port : %s connect me.",
                    remoteaddress.to_string(ip_addr_str,IP_ADDR_LEN,use_len));
            return SOAR_RET::ERR_ZERG_IP_RESTRICT_CHECK_FAIL;
        }
    }

    return 0;
}

//����ʵ������
Zerg_IPRestrict_Mgr *Zerg_IPRestrict_Mgr::instance()
{
    //���û�г�ʼ��
    if (instance_ == NULL)
    {
        instance_ = new Zerg_IPRestrict_Mgr();
    }

    return instance_;
}

//����ʵ��
void Zerg_IPRestrict_Mgr::clean_instance()
{
    if (instance_)
    {
        delete instance_;
        instance_ = NULL;
    }
}

