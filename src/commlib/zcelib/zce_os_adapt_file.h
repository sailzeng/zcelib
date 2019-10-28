/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_os_adapt_file.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2011年7月10日
* @brief      文件操作的适配器层，主要还是向LINUX下靠拢
*
* @details    为什么在WINDOWS平台要提供一组以WINDOWS的HANDLE函数呢，而不是C文件描述符的函数呢
*             1.因为WINDOWS内部的API大量使用HANDLE作为参数而不是int fd,所以我不能完全依赖CRT
*             2.因为WINDOWS下CRT的函数实现都有一些瑕疵，比如lseek,(新的倒是有了_lseek64)
*
*             还有一个问题必须说明一下，文件长度的问题，文件操作的很多参数我用了ssize_t这个类型，
*             LINUX下往往是off_t，off_t大部分情况下和ssize_t表现一样，（除非你自己去编译内核）
*             ssize_t在32位系统下最大值就是2G，当然各种OS（包括32位）的文件肯定是可以突破2G,
*             这就给使用带来了麻烦，其实这是OS自己的麻烦，OS大部分都提供一组XXXX64的函数，比如
*             lseek64这样的函数来解决这个问题，
*             但如果我也要这样，我又觉得有点小麻烦，呵呵。作为一个服务器开发人员，我的运行环境其实
*             基本上都是64位的，而sszie_t在64位OS上也是64位（一个符号位），所以大部分情况下我没
*             麻烦。但如果哪天非要在32位的系统上折腾，估计还要折腾一下。
*
* @note       我曾经在其中的很多函数采用了先用C函数进行操作，然后得到WINDOWS句柄的方式
*             _get_osfhandle进行转换，需要转换回来的时候用_open_osfhandle，
*             但其实是完全误读了_open_osfhandle函数的意图，他不是转换，而是创建，
*             http://www.cnblogs.com/fullsail/archive/2012/10/21/2732873.html
*/

#ifndef ZCE_LIB_OS_ADAPT_FILE_H_
#define ZCE_LIB_OS_ADAPT_FILE_H_

#include "zce_os_adapt_predefine.h"

namespace zce
{

/*!
* @brief      打开一个文件
* @return     ZCE_HANDLE 打开文件的句柄
* @param      filename   文件名称
* @param      open_mode  打开文件的模式，两个平台通用的参数O_CREAT，O_APPEND，O_EXCL，O_TRUNC，O_RDONLY, O_WRONLY, and O_RDWR大致这几个
* @param      perms      文件的共享模式，WINDOWS下我会根据你的输入进行转换（其实差别不小），你可以使用LINUX下的共享方式参数
* @note       ZCE_DEFAULT_FILE_PERMS 应该是0660
*/
ZCE_HANDLE open(const char* filename,
                int open_mode,
                mode_t perms=ZCE_DEFAULT_FILE_PERMS);

/*!
* @brief      关闭一个文件
* @return     int ==0标识成功
* @param      handle 文件句柄
*/
int close(ZCE_HANDLE handle);

/*!
* @brief      读取文件，WINDOWS下，长度无法突破32位的
* @return     ssize_t 错误返回-1，，正确返回读取的字节长度（也可能为0），errno 表示错误原因
* @param      file_handle 文件句柄
* @param      buf 读取的buffer参数
* @param      count buffer的长度，WINDOWS下，长度无法突破32位的，当然……（当然其实函数的语义看32位下只能是2G），你懂得
*/
ssize_t read(ZCE_HANDLE file_handle,void* buf,size_t count);

/*!
* @brief      写如文件，
* @return     ssize_t 错误返回-1，，正确返回读取的字节长度（也可能为0），errno 表示错误原因
* @param      file_handle 文件句柄
* @param      buf 读取的buffer参数
* @param      count buffer的长度，WINDOWS下，长度无法突破32位的，当然有人需要写入4G数据吗？
* @note       注意Windows下默认调用的WriteFile还是有缓冲的，我为了和POSIX统一，还是用了FlushFileBuffers
*/
ssize_t write(ZCE_HANDLE file_handle,const void* buf,size_t count);

/*!
* @brief      在文件内进行偏移
* @return     ssize_t 返回当前的位置
* @param      file_handle
* @param      offset
* @param      whence
*/
ssize_t lseek(ZCE_HANDLE file_handle,ssize_t offset,int whence);

/*!
* @brief      断文件，倒霉的是WINDOWS下又TMD 没有，
* @return     int ==0 标识成功
* @param      file_handle 操作的文件句柄
* @param      offset      截断的大小
*/
int ftruncate(ZCE_HANDLE file_handle,size_t  offset);

///截断文件,根据文件名称
int truncate(const char* filename,size_t offset);

/*!
* @brief      通过文件句柄,得到文件的长度，关于文件长度的问题，你应该看看文件开始的注释段落
* @return     int 返回-1标识失败，返回0标识成功
* @param      file_handle 文件句柄
* @param      file_size   文件的大小，
* @note       这是一个非标准函数，少量系统提供函数off_t filesize(int file_handle);
*             考虑来考虑去，不使用off_t作为返回值或者参数，因为在win下只被定义成long,
*             原来用作为返回值，但由于在WINDOWS32下不超过2G，感觉不好，又改了成了size_t，
*             但是用size_t作为返回值又恶心,0到底是错误还是尺寸0？所以函数被改成了这样，中规中矩
*/
int filesize(ZCE_HANDLE file_handle,size_t* file_size);

///
/*!
* @brief      通过文件名称,取得文件的长度，
* @return     int  返回-1标识失败，返回0标识成功
* @param      filename  文件
* @param      file_size 文件的大小
* @note       不重载filesize的原因是filelen，filesize第一个参数都可能是指针
*/
int filelen(const char* filename,size_t* file_size);

/*!
* @brief      用模版名称建立并且打开一个临时文件，
* @return     ZCE_HANDLE 文件句柄
* @param      template_name 临时文件的模版名称
*/
ZCE_HANDLE mkstemp(char* template_name);

//得到文件的stat信息，你可以认为zce_os_stat就是stat，只是在WINDOWS下stat64,主要是为了长文件考虑的
int stat(const char* path,zce_os_stat* buf);
//通过文件句柄得到文件stat
int fstat(ZCE_HANDLE file_handle,zce_os_stat* buf);


///路径是否是一个目录，如果是返回true，如果不是返回false
bool is_directory(const char* path_name);


/*!
* @brief      删除文件
* @return     int == 0标识成功，
* @param      filename 文件名称
*/
int unlink(const char* filename);

///设置umask
mode_t umask(mode_t cmask);

/*!
* @brief      检查文件是否OK，吼吼
* @return     int       ==0 表示正确，错误返回-1
* @param      pathname  文件路径名称
* @param      mode      文件模式，支持F_OK,R_OR,W_OK,或者几个的|， Windows 不支持X_OK，LINUX支持
*/
int access(const char* pathname,int mode);

//-------------------------------------------------------------------------------------------------

/*!
* @brief      非标准函数
* @return     int
* @param[in]  filename 用只读方式读取一个文件的内容，
* @param[in]  buff     读取的buffer
* @param[in]  buf_len  buffer的长度
* @param[out] read_len 输出参数，返回读取的长度
* @param[in]  offset   开始读取的偏移是多少，从文件头位置计算，
*/
int read_file_data(const char* filename,
                   char* buff,
                   size_t buf_len,
                   size_t* read_len,
                   size_t offset=0);



/*!
* @brief      用C++ 11的方式包装一个给你。
* @return     std::pair<int,std::unique_ptr<char*>>
* @param[in]  filename
* @param[out] read_len 输出参数，返回读取的长度
* @param[in]  offset
* @note       Windows下面不可能一次读取超过4G大小的数据但，超大文件别指望这个函数
*/
std::pair<int,std::shared_ptr<char>> read_file_all(const char* filename,
                                                   size_t* file_len,
                                                   size_t offset=0);



/*!
* @brief     辅助 unique_ptr 用于文件处理过程的自动释放
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

