/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_event_handle_base.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2011��8��29��
* @brief      IO��Ӧ����ʹ�õ��¼������װ
*
* @details
*
* @note
*
*/

#ifndef ZCE_LIB_EVENT_HANDLE_BASE_H_
#define ZCE_LIB_EVENT_HANDLE_BASE_H_

class ZCE_Reactor;

/*!
* @brief      IO��Ӧ����ʹ�õ��¼��������IO�¼���������Ӧ�ĺ���
*             ������
*             �����������û���ڷ�Ӧ�����漯�ɶ�ʱ���Ĵ���
*/
class ZCE_Event_Handler
{
public:

    ///IO�����¼�
    enum EVENT_MASK
    {
        NULL_MASK    = 0,


        ///SOCKET��ȡ�¼����������Ϊ�ɶ��ǣ�����handle_input
        READ_MASK    = (1 << 1),
        ///SOCKETд�¼����������Ϊ��д�ǣ�handle_output
        WRITE_MASK   = (1 << 2),
        ///SOCKET�쳣�¼������������handle_exception
        EXCEPT_MASK  = (1 << 3),
        ///SOCKET�첽CONNECT�ɹ��󣬵���handle_output���첽CONNECTʧ�ܺ�
        ///�����handle_input��(��ϵͳ�����ͳһ)
        CONNECT_MASK = (1 << 4),
        ///SOCKET ACCEPT�¼�����һ��ACCEPT�˿ڿ������Ӻ󣬵���handle_input��
        ACCEPT_MASK  = (1 << 5),

        ///iNotify֪ͨ�¼����ļ�ϵͳ�ĸı�֪ͨ,����handle_input��
        INOTIFY_MASK = (1 << 9),
    };

public:

    /*!
    * @brief      ���캯��
    */
    ZCE_Event_Handler();
    /*!
    * @brief      ���캯����ͬʱ������۵ķ�Ӧ��ָ��
    * @param      reactor �����صķ�Ӧ��ָ��
    */
    ZCE_Event_Handler(ZCE_Reactor *reactor);
    /*!
    * @brief      ��������
    */
    virtual ~ZCE_Event_Handler();

public:

    /*!
    * @brief      ȡ�ض�Ӧ��ZCE_HANDLE ���
    * @return     ZCE_HANDLE ZCE_Event_Handler ��Ӧ��ZCE_HANDLE ���
    */
    virtual ZCE_HANDLE get_handle (void) const = 0;

    /*!
    * @brief      ��ȡ�¼��������ú��������ڶ�ȡ���ݣ�accept�ɹ�����connectʧ�ܣ�����
    * @return     int ����0��ʾ�������������return -1�󣬷�Ӧ��������handle_close�������������
    * @note       return -1 ��Ӧ��������������handle_close������ԣ���ʵ�ҳֱ����������Ϊ��ʵ
    *             ����ֱ�ӵ���handle_close���������ַ�ʽ�������ݣ�����ֱ�ӵ��ÿ��ܸ�������һ�£��Ҹ�
    *             ������ʹ��������ԣ���������Ҫ��Ϊ�˺�ACE����
    */
    virtual int handle_input ();

    /*!
    * @brief      д���¼��������ú���������д���ݣ�connect�ɹ���
    * @return     int int ����0��ʾ�������������return -1�󣬷�Ӧ��������handle_close�������������
    */
    virtual int handle_output ();

    /*!
    * @brief      �����쳣��return -1��ʾ����handle_close
    * @return     int
    */
    virtual int handle_exception();

    /*!
    * @brief      ����رմ����������ຯ��������remove
    * @return     int
    */
    virtual int handle_close ();

    /*!
    * @brief      ȡ�õ�ǰ�ı�־λ
    * @return     int ���صĵ�ǰ��MASKֵ
    */
    inline int get_mask();

    /*!
    * @brief      ���õ�ǰ��־λ
    * @param      mask ���õ�MASKֵ
    */
    inline void set_mask(int mask);

    /*!
    * @brief      enable mask�����ı�־λ
    * @param      en_mask �򿪵�MASK��ֵ
    */
    inline void enable_mask(int en_mask);

    /*!
    * @brief      disable mask�����ı�־λ
    * @param      dis_mask �رյ�MASKֵ
    */
    inline void disable_mask(int dis_mask);

    /*!
    * @brief      ���÷�Ӧ��
    * @param      reactor
    */
    virtual void reactor (ZCE_Reactor *reactor);

    /*!
    * @brief      ȡ���Լ������ķ�Ӧ��
    * @return     ZCE_Reactor*
    */
    virtual ZCE_Reactor *reactor (void) const;

    //��ʱ���������������û���ڷ�Ӧ�����漯�ɶ�ʱ���Ĵ���
    //virtual int timer_timeout (const ZCE_Time_Value &tv, const void *arg = 0);

protected:

    ///��Ӧ��
    ZCE_Reactor       *zce_reactor_;

    ///��������ӦҪ������¼�MASK
    int                event_mask_;
};

//ȡ�õ�ǰ�ı�־λ
inline int ZCE_Event_Handler::get_mask()
{
    return event_mask_;
}

//���õ�ǰ��־λ
inline void ZCE_Event_Handler::set_mask(int mask)
{
    event_mask_ = mask;
}

//enable mask�����ı�־λ
inline void ZCE_Event_Handler::enable_mask(int en_mask)
{
    event_mask_ |= en_mask;
}
//disable mask�����ı�־λ
inline void ZCE_Event_Handler::disable_mask(int dis_mask)
{
    event_mask_ &= (~dis_mask);
}

#endif //ZCE_LIB_EVENT_HANDLE_BASE_H_

