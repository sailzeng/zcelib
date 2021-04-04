/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_os_adapt_flock.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2011��9��15��
* @brief      �ļ�����¼�����ĺ�������WIN32����LINUX ƽ̨ͨ��
*             ��ʵ����flock����������м�¼��������һ�㣬��Ϊ�����ֻ���ļ���һ���ֽ��в���
*
* @details    �ļ�����¼����������㣬����2��ƽ̨���ο�ACEʵ�ֵģ������������COPY
*             flock_xxx ���ĺ������ǷǱ�׼�ģ���ACE�Լ���һ���װ��������˵��ACE�ķ�װ
*             ���Ǳȵ���ԭ��API fcntl���
*             ����ACEû���ṩflock���������ṩ�ˣ���ĳ���ļ���ȫ�����ݽ��в�����ʹ��
*
* @note       �����ڼ�¼��������ʵҲ�ǲ���ϵͳ����Ȥ��һ���֣�������ʵ����ƽ̨���С��
*             LINUX�£���¼����Ȱ���Ե�����Ĭ���ǣ���
*             WINDOWS���ǣ���¼����ǿ�Ƶģ�
*             http://www.ibm.com/developerworks/cn/linux/l-cn-filelock/index.html
*             http://www.cnblogs.com/hustcat/archive/2009/03/10/1408208.html
*             �����Ҫ��UNP V2�ͻ����ף���ʵ��ô�һ�������໥��Լ����ʹ������
*
*/

#ifndef ZCE_LIB_OS_ADAPT_FLOCK_H_
#define ZCE_LIB_OS_ADAPT_FLOCK_H_

#include "zce_os_adapt_predefine.h"

/*!
* @brief      �ļ�������ķ�װ��
*/
struct zce_flock_t
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

namespace zce
{

//----------------------------------------------------------------------------------------
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
void flock_adjust_params (zce_flock_t *lock,
                          int whence,
                          size_t &start,
                          size_t &len);

/*!
* @brief      �ļ�����ʼ��,ֱ����fd
* @return         int        0�ɹ���-1ʧ��
* @param[in,out]  lock       �ļ�������
* @param[in]      file_hadle �������ļ����
*/
int flock_init (zce_flock_t *lock,
                ZCE_HANDLE file_hadle);

/*!
* @brief      �����ļ�������zce_flock_t��Ҳ����ͬʱ�⿪��������ͬʱ�ر��ļ�
* @param[in]  lock �ļ�������
*/
void flock_destroy (zce_flock_t *lock);

/*!
* @brief   ���ļ���ȡ������������������ܼ��������������ȴ���������������ֹ�����˶�ȡ
* @return        int     0�ɹ���-1ʧ��
* @param[in,out] lock    �ļ�������
* @param[in]     whence  �������ʼ��Դλ�ã���SEEK_SET��SEEK_CUR��SEEK_END
* @param[in]     start   �Ӹ�Դ��ʼ�����λ��
* @param[in]     len     ��������ĳ��ȣ�
*/
int flock_rdlock (zce_flock_t *lock,
                  int  whence = SEEK_SET,
                  size_t start = 0,
                  size_t len = 0);

/*!
* @brief  ���Խ��мӶ�ȡ��,������ܼ�����������������
* @return        int     0�ɹ���-1ʧ��
* @param[in,out] lock    �ļ�������
* @param[in]     whence  �������ʼ��Դλ�ã���SEEK_SET��SEEK_CUR��SEEK_END
* @param[in]     start   �Ӹ�Դ��ʼ�����λ��
* @param[in]     len     ��������ĳ��ȣ�
*/
int flock_tryrdlock (zce_flock_t *lock,
                     int  whence = SEEK_SET,
                     size_t start = 0,
                     size_t len = 0);

/*!
* @brief      ���ļ���д������������������ܼ��������������ȴ�
* @return        int     0�ɹ���-1ʧ��
* @param[in,out] lock    �ļ�������
* @param[in]     whence  �������ʼ��Դλ�ã���SEEK_SET��SEEK_CUR��SEEK_END
* @param[in]     start   �Ӹ�Դ��ʼ�����λ��
* @param[in]     len     ��������ĳ��ȣ�
*/
int flock_wrlock (zce_flock_t *lock,
                  int  whence = SEEK_SET,
                  size_t start = 0,
                  size_t len = 0);

/*!
* @brief      ���Խ��м�д������������,������ܼ�����������������
* @return        int     0�ɹ���-1ʧ��
* @param[in,out] lock    �ļ�������
* @param[in]     whence  �������ʼ��Դλ�ã���SEEK_SET��SEEK_CUR��SEEK_END
* @param[in]     start   �Ӹ�Դ��ʼ�����λ��
* @param[in]     len     ��������ĳ��ȣ�
*/
int flock_trywrlock (zce_flock_t *lock,
                     int  whence = SEEK_SET,
                     size_t start = 0,
                     size_t len = 0);

/*!
* @brief      ����
* @return        int     0�ɹ���-1ʧ��
* @param[in,out] lock    �ļ�������
* @param[in]     whence  �������ʼ��Դλ�ã���SEEK_SET��SEEK_CUR��SEEK_END
* @param[in]     start   �Ӹ�Դ��ʼ�����λ��
* @param[in]     len     ����������ĳ��ȣ�
*/
int flock_unlock (zce_flock_t *lock,
                  int  whence = SEEK_SET,
                  size_t start = 0,
                  size_t len = 0);


};

#endif //ZCE_LIB_OS_ADAPT_FLOCK_H_

