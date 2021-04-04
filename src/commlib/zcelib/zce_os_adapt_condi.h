/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_os_adapt_condi.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2011��10��2��
* @brief      ����������ƽ̨�ķ�װ����WINDOWS����ģ��ģ�����ֻ�����߳���ʹ�ã�
*             �Ƽ�����WIN SVR 2008��VISTA�Ժ�ʹ�������װ����ʱ��ϵͳ���ò���ϵͳWindows������ͬ
*             ��ԭ��Cond�����������źŵƣ��ٽ���ģ��ķ�װ��
*             ����ģ���ʵ�֣���Ȼ��Ҳ���Թ�������ʵ�ڲ��ұ�֤ʲô��������ѧ���о����ԣ����ã��������˰ѡ�
*             �������źŵưѡ�
*
*             ����������ʵһ���Ͼ����Խ��͵Ļ��ⷽʽ���Ҿ����������׵��˲��࣬
*             ������涮�ˣ���ش������������⣬
*             0.�������ж�����if����where��
*             1.�㽲��pthread_cond_wait�Ĳ������ⲿ�����ݽ�ȥ��Ŀ����ʲô��
*             2.�㽲������signal��broadcast��ʱ���ⲿ�����Ƿ�Ӧ�ü��ϣ�
*             3.�����һ���߳�broadcast�ˣ������̼߳�����ֻ��ڵȴ�������һ���߳�broadcast��
*               ��ʱ��Щ�ȴ����̻߳ᱻ����Σ�
*
* @details    WINDOWS��VISTA��SERVER2008��Ҳ֧�������������������ⷽʽ�ˡ�
*             ǰ��İ汾ֻ����ģ����ˣ���Ȼѡ����ACE���㷨��ʵ��û�ޣ�PTHREAD WIN32���ҿ�������
*
*             ������XP��NT4��WINSERVER2003�ķ����ϣ��������ٽ�������������+�źŵƣ�ģ��
*             ��VISTA�Ժ�Visual studio 2008�Ժ�Ļ�����ע�������ⲿ������ֻ���ٽ���ֻ���ã�����
*             ����ֱ����WINDOLWS���ٽ�����
*
*             ��С�������ù��condiģ��,������ACE,BOOST,PTHREAD WIN32�Ļ����ϲο��£���Ȼ��������3��
*             �⻹�������Լ���Eventģ��ʧ�ܵ�һ������ֻ�ܸ���Щ����߹��ˣ���ȷ�����ף�һ��ϸ��û���뵽�Ϳ�
*             �������⣬�治���ס�
*
*             �ðɣ����ǽ�����ʷ�ѣ���ʵ��Щʵ����ʵ���ǲο�ACE�ı�������ģ�
*             http://www.cse.wustl.edu/~schmidt/win32-cv-1.html
*             ����Douglas C. Schmidt and Irfan Pyarali
*
*             PTHREAD WIN32��Ӧ���ǲο���ACE��ʵ�ֵģ�������ΪACE��ʵ�ִ������⣬������һ��֤����
*             ���Ǹ�֤������������WHERE�������������������������ˣ�����ˮ�ˡ�
*             BOOST������ǲο�PTHREAD WIN32��һ������һ��İ汾�������ҵ�ԨԴ��ˡ�
*
*             �ʼ�ҿ��ľ���ACE�İ汾�����ڲο���ACE ��������Сʱ�޷�֤������ΪACEĬ��ʵ����
*             EVENT,�Ұ�Ŀ��Ͷ����PTHREAD WIN32���������һ��α���빩�ο�����Ϊα�����ǰ������
*             �ĺö�������������1��뿴���α���룬���������˱�ACE��������������⣬û��ʲô�仯����Ѫ��
*             BOOST��ʵ��Ҳ�ǲο�PTHREAD WIN32��
*             �������ֻ�ͷ�ο�ACE,����ACE�Ĵ���������˼·��û��ʲôƫ���ĵط�������������Ϊɶ�������ģ�
*             ��PTHREAD��ʵ���ܼ���3������������ȥ,��һ���źŵƺ�����ͷ����ȻBOOST�Ĵ���Ҳ�ò����Ķ�ȥ��
*
*             ACE��ʵ�֣��ðɣ��һ�����Լ�Ѿ�����ACE��ʵ�֣����һ����޷�����֤�����������ģ����Ҹо���ֻ��
*             ͬʱ֧��һ�ֻ��ѷ�ʽ��broadcast����singalһ�֣��������ͬʱ֧�֣��϶��ᵼ���鷳��
*             ����broadcast��ʵ���ǻ��ڷ����㲥�󣬱���ȴ����еȴ����յ��㲥��
*             ACE��broadcast����Ҳ��ȷҪ�󣬵���ʱ���ⲿ���Ǳ������ϵģ�
*             BTW:�ҵ����ڲ�����ȫ֤����������������ģ��е�С�Σ������еĵط��Ƿ�Ҫ����EVENT���еĵط�
*             �Ƿ�Ҫ��SignalObjectAndWait?
*
*             PTHREAD WIN32 ��BOOST��ʵ�֣�����Ӧ��֧��2��һ����á��������ϣ���������ǵ�ʵ�֣�����һ
*             �����ѵ����飬������3��������¼����wait���������˳�wait����ʱ��ȡ���˳�wait��������������
*             �㷨����Щ�����ĸ�д����������������Ѫ�ģ�
*             ��Ĺ㲥ʵ���ǻ���broadcast����singal���ٽ���һ��singal����broadcastʱ���ȵȴ���һ��
*             �ĵȴ����Ƿ��Ѿ���ȫ�������ˡ�
*
*             �������Լ�����һ���򻯵����¼�ģ��İ汾����ͬʱֻ֧��һ�ּ�����ʽ�������ⲿ��Ҫ��MUTEX��
*             ����ڲ����в���ʧ�ܣ�
*
*             ���ϴ𰸷ֱ��ǣ�
*             1.WHERE����Ϊ���ܷ�����ٵĻ���
*             2.���Ǽ�����ǡǡ�ǽ⿪����������ͨ�С�
*             3.���ǿ������ϵͳ��ʵ�����������û�д���������ݣ��ǲ��üӵģ����������ϲ���Ӧ�ø��죬
*             ����й������ݣ�������ο�steven ��UNP V2 ����POSIX��
*             ϵͳĬ��ʵ��ָPTHRADĬ�ϵ�ʵ�ֺ�WINDOWS 2008����ͬ��ԭ�
*             ������ģ��ʵ�֣�Ŀǰ�������ģ��ʵ��ʵ�ֺ�ACEһ����Ҫ����signal��broadcast��ʱ���ⲿ����
*             Ӧ�ü��ϡ���PTHREAD WIN32û�иĳ���ȷ��˵��������˵����Ϊδ���壬���Ի��ǼӰɡ�
*
*             4.��֪���������ϵͳʵ�֣�����Ե�����ĵ���Ӧ���ǻ��ڵȴ��Ĳ��֡�
*             �����ģ��ʵ�֣�����Ҫ���ⲿ��Ҫ����ϣ�����Ӧ�ò���������������
*
* @note       ��WINDOWSƽ̨����WIN SERVER2008���ڶ��߳��£��ⲿ���������ٽ���������£�ʹ������������
*
*             ACE��ģ���Ƿ��������Ҳ�ȷ�ϣ�������֪��ACE��ʵ���������broadcast��signalʱ���ⲿ��������ϣ�����
*             ��LINUX�ֲᣬsignal �� broadcast ������ȷ�Ƿ�Ҫ������
*
*             pthread_condxxx_xxxx�ĺ���Ҳ���ڷ���ֵ�м�¼����ID�ģ�����ʱע��
*
*/

#ifndef ZCE_LIB_OS_ADAPT_CONDITION_H_
#define ZCE_LIB_OS_ADAPT_CONDITION_H_

#include "zce_os_adapt_predefine.h"

namespace zce
{

//====================================================================================================

/*!
@brief      ����������������
@return     int    ����0��ʶ�ɹ�
@param      attr   ����condi�����Ա���
*/
int pthread_condattr_destroy(pthread_condattr_t *attr);

/*!
@brief      ��ʼ��������������
@return     int    ����0��ʶ�ɹ�
@param      attr   ��ʼ��condi�����Ա���
*/
int pthread_condattr_init(pthread_condattr_t *attr);

/*!
 @brief      ����������������
 @return     int   ����0��ʶ�ɹ�,
 @param      cond  ������������
*/
int pthread_cond_destroy(pthread_cond_t *cond);

/*!
* @brief      ��ʼ��������������
* @return     int     ����0��ʶ�ɹ�,
* @param      cond    ������������
* @param      attr    ������������
*/
int pthread_cond_init(pthread_cond_t *cond,
                      const pthread_condattr_t *attr);

/*!
* @brief      ��ʼ�������������󣬲�ͬ��ƽ̨����ͬ��Ĭ�϶���
*             �Ǳ�׼�����ǽ�����ʹ�ã��򵥶���,���Ҫ����̹����鷳���ϸ������֣�����û����
* @return     int            ����0��ʶ�ɹ�
* @param      cond              CV����������������
* @param      win_mutex_or_sema �ⲿ�ȴ��������Ƿ��о���������MUTEX���źŵƾ��У�������ٽ�����û��
*/
int pthread_cond_initex(pthread_cond_t *cond,
                        bool win_mutex_or_sema = false);

/*!
* @brief      ���������ȴ���һ�µȴ�
* @return     int            ����0��ʶ�ɹ�,-1��ʶʧ��
* @param      cond           ������������
* @param      external_mutex �ⲿ��MUTEX���󣬽���waitǰӦ����������
*/
int pthread_cond_wait(pthread_cond_t *cond,
                      pthread_mutex_t *external_mutex);

/*!
* @brief      ���������ȴ�һ��ʱ�䣬��ʱ�����
* @return     int               ����0��ʶ�ɹ�,-1��ʶʧ��
* @param      cond              ������������
* @param      external_mutex    �ⲿ��MUTEX����
* @param      abs_timespec_out  ��ʱ��ʱ�䣬����ֵʱ�䣬timespec����
* @note
*/
int pthread_cond_timedwait(pthread_cond_t *cond,
                           pthread_mutex_t *external_mutex,
                           const ::timespec *abs_timespec_out);

/*!
* @brief      �Ǳ�׼���������������ȴ�һ��ʱ�䣬��ʱ�����,ʱ����������ڲ�ͳһ��timeval
* @return     int               ����0��ʶ�ɹ�,-1��ʶʧ��
* @param      cond              ������������
* @param      external_mutex    �ⲿ��MUTEX����
* @param      abs_timeval_out   ��ʱ��ʱ�䣬����ֵʱ�䣬timeval����
*/
int pthread_cond_timedwait(pthread_cond_t *cond,
                           pthread_mutex_t *external_mutex,
                           const timeval *abs_timeval_out);

/*!
* @brief      �������������㲥
* @return     int
* @param      cond
* @note       ����ǰ���ⲿ�����Ƿ�Ҫ����?��(POSIX����������ʾ���죬����ο�steven ��UNP V2 ����POSIX)
*             �����WIN2008���°汾ģ��ʵ�֣�������ϣ�����������⣬
*             �������ʵ��������������ôӦ���ǿ��Բ����ⲿ����������޸ĵ�������һ���������ݣ�Ҫ����
*/
int pthread_cond_broadcast(pthread_cond_t *cond);

/*!
* @brief      ���������������źš�
* @return     int  ����ֵΪ0��ʾ�ɹ�
* @param      cond ������������
* @note       ����ο�pthread_cond_broadcast˵���Ρ�
*/
int pthread_cond_signal(pthread_cond_t *cond);

//====================================================================================================

};

#endif //ZCE_LIB_OS_ADAPT_CONDITION_H_

