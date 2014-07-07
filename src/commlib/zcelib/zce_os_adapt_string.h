/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_os_adapt_string.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2002年06月22日
* @brief      用于ASNI string函数的补充
*
*
* @details    定义基本LIB函数，供大家一起使用
*
*
*
* @note       这里面有部分应该是ZCELIB(ZENLIB)最早的代码。
*             多年后整理这段代码，发现有自己的工号信息，哈哈，
*             2002年，我应该在北京出差回来，HW的商业网络部门刚刚成立的时候
*             当然后来，这里面的相当部分代码还是重写了，
*
*/


#ifndef ZCE_LIB_OS_ADAPT_STRING_H_
#define ZCE_LIB_OS_ADAPT_STRING_H_






namespace ZCE_OS
{




//==========================================================================================================

/*!
* @brief      左规整字符串，去掉字符串左边的空格，换行，回车，Tab
* @return     char*
* @param      str
*/
char *strtrimleft(char *str);


/*!
* @brief      右规整字符串，去掉字符串右边的空格，换行，回车，Tab
* @return     char*
* @param      str
*/
char *strtrimright(char *str);


/*!
* @brief      规整字符串，去掉字符串两边的空格，换行，回车，Tab
* @return     char*
* @param      str
*/
char *strtrim(char *str);

//==========================================================================================================
//下面这几个函数目前的平台可能有，但当时的某个平台可能缺失，是HP？还是IBM？SUN？无法确认了，


/*!
* @brief      将字符串全部转换为大写字符
*/
char *strupr(char *str);

/*!
* @brief      将字符串全部转换为小写字符
*/
char *strlwr(char *str);


/*!
* @brief      字符串比较，忽视大小写
* @return     int
* @param      string1
* @param      string2
*/
int strcasecmp(const char *string1, const char *string2);

/*!
* @brief      字符串定长比较，忽视大小写
* @return     int
* @param      string1
* @param      string2
* @param      maxlen
* @note
*/
int strncasecmp(const char *string1, const char *string2, size_t maxlen);




//==========================================================================================================
//产生唯一名字的一些函数


/*!
* @brief      通过对象指针，取得一个唯一的名称,用于一些需要名字的地方,保证传入的空间有48个字节，
* @return     char*           返回的名字
* @param[in]  object_ptr  对象指针，某个对象的指针地址，通过这个指针地址转换得到唯一名字
* @param[out] name        名字的buffer
* @param[in]  length      buffer的长度
*/
char *object_unique_name (const void *object_ptr,
                          char *name,
                          size_t length);


/*!
* @brief      通过前缀式，得到一个唯一的名称,唯一名称包括，前缀+进程ID+内部计数器
* @return     char*        返回的名字
* @param[in]  prefix_name  前缀的字符串
* @param[out] name         名字的buffer
* @param[in]  length       buffer的长度
*/
char *prefix_unique_name(const char *prefix_name,
                         char *name,
                         size_t length);






//==========================================================================================================
//用于一些表格字符串分析函数

/*!
* @brief      跨越空白符，指空格、水平制表、垂直制表、换页、回车和换行符，这类字符都跨越，
* @return     const char* 跳过后的地址
* @param      str         参数字符串
* @note       为了分析一些表格信息，增加的一组函数，一致一些类top的代码时翻译的
*/
const char *skip_whitespace(const char *str);

//
/*!
* @brief      跨越某个token，跳过某个单词
* @return     const char* 跳过后的地址
* @param      str         参数字符串
*/
const char *skip_token(const char *str);


/*!
* @brief      跳过一行
* @return     const char* 跳过后的地址
* @param[in]  str         参数字符串
* @note
*/
const char *skip_line(const char *str);


/*!
* @brief      跨越某个token,直到分隔符后（或者结束）
* @return     const char*    跳过后的地址
* @param[in]  str            参数字符串
* @param[in]  separator_char 分割字符，要找到这个字符为止，（跳到这个字符后面）
*/
const char *skip_separator(const char *str, char separator_char);





//==========================================================================================================


/*!
* @brief      一个简单的打印输出某个内存地址信息的函数，就是简单的内存翻译为16进制字符串
* @param      stream  文件流对象，stdout,stderr也都可以
* @param      mem     调试的内存指针
* @param      mem_len 指针长度，
* @note       如果是日志输出提供了方便的辅助类，这仅仅由于不需要使用日志的地方
*/
void memory_debug(FILE *stream, const unsigned char *mem, size_t mem_len);



/*!
* @brief      用 11 02 03 0E E0         ..... 格式的输出，指针信息。调试打印内存信息
*             格式信息更加丰富，和清晰，当然打印也更加慢
* @param      stream  文件流对象，stdout,stderr也都可以
* @param      mem     调试的内存指针
* @param      mem_len 指针长度
*/
void memory_debug_ex(FILE *stream, const unsigned char *mem, size_t mem_len);


//==========================================================================================================

/*!
* @brief      快速内存拷贝函数，主要就是利用字节对齐，8字节的拷贝快过单字节进行，
*             主要是受到文章  http://blog.chinaunix.net/uid-20043340-id-154852.html 的影响
*             但其实这文章可信度不大，我字节实际测试仍然是memcpy更快，作者可能是在非优化条件下的测试。
*             有兴趣的参考阅读。
*             http://www.cnblogs.com/fullsail/p/3160098.html
*             里面参考的阅读的部分也都可以读一下。
* @return     void*  返回dst
* @param      dst    目标地址
* @param      src    源数据地址
* @param      sz     拷贝的长度
* @note       任何时候都，都请优先选择 C运行库的 memcpy,
*/
void *fast_memcpy(void *dst, const void *src, size_t sz);


void *fast_memcpy2(void *dst, const void *src, size_t sz);

};














#endif //ZCE_LIB_STRING_EX_H_

