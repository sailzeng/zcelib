/*!
* @copyright  2002-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_config_ini_pasttime.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2002��1��3��
* @brief      INI �ļ��Ķ�д�Ļ��ɰ棬��WINDOWS��API�������ƣ�
*             ��������д�ص�ʱ���лᱣ֤�ļ�ԭ���ĸ�ʽ��ע�ͣ������ƻ��ļ��ĸ�ʽ��
*             ���⣬���API��һЩ���⹦�ܣ������ȡһ��section�����е��ֶ����ݵȣ�
* @details    ����INI�ļ���һ�е���󳤶���8K
*
* @note       ��Ӧ����2002�꣬2003���ڼƷ���Ŀʱ���Լ��ڼ���д�Ĵ��롣ĳ�ֳ�
*             ���ϣ������������ǲ��ٻ�ʹ�õ��ˣ�
*             ������ĳ�̶ֳ��Ǹ���Ļ��ɡ��������Ҳ���ǰ���Щ��������������µ�
*             ����淶
*             ����ע�ͷ������滹��WIN95����ʾ���Ǻ�
*/

#ifndef ZCE_LIB_CONFIG_INI_PASTTIME_H_
#define ZCE_LIB_CONFIG_INI_PASTTIME_H_


/*!
* @brief      INI�ļ���д�Ĵ����ӿڶ�ģ��WIndows��API
*/
class ZCE_INI_Pt
{

public:

    ///���캯��,
    ///@param file_name �ļ�����
    ZCE_INI_Pt(const char *file_name);
    //��������
    ~ZCE_INI_Pt();

public:

    /*!
    * @brief      ��ȡINI�ļ����ֶΣ����䣬ģ��WIN32�ӿ�GetPrivateProfileString,
    *             ���в����Ҿ�ֱ�Ӹ���MSDN��
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
    size_t get_private_str(const char *sec_name,
                           const char *key_name,
                           const char *default_str,
                           char *return_str,
                           const size_t size_ret_str);

    ///�õ����е�section,��'\0'�ָ�����'\0\0'��������������ֵ�ο�get_privateprofile_string
    size_t get_private_allsection(char *return_str,
                                  const size_t size_ret_str);

    ///�õ�ĳ��Section�����е�Keyֵ����������ֵ�ο�get_privateprofile_string
    size_t getprivate_allkey(const char *sec_name,
                             char *return_str,
                             const size_t size_ret_str);

    ///�õ�Int�� Keyֵ����������ֵ�ο�get_privateprofile_string
    int get_private_int(const char *sec_name,
                        const char *key_name,
                        const int default_int);

    ///�õ�bool�� Keyֵ����������ֵ�ο�get_privateprofile_string
    bool   get_private_bool(const char *sec_name,
                            const char *key_name,
                            const bool default_int);

    /*!
    * @brief      дĳ���ֶΣ�ģ��WIN32�ӿ�WritePrivateProfileString
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
    * @param      key_name��
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
    bool   write_private_string(const char *sec_name,
                                const char *key_name,
                                const char *write_string);

    ///�޸�Section
    bool   write_private_section(const char *sec_name,
                                 const char *write_string);
    ///д��Int
    bool   write_private_int(const char *sec_name,
                             const char *key_name,
                             const int nvalue);
    ///д��Bool����
    bool   write_private_bool(const char *sec_name,
                              const char *key_name,
                              const bool bvalue);

    ///ɾ��Key
    bool   del_private_key(const char *sec_name,
                           const char *key_name);
    ///ɾ��Section
    bool   del_private_section(const char *sec_name);




protected:
    //ÿ�е���󳤶�
    static const size_t LINE_BUFFER_LEN = 8191;

protected:

    ///������ļ�����
    std::string            file_name_;

    //
    char *one_line_ = NULL;
    //
    char *str_key_ = NULL;
    //
    char *str_value_ = NULL;
    //
    char *write_line_ = NULL;

};

#endif //ZCE_LIB_CONFIG_INI_PASTTIME_H_


