#include "ogre/predefine.h"
#include "ogre/configure.h"
#include "ogre/ip_restrict.h"

namespace ogre
{
//单子实例
ip_restrict* ip_restrict::instance_ = nullptr;

/****************************************************************************************************
class  OgreIPRestrict 处理通讯中间的IP限制问题
****************************************************************************************************/
ip_restrict::ip_restrict()
{
}
//自己清理的类型，统一关闭在event_close,这个地方不用关闭
ip_restrict::~ip_restrict()
{
}

//从配置文件中得到相关的配置
int ip_restrict::get_config(const configure* config)
{
    int ret = 0;
    //unsigned int tmp_uint= 0;

    allow_ip_set_.clear();
    reject_ip_set_.clear();

    std::vector<std::string> v;
    zce::str_split(config->ogre_cfg_data_.allow_ip_.c_str(), "|", v);
    allow_ip_set_.rehash(v.size());

    //读取运行连接的服务器IP地址
    for (unsigned int i = 0; i < v.size(); ++i)
    {
        zce::skt::addr_in     inetadd;
        //0是一个默认端口
        ret = inetadd.set(v[i].c_str(), 0);
        if (0 != ret)
        {
            return -1;
        }

        allow_ip_set_.insert(inetadd.get_ip_address());
    }

    //读取拒绝连接的服务器IP地址
    v.clear();
    zce::str_split(config->ogre_cfg_data_.reject_ip_.c_str(), "|", v);
    reject_ip_set_.rehash(v.size());

    for (unsigned int i = 0; i < v.size(); ++i)
    {
        zce::skt::addr_in     inetadd;
        ret = inetadd.set(v[i].c_str(), 0);
        if (0 != ret)
        {
            return -1;
        }
        reject_ip_set_.insert(inetadd.get_ip_address());
    }

    return 0;
}

//
int ip_restrict::check_ip_restrict(const zce::skt::addr_in& remoteaddress)
{
    const size_t IP_ADDR_LEN = 31;
    char ip_addr_str[IP_ADDR_LEN + 1];
    size_t use_len = 0;
    //如果允许的连接的服务器地址中间没有... kill
    if (allow_ip_set_.empty() == false)
    {
        std::unordered_set<unsigned int>::iterator iter = allow_ip_set_.find(remoteaddress.get_ip_address());

        if (iter == allow_ip_set_.end())
        {
            ZCE_LOG(RS_INFO, "A NO Allowed IP|Port : %s connect me.\n",
                    remoteaddress.to_str(ip_addr_str, IP_ADDR_LEN, use_len));
            return SOAR_RET::ERR_OGRE_IP_RESTRICT_CHECK_FAIL;
        }
    }

    //如果是要被拒绝连接的IP地址...kill
    if (reject_ip_set_.empty() == false)
    {
        std::unordered_set<unsigned int>::iterator iter = reject_ip_set_.find(remoteaddress.get_ip_address());

        if (iter != reject_ip_set_.end())
        {
            ZCE_LOG(RS_INFO, "Reject IP|Port : %s connect me.\n",
                    remoteaddress.to_str(ip_addr_str, IP_ADDR_LEN, use_len));
            return SOAR_RET::ERR_OGRE_IP_RESTRICT_CHECK_FAIL;
        }
    }

    return 0;
}

//单子实例函数
ip_restrict* ip_restrict::instance()
{
    //如果没有初始化
    if (instance_ == nullptr)
    {
        instance_ = new ip_restrict();
    }

    return instance_;
}

//清理实例
void ip_restrict::clear_inst()
{
    if (instance_)
    {
        delete instance_;
        instance_ = nullptr;
    }
}
} //namespace ogre