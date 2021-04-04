/*!
* @copyright  2004-2014  Apache License, Version 2.0 FULLSAIL
* @filename   zce_socket_connector.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2011��6��19��
* @brief      connect�Ĺ���
*
*
* @details
*
*
*
* @note
*
*/
#ifndef ZCE_LIB_SOCKET_CONNECTOR_H_
#define ZCE_LIB_SOCKET_CONNECTOR_H_

#include "zce_boost_non_copyable.h"

class ZCE_Time_Value;
class ZCE_Sockaddr;



/*!
* @brief      connect�Ĺ������ɹ����Ӻ����,���ڲ���ZCE_Socket_Stream
*
* @note
*/
class ZCE_Socket_Connector
{
public:
    //���캯������������
    ZCE_Socket_Connector (void);
    ~ZCE_Socket_Connector (void);

    /*!
    * @brief      ����ʱ������
    * @return     int
    * @param[out] new_stream  ���ص����ӵ�SOCKET �����
    * @param      remote_addr Զ�˵ĵ�ַ��ʵ�ʽṹ��sockadd_in��sockadd_in6��
    * @param      timeout     ��ʱ��ʱ�䳤��
    * @param      reuse_addr  �Ƿ���Ϊreuse_addrѡ��
    * @param      protocol    Э�飬����RAW Socket������û��
    * @param      local_addr  ���ص�BIND��ַ��Ϣ��CONNECT����û��
    */
    int connect (ZCE_Socket_Stream &new_stream,
                 const ZCE_Sockaddr *remote_addr,
                 ZCE_Time_Value  &timeout,
                 bool reuse_addr = false,
                 int protocol = 0,
                 const ZCE_Sockaddr *local_addr = NULL);


    /*!
    * @brief      �������߷�����������
    * @return     int
    * @param      new_stream  ���ص����ӵ�SOCKET �����
    * @param      remote_addr Զ�˵ĵ�ַ��ʵ�ʽṹ��sockadd_in��sockadd_in6��
    * @param      non_blocing �Ƿ������������ӡ�
    * @param      reuse_addr  �Ƿ���Ϊreuse_addrѡ��
    * @param      protocol    Э�飬����RAW Socket������û��
    * @param      local_addr  ���ص�BIND��ַ��Ϣ��CONNECT����û��
    */
    int connect (ZCE_Socket_Stream &new_stream,
                 const ZCE_Sockaddr *remote_addr,
                 bool non_blocing,
                 bool reuse_addr = false,
                 int protocol = 0,
                 const ZCE_Sockaddr *local_addr = NULL);

};

#endif //#ifndef ZCE_LIB_SOCKET_CONNECTOR_H_

