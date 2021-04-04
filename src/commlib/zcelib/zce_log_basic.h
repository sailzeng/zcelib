/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_log_basic.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2004��7��11��
* @brief
*
*
* @details    ������־���Ż�����2������Ҳ���ǿ��Կ��ǵģ�
*             1.���������Ƚ��ͣ�д��־������һ����2���ط����ã���־�ļ��ļ����Ʊ仯��
*             ��־Ҫд������һ���ļ�ʱ����Ҫ��������������������޷��Ż�
*             �ڶ����ط���д���ļ��ط�������������˻����д�뷽ʽ����ô�����������
*             ���������Ϣ���ҵ���������ǿ��ܿ����Ż����ģ���Ϊwrite������д��BUFFER
*             ��4K���µ����ʱ����ʵ�ǲ���Ҫ�����ģ��������������LINUX������ȷ�ģ�����
*             ��ƽ̨�أ������ú�֤���������������ط�ֱ����wirte��������ô������С����
*             �ķ�Χ��
*             2.�ڶ����Ż��ķ�����ǻ�������⣬��־�����һ���ǲ����û���ģ�������־��
*             �ص�����˵ģ����ļ����ʹ�û���Ͳ�ʵ�û��壬�ٶ��Ǹ�������൱�൱��ม�
*             �����ܷ���һ�ַ�ʽ�����û���ȷ�Լ����û��壿�����Լ��ڶ�ʱ���е��û��壬
*             ��Ȼ�����������������������������������᲻�ᶪ��־�ء�����Ȼ���Խػ�
*             �źŵȣ������ַ�ʽ���ͨ���أ�
*
*
* @note
*
*/

#ifndef ZCE_LIB_TRACE_LOG_BASE_H_
#define ZCE_LIB_TRACE_LOG_BASE_H_

#include "zce_boost_non_copyable.h"
#include "zce_lock_thread_mutex.h"
#include "zce_log_priority.h"


///��־�ļ��ķָ��,�Լ���Ӧ�����ƹ�ϵ
///Ĭ�ϵķָ���ǰ���ʱ��.�ָ���ǰ���ÿ��һ���ļ�,�ļ������м�¼ʱ��
///��������ļ�SIZE,������־�������ָ��ļ�,��һ��ID��ʶ�ļ���־�ļ�
///ʹ��ID���ַָ���־��һ������.����ÿ�γ�ʼ��ʱҪ�õ�ID,����Ҫ����ԭ��
///��־�ļ�,����Ҫ��ѯԭ�����һ����־�ļ�,��Ҫ�õ���ļ�¼�����Լ��ļ��ߴ�
enum ZCE_LOGFILE_NAME_DEVIDE
{
    ///��¼��������־�ļ�,���ָ�
    LOGDEVIDE_NONE          = 1,

    ///���ճߴ��¼�ָ���־�ļ�,��ID��ʶ,���ID����˼�ǹ��������ID��
    ///��ǰ����־�ļ���.log����ļ��������ľ�����־�ļ�������1-N����ǰ������ļ�
    ///д����ɾ����N���ļ��������ļ����θ�������ǰ�ļ�����Ϊlog.1,Ȼ��������һ��
    ///�ļ���Ϊ��ǰд�����־�ļ������ַ�ʽ�ô��Ǵ�С���Թ̶����ô����ļ��������ڱ�
    ///������������ά���͹���
    ///��32λ�Ļ������ָ���־�Ĵ�С��Ҫ����4G��64λ����������,�����ڲ�������16G
    NAME_ID_DEVIDE_SIZE       = 101,


    ///����Сʱ�ָ���־,
    NAME_TIME_HOUR_DEVIDE_TIME       = 201,
    ///����6��Сʱ�ָ���־,1��ָ�Ϊ4��
    NAME_SIXHOUR_DEVIDE_TIME    = 202,
    ///�������ڷָ���־,�ļ������¼
    NAME_TIME_DAY_DEVIDE_TIME  = 203,
    ///�����·ݷָ���־,
    NAME_TIME_MONTH_DEVIDE_TIME = 204,
    ///����ʱ��ָ���־,�ļ������¼
    NAME_TIME_YEAR_DEVIDE_TIME  = 205,

    //�ļ����ư������¼,��־����С�ָ
    NAME_TIME_MILLISECOND_DEVIDE_SIZE = 301,


};
//��������ͼʵ�������𣬵��Ǹо��ܲ����ƣ���������ƾ���һ��ȡ��
//LOGDEVIDE_BY_RECORD                ���ռ�¼�ĸ���(��������)Ŀ�ָ���־�ļ�,��ID��ʶ,���Ƽ�ʹ��



///Ĭ�ϼ�¼������,��ÿ����־��ͷ��
enum LOG_HEAD_RECORD_INFO
{
    ///ʲô������¼
    LOG_HEAD_RECORD_NONE          = 0,
    ///��¼��ǰ��ʱ��
    LOG_HEAD_RECORD_CURRENTTIME   = 1,
    ///��¼��־�ļ�����Ϣ
    LOG_HEAD_RECORD_LOGLEVEL      = 2,
    ///��¼����ID
    LOG_HEAD_RECORD_PROCESSID     = 4,
    ///��¼�߳�ID
    LOG_HEAD_RECORD_THREADID      = 8,

};

///ѡ������ķ�ʽ
enum LOG_OUTPUT_WAY
{
    ///�����κεط����
    LOG_OUTPUT_NONE     = 0,
    ///ͬ�����������ط����,Ĭ��
    LOG_OUTPUT_FILE     = 1,
    ///ͬ�����׼������.�����ĳ�����CGI����,����
    LOG_OUTPUT_STDOUT   = 2,
    ///ͬ�����׼�������.
    LOG_OUTPUT_ERROUT   = 4,
    ///�����ڴ��ļ��������
    LOG_OUTPUT_MMAP_FILE = 8,
    ///ͬ����WINDOWS�ĵ��Դ������,������WIN32����������
    LOG_OUTPUT_WINDBG   = 32
};



/*!
@brief      ��־����Ļ������ܶ�����������棬�����߳�ͬ�����ļ����ƣ��ļ���С���ƣ�
            ��־����������ֳ�ʼ�������ƺ�����
            ���������඼�Ƕ����ں����ӿ��Ͻ����˷�װ��
            LOG_MSG  C printf���ַ�����ʽ������������ǳ��õķ�ʽһ����������
            �ڲ���������װ����������������ģ�
            LOG_PLUS CPP ��һ������C++ģ��������ʽ��ĳ�̶ֳ���������ַ�����
            ʽ�������ܣ���ע������ƿ����ͷ���ļ�����ǿ飩���ڲ��õĺ���
            zce::foo_snprintf��

*/
class ZCE_LogTrace_Basic : public ZCE_NON_Copyable
{
protected:

    //time(NULL)������һ���ȽϺ�ʱ�Ĳ���,������������ǿ�ҵ���,����ʹ��ʱ������Ч��
    //���Լ�ʹ�ö�ʱ��ÿ���д��ʱ��,Ȼ�����е���־ϵͳʱ�佫ʹ�����ʱ���
    //�����ⷽ��ACEҲû���ÿ���,

public:

    ///���캯��,
    ZCE_LogTrace_Basic();
    ///��������
    virtual ~ZCE_LogTrace_Basic();

    /*!
    @brief      ��ʼ������,����ʱ��ָ���־�Ĺ���
    @return     int               ����0��ʶ��ʼ���ɹ�
    @param[in]  div_log_file      �ָ���־�ķ�ʽ
    @param[in]  log_file_prefix   ��־��ǰ׺
    @param[in]  if_thread_synchro �Ƿ�����߳�ͬ����
    @param[in]  auto_new_line     ��־��¼��ĩβ�Ƿ��Զ��Ļ��У�newһ��
    @param[in]  reserve_file_num  ��������־�ļ����������������������־����ɾ��
    @param[in]  output_way        ��־����ķ�ʽ,���Զ��ַ�ʽ���棬�ο� @ref LOG_OUTPUT_WAY
    @param[in]  head_record       ��־ͷ����������Ϣ�������ο� @ref LOG_HEAD_RECORD_INFO
    */
    int init_time_log(ZCE_LOGFILE_NAME_DEVIDE div_log_file,
                      const char *log_file_prefix,
                      bool if_thread_synchro = false,
                      bool auto_new_line = true,
                      size_t reserve_file_num = DEFAULT_RESERVE_FILENUM,
                      unsigned int output_way = LOG_OUTPUT_FILE | LOG_OUTPUT_ERROUT,
                      unsigned int head_record = LOG_HEAD_RECORD_CURRENTTIME | LOG_HEAD_RECORD_LOGLEVEL
                     );

    /*!
    @brief      ��ʼ������,���ڳߴ�ָ���־�Ĺ��� �ڲ��� ZCE_LOGFILE_DEVIDE_NAME = LOGDEVIDE_BY_SIZE
    @return     int                ����0��ʶ��ʼ���ɹ�
    @param[in]  log_file_prefix    ��־��ǰ׺
    @param[in]  if_thread_synchro  �Ƿ�����߳�ͬ��
    @param[in]  auto_new_line      ��־��¼��ĩβ�Ƿ��Զ��Ļ��У�newһ��
    @param[in]  max_size_log_file  ��־�ļ������ߴ磬Ŀǰ���ߴ��ڲ��õ�4G
    @param[in]  reserve_file_num   ��������־�ļ����������������������־����ɾ��
    @param[in]  output_way         ��־����ķ�ʽ���ο� @ref LOG_OUTPUT_WAY
    @param[in]  head_record        ��־ͷ����������Ϣ�������ο� @ref LOG_HEAD_RECORD_INFO
    */
    int init_size_log(const char *log_file_prefix,
                      bool if_thread_synchro = false,
                      bool auto_new_line = true,
                      size_t max_size_log_file = DEFAULT_LOG_SIZE,
                      unsigned int reserve_file_num = DEFAULT_RESERVE_FILENUM,
                      unsigned int output_way = LOG_OUTPUT_FILE | LOG_OUTPUT_ERROUT,
                      unsigned int head_record = LOG_HEAD_RECORD_CURRENTTIME | LOG_HEAD_RECORD_LOGLEVEL);


    /*!
    @brief      ��ʼ�����������ڱ�׼���
    @return     int                ����0��ʶ��ʼ���ɹ�
    @param[in]  if_thread_synchro  �Ƿ�����߳�ͬ��
    @param[in]  use_err_out        �Ƿ���ʹ�ô�����������������Ϊ��׼������鷳��һ���ô������
    @param[in]  auto_new_line      ��־��¼��ĩβ�Ƿ��Զ��Ļ��У�newһ��
    @param[in]  head_record        ��־ͷ����������Ϣ�������ο� @ref LOG_HEAD_RECORD_INFO
    */
    int init_stdout(bool if_thread_synchro = false,
                    bool use_err_out = true,
                    bool auto_new_line = true,
                    unsigned int head_record = LOG_HEAD_RECORD_CURRENTTIME | LOG_HEAD_RECORD_LOGLEVEL);

    /*!
    @brief      ��ʼ�������������󼯺��ͺ�,���ݸ��ֲ������ѡ��,
    @return     int                ����0��ʶ��ʼ���ɹ�
    @param[in]  output_way        ��־����ķ�ʽ,���Զ��ַ�ʽ���棬�ο� @ref LOG_OUTPUT_WAY
    @param[in]  div_log_file
    @param[in]  log_file_prefix
    @param[in]  if_thread_synchro
    @param[in]  auto_new_line
    @param[in]  max_size_log_file ��־�ļ������ߴ�
    @param[in]  reserve_file_num  ��������־�ļ����������������������־����ɾ��
    @param[in]  head_record       ��־ͷ����������Ϣ�������ο� @ref LOG_HEAD_RECORD_INFO
    */
    int initialize(unsigned int output_way,
                   ZCE_LOGFILE_NAME_DEVIDE div_log_file,
                   const char *log_file_prefix,
                   bool if_thread_synchro,
                   bool auto_new_line,
                   size_t max_size_log_file,
                   size_t reserve_file_num,
                   unsigned int head_record);


    /*!
    @brief      �ر���־��ע��رպ󣬱������³�ʼ��
    */
    void finalize();

    /*!
    @brief      ��/�ر���־�������
    */
    void enable_output(bool enable_out);


    /*!
    @brief      ������־��������Level
    @return     ZCE_LOG_PRIORITY  �ɵ���־�������
    @param[in]  outmask           ���õ���־�������
    */
    ZCE_LOG_PRIORITY set_log_priority(ZCE_LOG_PRIORITY out_level );
    /*!
    @brief      ȡ�����Level
    @return     ZCE_LOG_PRIORITY
    */
    ZCE_LOG_PRIORITY get_log_priority(void );

    /*!
    @brief      ����Ĭ���������Ϣ����
    @return     unsigned int
    @param      recdinfo
    */
    unsigned int set_log_head(unsigned int recdinfo);
    /*!
    @brief      ȡ��Ĭ���������Ϣ����
    @return     unsigned int
    */
    unsigned int get_log_head(void);

    /*!
    @brief      ����ͬ�������λ��
    @return     unsigned int
    @param      output_way
    */
    unsigned int set_output_way(unsigned int output_way);
    /*!
    @brief      ����Ĭ�������λ��
    @return     unsigned int
    */
    unsigned int get_output_way(void);


    /*!
    @brief      �����Ƿ��߳�ͬ��
    @return     bool              �ɣ�ԭ���е��Ƿ���߳�ͬ��ֵ��
    @param      if_thread_synchro �Ƿ���ж��߳�ͬ������
    */
    bool set_thread_synchro(bool if_thread_synchro);
    /*!
    @brief      ȡ���Ƿ�����߳�ͬ��
    @return     bool   ��ǰ���Ƿ���߳�ͬ��ֵ
    */
    bool get_thread_synchro(void);


    /*!
    @brief      ����ļ���־��Ϣ
    @param      now_time         ��ǰ��ʱ��
    @param      log_tmp_buffer   �����buffer��
    @param      sz_use_len       �����buffer����
    */
    void output_log_info(const timeval &now_time,
                         char *log_tmp_buffer,
                         size_t sz_use_len);

protected:

    /*!
    @brief      ����ID�õ��ļ�����
    @param      logfileid     ��־��ID
    @param      idlogfilename ���ɵ���־�ļ�����
    */
    void create_id_logname(size_t logfileid,
                           std::string &idlogfilename);

    /*!
    @brief      �������ڵõ��ļ�����
    @param      tmt         ����������־�ļ����Ƶ�ʱ���
    @param      logfilename ���ɵ���־�ļ�����
    */
    void create_time_logname(const timeval &cur_time,
                             std::string &logfilename);



    /*!
    @brief      �����ڵ���־�ļ���
    */
    void del_old_logfile();


    /*!
    @brief      ����־��ͷ����Ϣ�����һ��Stringbuf��
    @param[in]  outlevel       �������־����
    @param[in]  now_time       ��ǰʱ��
    @param[out] log_tmp_buffer ͷ����Ϣ�����buffer
    @param[in]  sz_buf_len     buffer�ĳ���
    @param[out] sz_use_len     ʹ����buffer�ĳ���
    */
    void stringbuf_loghead(ZCE_LOG_PRIORITY outlevel,
                           const timeval &now_time,
                           char *log_tmp_buffer,
                           size_t sz_buf_len,
                           size_t &sz_use_len);



    /*!
    @brief      ����������Ϣ,�޸Ĵ�������,
    */
    void make_configure(void);

    /*!
    @brief      ������б�Ҫ�����õ��µ���־�ļ���
    @param      init     �Ƿ��ǳ�ʼ���׶�
    @param      current_time  ��ǰʱ��
    */
    void open_new_logfile(bool initiate, const timeval &current_time);

public:

    ///�����ַ������õ���־����
    static ZCE_LOG_PRIORITY log_priorities(const char *str_priority);

    ///�����ַ���,�õ���־�ָʽ��ö��
    static ZCE_LOGFILE_NAME_DEVIDE log_file_devide(const char *str_devide);

protected:

    ///��־�Ļ������ĳߴ�,�������8K���ܳ��ˣ�
    ///�������ڲ�����ʹ�õ�C++��stream ��Ϊ��������������ڶ��߳��»���ʹ��������
    ///�����ֱ����write ����д��4096(-1)��һ�������ʵ�ֵ��
    static const size_t  LOG_TMP_BUFFER_SIZE    = 8191;

    ///Ĭ�ϵı������ļ�������
    static const size_t DEFAULT_RESERVE_FILENUM = 100;
    ///���ı������ļ�������
    static const size_t MAX_RESERVE_FILENUM = 9999;
    ///��С�ı������ļ�������
    static const size_t MIN_RESERVE_FILENUM = 3;

    ///��־�ļ�����С����ߴ���8M
    static const size_t MIN_LOG_SIZE            = 4096000UL;
    ///��־�ļ����������ߴ���4G
    static const size_t MAX_LOG_SIZE            = 4096000000UL;
    ///��־�ļ���Ĭ������ߴ���40M
    static const size_t DEFAULT_LOG_SIZE        = 40960000UL;

    ///��־��ͳһ��׺����
    static const size_t LEN_LOG_POSTFIX         = 4;
    ///��־��׺��4����ĸ.log��ʲô��������.tlog��BS
    static const char STR_LOG_POSTFIX[LEN_LOG_POSTFIX + 1];

protected:

    ///��־��Ƭ�Ĵ���ʽ
    ZCE_LOGFILE_NAME_DEVIDE div_log_file_;

    ///��־�ļ�����ǰ׺,����·��
    std::string log_file_prefix_;

    ///��־���ļ�����,��ǰ���������־�ļ�����
    std::string log_file_name_;

    ///��־�ļ��ĵ�ǰĿ¼��Ϣ
    std::string log_file_dir_;

    ///����ķ�ʽ��LOG_OUTPUT_WAY��ö��ֵ��� @ref LOG_OUTPUT_WAY
    unsigned int output_way_;

    ///�����̰߳�ȫ,�ҵĿ�������,�����ģ��,�������,ֻ�öԶ��߳�ģ�ͼ���,
    //�����ģ�͹���һ���ļ�������,���Ҳ������ļ�������,���Բ��ÿ���ͬ��,
    //���߳�ģ��,�����ļ�������(��ȫ�ö�ջ���Խ��),IO����Ļ���Ҳ���ܹ���,���Ա���ͬ����
    //��ͬ���ĵ�Ӧ����������1.�ļ��ĸ��������Ҫ���⼸����ͬʱ���룬2.�򻺳���д���ʱ��
    //����2����ʵ������д����������������ֻ��4K��������ʵ�����Ͽ����ӱ�������⣬��Ȼ��������ʹ�ô��л�������,ֻ����write


    ///�Ƿ���ж��̵߳�ͬ��
    bool if_thread_synchro_;

    ///ͬ����
    ZCE_Thread_Light_Mutex protect_lock_;

    ///�Ƿ�����Զ�����
    bool                   auto_new_line_;

    ///�ļ������ߴ�
    size_t                 max_size_log_file_;

    ///�����ļ��ĸ���,�����̫���ļ�Ҫɾ��,Ϊ0��ʾ��ɾ��
    size_t                 reserve_file_num_;



    ///Ĭ�ϼ�¼������,���պ�LOG_HEAD_RECORD_INFO ���
    unsigned int           record_info_;


    ///��ǰ�Ĵ��ʱ��,��Сʱ��¼,������й����ʱ���ж�
    time_t                current_click_;

    ///�����־��Ϣ��Maskֵ,С�������Ϣ����Ϣ���������
    ZCE_LOG_PRIORITY      permit_outlevel_;


    ///��־�ļ��ĳߴ�
    size_t                size_log_file_;


    ///�Ƿ������־��Ϣ,����������ʱ����
    bool                  if_output_log_;

    ///��־���ļ����
    std::ofstream         log_file_handle_;

    //r
    std::list<std::string> time_logfile_list_;

};





#endif //ZCE_LIB_TRACE_LOG_BASE_H_

