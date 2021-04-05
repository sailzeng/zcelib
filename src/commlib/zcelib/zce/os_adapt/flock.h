/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/os_adapt/flock.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2011��9��15��
* @brief      �ļ�����¼�����ĺ�������WIN32����LINUX ƽ̨ͨ��
*             ��ʵ����flock����������м�¼��������һ�㣬��Ϊ�����ֻ���ļ���һ���ֽ��в���
*
* @details    �ļ�����¼����������㣬����2��ƽ̨��
*             ��¼���ģ�fcntl�Ĳ�����û��ֱ��ģ��fcntl�Ĳ��������ֲο�ACE��װ��������˵��
*             ACE�ķ�װ���Ǳȵ���ԭ��API fcntl����ġ�
*             ���⣬��ͬ���װ�� fcntl ������һ��Ը����fcntl��һ����ϼ���ֱ�����������
*             ���ַ������������
*             �ļ���ʹ�õ���flock�������������ǣ���ʵWindows����Ȼ���õļ�¼��ʵ�ֵ�
*
* @note       �����ڼ�¼��������ʵҲ�ǲ���ϵͳ����Ȥ��һ���֣�������ʵ����ƽ̨���С��
*             LINUX�£���¼����Ȱ���Ե�����Ĭ���ǣ�������ɻ��»����ܸɵġ�
*             WINDOWS���ǣ���¼����ǿ�Ƶģ�
*             http://www.ibm.com/developerworks/cn/linux/l-cn-filelock/index.html
*             http://www.cnblogs.com/hustcat/archive/2009/03/10/1408208.html
*             �����Ҫ��UNP V2�ͻ����ף���ʵ��ô�һ�������໥��Լ����ʹ������
*
*/

#ifndef ZCE_LIB_OS_ADAPT_FLOCK_H_
#define ZCE_LIB_OS_ADAPT_FLOCK_H_

#include "zce/os_adapt/common.h"



namespace zce
{



/*!
* @brief      ��¼������ķ�װ��
*/
struct file_lock_t
{
public:

# if defined (ZCE_OS_WINDOWS)

    //OVERLAPPED ��Ҫ����һЩ�ļ�ƫ����Ϣ
    OVERLAPPED   overlapped_ = {0};

# elif defined (ZCE_OS_LINUX)
    //�ļ���flock����
    struct flock lock_;
# endif

    ///������ļ���� Handle to the underlying file.
    ZCE_HANDLE   handle_ = ZCE_INVALID_HANDLE;
};

/*!
* @brief      ��¼�����ļ�����ʼ��,ֱ�����ļ������ʼ��
* @return         int        0�ɹ���-1ʧ��
* @param[in,out]  lock       �ļ�������
* @param[in]      file_hadle �������ļ����
*/
int file_lock_init(file_lock_t *lock,
                   ZCE_HANDLE file_hadle);



//----------------------------------------------------------------------------------------
// ��¼���Ĺ��ܣ�ģ�µ���fcntl�Ĺ���

//

/*!
* @brief      ������������Ҫ��������Windowsƽ̨Ҳ��ʹ�� �ڲ�����,�ⲿ��Ҫʹ�ã�
*             ����ƽ̨�ڲ�����ʹ���ϲ���̫��ͬ��WINDOWS��APIû�п������λ����Щ���
*             ���Ա�����ʹ��ǰ����һ�µ��������磬��ʼλ��SEEK_SET,0,����0����ʵ������
*             �����ļ�����LockFileExû�������ı�ʾ�������������
* @param[in,out] lock    �ļ�������
* @param[in]     whence  �������ʼ��Դλ�ã���SEEK_SET��SEEK_CUR��SEEK_END
* @param[in]     start   �Ӹ�Դ��ʼ�����λ��
* @param[in]     len     ��������ĳ��ȣ�
* @note          ƽ̨�Ĳ����ݻ����ĳ�ַ��գ�Windows��һ���ļ���С������������������Ͳ����ˣ�
*                ��������Ҫ���ݵĻ���������ļ���С�ǲ������ģ�
*/
void fcntl_lock_adjust_params(file_lock_t *lock,
                              int whence,
                              ssize_t &start,
                              ssize_t &len);





/*!
* @brief   ���ļ���ȡ������������������ܼ��������������ȴ���������������ֹ�����˶�ȡ
* @return        int     0�ɹ���-1ʧ��
* @param[in,out] lock    �ļ�������
* @param[in]     whence  �������ʼ��Դλ�ã���SEEK_SET��SEEK_CUR��SEEK_END
* @param[in]     start   �Ӹ�Դ��ʼ�����λ��
* @param[in]     len     ��������ĳ��ȣ�
*/
int fcntl_rdlock(file_lock_t *lock,
                 int  whence = SEEK_SET,
                 ssize_t start = 0,
                 ssize_t len = 0);

/*!
* @brief  ���Խ��мӶ�ȡ��,������ܼ�����������������
* @return        int     0�ɹ���-1ʧ��
* @param[in,out] lock    �ļ�������
* @param[in]     whence  �������ʼ��Դλ�ã���SEEK_SET��SEEK_CUR��SEEK_END
* @param[in]     start   �Ӹ�Դ��ʼ�����λ��
* @param[in]     len     ��������ĳ��ȣ�
*/
int fcntl_tryrdlock(file_lock_t *lock,
                    int whence = SEEK_SET,
                    ssize_t start = 0,
                    ssize_t len = 0);

/*!
* @brief      ���ļ���д������������������ܼ��������������ȴ�
* @return        int     0�ɹ���-1ʧ��
* @param[in,out] lock    �ļ�������
* @param[in]     whence  �������ʼ��Դλ�ã���SEEK_SET��SEEK_CUR��SEEK_END
* @param[in]     start   �Ӹ�Դ��ʼ�����λ��
* @param[in]     len     ��������ĳ��ȣ�
*/
int fcntl_wrlock(file_lock_t *lock,
                 int whence = SEEK_SET,
                 ssize_t start = 0,
                 ssize_t len = 0);

/*!
* @brief      ���Խ��м�д������������,������ܼ�����������������
* @return        int     0�ɹ���-1ʧ��
* @param[in,out] lock    �ļ�������
* @param[in]     whence  �������ʼ��Դλ�ã���SEEK_SET��SEEK_CUR��SEEK_END
* @param[in]     start   �Ӹ�Դ��ʼ�����λ��
* @param[in]     len     ��������ĳ��ȣ�
*/
int fcntl_trywrlock(file_lock_t *lock,
                    int whence = SEEK_SET,
                    ssize_t start = 0,
                    ssize_t len = 0);

/*!
* @brief      ����
* @return        int     0�ɹ���-1ʧ��
* @param[in,out] lock    �ļ�������
* @param[in]     whence  �������ʼ��Դλ�ã���SEEK_SET��SEEK_CUR��SEEK_END
* @param[in]     start   �Ӹ�Դ��ʼ�����λ��
* @param[in]     len     ����������ĳ��ȣ�
*/
int fcntl_unlock(file_lock_t *lock,
                 int whence = SEEK_SET,
                 ssize_t start = 0,
                 ssize_t len = 0);


//----------------------------------------------------------------------------------------
// �ļ����Ĺ��ܣ�ģ�µ���flock�Ĺ���

/*!
* @brief      �ļ�������
* @return     int
* @param      lock_hadle ע������ط���file_lock_t����Ҫ��file_lock_init�ȳ�ʼ��
* @param      operation ��Ӧ�Ĳ�������LOCK_SH LOCK_EX LOCK_UN LOCK_NB �����
*             LOCK_SH ���Լ���
*             LOCK_EX ǿ�Ƽ���
*             LOCK_NB ���������Ժ�LOCK_SH��LOCK_EX���ʹ�á���|
*             LOCK_UN ����
*/
int flock(file_lock_t& lock_hadle,int operation);



};

#endif //ZCE_LIB_OS_ADAPT_FLOCK_H_

