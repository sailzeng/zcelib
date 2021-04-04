
//Jovi(HuangHao),Sail(ZENGXING)

#ifndef OGRE_SERVER_AUTO_CONNECT_H_
#define OGRE_SERVER_AUTO_CONNECT_H_


#include "ogre_configure.h"

class Ogre_TCP_Svc_Handler;


/*!
* @brief      �������������������Ĺ�����
*
* @note
*/
class Ogre_Connect_Server
{
public:

    //���캯��
    Ogre_Connect_Server();
    ~Ogre_Connect_Server();

    ///��ȡ����
    int get_config(const Ogre_Server_Config *config);

    /*!
    * @brief      �����еķ�����������������
    * @return     int == 0��ʾ�ɹ�
    * @param      num_vaild   ��Ȼ��Ч���������ӵķ���������
    * @param      num_succ    �ɹ��������ӵķ�����������
    * @param      num_fail    ʧ�ܵ�����
    */
    int connect_all_server(size_t &num_vaild, size_t &num_succ, size_t &num_fail);

    /*!
    * @brief      ����SVRINFO,����Ƿ����������ӵķ���.����������
    * @return     int  == 0��ʾ�ɹ�
    * @param      peer_id Ҫ���ӷ�������PEER ID
    */
    int connect_server_by_peerid(const OGRE_PEER_ID &peer_id);

protected:
    /*!
    * @brief      ����ĳ��������
    * @return     int == 0��ʾ�ɹ�
    * @param      peer_module ��������������Ϣ
    */
    int connect_one_server(const TCP_PEER_MODULE_INFO &peer_module);

protected:

    typedef std::unordered_set < TCP_PEER_MODULE_INFO,
            HASH_OF_PEER_MODULE,
            EQUAL_OF_PEER_MODULE > SET_OF_TCP_PEER_MODULE;

    ///������
    ZCE_Socket_Connector ogre_connector_;

    ///
    size_t auto_connect_num_ = 0;
    ///Ҫ�������ӵ�PEER��SET����SET�ǲ�ѯ����
    SET_OF_TCP_PEER_MODULE     autocnt_module_set_;

};

#endif //OGRE_SERVER_AUTO_CONNECT_H_

