#include "zce_predefine.h"
#include "zce_trace_log_debug.h"
#include "zce_mml_command.h"

/****************************************************************************************************
class MML_Console_Command 控制台命令
****************************************************************************************************/
//构造函数
MML_Console_Command::MML_Console_Command(const char *mml_string,
    MML_Console_Command::MML_STRING_PATTERN pattern)
{
    mml_string_ = mml_string;
    mml_command_.reserve(32);
    parse_mml_cnd_string(mml_string, pattern);
}
//
MML_Console_Command::MML_Console_Command()
{
}
MML_Console_Command::~MML_Console_Command()
{
}

//取得MML的语句
int MML_Console_Command::get_mml_string(std::string &mml_string) const
{
    if (mml_string_.length() > 0)
    {
        mml_string = mml_string_;
        return 0;
    }
    return -1;
}

//
const char *MML_Console_Command::get_mml_string() const
{
    return mml_string_.c_str();
}

//
int MML_Console_Command::get_mml_command(std::string &mml_cmd) const
{
    if (mml_command_.length() == 0)
    {
        return -1;
    }

    mml_cmd = mml_command_;
    return 0;
}

//
const char *MML_Console_Command::get_mml_command() const
{
    return mml_command_.c_str();
}

//拥有某个命令选项
int MML_Console_Command::have_cmd_option(const std::string &mml_option) const
{
    MMLCMD_OPTION::const_iterator option_iter =  mml_cmd_option_.find(mml_option);

    if ( mml_cmd_option_.end() == option_iter)
    {
        return -1;
    }

    return 0;
}

//得到某个命令参数
int MML_Console_Command::get_cmd_parameter(const std::string &para_key, std::string &para_value) const
{
    MMLCMD_PARAMETER::const_iterator para_iter = mml_cmd_parameter_.find(para_key);
    if (mml_cmd_parameter_.end() == para_iter )
    {
        return -1;
    }

    para_value = para_iter->second;
    return 0;
}

//得到某个命令参数
const char *MML_Console_Command::get_cmd_parameter(const std::string &para_key)  const
{
    static const char NOT_FIND_RETURN[] = "";
    MMLCMD_PARAMETER::const_iterator para_iter = mml_cmd_parameter_.find(para_key);
    if (mml_cmd_parameter_.end() == para_iter)
    {
        return NOT_FIND_RETURN;
    }

    return para_iter->second.c_str();
}


//MML语句分析
int MML_Console_Command::parse_mml_cnd_string(const char *mml_string,
    MML_Console_Command::MML_STRING_PATTERN pattern)
{

    switch (pattern)
    {
    case MML_STRING_PATTERN_1:
        parse_mml_cnd_string1(mml_string);
        break;
    case MML_STRING_PATTERN_2:
        parse_mml_cnd_string2(mml_string);
        break;
    default:
        ZCE_ASSERT_ALL(false);
        return -1;
    }
    return 0;
}



//分析命令行参数
//支持的分析的命令格式包括 CMD_1 A=1 B=2 C=" i love c++!" D
int MML_Console_Command::parse_mml_cnd_string2(const char *mml_string)
{
    mml_string_ = mml_string;

    //清理遗留的现场
    mml_command_ = "";
    mml_cmd_option_.clear();
    mml_cmd_parameter_.clear();

    //
    //读取字段的位置标示符号
    const char *part_tmp =  mml_string;
    const char *part_end = NULL;
    const char *part_start = NULL;

    //跳过空格
    for (; isspace(static_cast<unsigned char>(*part_tmp)) && '\0' !=  *part_tmp ; ++part_tmp);

    part_start = part_tmp;

    //选择空格前的数据
    for (; (isspace(static_cast<unsigned char>(*part_tmp))  == false) && *part_tmp != '\0'; ++part_tmp);

    part_end = part_tmp;

    //分析错误
    if (part_start >= part_end)
    {
        return -1;
    }

    //得到命令
    mml_command_.assign(part_start, part_end);

    //剔除所有的空格
    for (; isspace(static_cast<unsigned char>(*part_tmp)) && '\0' !=  *part_tmp ; ++part_tmp);

    if ('\0' == *part_tmp)
    {
        return 0;
    }

    std::string key , value;

    //解析参数
    while ('\0' != *part_tmp)
    {
        //去除空格的影响
        for (; isspace(static_cast<unsigned char>(*part_tmp)) && *part_tmp != '\0'; ++part_tmp);

        part_start = part_tmp;

        //选择空格和'='符号前的数据
        for (; (isspace(static_cast<unsigned char>(*part_tmp))  == false) && *part_tmp != '=' && *part_tmp != '\0'; ++part_tmp);

        part_end = part_tmp;

        //分析错误
        if ( part_start >= part_end)
        {
            return -1;
        }

        key.assign(part_start, part_end);

        //去除空格的影响
        for (; isspace(static_cast<unsigned char>(*part_tmp)) && *part_tmp != '\0'; ++part_tmp);

        if (*part_tmp !=  '=' )
        {
            mml_cmd_option_.insert(key);
            continue;
        }
        else
        {
            ++part_tmp;

            //去除空格的影响
            for (; isspace(static_cast<unsigned char>(*part_tmp)) && *part_tmp != '\0'; ++part_tmp);

            if (*part_tmp == '\"')
            {
                ++part_tmp;
                part_start = part_tmp;

                //选择'\"'符号前的数据
                for (; *part_tmp != '\"' && *part_tmp != '\0'; ++part_tmp);

                part_end = part_tmp;
            }
            else
            {
                part_start = part_tmp;

                //选择空格前的数据
                for (; (isspace(static_cast<unsigned char>(*part_tmp))  == false) && *part_tmp != '\0'; ++part_tmp);

                part_end = part_tmp;
            }

            //分析错误
            if (part_start >= part_end)
            {
                return -1;
            }

            value.assign(part_start, part_end);
        }

        mml_cmd_parameter_[key] = value;

        //去除空格的影响
        for (; isspace(static_cast<unsigned char>(*part_tmp)) && *part_tmp != '\0'; ++part_tmp);
    }

    //mml_command_
    return 0;
}

//分析命令行参数
//支持的分析的命令格式包括 CMD SVR :A=1,B=2,C=" i love c++!";
//分析的思路有点奇怪,基本是一个个找到id
int MML_Console_Command::parse_mml_cnd_string1(const char *mml_string )
{
    mml_string_ = mml_string;
    //清理遗留的现场
    mml_command_ = "";
    mml_cmd_option_.clear();
    mml_cmd_parameter_.clear();

    //分割符号,
    const char CMD_SEPARATOR = ':';
    const char END_SEPARATOR = ';';
    const char OPTION_SEPARATOR = ',';
    const char KEY_SEPARATOR = '=';
    const char STRING_SEPARATOR = '\"';

    //读取字段的位置标示符号
    const char *part_tmp =  mml_string;
    const char *part_end = NULL;
    const char *part_start = NULL;

    //过滤前面的空格，没有用trim函数，
    for (; isspace(static_cast<unsigned char>(*part_tmp)) && '\0' !=  *part_tmp  && END_SEPARATOR != *part_tmp ; ++part_tmp);

    part_start = part_tmp;

    //取命令字
    for (; CMD_SEPARATOR != *part_tmp &&  '\0' != *part_tmp && END_SEPARATOR != *part_tmp ; ++part_tmp);

    part_end = part_tmp;

    //过滤后面的空格
    for (; isspace(static_cast<unsigned char>(*part_end)) && part_end != part_start; --part_end);

    //
    if (part_end <= part_start)
    {
        return -1;
    }

    mml_command_.assign(part_start, part_end);

    //如果已经结束，只有命令字
    if ('\0' == *part_tmp || END_SEPARATOR == *part_tmp)
    {
        return 0;
    }

    //跳过CMD_SEPARATOR
    ++part_tmp;

    //跳过空格
    std::string key , value;

    for (; isspace(static_cast<unsigned char>(*part_tmp)) && *part_tmp != '\0'     && *part_tmp != END_SEPARATOR; ++part_tmp);

    //结束条件
    for (; *part_tmp !=  '\0' &&  *part_tmp != END_SEPARATOR; ++part_tmp)
    {
        //跳过空格
        for (; isspace(static_cast<unsigned char>(*part_tmp)) && '\0' !=  *part_tmp  && END_SEPARATOR != *part_tmp; ++part_tmp);

        part_start = part_tmp;

        //得到option字段
        for (; *part_tmp != OPTION_SEPARATOR && *part_tmp != KEY_SEPARATOR && *part_tmp != '\0' && *part_tmp != END_SEPARATOR ; ++part_tmp);

        part_end = part_tmp;

        //去掉后面的空格
        for (; isspace(static_cast<unsigned char>(*part_end)) && part_end != part_start; --part_end);

        //分析错误
        if (part_end <= part_start )
        {
            return -1;
        }

        //得到Key
        key.assign(part_start, part_end);

        //如果是选项
        if (*part_tmp == OPTION_SEPARATOR || *part_tmp == END_SEPARATOR || *part_tmp == '\0' )
        {
            mml_cmd_option_.insert(key);
            continue;
        }

        //如果是参数，分离参数的名称和值
        if (*part_tmp == KEY_SEPARATOR)
        {
            //跳过KEY_SEPARATOR
            ++part_tmp;

            for (; isspace(static_cast<unsigned char>(*part_tmp)) && '\0' !=  *part_tmp  && END_SEPARATOR != *part_tmp ; ++part_tmp);

            part_start = part_tmp;

            //如果是字符串,这样处理
            if (*part_tmp == STRING_SEPARATOR)
            {
                ++part_tmp;
                part_start = part_tmp;

                for (; *part_tmp != STRING_SEPARATOR && *part_tmp != '\0' && *part_tmp != END_SEPARATOR ;  ++part_tmp);

                part_end = part_tmp;

                //如果结束分割符号是...
                if ( *part_end != STRING_SEPARATOR )
                {
                    return -1;
                }

                value.assign(part_start, part_end);
                mml_cmd_parameter_[key] = value;
                continue;
            }

            //得到value
            for (; *part_tmp != OPTION_SEPARATOR && *part_tmp != '\0' && *part_tmp != END_SEPARATOR ; ++part_tmp);

            part_end = part_tmp;

            //去掉空格
            for (; isspace(static_cast<unsigned char>(*part_end)) && part_end != part_start; --part_end);

            if (part_end <= part_start )
            {
                return -1;
            }

            //命令参数
            value.assign(part_start, part_end);
            mml_cmd_parameter_[key] = value;
            continue;
        }
    }

    return 0;
}

