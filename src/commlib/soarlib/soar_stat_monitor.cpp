#include "soar_predefine.h"
#include "soar_stat_monitor.h"
#include "soar_error_code.h"
#include "soar_stat_define.h"

/******************************************************************************************
class Soar_Stat_Monitor ���̰߳汾��ʵ��
******************************************************************************************/
Soar_Stat_Monitor *Soar_Stat_Monitor::instance_ = NULL;



Soar_Stat_Monitor::Soar_Stat_Monitor()
{
}

Soar_Stat_Monitor::~Soar_Stat_Monitor()
{
}

Soar_Stat_Monitor *Soar_Stat_Monitor::instance()
{
    if (instance_ == NULL)
    {
        instance_ = new Soar_Stat_Monitor();
    }

    return instance_;
}

void Soar_Stat_Monitor::clean_instance()
{
    delete instance_;
    instance_ = NULL;
}

//��ʼ��,����С���ҵ�����������ͬID�Ĺ��棬��������һ��ǰ׺
int Soar_Stat_Monitor::initialize(const char *app_base_name,
                                  unsigned int business_id,
                                  const SERVICES_ID &service_info,
                                  size_t num_stat_item,
                                  const ZCE_STATUS_ITEM_WITHNAME item_ary[],
                                  bool mutli_thread)
{
    create_stat_fname(app_base_name, business_id, service_info);

    // ��stat_mmap_filename_ת��Ϊ��д
    zce::strupr(stat_mmap_filename_);

    int ret = ZCE_Server_Status::initialize(stat_mmap_filename_,
                                            num_stat_item,
                                            item_ary,
                                            mutli_thread);

    return ret;
}

//����stat�ļ�����
void Soar_Stat_Monitor::create_stat_fname(const char *app_base_name,
                                          unsigned int business_id,
                                          const SERVICES_ID &service_info)
{
    snprintf(stat_mmap_filename_,
             STAT_MMAP_FILENAME_LEN,
             "STATS_%s_%u_%hu.%u.SHM",
             app_base_name,
             business_id,
             service_info.services_type_,
             service_info.services_id_);

    stat_mmap_filename_[STAT_MMAP_FILENAME_LEN] = '\0';
}


//���ļ������еõ���Ӧ����Ϣ
int Soar_Stat_Monitor::get_info_from_fname(const char *stat_file_name,
                                           unsigned int *business_id,
                                           SERVICES_ID *svc_id,
                                           char *app_base_name)
{

    ZCE_ASSERT(stat_file_name != NULL);
    ZCE_ASSERT(svc_id != NULL);

    int ret = 0;
    char file_name[STAT_MMAP_FILENAME_LEN + 1];
    strncpy(file_name, stat_file_name, STAT_MMAP_FILENAME_LEN);
    file_name[STAT_MMAP_FILENAME_LEN] = '\0';

    //����ļ�����
    const size_t MIN_STATS_FILENAME_LEN = 14;
    size_t name_len = strlen(file_name);
    if (name_len < MIN_STATS_FILENAME_LEN)
    {
        return SOAR_RET::ERROR_BAD_STAT_FILE_NAME;
    }

    //����ļ�����ǰ׺
    if (strncmp(file_name, "STATS_", 6) != 0)
    {
        return SOAR_RET::ERROR_BAD_STAT_FILE_NAME;
    }
    //����׺
    if (strncmp(file_name + name_len - 4, ".SHM", 4) != 0)
    {
        return SOAR_RET::ERROR_BAD_STAT_FILE_NAME;
    }

    //�Ӻ��濪ʼ��_
    SERVICES_ID tmp_svc_id;
    char *find_pos = NULL;
    //�����ѯ�ģ��Ƚ��svc id��
    find_pos = strrchr(file_name, '_');
    if (NULL == find_pos)
    {
        return SOAR_RET::ERROR_BAD_STAT_FILE_NAME;
    }
    ret = sscanf(find_pos + 1, "%hu.%u",
                 &tmp_svc_id.services_type_,
                 &tmp_svc_id.services_id_);
    //!=2 ��ʾû�еõ���������
    if (ret != 2)
    {
        return SOAR_RET::ERROR_BAD_STAT_FILE_NAME;
    }

    *svc_id = tmp_svc_id;

    //�ٴ���ҵ��ID
    *find_pos = '\0';
    unsigned int tmp_business_id;
    find_pos = strrchr(file_name, '_');
    if (NULL == find_pos)
    {
        return SOAR_RET::ERROR_BAD_STAT_FILE_NAME;
    }
    ret = sscanf(find_pos + 1, "%u",
                 &tmp_business_id);
    if (ret != 1)
    {
        return SOAR_RET::ERROR_BAD_STAT_FILE_NAME;
    }
    *business_id = tmp_business_id;

    //�õ�APP������
    *find_pos = '\0';
    strncpy(app_base_name, file_name + 6, STAT_MMAP_FILENAME_LEN);

    return 0;
}

