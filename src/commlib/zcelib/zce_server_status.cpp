#include "zce_predefine.h"
#include "zce_log_logging.h"
#include "zce_server_status.h"
#include "zce_lock_thread_mutex.h"
#include "zce_lock_ptr_guard.h"

ZCE_Server_Status *ZCE_Server_Status::instance_ = NULL;

/******************************************************************************************
class ZCE_STATUS_ITEM_ID
******************************************************************************************/

ZCE_STATUS_ITEM_ID::ZCE_STATUS_ITEM_ID(uint32_t statics_id,
                                       uint32_t classify_id,
                                       uint32_t subclassing_id) :
    statics_id_(statics_id),
    classify_id_(classify_id),
    subclassing_id_(subclassing_id)
{

}

ZCE_STATUS_ITEM_ID::ZCE_STATUS_ITEM_ID():
    statics_id_(0),
    classify_id_(0),
    subclassing_id_(0)
{
}

ZCE_STATUS_ITEM_ID::~ZCE_STATUS_ITEM_ID()
{

}

bool ZCE_STATUS_ITEM_ID::operator == (const ZCE_STATUS_ITEM_ID &others) const
{
    if (this->statics_id_ == others.statics_id_
        && this->classify_id_ == others.classify_id_
        && this->subclassing_id_ == others.subclassing_id_)
    {
        return true;
    }

    return false;
}

/******************************************************************************************
struct ZCE_STATUS_ITEM ״̬��������
******************************************************************************************/
ZCE_STATUS_ITEM::ZCE_STATUS_ITEM():
    statics_type_(STATICS_PER_FIVE_MINTUES),
    counter_(0)
{
}

ZCE_STATUS_ITEM::ZCE_STATUS_ITEM(unsigned int statics_id,
                                 ZCE_STATUS_STATICS_TYPE statics_type):
    item_id_(statics_id, 0, 0),
    statics_type_(statics_type),
    counter_(0)
{
}

ZCE_STATUS_ITEM::~ZCE_STATUS_ITEM()
{
}

/******************************************************************************************
class ZCE_STATUS_ITEM_WITHNAME ״̬������+���֣��������ã�DUMP�����
******************************************************************************************/
ZCE_STATUS_ITEM_WITHNAME::ZCE_STATUS_ITEM_WITHNAME(unsigned int statics_id,
                                                   ZCE_STATUS_STATICS_TYPE statics_type,
                                                   const char *stat_name):
    statics_item_(statics_id, statics_type)
{
    strncpy(item_name_, stat_name, MAX_COUNTER_NAME_LEN);
    item_name_[MAX_COUNTER_NAME_LEN] = '\0';
}

ZCE_STATUS_ITEM_WITHNAME::ZCE_STATUS_ITEM_WITHNAME()
{
    item_name_[MAX_COUNTER_NAME_LEN]='\0';
}

ZCE_STATUS_ITEM_WITHNAME::~ZCE_STATUS_ITEM_WITHNAME()
{

}

/******************************************************************************************
ZCE_Server_Status
******************************************************************************************/

//���캯��,Ҳ���㵥��ʹ�õĻ��ᣬ���Բ���protected
ZCE_Server_Status::ZCE_Server_Status():
    stat_lock_(NULL),
    stat_file_head_(NULL),
    status_stat_sandy_(NULL),
    status_copy_mandy_(NULL),
    multi_thread_guard_(false),
    initialized_(false)
{
}

//
ZCE_Server_Status::~ZCE_Server_Status()
{
    //���������ָ��
    if (stat_lock_)
    {
        delete stat_lock_;
        stat_lock_ = NULL;
    }

    if (status_stat_sandy_)
    {
        delete status_stat_sandy_;
        status_stat_sandy_ = NULL;
    }

    if (status_copy_mandy_)
    {
        delete status_copy_mandy_;
        status_copy_mandy_ = NULL;
    }

}

//��ʼ���ķ���,ͨ�õĵײ㣬
//Param1: char* statfilename MMAPӰ���״̬�ļ�����
//Param2: bool restore_mmap �Ƿ����ڻָ�MMAP������ǻָ����ļ������Ǵ��ڵ�,
int ZCE_Server_Status::initialize(const char *stat_filename,
                                  bool restore_mmap,
                                  bool multi_thread)
{
    //ӳ���ļ����Ʋ���ΪNULL��ͳ����������Ϊ0��ͳ�Ƴ�ʼ�����鲻��ΪNULL
    int ret = 0;

    //��������Ŀռ�
    size_t size_alloc = ARRYA_OF_SHM_STATUS::getallocsize(MAX_MONITOR_STAT_ITEM);

    //�����������������½���һ��״̬�ļ���
    //ԭ����restore_mmap == false�ǣ�����open_mode���沢��һ��O_TRUNC,��û�п��ǵ������ϱ��������
    //һֱ������������ڴ��ļ�,
    //ȥ��������裬�ؽ��������������MMAP���鲿�֣����������ؽ��ļ�
    int open_mode = O_CREAT | O_RDWR;

    //��Ҫ��һ������������ȥ������*2������Ҫһ��ͷ
    ret = stat_file_.open(stat_filename,
                          sizeof(ZCE_STATUS_HEAD) + size_alloc * 2,
                          open_mode);

    if (ret != 0)
    {
        return ret;
    }

    stat_file_head_ = static_cast<ZCE_STATUS_HEAD *>(stat_file_.addr());

    //�����������ֶ��ԣ����������ʹ�ô�����.
    ZCE_ASSERT(NULL == status_stat_sandy_
               && NULL == status_copy_mandy_
               && NULL == stat_lock_ );

    // ͳ����������ʼ��
    char *stat_ptr = static_cast<char *>(stat_file_.addr()) + sizeof(ZCE_STATUS_HEAD);
    status_stat_sandy_ = ARRYA_OF_SHM_STATUS::initialize(MAX_MONITOR_STAT_ITEM,
                                                         stat_ptr,
                                                         restore_mmap);

    if (!status_stat_sandy_)
    {
        return -1;
    }

    // ������������ʼ��
    char *copy_ptr = static_cast<char *>(stat_file_.addr()) + sizeof(ZCE_STATUS_HEAD) + size_alloc;
    status_copy_mandy_ = ARRYA_OF_SHM_STATUS::initialize(MAX_MONITOR_STAT_ITEM,
                                                         copy_ptr,
                                                         restore_mmap);

    if (!status_copy_mandy_)
    {
        return -1;
    }

    //�޸��߳�����������Ϊ
    multi_thread_guard(multi_thread);

    //
    return 0;
}

//����һЩ�����Ŀ����ʱ���ܻ���һ���ּ����Ŀ��������Ŀ����һЩ�����Ŀ
void ZCE_Server_Status::add_status_item(size_t num_add_stat_item,
                                        const ZCE_STATUS_ITEM_WITHNAME item_ary[])
{
    // ��������Ŀռ�, �����󼸸��ռ�Ҳû�й�ϵ
    size_t num_old_stat_item = conf_stat_map_.size();
    conf_stat_map_.rehash(num_old_stat_item + num_add_stat_item + 128);

    for (size_t i = 0; i < num_add_stat_item; ++ i)
    {
        //��дauto����
        STATUS_WITHNAME_MAP::iterator iter = conf_stat_map_.find(item_ary[i].statics_item_.item_id_.statics_id_);
        if (iter == conf_stat_map_.end())
        {
            conf_stat_map_.insert(STATUS_WITHNAME_MAP::value_type(item_ary[i].statics_item_.item_id_.statics_id_,
                                                                  item_ary[i]) );
        }
        else
        {
            ZCE_LOG(RS_ERROR, "Add repeat ZCE_STATUS_ITEM_WITHNAME statics_id_[%u] name[%s],please check your code.",
                    item_ary[i].statics_item_.item_id_.statics_id_,
                    item_ary[i].item_name_);
            continue;
        }
    }
}

bool ZCE_Server_Status::is_exist_stat_id(unsigned int stat_id,
                                         ZCE_STATUS_ITEM_WITHNAME *status_item_withname) const
{
    //��дauto����
    STATUS_WITHNAME_MAP::const_iterator iter = conf_stat_map_.find(stat_id);
    if (iter == conf_stat_map_.end())
    {
        return false;
    }
    *status_item_withname = iter->second;
    return true;
}

//�޸��Ƿ���Ҫ���̱߳���
void ZCE_Server_Status::multi_thread_guard(bool multi_thread)
{

    //������е�����Ȼ���ڣ���ɾ��
    if (stat_lock_)
    {
        delete stat_lock_;
        stat_lock_ = NULL;
    }

    multi_thread_guard_ = multi_thread;

    //���ö�̬����������Ϊ
    if (multi_thread)
    {
        stat_lock_ = new ZCE_Thread_Light_Mutex();
    }
    else
    {
        stat_lock_ = new ZCE_Null_Mutex();
    }
}

//��sandy���������棬��������Ŀ
//����������治Ҫ���������ϲ�ӣ�����Ϊ���������һ�����ú��������ܻᡭ��
int ZCE_Server_Status::find_insert_idx(uint32_t statics_id,
                                       uint32_t classify_id,
                                       uint32_t subclassing_id,
                                       size_t *sandy_idx)
{
    *sandy_idx = static_cast<size_t>(-1);

    ZCE_STATUS_ITEM_ID stat_item_id(statics_id, classify_id, subclassing_id);
    STATID_TO_INDEX_MAP::iterator iter_tmp = statid_to_index_.find(stat_item_id);

    if ( iter_tmp != statid_to_index_.end() )
    {
        *sandy_idx = iter_tmp->second;
        return 0;
    }

    //���ԭ��û���ͳ����Ŀ,�����Ƿ���ҪҪ��ӣ����statics_id_
    STATUS_WITHNAME_MAP::iterator iter = conf_stat_map_.find(statics_id);
    if (iter == conf_stat_map_.end())
    {
        ZCE_LOG(RS_ERROR, "Use one statics_id_,it don't config [%u] ,please check your code.",
                statics_id);
        return -1;
    }


    //����Ѿ����ˣ�Ҳ����
    if (status_stat_sandy_->full())
    {
        ZCE_LOG(RS_ERROR, "Statics array is full,please extend in start. ary size is [%lu]",
                status_stat_sandy_->size());
        return -1;
    }

    size_t idx = status_stat_sandy_->size();

    ZCE_STATUS_ITEM status_item;
    status_item.item_id_ = stat_item_id;
    status_item.counter_ = 0;

    //��������ͳ����Ŀ��ֻ��û��û�����
    status_stat_sandy_->push_back(status_item);

    //��MAP�м������������
    statid_to_index_[stat_item_id] = idx;

    *sandy_idx = idx;
    return 0;
}

//����һ���Ѿ����ڵ��ļ����г�ʼ��,���ڻָ�������,�ļ������Ѿ����ڣ�
int ZCE_Server_Status::initialize(const char *stat_filename, bool multi_thread)
{
    ZCE_ASSERT(stat_filename != NULL );

    int ret = 0;
    //
    ret = initialize(stat_filename, true, multi_thread);

    if ( 0 != ret)
    {
        return ret;
    }

    return 0;
}

//����һ���Ѿ����ڵ��ļ����г�ʼ��,���ڻָ�������,����ļ������Ѿ����ڣ������´���
//Param1: char* statfilename MMAPӰ���״̬�ļ�����
//Param2: size_t num_stat_ary     ״̬�������ĸ���,
//Param3: ZCE_STATUS_ITEM item_ary[] ״̬��������
//Param4: multi_thread ���̻߳���
int ZCE_Server_Status::initialize(const char *stat_filename,
                                  size_t num_stat_item,
                                  const ZCE_STATUS_ITEM_WITHNAME item_ary[],
                                  bool multi_thread)
{
    // �������num_stat_item == 0, ��Ϊ��Щapp�ǲ���Ҫapp��ļ�ص�
    ZCE_ASSERT(stat_filename != NULL);
    //
    int ret = 0;
    ret = initialize(stat_filename, false, multi_thread);

    if ( 0 != ret)
    {
        return ret;
    }

    add_status_item(num_stat_item, item_ary);

    //������һЩ�ռ䣬Ŀ���Ǽ�С��ͻ���ռ任ʱ��
    statid_to_index_.rehash( static_cast<size_t>(MAX_MONITOR_STAT_ITEM * 1.2 ));

    //
    stat_file_head_->monitor_start_time_ = static_cast<uint64_t>(time(NULL));
    stat_file_head_->copy_time_ = static_cast<uint64_t>(time(NULL));
    stat_file_head_->active_time_ = static_cast<uint64_t>(time(NULL));

    initialized_ = true;

    return 0;
}

//���ֵ�޸�mandy����sandyͳ�Ƽ�����ʹ��ͳ��ID�ͷ���ID��Ϊkey,�ӿ�ʹ�÷���һ�㣬�㲻�ü�¼�ܶ��Ӧ��ϵ,���ٶ���һ��,
int ZCE_Server_Status::increase_by_statid(uint32_t statics_id,
                                          uint32_t classify_id,
                                          uint32_t subclassing_id,
                                          int64_t incre_value)
{
    if (!initialized_)
    {
        // δ��ʼ��
        ZCE_ASSERT(0);
        return -1;
    }

    int ret = 0;
    size_t sandy_idx = static_cast<size_t>(-1);

    ZCE_Lock_Ptr_Guard guard(stat_lock_);

    ret = find_insert_idx(statics_id, classify_id, subclassing_id, &sandy_idx);

    if ( 0 != ret )
    {
        return ret;
    }

    //����ͳ����ֵ
    (status_stat_sandy_->begin() + sandy_idx)->counter_ += incre_value;
    return 0;
}

//����ֵ�޸ļ��ͳ����Ŀ��
int ZCE_Server_Status::set_by_statid(uint32_t statics_id,
                                     uint32_t classify_id,
                                     uint32_t subclassing_id,
                                     uint64_t set_value)
{
    if (!initialized_)
    {
        // δ��ʼ��
        ZCE_ASSERT(0);
        return -1;
    }

    int ret = 0;
    size_t sandy_idx = static_cast<size_t>(-1);

    ZCE_Lock_Ptr_Guard guard(stat_lock_);

    ret = find_insert_idx(statics_id, classify_id, subclassing_id, &sandy_idx);

    if ( 0 != ret )
    {
        return ret;
    }

    //����ͳ����ֵ
    (status_stat_sandy_->begin() + sandy_idx)->counter_ = set_value;
    return 0;
}

//����ͳ��ID�ͷ���ID��Ϊkey���õ�ͳ����ֵ
uint64_t ZCE_Server_Status::get_counter(uint32_t statics_id,
                                        uint32_t classify_id,
                                        uint32_t subclassing_id)
{
    ZCE_STATUS_ITEM_ID stat_item_id(statics_id, classify_id, subclassing_id);
    STATID_TO_INDEX_MAP::iterator iter_tmp = statid_to_index_.find(stat_item_id);

    if ( iter_tmp != statid_to_index_.end() )
    {
        ZCE_Lock_Ptr_Guard guard(stat_lock_);
        size_t index = iter_tmp->second;
        return (status_stat_sandy_->begin()  + index)->counter_;
    }
    else
    {
        return 0;
    }
}

//ȡ�ü������ĸ���
size_t ZCE_Server_Status::num_of_counter()
{
    //����ı����ֵ��������
    return status_stat_sandy_->size();
}

//��ȡcopy_time
uint64_t ZCE_Server_Status::get_copy_time()
{
    return stat_file_head_->copy_time_;
}

//������ڵ����ݣ�����Ķ�ʱ������ʱ���ã���Ȼǰ�����Ӧ���ϱ��������ڽ�һЩ������0��
//������ÿ5���ӵ���һ�ξ�OK
void ZCE_Server_Status::check_overtime(time_t now_time)
{
    ZCE_ASSERT(initialized_);

    //
    int clear_type = STATICS_INVALID_TYPE;
    time_t last_clear_time = static_cast<time_t>(stat_file_head_->active_time_);

    //��ʱ�����ڷ�����ʲô�仯û��
    if (last_clear_time / FIVE_MINTUE_SECONDS !=
        now_time / FIVE_MINTUE_SECONDS)
    {
        clear_type = STATICS_PER_FIVE_MINTUES;

        //���5���Ӷ�û�б仯��Сʱ����仯
        if (last_clear_time / zce::ONE_HOUR_SECONDS !=
            now_time / zce::ONE_HOUR_SECONDS)
        {
            clear_type = STATICS_PER_HOUR;

            //���Сʱ��û�б仯���첻��仯
            if (last_clear_time / zce::ONE_DAY_SECONDS !=
                now_time / zce::ONE_DAY_SECONDS)
            {
                clear_type = STATICS_PER_DAYS;
            }
        }
    }

    size_t num_of_counter = status_stat_sandy_->size();

    //���û�д���
    if (clear_type == STATICS_INVALID_TYPE)
    {
        return;
    }

    //�ȸ��Ƶ������ڴ�
    copy_stat_counter();

    //������ڵ�ͳ������
    ZCE_Lock_Ptr_Guard guard(stat_lock_);

    for (size_t i = 0; i < num_of_counter; ++i)
    {
        ZCE_STATUS_ITEM *cur_item =  status_stat_sandy_->begin() + i;

        if (clear_type >= cur_item->statics_type_)
        {
            //����zergsvr���ܰ������ַ�ʽ�ϱ��Լ��ļ������
            //����ȡ��������ϱ����ܣ�ͳһ���ⲿ�������ϱ�copy�ڴ���������
            cur_item->counter_ = 0;
        }
    }

    //��¼����ʱ��
    stat_file_head_->active_time_ = static_cast<uint64_t>(now_time);
}

//���ڽ��ڲ�����ȫ��ȡ�����������ⲿ���֮��
//������һ�����ٵ��õĺ������ҽ������������ܣ���֤��¼ʹ�õ��ڴ�ռ��С
void ZCE_Server_Status::dump_all(ARRAY_OF_STATUS_WITHNAME &array_status, bool dump_copy)
{
    ZCE_Lock_Ptr_Guard guard(stat_lock_);


    //������������СӦ��һ��
    size_t num_of_counter = status_stat_sandy_->size();
    array_status.resize(num_of_counter);


    for (size_t i = 0; i < num_of_counter; ++i)
    {
        if (false == dump_copy)
        {
            array_status[i].statics_item_ = (*status_stat_sandy_)[i];
        }
        else
        {
            array_status[i].statics_item_ = (*status_copy_mandy_)[i];
        }


        STATUS_WITHNAME_MAP::iterator iter = conf_stat_map_.find(array_status[i].statics_item_.item_id_.statics_id_);

        if (iter != conf_stat_map_.end())
        {
            strncpy(array_status[i].item_name_,
                    iter->second.item_name_,
                    ZCE_STATUS_ITEM_WITHNAME::MAX_COUNTER_NAME_LEN);
        }
        else
        {
            //��Ϊǰ��Ĵ��붼������飬���ԣ�������۲�������Ҳ����Ŀ��ܣ����Ǵ������
            ZCE_ASSERT(false);
        }
    }
}

//���ݼ�������Ϣ
void ZCE_Server_Status::copy_stat_counter()
{
    //��������������ȥ��ֵ
    size_t num_of_counter = status_stat_sandy_->size();
    status_copy_mandy_->resize(num_of_counter);
    ZCE_STATUS_ITEM *stat_sandy_begin = status_stat_sandy_->begin();
    ZCE_STATUS_ITEM *stat_mandy_begin = status_copy_mandy_->begin();
    std::copy_n(stat_sandy_begin,num_of_counter,stat_mandy_begin);

    //ˢ�±���ʱ��
    ZCE_ASSERT(stat_file_head_ != NULL);
    stat_file_head_->copy_time_ = static_cast<uint32_t>(time(NULL));
}

//Dump���е�����
void ZCE_Server_Status::dump_status_info(std::ostringstream &strstream, bool dump_copy)
{
    size_t num_of_counter = 0;
    strstream << "Statistics Number:" << static_cast<unsigned int>(num_of_counter) << std::endl;

    ZCE_STATUS_ITEM *stat_process_iter = NULL;

    //����ȷ���Ǵ��ĸ���������ȡ����
    if (dump_copy)
    {
        num_of_counter = status_stat_sandy_->size();
        stat_process_iter = status_stat_sandy_->begin();
    }
    else
    {
        num_of_counter = status_copy_mandy_->size();
        stat_process_iter = status_copy_mandy_->begin();
    }

    //��¼������õ����ֵı���
    char statics_item_name[ZCE_STATUS_ITEM_WITHNAME::MAX_COUNTER_NAME_LEN + 1];
    statics_item_name[ZCE_STATUS_ITEM_WITHNAME::MAX_COUNTER_NAME_LEN] = '\0';

    ZCE_Lock_Ptr_Guard guard(stat_lock_);


    for (size_t i = 0; i < num_of_counter; ++i)
    {

        STATUS_WITHNAME_MAP::iterator iter = conf_stat_map_.find((stat_process_iter + i)->item_id_.statics_id_);

        if (iter != conf_stat_map_.end())
        {
            strncpy(statics_item_name,
                    iter->second.item_name_,
                    ZCE_STATUS_ITEM_WITHNAME::MAX_COUNTER_NAME_LEN);
        }
        else
        {
            //��Ϊǰ��Ĵ��붼������飬���ԣ�������۲�������Ҳ����Ŀ��ܣ����Ǵ������
            ZCE_ASSERT(false);
        }

        strstream << std::setw(6) << i << ".";
        strstream << "statics id<" << std::setw(10) << (stat_process_iter + i)->item_id_.statics_id_ << ">";
        strstream << "game app id<" << std::setw(10) << (stat_process_iter + i)->item_id_.classify_id_ << ">";
        strstream << "classify id<" << std::setw(10) << (stat_process_iter + i)->item_id_.subclassing_id_ << ">";
        strstream << std::setw(ZCE_STATUS_ITEM_WITHNAME::MAX_COUNTER_NAME_LEN) << statics_item_name << ":" << (stat_process_iter + i)->counter_ << std::endl;
    }
}

//Dump���е�����
void ZCE_Server_Status::dump_status_info(ZCE_LOG_PRIORITY log_priority, bool dump_copy)
{
    size_t num_of_counter = 0;
    ZCE_STATUS_ITEM *stat_process_iter = NULL;


    //��¼������õ����ֵı���
    char statics_item_name[ZCE_STATUS_ITEM_WITHNAME::MAX_COUNTER_NAME_LEN + 1];
    statics_item_name[ZCE_STATUS_ITEM_WITHNAME::MAX_COUNTER_NAME_LEN] = '\0';

    //����ȷ���Ǵ��ĸ���������ȡ����
    if (dump_copy)
    {
        num_of_counter = status_stat_sandy_->size();
        stat_process_iter = status_stat_sandy_->begin();
    }
    else
    {
        num_of_counter = status_copy_mandy_->size();
        stat_process_iter = status_copy_mandy_->begin();
    }

    ZCE_LOG(RS_INFO, "Statistics Number: %u", num_of_counter);
    ZCE_LOG(RS_INFO, "index.<statics id,classify id> name                            :number");

    ZCE_STATUS_ITEM_WITHNAME tmp_check;
    ZCE_Lock_Ptr_Guard guard(stat_lock_);

    for (size_t i = 0; i < num_of_counter; ++i)
    {
        STATUS_WITHNAME_MAP::iterator iter = conf_stat_map_.find((stat_process_iter + i)->item_id_.statics_id_);

        if (iter != conf_stat_map_.end())
        {
            strncpy(statics_item_name,
                    iter->second.item_name_,
                    ZCE_STATUS_ITEM_WITHNAME::MAX_COUNTER_NAME_LEN);
        }
        else
        {
            //��Ϊǰ��Ĵ��붼������飬���ԣ�������۲�������Ҳ����Ŀ��ܣ����Ǵ������
            ZCE_ASSERT(false);
        }

        ZCE_LOG(log_priority, "%5u.<%10u, %10u, %10u> %32s : %llu ",
                i,
                (stat_process_iter + i)->item_id_.statics_id_,
                (stat_process_iter + i)->item_id_.classify_id_,
                (stat_process_iter + i)->item_id_.subclassing_id_,
                statics_item_name,
                (stat_process_iter + i)->counter_
               );
    }
}

//�õ��ļ���ͷ����Ϣ
void ZCE_Server_Status::get_stat_head(ZCE_STATUS_HEAD *stat_head )
{
    *stat_head = *stat_file_head_;
}

//��¼��ص��ϱ�ʱ��
void ZCE_Server_Status::report_monitor_time(uint64_t report_time)
{
    stat_file_head_->report_monitor_time_ = report_time;
}

//�õ�Ψһ�ĵ���ʵ��
ZCE_Server_Status *ZCE_Server_Status::instance()
{
    if (instance_ == NULL)
    {
        instance_ = new ZCE_Server_Status();
    }

    return instance_;
}

//��ֵΨһ�ĵ���ʵ��
void ZCE_Server_Status::instance(ZCE_Server_Status *pinstatnce)
{
    clean_instance();
    instance_ = pinstatnce;
    return;
}

//�������ʵ��
void ZCE_Server_Status::clean_instance()
{
    if (instance_)
    {
        delete instance_;
    }

    instance_ = NULL;
    return;
}

