#ifndef ZCE_LIB_SOCKET_ADDR_IN_
#define ZCE_LIB_SOCKET_ADDR_IN_

#include "zce_socket_addr_base.h"



/*!
* @brief      IPv4������ṹ
*
* @note
*/
class ZCE_Sockaddr_In : public ZCE_Sockaddr
{
public:

    ///Ĭ�Ϲ��캯��
    ZCE_Sockaddr_In (void);

    ///����sockaddr_in���죬
    ZCE_Sockaddr_In (const sockaddr_in *addr);

    ///����IP��ַ(XXX.XXX.XXX.XXX)�ַ������˿ںų�ʼ������
    ZCE_Sockaddr_In (const char *ip_addr_str,
                     uint16_t port_number);

    ///���ݵ�ַ(����)���˿ںų�ʼ������
    ZCE_Sockaddr_In (uint32_t ip_addr,
                     uint16_t port_number);

    ///�������죬һ��Ҫд�������Ļ���ָ����ָ���Լ���һ����ַ�ģ�
    explicit ZCE_Sockaddr_In(const ZCE_Sockaddr_In &others);

    ///��������
    virtual ~ZCE_Sockaddr_In();

public:

    ///���õ�ַ��Ϣ
    virtual  void set_sockaddr (sockaddr *addr, socklen_t len);

    /*!
    * @brief      ����IP��ַ(XXX.XXX.XXX.XXX)�ַ������˿ںŸ��ݲ�������
    * @return     int  ����0��ʾת���ɹ�
    * @param      ip_addr_str
    * @param      port_number
    * @note
    */
    int set(const char ip_addr_str[16],
            uint16_t port_number);

    /*!
    * @brief      ���ݵ�ַIP�������˿ں�����
    * @return     int ����0��ʾת���ɹ�
    * @param      ip_addr
    * @param      port_number
    */
    int set(uint32_t ip_addr,
            uint16_t port_number);


    /*!
    * @brief      �����ַ���ȡ��IP��ַ��Ϣ���Լ��˿ں���Ϣ,
    * @return     int  ����0��ʾת���ɹ�
    * @param      ip_addr_str ��ַ�ַ���
    * @note       ����ַ���������#,�ᱻ��Ϊ�ж˿ںţ����û�У��˿ں�Ϊ0
    */
    int set(const char *ip_addr_str);

    ///���ö˿ںã�
    inline void set_port_number (uint16_t);
    ///ȡ�ö˿ں�
    inline uint16_t get_port_number (void) const;

    ///���˿ں��Ƿ���һ����ȫ�˿�
    bool check_safeport();

    //�˿ڵĴ�ӡ���ʹ��to_string����


    //ȡ��IP��ַ����������
    inline uint32_t get_ip_address (void) const;

    ///����ַ�Ƿ���һ��������ַ
    bool is_internal();

    ///����ַ�Ƿ���һ��������ַ����ʵ�Ҿͼ���Ϊ����������ַ����������ַ
    bool is_internet();

    //�Ƚ�������ַ�Ƿ����
    bool operator == (const ZCE_Sockaddr_In &others) const;
    //�Ƚ�������ַ�Ƿ����
    bool operator != (const ZCE_Sockaddr_In &others) const;

    //���IP��ַ�Ƿ����,���Ӷ˿�
    bool is_ip_equal (const ZCE_Sockaddr_In &others) const;

    ///DNS��غ�����
    ///ȡ��IP��ַ��ص�������Ϣ,���ú�����getnameinfo
    int get_name_info(char *host_name, size_t name_len) const;

    ///ȡ��������ص�IP��ַ��Ϣ�����ú�����getaddrinfo��notename��������ֵ��ַ����������
    int getaddrinfo_to_addr(const char *nodename);

    ///���ֲ�������ת���������������ʹ�ã���ZCE_Sockaddr_In����Ϊ��sockaddr_in����һ��
    ///����sockaddr_in
    operator sockaddr_in () const;
    ///�����ڲ�const sockaddr_in��ָ�룬�������Ա��޸ģ�
    operator const sockaddr_in *() const;
    ///�����ڲ�sockaddr_in��ָ�룬�����Ա��޸ģ�
    operator sockaddr_in *();

protected:

    ///IPV4�ĵ�ַ
    sockaddr_in           in4_addr_;
};

//ȡ��IP��ַ����������
inline uint32_t ZCE_Sockaddr_In::get_ip_address (void) const
{
    return ntohl(in4_addr_.sin_addr.s_addr);
}

//���ö˿ںã�
inline void ZCE_Sockaddr_In::set_port_number (uint16_t port_number)
{
    in4_addr_.sin_port = ntohs(port_number);
}
//ȡ�ö˿ں�
inline uint16_t ZCE_Sockaddr_In::get_port_number (void) const
{
    return ntohs(in4_addr_.sin_port);
}



#endif  //ZCE_LIB_SOCKET_ADDR_IN_

