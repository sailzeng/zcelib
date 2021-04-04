/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_os_adapt_dirent.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2011��7��16��
* @brief      Ŀ¼�ĸ��ֲ�����ƽ̨���������ڸ��ֶ���Ŀ¼�Ĵ���������ȡ������Ŀ¼��ɾ���ȵȣ�
*
* @details
*
* @note
*
*/

#ifndef ZCE_LIB_OS_ADAPT_DIRENT_H_
#define ZCE_LIB_OS_ADAPT_DIRENT_H_

#include "zce_os_adapt_predefine.h"


#if defined (ZCE_OS_WINDOWS)

///
struct dirent
{
    /// inode number,WIN32��û��
    ino_t          d_ino;
    /// offset to the next dirent,WIN32��û��
    off_t          d_off;

    //���ȼ�¼
    unsigned short d_reclen;

    ///�ļ�����,LINUX��2.6.XX����İ汾��֧�����ѡ��
    unsigned char  d_type;

    //�ļ�����
    char           d_name[PATH_MAX + 1];
};

///readdir�Ⱥ��������ľ��
struct DIR
{
    ///Ŀ¼������
    char              directory_name_[PATH_MAX + 1];

    ///��ǰ����ľ��
    HANDLE            current_handle_;

    ///���ص�dirent_���
    dirent *dirent_;

    /// The struct for intermediate results.
    WIN32_FIND_DATAA  fdata_;

    ///�Ƿ��Ѿ��Ϳ�ʼ�ҵı�־
    int               started_reading_;
};

#endif

namespace zce
{

/*!
* @brief      ��һ��Ŀ¼���ж�ȡ
* @return     DIR* ���ص���Ŀ¼�ľ�����ɹ�����һ����NULLֵ���������closedir�ͷŵ���ʧ�ܷ���NULL
* @param      dir_name Ŀ¼����
* @note       ������Ϊ����������opendir��readdir��readdir_r����closedir���ο�closedir��readdir
*/
DIR *opendir (const char *dirname);

/*!
* @brief      �رմ򿪵�Ŀ¼���ľ������
* @return     int ==0��ʶ�ɹ�
* @param      dir_handle DIR�ľ��,�ڶ�ȡĿ¼��Ϻ󣬱���Ҫ�رգ�
* @note       ������Ϊ����������opendir��readdir��closedir
*/
int closedir (DIR *dir_handle);

/*!
* @brief      ��ȡһ��Ŀ¼����ҷ��أ�
* @return     struct dirent* ����NULL��ʶ��ȡʧ�ܣ�������ϣ�
* @param      dir_handle ��ȡĿ¼DIR���������ʲôλ�õ���Ϣ��ʵ�Ǽ�¼��DIR����ڲ�
* @note       WIN��ʵ����Ϊreaddir�ķ���dirent �ǲ���DIR *dir_handle�ڲ����ݣ���ȡ
*             ��λ����ϢҲ�Ǽ�¼�ڲ���DIR *dir_handle�ڲ����������ǲ�������汾��
*             LINUX��ʵ�ֿ��ܷ��ص���static���ݣ�,����Ҳ����������
*             ���һ������ע�͵�ʱ�򣬶��Լ�˵�Ĳ��������ֿ���һ�£����Ի��Ǽӵ�ע�ͼ�¼
*             ���������ڷ���ֵ�ǲ���dir_handle��һ�������ݣ�����������ε��ú�ǰ��һ��
*             �Ľṹ��ʧЧ�ˡ�
*/
struct dirent *readdir (DIR *dir_handle);

/*!
* @brief      read dir ��������汾��
* @return     int ==0 ��ʶ�ɹ�
* @param[in]  dir_handle ��ȡĿ¼DIR�����
* @param[out] entry ���صĶ�ȡ����Ŀ¼��Ŀ,entry�������ⲿ�Ѿ�����õı���
* @param[out] result ��ȡ����Ŀ¼��Ŀָ�룬������Ѿ���ȡ�������*result������ΪNULL
*                    �����ȡ������Ŀ��resultָ��entry
*/
int readdir_r (DIR *dir_handle,
               struct dirent *entry,
               struct dirent **result);


/*!
* @brief      ����ǰ׺�ͺ�׺����ȡĿ¼����ĸ����ļ���
* @return     int == 0 ��ʾ�ɹ�
* @param[in]  dirname ��ȡĿ¼
* @param[in]  prefix_name ǰ׺���ƣ�����ΪNULL��Ҳ���Ծ���ĳ���ļ�����
* @param[in]  ext_name ��׺���ƣ�����ΪNULL��ΪNULL��ʾ�����
* @param[in]  select_dir ѡ��Ŀ¼
* @param[in]  select_file ѡ���ļ�
* @param[out] file_name_ary �ļ����ƶ���
*/
int readdir_nameary(const char *dirname,
                    const char *prefix_name,
                    const char *ext_name,
                    bool select_dir,
                    bool select_file,
                    std::vector<std::string> &file_name_ary);

/*!
* @brief      ɨ��һ��Ŀ¼�����Ŀ¼��Ŀ����һ������������ȥ�򵥣����ҿ�������ѡ�����ȹ��߼ӿ쿪���ٶȣ���Ҫע�����ͷ�
* @return     int           ����ɨ�赽����Ŀ������������ֵ<0��ʾʧ��
* @param      dirname       Ŀ¼�����֣�
* @param      namelist      ע�������һ��ָ������ָ���ָ�룬�����ÿ������ҲҪʩ�ţ�����ҲҪ�ͷ�
* @param      (*selector)   ѡ�����ĺ���ָ�룬����ΪNULL��ѡ�������ط�0��ʾѡ��
* @param      (*comparator) �������ߵĺ���ָ�룬����ΪNULL
* @note       namelist ���ص�����һ���ͷţ�������2���ͷţ�������free_scandir_list�����ͷ�
*/
int scandir (const char *dirname,
             dirent **namelist[],
             int (*selector)(const struct dirent *),
             int (*comparator)(const struct dirent **, const struct dirent **));

/*!
* @brief      �ͷ�scandir ���ز���������ĸ��ַ������ݣ��Ǳ�׼����
* @param      list_number scandir �����ĳɹ�����ֵ,>0
* @param      namelist    scandir �������ص�namelist����
*/
void free_scandir_result(int list_number, dirent *namelist[]);

/*!
* @brief      ����Ŀ¼����ıȽϣ������Ǹ�comparator��������ָ��Ĳ���
* @return     int   ���ֵ�string�ȽϽ��
* @param      left  �Ƚϵ�Ŀ¼��Ŀ
* @param      right �Ƚϵ�Ŀ¼��Ŀ
*/
int scandir_namesort (const struct dirent **left,
                      const struct dirent **right);

/*!
* @brief      ��һ��·���õ��ļ������ƣ�Ӧ���ǷǱ�׼����
* @return     const char*    �����ļ��������ַ��������ص���ʵ����filename������BUFFER
* @param[in]  pathname  ·���ַ���
* @param[out] filename  ���ص��ļ������ַ����Ĵ��buf
* @param[in]  buf_len   filename����BUFFER�ĳ���
* @note
*/
const char *basename (const char *pathname, char *filename, size_t buf_len);

/*!
* @brief      ��һ��·���м�õ�Ŀ¼����
* @return     const char*    ���ص�Ŀ¼�ַ��������صľ���dir_name������BUFFER
* @param[in]  path_name  ·���ַ���
* @param[out] dir_name  ���Ŀ¼�ַ�����BUFFER
* @param[in]  buf_len    dir_name����BUFFER�ĳ���
* @note       Ŀ¼���Ƶ�ĩβû�д��ָ���
*/
const char *dirname (const char *path_name, char *dir_name, size_t buf_len);

/*!
* @brief      �õ���ǰĿ¼
* @return     char*   ��ǰĿ¼���ַ���ָ�룬����buffer��
* @param      buffer  ȡ�ص��ַ���
* @param      maxlen  �ַ�������
*/
char *getcwd(char *buffer, int maxlen  );

/*!
* @brief      CDĳ��Ŀ¼,�ı䵱ǰĿ¼
* @return     int     0�ɹ���-1ʧ��
* @param[in]  dirname Ŀ¼
*/
int chdir(const char *dirname );

/*!
* @brief      ����ĳ��Ŀ¼��ֻ�ܽ���һ��Ŀ¼
* @return     int      0�ɹ���-1ʧ��
* @param      pathname ·���ַ�����
* @param      mode     Ŀ¼�Ĺ���ģʽ��WINDOWS�£��˲�����Ч,
*/
int mkdir(const char *pathname, mode_t mode = ZCE_DEFAULT_DIR_PERMS);

/*!
* @brief      �ݹ�Ľ���Ŀ¼���Ǳ�׼�����������һ�ν������Ŀ¼�����������
* @return     int =0 ��ʾ�ɹ�
* @param      pathname ·���ַ�����
* @param      mode Ŀ¼�Ĺ���ģʽ��WINDOWS�£��˲�����Ч,
*/
int mkdir_recurse(const char *pathname, mode_t mode = ZCE_DEFAULT_DIR_PERMS);

/*!
* @brief      ɾ��ĳ��Ŀ¼
* @return     int ����0��ʶ�ɹ�
* @param      pathname Ŀ¼·��
* @note       �����������Ŀ¼�����ǿյģ�
*/
int rmdir(const char *pathname);


/*!
* @brief      ·��ƴ��
* @return     const char*
* @param      dst
* @param      src
* @note
*/
inline const char *path_str_cat(char *dst,
                                const char *src)
{
    size_t dst_len = ::strlen(dst);
    if ( dst_len > 0 )
    {
        if (ZCE_IS_DIRECTORY_SEPARATOR(dst[dst_len - 1]) == false)
        {
            ::strcat(dst, ZCE_DIRECTORY_SEPARATOR_STR);
        }
    }
    else
    {
        ::strcat(dst, ZCE_CURRENT_DIRECTORY_STR);
    }
    ::strcat(dst, src);
    return dst;
}


inline std::string &path_string_cat(std::string &dst,
                                    const std::string &src)
{
    size_t dst_len = dst.length();
    if ( dst_len > 0 )
    {
        if (ZCE_IS_DIRECTORY_SEPARATOR(dst[dst_len - 1]) == false)
        {
            dst += ZCE_DIRECTORY_SEPARATOR_STR;
        }
    }
    else
    {
        dst = ZCE_CURRENT_DIRECTORY_STR;
    }
    dst += src;
    return dst;
}


};


#endif //ZCE_LIB_OS_ADAPT_DIRENT_H_

