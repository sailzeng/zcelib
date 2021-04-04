/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_lock_base.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2011��9��13��
* @brief      ��������(��¼������)�Ļ����࣬����Ҳ�ж�̬ʹ�ã��ı䣩������Ϊ�ķ�ʽ
*             Ϊ�˼�������ACE��ô���꣬�һ���������ACE��һЩ��ʽ������⡣
*             ��Ȼ��Ҳ�����һ��ǻ�ѧϰһЩBOOST������
*
*             ��ο�
*             http://www.cnblogs.com/fullsail/archive/2012/07/31/2616106.html
* @details
*
* @note
*
*/

#ifndef ZCE_LIB_LOCK_BASE_H_
#define ZCE_LIB_LOCK_BASE_H_

#include "zce_lock_guard.h"
#include "zce_boost_non_copyable.h"

class ZCE_Time_Value;

/*!
* @brief      ��������(��¼������)�Ļ����࣬����Ҳ�ж�̬ʹ�ã��ı䣩������Ϊ�ķ�ʽ
*             ����Ľӿ�������BOOST�Ľӿڣ����粻���Ʒ���ֵ��Ҳ�ο���һЩACE�Ĵ���
* @detail     �����˺ܾã�����Ƿ�����͵����������ZCE_Null_Mutex���ܷǳ��ӽ������濴��������
*             ��ZCE_Lock_Base�ڲ��ĳ�Ա����Ӧ����private�ģ���ZCE_Null_Mutex���ڲ���Ա��Ӧ����
*             public�����ì���ǲ��ɵ��͵�
*/
class ZCE_Lock_Base : public ZCE_NON_Copyable
{

protected:
    ///���캯���������������������������������д��
    ZCE_Lock_Base (const char * = NULL);
public:
    ///��������
    virtual ~ZCE_Lock_Base (void);

    ///����ZCE_Lock_Ptr_Guardʹ��һЩ����
    friend class ZCE_Lock_Ptr_Guard;

    //Ϊ�˱��������˵�ʹ�ã��ش˽���Щ������������
private:
    ///����
    virtual void lock();

    ///��������
    virtual bool try_lock();

    ///����,
    virtual void unlock();

    ///������
    virtual void unlock_read();

    ///����д
    virtual void unlock_write();

    /*!
    * @brief      ����ʱ�䳬ʱ�ĵ���������ʱ������������Ƿ�ʱ
    * @return     virtual bool
    * @param      abs_time
    * @note
    */
    virtual bool systime_lock(const ZCE_Time_Value &abs_time);

    /*!
    * @brief      ��������ȴ�һ�����ʱ��
    * @return     bool          ����true�ɹ���ȡ����falseʧ�ܣ�����ʱ�ȣ�
    * @param      relative_time �ȴ������ʱ�䣬
    */
    virtual bool duration_lock(const ZCE_Time_Value &relative_time );

    ///��ȡ��
    virtual void lock_read();
    ///���Զ�ȡ��
    virtual bool try_lock_read();

    ///����ʱ��,��ȡ��ȡ���������ģ��ȴ�������ʱ�䳬ʱ
    ///@param  abs_time ����ʱ��
    virtual bool timed_lock_read(const ZCE_Time_Value & /*abs_time*/);
    ///���ʱ��,��ȡ��ȡ���������ģ��ȴ������ʱ�䳬ʱ
    ///@param  relative_time ���ʱ��
    virtual bool duration_lock_read(const ZCE_Time_Value & /*relative_time*/);

    ///д����
    virtual void lock_write();
    /*!
    * @brief      ���Զ�ȡ��
    * @return     bool  ����true�ɹ���ȡ����falseʧ�ܣ�
    */
    virtual bool try_lock_write();

    /*!
    * @brief      ����ʱ��,��ȡд�루��ռ�����ģ��ȴ�������ʱ�䳬ʱ
    * @return     bool     ����true�ɹ���ȡ����falseʧ�ܣ�
    * @param      abs_time ����ʱ��
    * @note
    */
    virtual bool timed_lock_write(const ZCE_Time_Value &abs_time);

    /*!
    * @brief      ���ʱ��,��ȡд�루��ռ�����ģ��ȴ������ʱ�䳬ʱ
    * @return     bool          ����true�ɹ���ȡ����falseʧ�ܣ�
    * @param      relative_time ���ʱ��
    */
    virtual bool duration_lock_write(const ZCE_Time_Value &relative_time);

};

/*!
* @brief      CV,Condition Variable ���������Ļ��࣬����������
*             ��չӦ�ö��Ǵ����������չ
*
*/
class ZCE_Condition_Base : public ZCE_NON_Copyable
{

protected:
    ///���캯��,protected���������������������д��
    ZCE_Condition_Base();
public:
    ///����������
    virtual ~ZCE_Condition_Base();

    //Ϊ�˱��������˵�ʹ�ã��ش˽���Щ������������
private:

    ///�ȴ�,
    virtual void wait (ZCE_Lock_Base *external_mutex );

    ///����ʱ�䳬ʱ�ĵĵȴ�����ʱ�����
    virtual bool systime_wait(ZCE_Lock_Base *external_mutex, const ZCE_Time_Value &abs_time);

    ///���ʱ��ĳ�ʱ�����ȴ�����ʱ�󣬽���
    virtual bool duration_wait(ZCE_Lock_Base *external_mutex, const ZCE_Time_Value &relative_time);

    /// ��һ���ȴ��̷߳����ź� Signal one waiting thread.
    virtual void signal (void);

    ///�����еĵȴ��̹߳㲥�ź� Signal *all* waiting threads.
    virtual void broadcast (void);

};

#endif //ZCE_LIB_LOCK_BASE_H_

