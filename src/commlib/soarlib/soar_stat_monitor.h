#ifndef SOARING_LIB_MONITOR_STAT_H_
#define SOARING_LIB_MONITOR_STAT_H_

#include "soar_services_info.h"

//ԭ����ʹ��ģ�����������������ģ�����׽��������󻯣�
//�ĳɶ�̬����
class Soar_Stat_Monitor : public ZCE_Server_Status
{



public:
    //gunner�����н��ն��cgiͳ���ϱ�������
    //�����޷�֧��
    Soar_Stat_Monitor();

    ~Soar_Stat_Monitor();

    /*!
    * @brief      ��ʼ��,����С���ҵ�����������ͬID�Ĺ��棬��������һ��ǰ׺
    * @return     int
    * @param[in]  app_base_name ���̵�����
    * @param[in]  business_id   ҵ��ID,
    * @param[in]  service_info  ����ID
    * @param[in]  num_stat_item ͳ����Ŀ����
    * @param[in]  item_ary      ͳ������У���������
    * @param[in]  mutli_thread  �Ƿ��ڶ��̻߳�����ʹ��
    */
    int initialize(const char *app_base_name,
                   unsigned int business_id,
                   const SERVICES_ID &service_id,
                   size_t num_stat_item,
                   const ZCE_STATUS_ITEM_WITHNAME item_ary[],
                   bool mutli_thread);

    /*!
    * @brief      ��stat�ļ���ȡ��service_id ��app name����Ϣ
    * @return     int ==0 ��ʾ�ɹ��������ʾ�ļ����ƴ���
    * @param[in]  stat_file_name �ļ�����
    * @param[out] service_id     ��������SVC ID
    * @param[out] app_base_name  ���̵�����
    * @note
    */
    static int get_info_from_fname(const char *stat_file_name,
                                   unsigned int  *business_id,
                                   SERVICES_ID *service_info,
                                   char *app_base_name);

protected:

    //����stat�ļ�����
    void create_stat_fname(const char *app_base_name,
                           unsigned int business_id,
                           const SERVICES_ID &service_info);


    //���ӵĺ���Ⱥ�������Ҳ�֪��������BOOST��ģ��ʹ�õ��ӣ�����������ֱ����ˬ�����������޸�һЩ
    //�Ҳ���Ϊ�˵��ӿ�����ν�ı������⣬���Լ���֤��ĳ�ʼ��������������
public:
    //
    static Soar_Stat_Monitor *instance();
    //
    static void clean_instance();

protected:
    // ���mmap�ļ�����󳤶�
    static const size_t STAT_MMAP_FILENAME_LEN = 128;

protected:
    // ���mmap�ļ���
    char stat_mmap_filename_[STAT_MMAP_FILENAME_LEN + 1];

protected:
    //����ʵ��
    static Soar_Stat_Monitor *instance_;
};

#endif //SOARING_LIB_MONITOR_STAT_H_

