#include "soar/predefine.h"
#include "soar/stat/monitor.h"
#include "soar/enum/error_code.h"
#include "soar/stat/define.h"

/******************************************************************************************
class stat_monitor 单线程版本的实例
******************************************************************************************/
namespace soar
{
stat_monitor* stat_monitor::instance_ = nullptr;

stat_monitor* stat_monitor::instance()
{
    if (instance_ == nullptr)
    {
        instance_ = new stat_monitor();
    }

    return instance_;
}

void stat_monitor::clear_inst()
{
    delete instance_;
    instance_ = nullptr;
}

//初始化,由于小虫和业务服务器以相同ID的共存，所以用了一个前缀
int stat_monitor::initialize(const char* app_base_name,
                             const soar::SERVICES_INFO& service_info,
                             size_t num_stat_item,
                             const zce::STATUS_ITEM_WITHNAME item_ary[],
                             bool mutli_thread)
{
    create_stat_fname(app_base_name, service_info);

    // 将stat_mmap_filename_转换为大写
    zce::strupr(stat_mmap_filename_);

    int ret = zce::server_status::initialize(stat_mmap_filename_,
                                             num_stat_item,
                                             item_ary,
                                             mutli_thread);

    return ret;
}

//生产stat文件名称
void stat_monitor::create_stat_fname(const char* app_base_name,
                                     const soar::SERVICES_INFO& service_info)
{
    snprintf(stat_mmap_filename_,
             STAT_MMAP_FILENAME_LEN,
             "STATS_%s_%hu_%hu.%u.SHM",
             app_base_name,
             service_info.business_id_,
             service_info.svc_id_.services_type_,
             service_info.svc_id_.services_id_);

    stat_mmap_filename_[STAT_MMAP_FILENAME_LEN] = '\0';
}

//从文件名称中得到相应的信息
int stat_monitor::get_info_from_fname(const char* stat_file_name,
                                      unsigned int* business_id,
                                      soar::SERVICES_ID* svc_id,
                                      char* app_base_name)
{
    ZCE_ASSERT(stat_file_name != nullptr);
    ZCE_ASSERT(svc_id != nullptr);

    int ret = 0;
    char file_name[STAT_MMAP_FILENAME_LEN + 1];
    strncpy(file_name, stat_file_name, STAT_MMAP_FILENAME_LEN);
    file_name[STAT_MMAP_FILENAME_LEN] = '\0';

    //检查文件长度
    const size_t MIN_STATS_FILENAME_LEN = 14;
    size_t name_len = strlen(file_name);
    if (name_len < MIN_STATS_FILENAME_LEN)
    {
        return SOAR_RET::ERROR_BAD_STAT_FILE_NAME;
    }

    //检查文件名称前缀
    if (strncmp(file_name, "STATS_", 6) != 0)
    {
        return SOAR_RET::ERROR_BAD_STAT_FILE_NAME;
    }
    //检查后缀
    if (strncmp(file_name + name_len - 4, ".SHM", 4) != 0)
    {
        return SOAR_RET::ERROR_BAD_STAT_FILE_NAME;
    }

    //从后面开始找_
    soar::SERVICES_ID tmp_svc_id;
    char* find_pos = nullptr;
    //反向查询的，先解决svc id，
    find_pos = strrchr(file_name, '_');
    if (nullptr == find_pos)
    {
        return SOAR_RET::ERROR_BAD_STAT_FILE_NAME;
    }
    ret = sscanf(find_pos + 1, "%hu.%u",
                 &tmp_svc_id.services_type_,
                 &tmp_svc_id.services_id_);
    //!=2 表示没有得到两个数字
    if (ret != 2)
    {
        return SOAR_RET::ERROR_BAD_STAT_FILE_NAME;
    }

    *svc_id = tmp_svc_id;

    //再处理业务ID
    *find_pos = '\0';
    unsigned int tmp_business_id;
    find_pos = strrchr(file_name, '_');
    if (nullptr == find_pos)
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

    //得到APP的名字
    *find_pos = '\0';
    strncpy(app_base_name, file_name + 6, STAT_MMAP_FILENAME_LEN);

    return 0;
}
}