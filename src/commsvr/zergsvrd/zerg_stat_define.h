/// @file       zerg_stat_define.h
/// @date       2012/07/21 14:02
///
/// @author     yunfeiyang
///
/// @brief      С�������
///
#ifndef ZERG_STAT_DEFINE_H_
#define ZERG_STAT_DEFINE_H_

//��ص�FEATURE_ID
enum ZERG_MONITOR_FEATURE_ID
{
    ZERG_SERVICE_STAT_BEGIN =  9000,
    ZERG_ACCEPT_PEER_COUNTER,               //��������Ѿ�Accept��PEER����
    ZERG_ACCEPT_PEER_NUMBER,                //����Accept��PEER����,����ֵ
    ZERG_CONNECT_PEER_COUNTER,              //��������Ѿ�Connect��PEER����
    ZERG_CONNECT_PEER_NUMBER,               //����Connect��PEER����,����ֵ
    ZERG_BUFFER_STORAGE_NUMBER,             //BUFFER��������CHUNK����,����ֵ
    ZERG_SEND_FRAME_COUNTER,                //������ڴӷ��͹ܵ�ȡ�����뷢�Ͷ��е�֡������
    ZERG_RECV_FRAME_COUNTER,                //������ڳɹ����շ�����չܵ���֡����
    ZERG_RECV_BYTES_COUNTER,                //������ڳɹ��յ��ֽ�������,�Ǿ�ȷֵ,��Ϊÿ��PEER���Ǹ�һ��ʱ��ͳ��һ��
    ZERG_SEND_BYTES_COUNTER,                //������ڳɹ����͵��ֽ�����,�Ǿ�ȷֵ
    ZERG_RECV_SUCC_COUNTER,                 //������ڽ��ܳɹ�֡����,,�Ǿ�ȷֵ
    ZERG_SEND_SUCC_COUNTER,                 //������ڷ��ͳɹ�֡����,�Ǿ�ȷֵ
    ZERG_RECV_FAIL_COUNTER,                 //������ڽ���ʧ�ܵ�����֡����
    ZERG_SEND_FAIL_COUNTER,                 //������ڷ�������ʧ�ܵ�����
    ZERG_RECV_BLOCK_COUNTER,                //������ڽ������ݷ�������������
    ZERG_SEND_BLOCK_COUNTER,                //������ڷ������ݷ�������������
    ZERG_UDP_RECV_COUNTER,                  //��������յ�UDP���ݵĴ���
    ZERG_UDP_SEND_COUNTER,                  //������ڷ���UDP���ݵĴ���
    ZERG_UDP_RECV_BYTES_COUNTER,            //��������յ�UDP�����ֽڵ�����
    ZERG_UDP_SEND_BYTES_COUNTER,            //��������ڷ���UDP�����ֽڵ�����
    ZERG_UDP_RECV_FAIL_COUNTER,             //��������յ�UDP���ݴ������
    ZERG_UDP_SEND_FAIL_COUNTER,             //������ڷ���UDP���ݴ������
    ZERG_RECV_PIPE_FULL_COUNTER,            //��������ڽ������ݹܵ�������ļ���,���ն�ʧ���ݸ���
    ZERG_RECV_PIPE_FULL_NUMBER,             //�������ݹܵ���������ۼ�ֵ
    ZERG_SEND_LIST_FULL_COUNTER,            //������� TCP�ķ��Ͷ������ˣ����Ͷ�ʧ���ݸ���
    ZERG_SEND_LIST_FULL_NUMBER,             //TCP�ķ��Ͷ������˵��ۼ�ֵ

    // �벻Ҫ����������м������
    ZERG_HEART_BEAT_RECV_TIME_GAP,          // ��������������շ�ʱ����
    ZERG_HEART_BEAT_RECV_COUNT,             // ��������������շ�����
    ZERG_HEART_BEAT_LESS_FIFTY_COUNT,          // ��������������շ�ʱ��������50ms����
    ZERG_HEART_BEAT_LESS_HNDRD_COUNT,          // ��������������շ�ʱ��������100ms����
    ZERG_HEART_BEAT_LESS_FIFHNDRD_COUNT,          // ��������������շ�ʱ��������500ms����
    ZERG_HEART_BEAT_MORE_FIFHNDRD_COUNT,          // ��������������շ�ʱ�������ڵ���500ms����

    ZERG_HEART_BEAT_APP_RECV_TIME_GAP,          // �����������APP�շ�ʱ����
    ZERG_HEART_BEAT_APP_RECV_COUNT,             // �����������APP�շ�����
    ZERG_HEART_BEAT_APP_LESS_FIFTY_COUNT,          // �����������APP�շ�ʱ��������50ms����
    ZERG_HEART_BEAT_APP_LESS_HNDRD_COUNT,          // �����������APP�շ�ʱ��������100ms����
    ZERG_HEART_BEAT_APP_LESS_FIFHNDRD_COUNT,          // �����������APP�շ�ʱ��������500ms����
    ZERG_HEART_BEAT_APP_MORE_FIFHNDRD_COUNT,          // �����������APP�շ�ʱ�������ڵ���500ms����
    ZERG_SEND_FRAME_COUNTER_BY_SVR_TYPE,                //������ڴӷ��͹ܵ�ȡ�����뷢�Ͷ��е�֡����,�ַ�������
    ZERG_RECV_FRAME_COUNTER_BY_SVR_TYPE,                //������ڳɹ����շ�����չܵ���֡����,�ַ�������
    ZERG_SEND_FRAME_COUNTER_BY_CMD,                     //������ڴӷ��͹ܵ�ȡ�����뷢�Ͷ��е�֡����,��������
    ZERG_RECV_FRAME_COUNTER_BY_CMD,                     //������ڳɹ����շ�����չܵ���֡����,��������
    ZERG_SEND_FAIL_COUNTER_BY_SVR_TYPE,
    ZERG_SEND_LIST_FULL_COUNTER_BY_SVR_TYPE,            // ���Ͷ��������µ��¶������������ַ���ID

    // ��������
    ZERG_SERVICE_STAT_END,                  //
};

#endif // ZERG_STAT_DEFINE_H_
