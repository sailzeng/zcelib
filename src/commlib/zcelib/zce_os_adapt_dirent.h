/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_os_adapt_dirent.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2011年7月16日
* @brief      目录的各种操作跨平台函数，用于各种对于目录的处理，包括读取，建立目录，删除等等，
*
* @details
*
* @note
*
*/

#ifndef ZCE_LIB_OS_ADAPT_DIRENT_H_
#define ZCE_LIB_OS_ADAPT_DIRENT_H_

#include "zce_os_adapt_predefine.h"

namespace ZCE_LIB
{

/*!
* @brief      打开一个目录进行读取
* @return     DIR* 返回的是目录的句柄，成功返回一个非NULL值，你必须用closedir释放掉，失败返回NULL
* @param      dir_name 目录名称
* @note       可以认为操作数序是opendir，readdir（readdir_r），closedir，参考closedir，readdir
*/
DIR *opendir (const char *dirname);

/*!
* @brief      关闭打开的目录（的句柄），
* @return     int ==0标识成功
* @param      dir_handle DIR的句柄,在读取目录完毕后，必须要关闭，
* @note       可以认为操作数序是opendir，readdir，closedir
*/
int closedir (DIR *dir_handle);

/*!
* @brief      读取一个目录项，并且返回，
* @return     struct dirent* 返回NULL标识读取失败，或者完毕，
* @param      dir_handle 读取目录DIR句柄，读到什么位置等信息其实是记录在DIR句柄内部
* @note       WIN的实现因为readdir的返回dirent 是参数DIR *dir_handle内部数据，读取
*             的位置信息也是记录在参数DIR *dir_handle内部，所以其是不可重入版本，
*             LINUX的实现可能返回的是static数据，,所以也不可以重入
*             最近一次增加注释的时候，对自己说的不可重入又考虑一下，所以还是加点注释记录
*             下来，由于返回值是参数dir_handle的一部分数据，所以如果两次调用后，前面一次
*             的结构就失效了。
*/
struct dirent *readdir (DIR *dir_handle);

/*!
* @brief      read dir 可以重入版本，
* @return     int ==0 标识成功
* @param[in]  dir_handle 读取目录DIR句柄，
* @param[out] entry 返回的读取到的目录项目,entry必须是外部已经分配好的变量
* @param[out] result 读取到的目录项目指针，如果是已经读取到了最后，*result将被置为NULL
*                    如果读取到了项目，result指向entry
*/
int readdir_r (DIR *dir_handle,
               struct dirent *entry,
               struct dirent **result);


/*!
* @brief      根据前缀和后缀，读取目录下面的各种文件，
* @return     int == 0 表示成功
* @param[in]  dirname 读取目录
* @param[in]  prefix_name 前缀名称，可以为NULL，也可以就是某个文件名称
* @param[in]  ext_name 后缀名称，可以为NULL，为NULL表示不检查
* @param[out] file_name_ary 文件名称队列
*/
int readdir_fileary(const char *dirname,
                    const char *prefix_name,
                    const char *ext_name,
                    std::vector<std::string> &file_name_ary);

/*!
* @brief      扫描一个目录里面的目录项目，就一个函数，看上去简单，而且可以利用选择器等工具加快开发速度，但要注意结果释放
* @return     int           返回扫描到的项目的数量，返回值<0表示失败
* @param      dirname       目录的名字，
* @param      namelist      注意这个是一个指向数组指针的指针，里面的每个数据也要施放，数组也要释放
* @param      (*selector)   选择器的函数指针，可以为NULL，选择器返回非0表示选择
* @param      (*comparator) 排序器具的函数指针，可以为NULL
* @note       namelist 返回的数据一定释放，而且是2次释放，可以用free_scandir_list函数释放
*/
int scandir (const char *dirname,
             dirent **namelist[],
             int (*selector)(const struct dirent *),
             int (*comparator)(const struct dirent **, const struct dirent **));

/*!
* @brief      释放scandir 返回参数的里面的各种分配数据，非标准函数
* @param      list_number scandir 函数的成功返回值,>0
* @param      namelist    scandir 函数返回的namelist参数
*/
void free_scandir_result(int list_number, dirent *namelist[]);

/*!
* @brief      用于目录排序的比较，就是那个comparator参数函数指针的参数
* @return     int   名字的string比较结果
* @param      left  比较的目录项目
* @param      right 比较的目录项目
*/
int scandir_namesort (const struct dirent **left,
                      const struct dirent **right);

/*!
* @brief      从一个路径得到文件的名称，应该是非标准函数
* @return     const char*    返回文件的名称字符串，返回的其实就是filename参数的BUFFER
* @param[in]  pathname  路径字符串
* @param[out] filename  返回的文件名称字符串的存放buf
* @param[in]  buf_len   filename参数BUFFER的长度
* @note
*/
const char *basename (const char *pathname, char *filename, size_t buf_len);

/*!
* @brief      从一个路径中间得到目录名称
* @return     const char*    返回的目录字符串，返回的就是dir_name参数的BUFFER
* @param[in]  path_name  路径字符串
* @param[out] dir_name  存放目录字符串的BUFFER
* @param[in]  buf_len    dir_name参数BUFFER的长度
* @note
*/
const char *dirname (const char *path_name, char *dir_name, size_t buf_len);

/*!
* @brief      得到当前目录
* @return     char*   当前目录的字符串指针，就是buffer，
* @param      buffer  取回的字符串
* @param      maxlen  字符串长度
*/
char *getcwd(char *buffer, int maxlen  );

/*!
* @brief      CD某个目录,改变当前目录
* @return     int     0成功，-1失败
* @param[in]  dirname 目录
*/
int chdir(const char *dirname );

/*!
* @brief      建立某个目录，只能建立一层目录
* @return     int      0成功，-1失败
* @param      pathname 路径字符串，
* @param      mode     目录的共享模式，WINDOWS下，此参数无效,
*/
int mkdir(const char *pathname, mode_t mode = ZCE_DEFAULT_DIR_PERMS);

/*!
* @brief      递归的建立目录，非标准函数，如果想一次建立多层目录，用这个函数
* @return     int =0 表示成功
* @param      pathname 路径字符串，
* @param      mode 目录的共享模式，WINDOWS下，此参数无效,
*/
int mkdir_recurse(const char *pathname, mode_t mode = ZCE_DEFAULT_DIR_PERMS);

/*!
* @brief      删除某个目录
* @return     int 返回0标识成功
* @param      pathname 目录路径
* @note       理论上上这个目录必须是空的，
*/
int rmdir(const char *pathname);


/*!
* @brief
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

