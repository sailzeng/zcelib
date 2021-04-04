/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_event_reactor_select.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2011��9��1��
* @brief      SELECT ��IO��Ӧ����IO��·����ģ��
*             �����ͻ�����ܵ�һ��ǰ�ᣬ���ZCE_Event_Handler�ϼ̳еõ�����ľ��
*             ��װ�������¼��Ĵ����Ѿ���װ�ˣ����������Ǻ�LINUXƽ̨һ�¡�
*
*             �����ŵ�����Windowsƽ̨Ҳ�����ã�ȱ�����SELECT�������е�ȱ�㣬
*             ��Ȼ����ͨ����װ���Ѿ���֤���ڸ���ƽ̨���в�������ܡ�
*             �����ס�
*
* @details    �����ACE��Reactor�е��񣬵����Ǽ��˺ܶ�ط������ǲ����붨ʱ����
*             ����Ҳ��������Ϣ���У�����򵥵�����Ϊ��IO������
*
*             ��ο���
*             http://www.cnblogs.com/fullsail/archive/2012/08/12/2634336.html
*             http://www.cnblogs.com/fullsail/archive/2011/11/06/2238464.html
* @note
*
*/
#ifndef ZCE_LIB_EVENT_REACTOR_SELECT_H_
#define ZCE_LIB_EVENT_REACTOR_SELECT_H_

#include "zce_event_reactor_base.h"

/*!
* @brief      SELECT ��IO��Ӧ����IO��·����ģ��
*
*/
class ZCE_Select_Reactor : public ZCE_Reactor
{

public:

    /*!
    * @brief    ���캯��
    */
    ZCE_Select_Reactor();

    /*!
    * @brief      ���캯������������
    * @param      max_event_number ���ľ�����������ڳ�ʼ��һЩ�����Ĵ�С���ӿ촦��
    */
    ZCE_Select_Reactor(size_t max_event_number);

    /*!
    * @brief      ���������� virtual��ม�
    */
    virtual ~ZCE_Select_Reactor();

public:

    /*!
    * @brief      ��ʼ��
    * @return     int              ����0��ʾ�ɹ�������ʧ��
    * @param      max_event_number ���ľ��������
    */
    int initialize(size_t max_event_number);

    /*!
    * @brief      ȡ��ĳЩmask��־��
    * @return     int            ����0��ʾ�ɹ�������ʧ��
    * @param      event_handler  �����ľ��
    * @param      cancel_mask    Ҫȡ����MASKֵ
    */
    virtual int cancel_wakeup(ZCE_Event_Handler *event_handler, int cancel_mask) override;


    /*!
    * @brief      ��ĳЩmask��־��
    * @return     virtual int    ����0��ʾ�ɹ�������ʧ��
    * @param      event_handler  �����ľ��
    * @param      event_mask     Ҫ���ӵ�MASKֵ
    */
    virtual int schedule_wakeup(ZCE_Event_Handler *event_handler, int event_mask) override;

    /*!
    * @brief      �¼�����
    * @return     int           ����0��ʾ�ɹ�������ʧ��
    * @param      time_out      ��ʱʱ��
    * @param      size_event    �����ľ������
    */
    virtual int handle_events(ZCE_Time_Value *time_out, size_t *size_event) override;

protected:

    /*!
    * @brief      ����ready��FD��������Ӧ���麯��
    * @param      out_fds    �����fd set
    * @param      proc_mask  Ҫ�����MASKֵ���ڲ��ᰴ�գ�����д���쳣��˳����д���
    */
    void process_ready(const fd_set *out_fds,
                       ZCE_Event_Handler::EVENT_MASK proc_mask);

protected:

    ///����ļ����+1����ֵ,��ù��SELECT����Ҫ��ѽ��
    int          max_fd_plus_one_ = 0;

    ///����ʹ�õ�Read FD SET
    fd_set       read_fd_set_;
    ///����ʹ�õ�Write FD SET
    fd_set       write_fd_set_;
    ///����ʹ�õ�Exception FD SET
    fd_set       exception_fd_set_;

    ///ÿ������ΪSELECT �����Ĳ���
    ///
    fd_set       para_read_fd_set_;
    ///
    fd_set       para_write_fd_set_;
    ///
    fd_set       para_exception_fd_set_;
};

#endif //ZCE_LIB_EVENT_REACTOR_EPOLL_H_

