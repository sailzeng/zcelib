/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_mml_command.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       Monday, December 30, 2013
* @brief      人机交互语言封装，用于一些通用的控制服务器的命令，文本类型
*             MML Module Management Language,  这是一个网管的术语.也可以翻译为慢慢来
*             一般为文本的命令行方式,类似UNIX的命令行,但是分割符号一般为','等
              用这个名称纪念我两年网管开发的生活,人生很多事情就是这样,慢慢来
* @details    支持了2种MML语句的封装
*             支持的分析的命令格式,请参考MML_STRING_PATTERN的说明
* @note
*
*/

#pragma once

namespace zce
{
/*!
* @brief      MML 控制台命令
*             用于一些文本交互场景，比如GM命令等
*/
class mml_cmd
{
    //忽视大小写
    struct lessofcasestr
    {
    public:
        bool operator()(const std::string& src, const std::string& dst) const
        {
            return (strcasecmp(src.c_str(), dst.c_str()) < 0);
        }
    };

    ///命令的选项
    typedef std::set < std::string, lessofcasestr > MMLCMD_OPTION;
    ///命令的参数
    typedef std::map <std::string, std::string, lessofcasestr > MMLCMD_PARAMETER;

public:

    ///MML语句的格式，
    enum class MML_PATTERN
    {
        ///第一种，对应函数parse_mml_str_1
        ///CMD_1 A = 1 B = 2 C = " i love c++!" D
        ///第一个单词是命令字，后面是选项和参数，之间用空格隔开, 一行只能有一个命令
        PATTERN_1,
        ///第二种，对应函数parse_mml_str_2
        ///CMD SVR : A = 1, B = 2, C = " i love c++!", D;
        ///:前面是命令字，命令字可以是两个单词，后面是参数，或者选项，之间用,隔开，字符串可以用""扩起来，用;结尾，
        ///没有考虑转意，
        PATTERN_2,
    };

public:

    ///默认构造函数
    mml_cmd() = default;
    ///析构函数
    ~mml_cmd() = default;

protected:

    ///格式1的MML命令分析语句
    int parse_mml_str_1(const char* mml_string);

    ///格式2的MML命令分析语句
    int parse_mml_str_2(const char* mml_string);

public:

    /*!
    * @brief      分析命令行参数,支持的分析的命令格式包括 CMD SVR :A=1,B=2,C=" i love c++!",D;
    * @return     int            分析是否成功
    * @param[in]  mml_string MML语句
    * @param[in]  pattern    命令格式
    */
    int parse_mml(const char* mml_string,
                  mml_cmd::MML_PATTERN pattern);

    /*!
    * @brief      取得（分析）MML的语句
    * @return     int 如果有MML语句，返回0，否则返回-1
    * @param[out] std::string& 取出的命令字
    */
    int get_mml_string(std::string& mml_string) const;

    /*!
    * @brief      返回（分析）MML的语句
    * @return     const char* 返回的MML语句，如果没有返回""
    */
    const char* get_mml_string() const;

    /*!
    * @brief      得到MML命令字
    * @return     int 如果有MML命令字，返回0，否则返回-1
    * @param      mml_cmd 取出的命令字
    */
    int get_command(std::string& mml_cmd) const;

    /*!
    * @brief      返回MML命令字
    * @return     const char* 返回的MML命令字，如果没有返回""
    */
    const char* get_command() const;

    /*!
    * @brief      是否有这个命令选项
    * @return     int 如果有这个选项返回0，如果没有这个选项返回-1
    * @param      mml_option 命令选项
    */
    int have_option(const std::string& mml_option) const;

    /*!
    * @brief      根据参数名称，得到命令的参数的值
    * @return     int  如果有这个参数返回0，如果没有这个选项返回-1
    * @param      para_key   参数名称
    * @param      para_value 参数的值
    */
    int get_parameter(const std::string& para_key, std::string& para_value)  const;

    /*!
    * @brief      根据参数名称，得到命令的参数的值
    * @return     const char * 返回取得的命令参数值，如果没有对应的参数名称，返回""
    * @param      para_key   参数名称
    */
    const char* get_parameter(const std::string& para_key)  const;

protected:

    ///MML 语句
    std::string             mml_string_;
    ///MML命令
    std::string             mml_command_;

    ///MML命令选项
    MMLCMD_OPTION           mml_cmd_option_;
    ///MML命令参数
    MMLCMD_PARAMETER        mml_cmd_parameter_;
};
}
