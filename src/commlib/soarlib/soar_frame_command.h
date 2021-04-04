/*!
* @copyright  2004-2014  Apache License, Version 2.0 FULLSAIL
* @filename   soar_frame_command.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2006��10��20��
* @brief
*
*
* @details    README:
* �����ڸ������ʱ���Ȱ�Ŀ��Ͷ�����
* �����Ĺ���
* 1.�����ڲ�����,���е��������������Ϊ����,Ӧ��Ϊż��,������ǿͻ��˷��͸���������Ӧ����Ƿ��������͸��ͻ���
* 2.���û��һ������û�����󣬻���Ӧ���뱣��ռλ��
* 3.û������ԭ���밴��˳�����
* 4.�ڲ����������������ϣ�
* 5.������REQ��β,Ӧ����RSP
* 6.ÿ��������������һ��_BEGIN,_END�ĺ���������.
* 7.���е������ֲ��ö�Ӧ��������TYPE*10000+����ID�ķ�ʽ����.1-9999��ΪͨѶ��������ҵ�����������ڲ�����
* 8.�������͸��ͻ��˵���RSP��ͷ
* 9.�鷳���룬�Һò����ܹ��Զ���ģ����ϸ㻵�ˣ��һửȦȦ������ġ�
*
*
*
* @note
*
*/


#ifndef SOARING_LIB_APPFRAME_COMMAND_H_
#define SOARING_LIB_APPFRAME_COMMAND_H_

#if 1

enum PET_APPFRAME_CMD
{
    //------------------------------------------------------------------------------------------------
    CMD_INVALID_CMD                            = 0,               //��Ч������,���ڳ�ʼ�����߱�ʾ

    //------------------------------------------------------------------------------------------------
    //ZERG��������ҵ�������ͨ���ڲ������� �ڲ��������ͨѶ��������ҵ����̼�Ĵ���
    //------------------------------------------------------------------------------------------------
    INTERNAL_COMMAND_BEGIN                     = 1,            //��ʼ
    INNER_REG_SOCKET_CLOSED                    = 3,            //�ڲ�����,֪ͨ����Ӧ��,ĳ��Socket�ر�
    INNER_RSP_SOCKET_CLOSED                    = 4,            //ռλ
    INNER_REQ_CLOSE_SOCKET                     = 5,            //ռλ
    INNER_RSP_CLOSE_SOCKET                     = 6,            //�ڲ�����,����ķ�����֪ͨ���������,�رն˿�
    INNER_REQ_ERROR_SEND_SOCKET                = 7,            //�ڲ�����,ĳ������δ������,���ʹ���ͬ־��˷���ԭ���ù�
    INNER_RSP_ERROR_SEND_SOCKET                = 8,            //ռλ
    INNER_REQ_ACCEPT_SOCKET                    = 9,            //�ڲ�����,��PEER���ӵ��ҵķ���˿ڣ������Ѿ�ע��,δʵ��
    INNER_RSP_ACCEPT_SOCKET                    = 10,           //ռλ
    INNER_REQ_CONNECT_SOCKET                   = 11,           //�ڲ�����,���ӵ��Է���PEER,δʵ��
    INNER_RSP_CONNECT_SOCKET                   = 12,           //ռλ
    INNER_REQ_HANDSHAKE_TIMEOUT                = 13,           //�ڲ�����,���ֳ�ʱ, �����Ҫ������Ӧ����, ��ָʾͨ�ŷ������ر�����,δʵ��
    INNER_RSP_HANDSHAKE_TIMEOUT                = 14,           //ռλ
    INNER_REQ_PEER_TIMEOUT                     = 15,           //�ڲ�����,�ڽ���Ӧ��ʱ�����ڲ���ʱ��Ϣ
    INNER_RSP_PEER_TIMEOUT                     = 16,           //ռλ
    INNER_REQ_RECV_NAKED_UDP_FRAME             = 17,           //�ڲ�����,�յ�һ��UDP�������ݣ���APPFRAME�����󴫵ݸ�ҵ����������������ݴ�����DATA��
    INNER_RSP_RECV_NAKED_UDP_FRAME             = 18,           //ռλ
    INNER_REQ_SEND_NAKED_UDP_FRAME             = 19,           //ռλ
    INNER_RSP_SEND_NAKED_UDP_FRAME             = 20,           //�ڲ�����,ҵ�������Ҫ����һ��UDP�������ݣ�����APPFRAME�������������ݴ�����DATA��
    INTERNAL_COMMAND_END                       = 9999,         //

    //------------------------------------------------------------------------------------------------
    //ͨѶ������������
    ZERG_COMMAND_BEGIN                        = 290000000,               //
    ZERG_CONNECT_REGISTER_REQ                 = 290000001,               //���Ӻ󣬷���һ��ע������,
    ZERG_CONNECT_REGISTER_RSP                 = 290000002,               //ռλ
    ZERG_WITH_SINGATRUE_KEY_REQ               = 290000003,               //�����FRAME����SINGATRUE_KEY����(QQ�����¼)
    ZERG_WITH_SINGATRUE_KEY_RSP               = 290000004,               //ռλ
    ZERG_RESEND_SINGATRUE_KEY_REQ             = 290000005,               //����Ƿ��͸��ͻ��˵ģ�Ҫ��ͻ������·���SINGATRUE_KEY
    ZERG_RESEND_SINGATRUE_KEY_RSP             = 290000006,               //ռλ
    ZERG_REPEAT_LOGIN_KICK_OLD_REQ            = 290000007,               //
    ZERG_REPEAT_LOGIN_KICK_OLD_RSP            = 290000008,               //ռλ
    ZERG_WITH_64_SINGATRUE_KEY_REQ            = 290000009,               //�����FRAME����SINGATRUE_KEY����(0x82��¼)
    ZERG_WITH_64_SINGATRUE_KEY_RSP            = 290000010,               //ռλ
    ZERG_HEART_BEAT_REQ                       = 290000011,               //����,�ڲ���������
    ZERG_HEART_BEAT_RSP                       = 290000012,               //ռλ
    ZERG_ERR_APPFRAME_SESSION_EXPIRE_REQ      = 290000013,               //��ʱ��SESSION����
    ZERG_ERR_APPFRAME_SESSION_EXPIRE_RSP      = 290000014,               //ռλ
    ZERG_WITH_VARIABLE_SINGATRUE_KEY_REQ      = 290000015,               //�����FRAME����SINGATRUE_KEY���� ������ȷ�������Ƕ���λ�� ���³��������ˡ�(ͳһ��¼)
    ZERG_WITH_VARIABLE_SINGATRUE_KEY_RSP      = 290000016,               //ռλ
    ZERG_COMMAND_END                          = 290009999,               //
    //------------------------------------------------------------------------------------------------

};

#endif

#endif //#ifndef SOARING_LIB_APPFRAME_COMMAND_H_

