/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_os_adapt_string.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2002��06��22��
* @brief      ����ASNI string�����Ĳ���
*
*
* @details    �������LIB�����������һ��ʹ��
*
*
*
* @note       �������в���Ӧ����ZCELIB(ZENLIB)����Ĵ��롣
*             �����������δ��룬�������Լ��Ĺ�����Ϣ��������
*             2002�꣬��Ӧ���ڱ������������HW����ҵ���粿�Ÿոճ�����ʱ��
*             ��Ȼ��������������൱���ִ��뻹����д�ˣ�
*
*/


#ifndef ZCE_LIB_OS_ADAPT_STRING_H_
#define ZCE_LIB_OS_ADAPT_STRING_H_


#include "zce_log_logging.h"



namespace zce
{
//==========================================================================================================

//


/*!
* @brief      �ַ��Ĺ��ܵ���ȡ�࣬���ڼ���char��wchar_t�ַ��������ʱ����ͦ���õ�
*             yunfeiyang GG���һ�׶�����
* @tparam     char_type �ַ����ͣ�char ���� wchar_t
* @note       char_traits���滹�� length,move�Ⱥ���
*/
template <typename char_type>
struct yun_char_traits : public std::char_traits < char_type >
{
};

template <>
struct yun_char_traits < char > : public std::char_traits < char >
{
    typedef std::string string_type;

    //ע�� int_type ����std::char_traits<wchar_t>::int_type
    static int tolower(int_type c)
    {
        return ::tolower(c);
    }
    static int toupper(int_type c)
    {
        return ::toupper(c);
    }

    static int isspace(int_type c)
    {
        return (isascii(c) && ::isspace(c));
    }

    static char_type *strend(char_type *s)
    {
        return ::strchr(s, 0);
    }
    static const char_type *strend(const char_type *s)
    {
        return ::strchr(s, 0);
    }

};

template <>
struct yun_char_traits < wchar_t > : public std::char_traits < wchar_t >
{
    typedef std::wstring    string_type;

    //ע�� int_type ����std::char_traits<wchar_t>::int_type
    static int tolower(int_type c)
    {
        return ::towlower(c);
    }

    static int toupper(int_type c)
    {
        return ::towupper(c);
    }

    static int isspace(int_type c)
    {
        return ::iswspace(c);
    }

    static char_type *strend(char_type *s)
    {
        return ::wcschr(s, 0);
    }

    static const char_type *strend(const char_type *s)
    {
        return ::wcschr(s, 0);
    }
};


//==========================================================================================================
//ԭ���Ĵ��������˶�ASCII 255���ַ��������ж�����ASCII255��Non-breaking space��
//����ʱȥ���ˡ��о�������������������������Ϊ��ʱû��дע�ͣ�������ȫ���ǵó����ˡ�

/*!
* @brief      ������ַ�����ȥ���ַ�����ߵĿո񣬻��У��س���Tab
* @tparam     char_type �ַ����ͣ�char or wchar_t
* @return     char_type *
* @param      str
*/
template <typename char_type>
char_type *strtrimleft(char_type *str)
{
    ZCE_ASSERT(str);

    char_type *lstr = str;
    for (; *lstr != '\0'; lstr++)
    {
        //ע��isspace�Ĳ�����int,��WIN�£�Ҫ���ַ���0~255,
        if (yun_char_traits<char_type>::isspace(*lstr) != 0)
        {
            continue;
        }
        else
        {
            break;
        }
    }

    if (lstr != str)
    {
        yun_char_traits<char_type>::move(str, lstr,
                                         yun_char_traits<char_type>::length(lstr) + 1);
    }

    return str;
}


/*!
* @brief      �ҹ����ַ�����ȥ���ַ����ұߵĿո񣬻��У��س���Tab
* @return     char*
* @param      str
*/
template <typename char_type>
char *strtrimright(char_type *str)
{
    ZCE_ASSERT(str);
    char_type *lstr = str + yun_char_traits<char_type>::length(str) - 1;

    for (; lstr >= str; lstr--)
    {
        //ע��isspace�Ĳ�����int,��WIN�£�Ҫ���ַ���0~255,
        if (yun_char_traits<char_type>::isspace(*lstr) != 0)
        {
            continue;
        }
        else
        {
            break;
        }
    }

    if (lstr != str + yun_char_traits<char_type>::length(str) - 1)
    {
        *++lstr = 0;
    }

    return str;
}


/*!
* @brief      �����ַ�����ȥ���ַ������ߵĿո񣬻��У��س���Tab
* @return     char*
* @param      str
*/
template <typename char_type>
inline char *strtrim(char_type *str)
{
    zce::strtrimleft<char_type>(str);
    zce::strtrimright<char_type>(str);
    return str;
}

template < typename char_type, typename char_traits_type, typename allocator_typ >
void trimleft_stdstr(std::basic_string<char_type, char_traits_type, allocator_typ> &str)
{
    typename std::basic_string<char_type, char_traits_type, allocator_typ>::iterator it = str.begin();

    while (it != str.end() && yun_char_traits<char_type>::isspace(*it))
    {
        ++it;
    }

    str.erase(str.begin(), it);
    return;
}

template < typename char_type, typename char_traits_type, typename allocator_typ >
void trimright_stdstr(std::basic_string<char_type, char_traits_type, allocator_typ> &str)
{
    typename std::basic_string<char_type, char_traits_type, allocator_typ>::iterator it = str.end();

    if (it == str.begin())
    {
        return;
    }

    while (it != str.begin() && yun_char_traits<char_type>::isspace(*--it));

    str.erase(it + 1, str.end());

    return;
}

template < typename char_type, typename char_traits_type, typename allocator_typ >
inline void trim_stdstr(std::basic_string<char_type, char_traits_type, allocator_typ> &str)
{
    trimright_stdstr(str);
    trimleft_stdstr(str);
    return;
}

//==========================================================================================================
//�����⼸������Ŀǰ��ƽ̨�����У�����ʱ��ĳ��ƽ̨����ȱʧ����HP������IBM��SUN���޷�ȷ���ˣ�
//��ʱ�����ˡ�


/*!
* @brief      ���ַ���ȫ��ת��Ϊ��д�ַ�
*/
char *strupr(char *str);


/*!
* @brief      ���ַ���ȫ��ת��ΪСд�ַ�
*/
char *strlwr(char *str);


/*!
* @brief      �ַ����Ƚϣ����Ӵ�Сд
* @return     int
* @param      string1
* @param      string2
*/
int strcasecmp(const char *string1, const char *string2);

/*!
* @brief      �ַ��������Ƚϣ����Ӵ�Сд
* @return     int
* @param      string1
* @param      string2
* @param      maxlen
* @note
*/
int strncasecmp(const char *string1, const char *string2, size_t maxlen);




//==========================================================================================================
//����Ψһ���ֵ�һЩ����


/*!
* @brief      ͨ������ָ�룬ȡ��һ��Ψһ������,����һЩ��Ҫ���ֵĵط�,��֤����Ŀռ���48���ֽڣ�
* @return     char*           ���ص�����
* @param[in]  object_ptr  ����ָ�룬ĳ�������ָ���ַ��ͨ�����ָ���ַת���õ�Ψһ����
* @param[out] name        ���ֵ�buffer
* @param[in]  length      buffer�ĳ���
*/
char *object_unique_name (const void *object_ptr,
                          char *name,
                          size_t length);


/*!
* @brief      ͨ��ǰ׺ʽ���õ�һ��Ψһ������,Ψһ���ư�����ǰ׺+����ID+�ڲ�������
* @return     char*        ���ص�����
* @param[in]  prefix_name  ǰ׺���ַ���
* @param[out] name         ���ֵ�buffer
* @param[in]  length       buffer�ĳ���
*/
char *prefix_unique_name(const char *prefix_name,
                         char *name,
                         size_t length);

//==========================================================================================================

/*!
* @brief      �ָ��ַ�����
* @tparam     iter_type1  ���ָ����ݵĵ���������
* @tparam     iter_type2  �ָ��ʾ�ĵ���������
* @tparam     compare_type �ȽϷ�ʽ
* @tparam     container_type ��������
* @param[in]  fs ���ָ����ݿ�ʼλ�õĵ�����
* @param[in]  ls ���ָ����ݽ���λ�õĵ�����
* @param[in]  fo �ָ��ʾ�Ŀ�ʼλ�õ�����
* @param[in]  lo �ָ��ʾ�Ľ���λ�õ�����
* @param[in]  pr �ȽϷ�ʽ
* @param[out] v  �����������һ����vector<string>
*/
template < typename iter_type1, typename iter_type2, typename compare_type, typename container_type>
void _str_split(iter_type1 fs, iter_type1 ls, iter_type2 fo, iter_type2 lo, compare_type pr, container_type &v)
{

    //ʹ��β������ĵ�����
    std::back_insert_iterator<container_type> o = std::back_inserter(v);
    if (fo == lo)
    {
        *o = typename container_type::value_type(fs, ls);
        return;
    }

    // current position old_str in str
    iter_type1 pos = std::search(fs, ls, fo, lo, pr);

    if (pos == ls)
    {
        if (fs < ls)
        {
            *o = typename container_type::value_type(fs, ls);
        }

        return;
    }

    size_t old_size = std::distance(fo, lo);

    for (; pos != ls;)
    {
        // append src string
        *o = typename container_type::value_type(fs, pos);
        // ignore old_str
        fs = pos + old_size;
        // find next
        pos = std::search(fs, ls, fo, lo, pr);
    }

    if (fs != ls)
    {
        *o = typename container_type::value_type(fs, ls);
    }

    return;
}


/*!
* @brief      ���ݷָ������һ���ַ������ָ�����ɸ��ַ�������������v
* @tparam     char_type      �ַ�����
* @tparam     container_type �������ͣ�
* @param[in]  str            ������ַ�����
* @param[in]  separator      �ָ��ַ�����ע������ǵ����ַ���ҲҪ��""������
* @param[out] v              ���������ڴ�ŷָ��Ľ��,һ����vector<string>
*/
template < typename char_type, typename container_type >
inline void str_split(const char_type *str, const char_type *separator, container_type &v)
{
    _str_split(str,
               yun_char_traits<char_type>::strend(str),
               separator,
               yun_char_traits<char_type>::strend(separator),
               std::equal_to<char_type>(),
               v);
    return;
}

template < typename container_type >
inline void string_split(const std::string &source_str,
                         const std::string &separator,
                         container_type &v)
{
    _str_split(source_str.begin(),
               source_str.end(),
               separator.begin(),
               separator.end(),
               std::equal_to<std::string::value_type>(),
               v);
    return;
}


/*!
* @brief
* @return     const char*
* @param      src
* @param      dst
* @param      sub
* @param      replace
*/
const char *str_replace(const char *src,
                        char *dst,
                        const char *sub,
                        const char *replace);

//----------------------------------------------------------------------------------------------------------
//����һЩ����ַ�����������

/*!
* @brief      ��Խ�հ׷���ָ�ո�ˮƽ�Ʊ���ֱ�Ʊ���ҳ���س��ͻ��з��������ַ�����Խ��
* @return     const char* ������ĵ�ַ
* @param      str         �����ַ���
* @note       Ϊ�˷���һЩ�����Ϣ�����ӵ�һ�麯����һ��һЩ��top�Ĵ���ʱ�����
*/
const char *skip_whitespace(const char *str);

//
/*!
* @brief      ��Խĳ��token������ĳ������
* @return     const char* ������ĵ�ַ
* @param      str         �����ַ���
*/
const char *skip_token(const char *str);


/*!
* @brief      ����һ��
* @return     const char* ������ĵ�ַ
* @param[in]  str         �����ַ���
* @note
*/
const char *skip_line(const char *str);


/*!
* @brief      ��Խĳ��token,ֱ���ָ����󣨻��߽�����
* @return     const char*    ������ĵ�ַ
* @param[in]  str            �����ַ���
* @param[in]  separator_char �ָ��ַ���Ҫ�ҵ�����ַ�Ϊֹ������������ַ����棩
*/
const char *skip_separator(const char *str, char separator_char);



//----------------------------------------------------------------------------------------------------------






/*!
* @brief      �� 11 02 03 0E E0         ..... ��ʽ�������ָ����Ϣ�����Դ�ӡ�ڴ���Ϣ
*             ��ʽ��Ϣ���ӷḻ������������Ȼ��ӡҲ������
* @param      mem_ptr ���Ե��ڴ�ָ��
* @param      mem_len ָ�볤��
* @param      str_ary �ַ��������飬Ϊʲô����std::ostream�أ���Ϊ�ҿ��ܻ�Ҫ���¸�ʽ���
*/
void memory_debug(const unsigned char *mem_ptr,
                  size_t mem_len,
                  std::vector<std::string> &str_ary);


/*!
* @brief      ��������Ϣ�����stream
* @param      mem     ���Ե��ڴ�ָ��
* @param      mem_len ָ�볤�ȣ�
* @param      stream  �ļ�������stdout,stderrҲ������
*/
void memory_debug(const unsigned char *mem_ptr,
                  size_t mem_len,
                  FILE *stream);


///��־��ӡ��ջ��Ϣ
///���Դ�ӡ��ָ��
///ָ�����ݵĳ���
void memory_debug(ZCE_LOG_PRIORITY dbg_lvl,
                  const char *dbg_info,
                  const unsigned char *mem_ptr,
                  size_t mem_len);

//----------------------------------------------------------------------------------------------------------

/*!
* @brief      �����ڴ濽����������Ҫ���������ֽڶ��룬8�ֽڵĿ���������ֽڽ��У�
*             ��Ҫ���ܵ�����  http://blog.chinaunix.net/uid-20043340-id-154852.html ��Ӱ��
*             ����ʵ�����¿��ŶȲ������Լ�ʵ�ʲ�����Ȼ��memcpy���죬���߿������ڷ��Ż������µĲ��ԡ�
*             ����Ȥ�Ĳο��Ķ���
*             http://www.cnblogs.com/fullsail/p/3160098.html
*             ����ο����Ķ��Ĳ���Ҳ�����Զ�һ�¡�
* @return     void*  ����dst
* @param      dst    Ŀ���ַ
* @param      src    Դ���ݵ�ַ
* @param      sz     �����ĳ���
* @note       �κ�ʱ�򶼣���������ѡ�� C���п�� memcpy,
*/
void *fast_memcpy(void *dst, const void *src, size_t sz);


void *fast_memcpy2(void *dst, const void *src, size_t sz);

//----------------------------------------------------------------------------------------------------------


/*!
* @brief      ��c str ת��Ϊ�ܶ���ֵ���ͣ���Ϊ����ֵ����
* @tparam     ret_type ����ֵ����
* @return     ret_type
* @param[in]  str  c string �ַ�������
*/
template<typename ret_type>
ret_type str_to_value(const char *str);


/*!
* @brief      ��c string ת��Ϊ�ܶ���ֵ���ͣ���Ϊָ���������
* @tparam     ptr_type ָ���������
* @param[in]  str c string �ַ�������
* @param[out] ptr ָ�����
*/
template<typename ptr_type>
void str_to_ptr(const char *str, ptr_type *ptr);


/*!
* @brief      ��std string�ַ���ת���õ���������
* @tparam     ret_type  ���ص�����
* @return     ret_type  ���ص���������
* @param[in]  str       �ַ���
*/
template<typename ret_type>
ret_type string_to_value(const std::string &stdstr)
{
    return str_to_value<ret_type>(stdstr.c_str());
}

/*!
* @brief      ��std string ת��Ϊ�ܶ���ֵ���ͣ���Ϊָ���������
* @tparam     ptr_type  ָ���������
* @param      stdstr
* @param      ptr
*/
template<typename ptr_type>
void string_to_ptr(const std::string &stdstr, ptr_type *ptr)
{
    str_to_ptr<ptr_type>(stdstr.c_str(), ptr);
}

};

#endif //ZCE_LIB_STRING_EX_H_

