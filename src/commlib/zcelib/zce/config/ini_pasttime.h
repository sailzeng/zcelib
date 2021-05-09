/*!
* @copyright  2002-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/config/ini_pasttime.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2002年1月3日
* @brief      INI 文件的读写的怀旧版，和WINDOWS的API功能类似，
*             优势在于写回的时候中会保证文件原来的格式和注释，不会破坏文件的格式。
*             另外，这个API有一些特殊功能，比如读取一个section下所有的字段内容等，
*             保留这个文件的目的仅仅是留作纪念
* @details    里面INI文件，一行的最大长度是8K
*
* @note       这应该是2002年，2003年在计费项目时，自己在家里写的代码。某种程
*             度上，这个代码估计是不再会使用到了，
*             保留他某种程度是更多的怀旧。所以最后也还是把这些代码整理成了最新的
*             代码规范
*             整理注释发现里面还有WIN95的提示，呵呵
*/

#pragma once

namespace zce
{
/*!
* @brief      INI文件读写的处理，接口都模仿WIndows的API
*/
class INI_Pt
{
public:

    ///构造函数,
    ///@param file_name 文件名称
    INI_Pt(const char* file_name);
    //析构函数
    ~INI_Pt();

public:

    /*!
    * @brief      读取INI文件的字段，段落，模仿WIN32接口GetPrivateProfileString,
    *             所有参数我就直接给你MSDN了
    * @return     size_t
    *             If the function succeeds, the return value is the number of characters
    *             copied to the buffer, not including the terminating null character.
    *             If neither sec_name nor key_name is NULL and the supplied destination
    *             buffer is too small to hold the requested string, the string is
    *             truncated and followed by a null character, and the return value is
    *             equal to size_ret_str minus one.
    *             If either sec_name or key_name is NULL and the supplied destination
    *             buffer is too small to hold all the strings, the last string is
    *             truncated and followed by two null characters. In this case, the return
    *             value is equal to size_ret_str minus two.
    * @param      sec_name
    *             Points to a null-terminated string that specifies the section containing
    *             the key name. If this parameter is NULL, the get_privateprofile_string
    *             function copies all section names in the file to the supplied buffer.
    * @param      key_name
    *             Pointer to the null-terminated string containing the key name whose
    *             associated string is to be retrieved. If this parameter is NULL, all key
    *             names in the section specified by the sec_name parameter are copied to
    *             the buffer specified by the return_str parameter.
    * @param      default_str
    *             Pointer to a null-terminated default string. If the key_name key cannot
    *             be found in the initialization file, get_privateprofile_string copies the
    *             default string to the return_str buffer. This parameter cannot be NULL.
    *             Avoid specifying a default string with trailing blank characters. The
    *             function inserts a null character in the return_str * buffer to strip any
    *             trailing blanks.
    * @param      return_str
    *             Pointer to the buffer that receives the retrieved string.
    * @param      size_ret_str
    *             Specifies the size, in characters, of the buffer pointed to by the return_str
    *             parameter.
    * @param      file_name
    *             Pointer to a null-terminated string that names the initialization file. If
    *             this parameter does not contain a full path to the file, Windows searches
    *             for the file in the Windows directory.
    */
    size_t get_private_str(const char* sec_name,
                           const char* key_name,
                           const char* default_str,
                           char* return_str,
                           const size_t size_ret_str);

    ///得到所有的section,用'\0'分隔，以'\0\0'结束，参数返回值参考get_privateprofile_string
    size_t get_private_allsection(char* return_str,
                                  const size_t size_ret_str);

    ///得到某个Section下所有的Key值，参数返回值参考get_privateprofile_string
    size_t getprivate_allkey(const char* sec_name,
                             char* return_str,
                             const size_t size_ret_str);

    ///得到Int的 Key值，参数返回值参考get_privateprofile_string
    int get_private_int(const char* sec_name,
                        const char* key_name,
                        const int default_int);

    ///得到bool的 Key值，参数返回值参考get_privateprofile_string
    bool   get_private_bool(const char* sec_name,
                            const char* key_name,
                            const bool default_int);

    /*!
    * @brief      写某个字段，模仿WIN32接口WritePrivateProfileString
    * @return     bool
    *             If the function successfully copies the string to the initialization
    *             file, the return value is nonzero.
    *             If the function fails, or if it flushes the cached version of the most
    *             recently accessed initialization file, the return value is zero. To get
    *             extended error information, call GetLastError.
    * @param      sec_name
    *             Points to a null-terminated string containing the name of the section
    *             to which the string will be copied. If the section does not exist, it
    *             is created. The name of the section is case-independent; the string
    *             can be any combination of uppercase and lowercase letters.
    * @param      key_name、
    *             Points to the null-terminated string containing the name of the key
    *             to be associated with a string. If the key does not exist in the
    *             specified section, it is created. If this parameter is NULL, the entire
    *             section, including all entries within the section, is deleted.
    * @param      write_string
    *             Points to a null-terminated string to be written to the file. If this
    *             parameter is NULL, the key pointed to by the key_name parameter is deleted.
    * @param      file_name
    *             Points to a null-terminated string that names the initialization file.
    */
    bool   write_private_string(const char* sec_name,
                                const char* key_name,
                                const char* write_string);

    ///修改Section
    bool   write_private_section(const char* sec_name,
                                 const char* write_string);
    ///写入Int
    bool   write_private_int(const char* sec_name,
                             const char* key_name,
                             const int nvalue);
    ///写入Bool变量
    bool   write_private_bool(const char* sec_name,
                              const char* key_name,
                              const bool bvalue);

    ///删除Key
    bool   del_private_key(const char* sec_name,
                           const char* key_name);
    ///删除Section
    bool   del_private_section(const char* sec_name);

protected:
    //每行的最大长度
    static const size_t LINE_BUFFER_LEN = 8191;

protected:

    ///处理的文件名称
    std::string            file_name_;

    //
    char* one_line_ = NULL;
    //
    char* str_key_ = NULL;
    //
    char* str_value_ = NULL;
    //
    char* write_line_ = NULL;
};
}
