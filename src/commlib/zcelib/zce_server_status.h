/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_server_status.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2006��3��7��
* @brief
*
*
* @details    ��������ͳ���࣬
*             ����뷨����rong,sonicmao��C4A�ķ�����,��rong,sonicmao�¾�.
*
*
* @note       �޸ļ�¼���ܼ򵥵�һ���࣬���Ķ��ܶ�
*     1.Date  :2011��10��30��
*     Author  :����ZCELIB�Ĵ�������еĴ�������Ż�
*     Modification  :
*     2.Date  :2012��1��15��
*     Author  :Sailzeng
*     Modification  :��N�η������Լ���Ѫ�ĸ���2�κ��ҿ�ʼ������һ����򵥵ķ�ʽ������⣬
*     3.Date  :2012��5��10��
*     Author  :Sailzeng
*     Modification  :�ܶడͬ�»����²���Ҫ����classify�����뻹�Ǹ��˰ѡ����ָ��sandy��mandy
*     4.Date  :2013��10��1��
*     Author  :Sailzeng
*     Modification  :��һ����ֹͳ��ID�ظ������󣬽�ԭ����ARRAY�������һ��MAPӳ�䣬
*
*/

#ifndef ZCE_LIB_SERVER_STATUS_H_
#define ZCE_LIB_SERVER_STATUS_H_

#include "zce_share_mem_posix.h"
#include "zce_shm_array.h"
#include "zce_lock_null_lock.h"
#include "zce_os_adapt_time.h"


//===========================================================================================
/*!
* @brief      ״̬ͳ�Ƶķ�ʽ������5���Ӽ�¼һ�εķ�ʽ��һ��Сʱ��¼�ķ�ʽ��
*
* @note
*/
enum ZCE_STATUS_STATICS_TYPE
{
    //��ʶ��Χ
    STATICS_INVALID_TYPE       = 0,

    //ÿ5���ӽ���һ��ͳ�ƣ�������5���ӵ�ͳ�Ʒ�ʽ
    STATICS_PER_FIVE_MINTUES   = 1,

    //ÿСʱ����һ��ͳ��,:0����գ�����ͳ�Ʒ�ʽ��Ҫ����1��Сʱ�仯���ݣ����ڶԱȵȣ�
    STATICS_PER_HOUR           = 2,
    //ÿ�����һ��ͳ�ƣ�0:0����գ�������ͳ�Ʒ�ʽ��Ҫ����1��仯����
    STATICS_PER_DAYS           = 3,

    //����ֵ
    STATICS_ABSOLUTE_VALUE     = 11,

    //ԭ������һ��ÿ������ʱ����ֵ�Ƿ���յ�ѡ����ˣ�û��Ҫ������

};

//��֤�����ļ���һ����
#pragma pack(8)

//===========================================================================================
/*!
* @brief      ͳ����ĿID,
*
* @note
*/
struct ZCE_STATUS_ITEM_ID
{
public:

    ZCE_STATUS_ITEM_ID(uint32_t statics_id,
                       uint32_t classify_id,
                       uint32_t subclassing_id);
    ZCE_STATUS_ITEM_ID();
    ~ZCE_STATUS_ITEM_ID();

    bool operator == (const ZCE_STATUS_ITEM_ID& others) const;

public:

    ///ͳ��ID
    uint32_t              statics_id_=0;
    ///����ID��Ŀǰ������Ҫ��ҵ��ID,����ǿ��Ա仯��
    uint32_t              classify_id_=0;
    ///�ӷ���ID�����Ҳ�ǿ��Ա仯�ģ�
    uint32_t              subclassing_id_=0;


};

//===========================================================================================
/*!
* @brief      ZCE_STATUS_ITEM_ID�õ�HASH���ӵĺ�����
*
*/
struct HASH_ZCE_STATUS_ITEM_ID
{
public:
    //�Ͱ�3����ֵ��+��������ͻ�о���Сһ�㣬�����Ʒ����о����ã�
    size_t operator()(const ZCE_STATUS_ITEM_ID &stat_item) const
    {
        return static_cast<size_t>(stat_item.statics_id_)
               + static_cast<size_t>(stat_item.classify_id_)
               + static_cast<size_t>(stat_item.subclassing_id_);
    }
};


//===========================================================================================
/*!
* @brief      ״̬��������,����ID��ͳ�Ʒ�ʽ���Լ���ֵ
*
*/
class ZCE_STATUS_ITEM
{
public:

    //���캯������������
    ZCE_STATUS_ITEM();
    ZCE_STATUS_ITEM(unsigned int statics_id,
                    ZCE_STATUS_STATICS_TYPE statics_type);
    ~ZCE_STATUS_ITEM();

public:

    ///ID��ʶ
    ZCE_STATUS_ITEM_ID        item_id_;

    ///�������¼���
    ZCE_STATUS_STATICS_TYPE   statics_type_ =STATICS_INVALID_TYPE;

    ///������
    uint64_t                  counter_=0;
};

//===========================================================================================
/*!
* @brief      ״̬������+���֣��������ã�DUMP�����
*
*/
class ZCE_STATUS_ITEM_WITHNAME
{
public:

    //
    ZCE_STATUS_ITEM_WITHNAME(unsigned int,
                             ZCE_STATUS_STATICS_TYPE,
                             const char *);
    ZCE_STATUS_ITEM_WITHNAME();
    ~ZCE_STATUS_ITEM_WITHNAME();

public:
    //ͳ����Ŀ���Ƶĳ���
    static const size_t       MAX_COUNTER_NAME_LEN = 64;

public:

    ZCE_STATUS_ITEM           statics_item_;

    //����������
    char                      item_name_[MAX_COUNTER_NAME_LEN + 1];

};



//���ڰ����㶨�� ZCE_STATUS_ITEM_WITHNAME����
#ifndef DEF_ZCE_STATUS_ITEM
#define DEF_ZCE_STATUS_ITEM(_statics_id,_statics_type) ZCE_STATUS_ITEM_WITHNAME(_statics_id,_statics_type,(#_statics_id))
#endif


//===========================================================================================

/*!
* @brief      ���ͳ����Ϣ��״̬�ļ���ͷ������¼���ʱ���ʱ���
*
*/
struct ZCE_STATUS_HEAD
{
    //��ؿ�ʼʱ��
    uint64_t monitor_start_time_;

    //������ݸ��Ƶ�ʱ���
    uint64_t copy_time_;

    //��monitor�ϱ���ʱ��
    uint64_t report_monitor_time_;

    //����ʱ�䳤�ȣ�Ŀǰû�м�¼
    uint64_t active_time_;

};

#pragma pack()

//===========================================================================================


/*!
* @brief      ʹ��Posix MMAP,��¼�����������һЩ������,״̬,
*
* @note       ����ʹ�õ�����ģʽ��������ʹ����������ᵼ�����������������,
*/
class ZCE_Server_Status : public ZCE_NON_Copyable
{
protected:

    ///���ͳ�����ݵĹ����ڴ����飬
    typedef zce::shm_array<ZCE_STATUS_ITEM>     ARRYA_OF_SHM_STATUS;
    ///ͳ��ID��������±��hash map
    typedef unordered_map<ZCE_STATUS_ITEM_ID, size_t, HASH_ZCE_STATUS_ITEM_ID>     STATID_TO_INDEX_MAP;
    ///statics_id_��key��ZCE_STATUS_ITEM_WITHNAME�Ľṹ
    typedef unordered_map<uint32_t, ZCE_STATUS_ITEM_WITHNAME>    STATUS_WITHNAME_MAP;

public:

    ///ͳ�����ݵ����飬����dump��������ݽṹ
    typedef std::vector<ZCE_STATUS_ITEM_WITHNAME>  ARRAY_OF_STATUS_WITHNAME;

public:

    ///���캯��,Ҳ���㵥��ʹ�õĻ��ᣬ���Բ���protected
    ZCE_Server_Status();
    ///��������
    virtual ~ZCE_Server_Status();

protected:


    /*!
    * @brief      ��ʼ���ķ���,ͨ�õĵײ㣬
    * @return     int
    * @param[in]  stat_filename MMAPӰ���״̬�ļ�����
    * @param[in]  restore_mmap  �Ƿ����ڻָ�MMAP������ǻָ����ļ������Ǵ��ڵ�,
    * @param[in]  multi_thread  �Ƿ��Ƕ��̻߳���ʹ��
    */
    int initialize(const char *stat_filename,
                   bool restore_mmap,
                   bool multi_thread);

    /*!
    * @brief      �����������棬��������Ŀ
    * @return     int ����ɹ��ҵ�����0
    * @param[in]  statics_id  ͳ��ID
    * @param[in]  classify_id ����ID
    * @param[in]  subclassing_id �ӷ���ID
    * @param[out] idx ��ѯ��������Ŀ�������
    */
    int find_insert_idx(uint32_t statics_id,
                        uint32_t classify_id,
                        uint32_t subclassing_id,
                        size_t *idx);

public:


    /*!
    * @brief      ����һ���Ѿ����ڵ��ļ����г�ʼ��,���ڻָ�������,�ļ������Ѿ����ڣ�
    *             һ���ѯ״̬����ʹ�����������
    * @return     int
    * @param[in]  stat_filename MMAPӰ���״̬�ļ�����
    * @param[in]  multi_thread  �Ƿ���߳�
    */
    int initialize(const char *stat_filename,
                   bool multi_thread);


    /*!
    * @brief      ����һ���Ѿ����ڵ��ļ����г�ʼ��,���ڻָ�������,����ļ������Ѿ����ڣ������´���
    *             ��Ҫ��¼״̬�ķ�����ʹ���������
    * @return     int
    * @param[in]  stat_filename MMAPӰ���״̬�ļ�����
    * @param[in]  num_stat_item item_ary��������
    * @param[in]  item_ary      ͳ����Ŀ��
    * @param[in]  multi_thread  �Ƿ���߳�
    * @note       ע��ͳ����Ŀ�������Ǽ��ID�����������ID����Ĭ���ǹ̶��� @ref MAX_MONITOR_STAT_ITEM
    */
    int initialize(const char *stat_filename,
                   size_t num_stat_item,
                   const ZCE_STATUS_ITEM_WITHNAME item_ary[],
                   bool multi_thread);

    /*!
    * @brief      ����һЩ�����Ŀ�������2�εĳ�ʼ����������һ�Σ�ҵ��һ�Σ�ʱʹ��
    * @return     void
    * @param      num_stat_item item_ary������
    * @param      item_ary      ���ӵ�ͳ����Ŀ
    */
    void add_status_item(size_t num_stat_item,
                         const ZCE_STATUS_ITEM_WITHNAME item_ary[]);


    ///������Ƿ��Ѿ�����
    bool is_exist_stat_id(unsigned int stat_id,
                          ZCE_STATUS_ITEM_WITHNAME *status_item_withname) const;

    ///��ʼ���Ժ��޸��Ƿ���Ҫ���̱߳���
    void multi_thread_guard(bool multi_thread);


    /*!
    * @brief      ʹ��ͳ��ID�ͷ���ID��Ϊkey,��ͳ��ֵ����1
    * @return     int
    * @param[in]  statics_id  ͳ��ID
    * @param[in]  classify_id ����ID
    * @param[in]  subclassing_id �ӷ���ID
    */
    inline int increase_once(uint32_t statics_id,
                             uint32_t classify_id,
                             uint32_t subclassing_id)
    {
        return increase_by_statid(statics_id, classify_id, subclassing_id, 1);
    }


    /*!
    * @brief      ʹ��ͳ��ID�ͷ���ID��Ϊkey,����ֵ�޸ļ��ͳ����Ŀ
    * @return     int
    * @param[in]  statics_id  ͳ��ID
    * @param[in]  classify_id ����ID
    * @param[in]  subclassing_id �ӷ���ID
    * @param[in]  set_value �޸ĵ�ͳ��ֵ
    */
    int set_by_statid(uint32_t statics_id,
                      uint32_t classify_id,
                      uint32_t subclassing_id,
                      uint64_t set_value);

    /*!
    * @brief      ʹ��ͳ��ID�ͷ���ID��Ϊkey,���ֵ�޸ļ��ͳ��ֵ
    * @return     int
    * @param[in]  statics_id  ͳ��ID
    * @param[in]  classify_id ����ID
    * @param[in]  subclassing_id �ӷ���ID
    * @param[in]  incre_value �޸ĵ����ֵ�������������ɼӿɼ�
    */
    int increase_by_statid(uint32_t statics_id,
                           uint32_t classify_id,
                           uint32_t subclassing_id,
                           int64_t incre_value);





    /*!
    * @brief      ����ͳ��ID�ͷ���ID��Ϊkey���õ�ͳ����ֵ
    * @return     uint64_t ���ص�ͳ��ֵ
    * @param[in]  statics_id  ͳ��ID
    * @param[in]  classify_id ����ID
    * @param[in]  subclassing_id �ӷ���ID
    */
    uint64_t get_counter(uint32_t statics_id,
                         uint32_t classify_id,
                         uint32_t subclassing_id);

    //ȡ�ü������ĸ���
    size_t num_of_counter();

    //��ȡcopy_time
    uint64_t get_copy_time();

    //������ڵ����ݣ�����Ķ�ʱ������ʱ���ã���Ȼǰ�����Ӧ���ϱ��������ڽ�һЩ������0��
    //������ÿ5���ӵ���һ�ξ�OK
    void check_overtime(time_t now_time);

    ///���ݼ�������Ϣ
    void copy_stat_counter();

    ///���ڽ��ڲ�����ȫ��ȡ�����������ⲿ���֮��
    void dump_all(ARRAY_OF_STATUS_WITHNAME &array_status,
                  bool dump_copy = false);

    ///Dump���е�����
    void dump_status_info(std::ostringstream &strstream,
                          bool dump_copy = false);

    ///Dump���е�����
    void dump_status_info(ZCE_LOG_PRIORITY log_priority,
                          bool dump_copy = false);

    ///�õ��ļ���ͷ����Ϣ
    void get_stat_head(ZCE_STATUS_HEAD *stat_head );

    ///��¼��ص��ϱ�ʱ��
    void report_monitor_time(uint64_t report_time = static_cast<uint64_t>(time(NULL)));

    //���ӵĺ���Ⱥ�������Ҳ�֪��������BOOST��ģ��ʹ�õ��ӣ�����������ֱ����ˬ�����������޸�һЩ
    //�Ҳ���Ϊ�˵��ӿ�����ν�ı������⣬���Լ���֤��ĳ�ʼ��������������

public:

    //�õ�����ʵ��
    static ZCE_Server_Status *instance();
    //����ʵ����ֵ
    static void instance(ZCE_Server_Status *);
    //������ʵ��
    static void clean_instance();

protected:

    //���ļ����ĿID,
    static const size_t MAX_MONITOR_STAT_ITEM = 64 * 1024;

    //����ӵ�����
    static const time_t FIVE_MINTUE_SECONDS   = 300;
    //һСʱ������
    static const time_t ONE_HOURS_SECONDS     = 3600;
    //һ���ʱ��
    static const time_t ONE_DAY_SECONDS       = 86400;

    //ͳ����Ŀ��ֵ����1
    static const uint64_t INCREASE_VALUE_ONCE = 1;

protected:

    //��̬����,
    ZCE_Lock_Base            *stat_lock_;

    //MMAP�ڴ�Ӱ��������ļ�
    ZCE_ShareMem_Posix        stat_file_;

    //�ڴ��ļ�ͷ
    ZCE_STATUS_HEAD          *stat_file_head_;

    //mandy��sandy��ԭ�������м�Ϊ�����������鶨λ��hash��λ�����������Ķ�����
    //��������ȫ����Ϊ����hansh��λ��������˼������СMM����������ǲ��Ź����ǣ�
    //http://t.qq.com/angelbaby22

    // ���״̬������������
    ARRYA_OF_SHM_STATUS      *status_stat_sandy_;

    //״̬��������һ�ݿ���(����)�����ݷ��ڹ����ڴ��У����ǿ����ÿ�����mandy��ȡ���ݣ����Կ��Խ�ÿ��
    ARRYA_OF_SHM_STATUS      *status_copy_mandy_;

    //��¼���õĵ�ͳ������SET�����ڼ�¼���õ�ͳ����Ŀ��Ҳ���ڷ�ֹ�ظ������dump ���ʱ��������Ϣ
    STATUS_WITHNAME_MAP       conf_stat_map_;

    //STAT_ID to idx������MAP
    STATID_TO_INDEX_MAP       statid_to_index_;

    //�Ƿ���ж��̱߳���
    bool                      multi_thread_guard_;

    // �Ƿ��Ѿ���ʼ��
    bool                      initialized_;

protected:

    //����ʵ��
    static ZCE_Server_Status *instance_;
};



#endif //_ZCE_LIB_SERVER_STATUS_H_

