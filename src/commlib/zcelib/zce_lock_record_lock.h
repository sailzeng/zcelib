/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_lock_record_lock.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2012��10��1��
* @brief      ��¼���ķ�װ�����������ļ��е�һ�����ֽڣ��ֶ�
*             ֻ��OS���ϱ�����һ���װ��
*             �������ڣ���¼������ԭ�������ܴ�ZCE_Lock_Base����̳��ˣ�
* @details    ��ЩDB�����Ҽ�����ĳЩ��Ʒ�����ü�¼������ͬ���û���ĳЩ������
*             ����ο�һ��@ref zce_os_adapt_flock.h
* @note       ����Windows �¼�¼�������������size Ϊ0ʱ��������������ʼƫ��
*             ���ļ�ƫ�Ƶ����ֵ�����ԣ���������ļ���С�仯������ֲ�������û������
*
*/
#ifndef ZCE_LIB_LOCK_RECORD_LOCK_H_
#define ZCE_LIB_LOCK_RECORD_LOCK_H_

#include "zce_lock_base.h"
#include "zce_os_adapt_flock.h"

/*!
* @brief      ��¼�������������ļ��е�һ�����ֽڣ��ֶ�
*
*/
class ZCE_Record_Lock : public ZCE_NON_Copyable
{

public:

    /*!
    * @brief      ���캯��
    * @param      file_handle �ļ����
    */
    ZCE_Record_Lock(ZCE_HANDLE file_handle);

    /// ���캯��
    ZCE_Record_Lock();
    /// ��������
    ~ZCE_Record_Lock();

public:

    /*!
    * @brief      ��һ���ļ�,ͬʱ��ʼ��������lock����
    * @return     int 0�ɹ���-1ʧ��
    * @param[in]  file_name �ļ�����
    * @param[in]  open_mode ��ģʽ
    * @param[in]  perms     Ȩ�޲���
    */
    int open(const char *file_name,
             int open_mode,
             mode_t perms = ZCE_DEFAULT_FILE_PERMS);

    /*!
    * @brief      ��һ���ļ�Handle��ʼ��
    * @return     int   0�ɹ���-1ʧ��
    * @param      file_handle �Ѿ��򿪵��ļ��ľ����
    */
    int open(ZCE_HANDLE file_handle);

    /*!
    * @brief      �ر�֮��
    */
    void close();

    /*!
    * @brief      �õ���¼���ļ��ľ��
    * @return     ZCE_HANDLE
    */
    ZCE_HANDLE get_file_handle();

    //ע������Ķ�д����ʶ��д����ʽ�������Ƕ�д����
    //���һ���Ȱ���Ի���ǿ���Ե��������ڶ�д

    ///�ļ���¼��д��,�����ο�@ref flock_rdlock
    int flock_rdlock (int  whence,
                      ssize_t start,
                      ssize_t len);

    ///���Զ��ļ���¼���мӶ�ȡ��
    int flock_tryrdlock (int  whence,
                         ssize_t start,
                         ssize_t len);

    ///���ļ���¼ֱ������
    int flock_wrlock (int  whence,
                      ssize_t start,
                      ssize_t len);

    ///���Զ��ļ���¼���м�дȡ��
    int flock_trywrlock (int  whence,
                         ssize_t start,
                         ssize_t len);

    ///����
    int flock_unlock (int  whence,
                      ssize_t start,
                      ssize_t len);

protected:

    ///��¼����
    zce_flock_t    record_lock_;

};

#endif //ZCE_LIB_LOCK_RECORD_LOCK_H_
