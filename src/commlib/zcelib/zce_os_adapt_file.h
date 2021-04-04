/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_os_adapt_file.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2011��7��10��
* @brief      �ļ��������������㣬��Ҫ������LINUX�¿�£
*
* @details    Ϊʲô��WINDOWSƽ̨Ҫ�ṩһ����WINDOWS��HANDLE�����أ�������C�ļ��������ĺ�����
*             1.��ΪWINDOWS�ڲ���API����ʹ��HANDLE��Ϊ����������int fd,�����Ҳ�����ȫ����CRT
*             2.��ΪWINDOWS��CRT�ĺ���ʵ�ֶ���һЩ覴ã�����lseek,(�µĵ�������_lseek64)
*
*             ����һ���������˵��һ�£��ļ����ȵ����⣬�ļ������ĺܶ����������ssize_t������ͣ�
*             LINUX��������off_t��off_t�󲿷�����º�ssize_t����һ�������������Լ�ȥ�����ںˣ�
*             ssize_t��32λϵͳ�����ֵ����2G����Ȼ����OS������32λ�����ļ��϶��ǿ���ͻ��2G,
*             ��͸�ʹ�ô������鷳����ʵ����OS�Լ����鷳��OS�󲿷ֶ��ṩһ��XXXX64�ĺ���������
*             lseek64�����ĺ��������������⣬
*             �������ҲҪ���������־����е�С�鷳���Ǻǡ���Ϊһ��������������Ա���ҵ����л�����ʵ
*             �����϶���64λ�ģ���sszie_t��64λOS��Ҳ��64λ��һ������λ�������Դ󲿷��������û
*             �鷳������������Ҫ��32λ��ϵͳ�����ڣ����ƻ�Ҫ����һ�¡�
*
* @note       �����������еĺܶຯ������������C�������в�����Ȼ��õ�WINDOWS����ķ�ʽ
*             _get_osfhandle����ת������Ҫת��������ʱ����_open_osfhandle��
*             ����ʵ����ȫ�����_open_osfhandle��������ͼ��������ת�������Ǵ�����
*             http://www.cnblogs.com/fullsail/archive/2012/10/21/2732873.html
*/

#ifndef ZCE_LIB_OS_ADAPT_FILE_H_
#define ZCE_LIB_OS_ADAPT_FILE_H_

#include "zce_os_adapt_predefine.h"

namespace zce
{

/*!
* @brief      ��һ���ļ�
* @return     ZCE_HANDLE ���ļ��ľ��
* @param      filename   �ļ�����
* @param      open_mode  ���ļ���ģʽ������ƽ̨ͨ�õĲ���O_CREAT��O_APPEND��O_EXCL��O_TRUNC��O_RDONLY, O_WRONLY, and O_RDWR�����⼸��
* @param      perms      �ļ��Ĺ���ģʽ��WINDOWS���һ��������������ת������ʵ���С���������ʹ��LINUX�µĹ���ʽ����
* @note       ZCE_DEFAULT_FILE_PERMS Ӧ����0660
*/
ZCE_HANDLE open(const char* filename,
                int open_mode,
                mode_t perms=ZCE_DEFAULT_FILE_PERMS);

/*!
* @brief      �ر�һ���ļ�
* @return     int ==0��ʶ�ɹ�
* @param      handle �ļ����
*/
int close(ZCE_HANDLE handle);

/*!
* @brief      ��ȡ�ļ���WINDOWS�£������޷�ͻ��32λ��
* @return     ssize_t ���󷵻�-1������ȷ���ض�ȡ���ֽڳ��ȣ�Ҳ����Ϊ0����errno ��ʾ����ԭ��
* @param      file_handle �ļ����
* @param      buf ��ȡ��buffer����
* @param      count buffer�ĳ��ȣ�WINDOWS�£������޷�ͻ��32λ�ģ���Ȼ��������Ȼ��ʵ���������忴32λ��ֻ����2G�����㶮��
*/
ssize_t read(ZCE_HANDLE file_handle,void* buf,size_t count);

/*!
* @brief      д���ļ���
* @return     ssize_t ���󷵻�-1������ȷ���ض�ȡ���ֽڳ��ȣ�Ҳ����Ϊ0����errno ��ʾ����ԭ��
* @param      file_handle �ļ����
* @param      buf ��ȡ��buffer����
* @param      count buffer�ĳ��ȣ�WINDOWS�£������޷�ͻ��32λ�ģ���Ȼ������Ҫд��4G������
* @note       ע��Windows��Ĭ�ϵ��õ�WriteFile�����л���ģ���Ϊ�˺�POSIXͳһ����������FlushFileBuffers
*/
ssize_t write(ZCE_HANDLE file_handle,const void* buf,size_t count);

/*!
* @brief      ���ļ��ڽ���ƫ��
* @return     ssize_t ���ص�ǰ��λ��
* @param      file_handle
* @param      offset
* @param      whence
*/
ssize_t lseek(ZCE_HANDLE file_handle,ssize_t offset,int whence);

/*!
* @brief      ���ļ�����ù����WINDOWS����TMD û�У�
* @return     int ==0 ��ʶ�ɹ�
* @param      file_handle �������ļ����
* @param      offset      �ضϵĴ�С
*/
int ftruncate(ZCE_HANDLE file_handle,size_t  offset);

///�ض��ļ�,�����ļ�����
int truncate(const char* filename,size_t offset);

/*!
* @brief      ͨ���ļ����,�õ��ļ��ĳ��ȣ������ļ����ȵ����⣬��Ӧ�ÿ����ļ���ʼ��ע�Ͷ���
* @return     int ����-1��ʶʧ�ܣ�����0��ʶ�ɹ�
* @param      file_handle �ļ����
* @param      file_size   �ļ��Ĵ�С��
* @note       ����һ���Ǳ�׼����������ϵͳ�ṩ����off_t filesize(int file_handle);
*             ����������ȥ����ʹ��off_t��Ϊ����ֵ���߲�������Ϊ��win��ֻ�������long,
*             ԭ������Ϊ����ֵ����������WINDOWS32�²�����2G���о����ã��ָ��˳���size_t��
*             ������size_t��Ϊ����ֵ�ֶ���,0�����Ǵ����ǳߴ�0�����Ժ������ĳ����������й��о�
*/
int filesize(ZCE_HANDLE file_handle,size_t* file_size);

///
/*!
* @brief      ͨ���ļ�����,ȡ���ļ��ĳ��ȣ�
* @return     int  ����-1��ʶʧ�ܣ�����0��ʶ�ɹ�
* @param      filename  �ļ�
* @param      file_size �ļ��Ĵ�С
* @note       ������filesize��ԭ����filelen��filesize��һ��������������ָ��
*/
int filelen(const char* filename,size_t* file_size);

/*!
* @brief      ��ģ�����ƽ������Ҵ�һ����ʱ�ļ���
* @return     ZCE_HANDLE �ļ����
* @param      template_name ��ʱ�ļ���ģ������
*/
ZCE_HANDLE mkstemp(char* template_name);

//�õ��ļ���stat��Ϣ���������Ϊzce_os_stat����stat��ֻ����WINDOWS��stat64,��Ҫ��Ϊ�˳��ļ����ǵ�
int stat(const char* path,zce_os_stat* buf);
//ͨ���ļ�����õ��ļ�stat
int fstat(ZCE_HANDLE file_handle,zce_os_stat* buf);


///·���Ƿ���һ��Ŀ¼������Ƿ���true��������Ƿ���false
bool is_directory(const char* path_name);


/*!
* @brief      ɾ���ļ�
* @return     int == 0��ʶ�ɹ���
* @param      filename �ļ�����
*/
int unlink(const char* filename);

///����umask
mode_t umask(mode_t cmask);

/*!
* @brief      ����ļ��Ƿ�OK�����
* @return     int       ==0 ��ʾ��ȷ�����󷵻�-1
* @param      pathname  �ļ�·������
* @param      mode      �ļ�ģʽ��֧��F_OK,R_OR,W_OK,���߼�����|�� Windows ��֧��X_OK��LINUX֧��
*/
int access(const char* pathname,int mode);

//-------------------------------------------------------------------------------------------------

/*!
* @brief      �Ǳ�׼����
* @return     int
* @param[in]  filename ��ֻ����ʽ��ȡһ���ļ������ݣ�
* @param[in]  buff     ��ȡ��buffer
* @param[in]  buf_len  buffer�ĳ���
* @param[out] read_len ������������ض�ȡ�ĳ���
* @param[in]  offset   ��ʼ��ȡ��ƫ���Ƕ��٣����ļ�ͷλ�ü��㣬
*/
int read_file_data(const char* filename,
                   char* buff,
                   size_t buf_len,
                   size_t* read_len,
                   size_t offset=0);



/*!
* @brief      ��C++ 11�ķ�ʽ��װһ�����㡣
* @return     std::pair<int,std::unique_ptr<char*>>
* @param[in]  filename
* @param[out] read_len ������������ض�ȡ�ĳ���
* @param[in]  offset
* @note       Windows���治����һ�ζ�ȡ����4G��С�����ݵ��������ļ���ָ���������
*/
std::pair<int,std::shared_ptr<char>> read_file_all(const char* filename,
                                                   size_t* file_len,
                                                   size_t offset=0);



/*!
* @brief     ���� unique_ptr �����ļ�������̵��Զ��ͷ�
*/
inline void close_FILE_assist(FILE* to_close)
{
    ::fclose(to_close);
}

inline void close_HANDLE_assist(ZCE_HANDLE to_close)
{
    zce::close(to_close);
}

inline void close_stdFILE_assist(std::FILE* to_close)
{
    std::fclose(to_close);
}

};



#endif //ZCE_LIB_OS_ADAPT_FILE_H_

