/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_lock_file_lock.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2013��1��14��
* @brief      �ļ������������������ļ���
*             ��¼����ο�@ref zce_lock_record_lock.h
*
* @details
*
* @note
*
*/

#ifndef ZCE_LIB_LOCK_FILE_LOCK_H_
#define ZCE_LIB_LOCK_FILE_LOCK_H_

#include "zce_boost_non_copyable.h"
#include "zce_lock_guard.h"

/*!
* @brief      �ļ��������������ļ����в���
*/
class ZCE_File_Lock : public ZCE_NON_Copyable
{
public:

    ///������GUARD
    typedef ZCE_Read_Guard<ZCE_File_Lock> LOCK_READ_GUARD;
    ///д����GUARD
    typedef ZCE_Write_Guard<ZCE_File_Lock> LOCK_WRITE_GUARD;

public:

    //���캯��
    ZCE_File_Lock();
    ~ZCE_File_Lock();

public:

    //ͨ���ļ����Ʋ�����ʼ���ļ������������ļ�
    int open(const char *file_name,
             int open_mode,
             mode_t perms = ZCE_DEFAULT_FILE_PERMS);

    //ͨ���ļ������ʼ���ļ���
    int open(ZCE_HANDLE file_handle);

    //�ر��ļ�����������Ҵ򿪵��ļ����ҹرգ�
    int close();

    //�õ����ļ��ľ��
    ZCE_HANDLE get_file_handle();

    //ע������Ķ�д����ʶ��д����ʽ�������Ƕ�д����

    //��ȡ��
    void lock_read();
    //���Զ�ȡ��
    bool try_lock_read();

    //д����
    void lock_write();
    //���Զ�ȡ��
    bool try_lock_write();

    //����,����Ƕ�д��Ҳֻ��Ҫ��һ������
    void unlock();

protected:

    ///�������ļ����
    ZCE_HANDLE    lock_file_hadle_;

    ///�Լ��򿪵�����ļ���Ҳ���ǵ��ù�open����
    bool          open_by_self_;

    ///
    size_t        file_len_;

    ///�ļ������
    zce_flock_t   file_lock_hdl_;

};

#endif //ZCE_LIB_LOCK_FILE_LOCK_H_

