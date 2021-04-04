#include "zce_predefine.h"
#include "zce_log_logging.h"
#include "zce_mml_command.h"

/****************************************************************************************************
class MML_Console_Command ����̨����
****************************************************************************************************/
//���캯��
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

//ȡ��MML�����
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

//ӵ��ĳ������ѡ��
int MML_Console_Command::have_cmd_option(const std::string &mml_option) const
{
    MMLCMD_OPTION::const_iterator option_iter =  mml_cmd_option_.find(mml_option);

    if ( mml_cmd_option_.end() == option_iter)
    {
        return -1;
    }

    return 0;
}

//�õ�ĳ���������
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

//�õ�ĳ���������
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


//MML������
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



//���������в���
//֧�ֵķ����������ʽ���� CMD_1 A=1 B=2 C=" i love c++!" D
int MML_Console_Command::parse_mml_cnd_string2(const char *mml_string)
{
    mml_string_ = mml_string;

    //�����������ֳ�
    mml_command_ = "";
    mml_cmd_option_.clear();
    mml_cmd_parameter_.clear();

    //
    //��ȡ�ֶε�λ�ñ�ʾ����
    const char *part_tmp =  mml_string;
    const char *part_end = NULL;
    const char *part_start = NULL;

    //�����ո�
    for (; isspace(static_cast<unsigned char>(*part_tmp)) && '\0' !=  *part_tmp ; ++part_tmp);

    part_start = part_tmp;

    //ѡ��ո�ǰ������
    for (; (isspace(static_cast<unsigned char>(*part_tmp))  == false) && *part_tmp != '\0'; ++part_tmp);

    part_end = part_tmp;

    //��������
    if (part_start >= part_end)
    {
        return -1;
    }

    //�õ�����
    mml_command_.assign(part_start, part_end);

    //�޳����еĿո�
    for (; isspace(static_cast<unsigned char>(*part_tmp)) && '\0' !=  *part_tmp ; ++part_tmp);

    if ('\0' == *part_tmp)
    {
        return 0;
    }

    std::string key, value;

    //��������
    while ('\0' != *part_tmp)
    {
        //ȥ���ո��Ӱ��
        for (; isspace(static_cast<unsigned char>(*part_tmp)) && *part_tmp != '\0'; ++part_tmp);

        part_start = part_tmp;

        //ѡ��ո��'='����ǰ������
        for (; (isspace(static_cast<unsigned char>(*part_tmp))  == false) && *part_tmp != '=' && *part_tmp != '\0'; ++part_tmp);

        part_end = part_tmp;

        //��������
        if ( part_start >= part_end)
        {
            return -1;
        }

        key.assign(part_start, part_end);

        //ȥ���ո��Ӱ��
        for (; isspace(static_cast<unsigned char>(*part_tmp)) && *part_tmp != '\0'; ++part_tmp);

        if (*part_tmp !=  '=' )
        {
            mml_cmd_option_.insert(key);
            continue;
        }
        else
        {
            ++part_tmp;

            //ȥ���ո��Ӱ��
            for (; isspace(static_cast<unsigned char>(*part_tmp)) && *part_tmp != '\0'; ++part_tmp);

            if (*part_tmp == '\"')
            {
                ++part_tmp;
                part_start = part_tmp;

                //ѡ��'\"'����ǰ������
                for (; *part_tmp != '\"' && *part_tmp != '\0'; ++part_tmp);

                part_end = part_tmp;
            }
            else
            {
                part_start = part_tmp;

                //ѡ��ո�ǰ������
                for (; (isspace(static_cast<unsigned char>(*part_tmp))  == false) && *part_tmp != '\0'; ++part_tmp);

                part_end = part_tmp;
            }

            //��������
            if (part_start >= part_end)
            {
                return -1;
            }

            value.assign(part_start, part_end);
        }

        mml_cmd_parameter_[key] = value;

        //ȥ���ո��Ӱ��
        for (; isspace(static_cast<unsigned char>(*part_tmp)) && *part_tmp != '\0'; ++part_tmp);
    }

    //mml_command_
    return 0;
}

//���������в���
//֧�ֵķ����������ʽ���� CMD SVR :A=1,B=2,C=" i love c++!";
//������˼·�е����,������һ�����ҵ�id
int MML_Console_Command::parse_mml_cnd_string1(const char *mml_string )
{
    mml_string_ = mml_string;
    //�����������ֳ�
    mml_command_ = "";
    mml_cmd_option_.clear();
    mml_cmd_parameter_.clear();

    //�ָ����,
    const char CMD_SEPARATOR = ':';
    const char END_SEPARATOR = ';';
    const char OPTION_SEPARATOR = ',';
    const char KEY_SEPARATOR = '=';
    const char STRING_SEPARATOR = '\"';

    //��ȡ�ֶε�λ�ñ�ʾ����
    const char *part_tmp =  mml_string;
    const char *part_end = NULL;
    const char *part_start = NULL;

    //����ǰ��Ŀո�û����trim������
    for (; isspace(static_cast<unsigned char>(*part_tmp)) && '\0' !=  *part_tmp  && END_SEPARATOR != *part_tmp ; ++part_tmp);

    part_start = part_tmp;

    //ȡ������
    for (; CMD_SEPARATOR != *part_tmp &&  '\0' != *part_tmp && END_SEPARATOR != *part_tmp ; ++part_tmp);

    part_end = part_tmp;

    //���˺���Ŀո�
    for (; isspace(static_cast<unsigned char>(*part_end)) && part_end != part_start; --part_end);

    //
    if (part_end <= part_start)
    {
        return -1;
    }

    mml_command_.assign(part_start, part_end);

    //����Ѿ�������ֻ��������
    if ('\0' == *part_tmp || END_SEPARATOR == *part_tmp)
    {
        return 0;
    }

    //����CMD_SEPARATOR
    ++part_tmp;

    //�����ո�
    std::string key, value;

    for (; isspace(static_cast<unsigned char>(*part_tmp)) && *part_tmp != '\0'     && *part_tmp != END_SEPARATOR; ++part_tmp);

    //��������
    for (; *part_tmp !=  '\0' &&  *part_tmp != END_SEPARATOR; ++part_tmp)
    {
        //�����ո�
        for (; isspace(static_cast<unsigned char>(*part_tmp)) && '\0' !=  *part_tmp  && END_SEPARATOR != *part_tmp; ++part_tmp);

        part_start = part_tmp;

        //�õ�option�ֶ�
        for (; *part_tmp != OPTION_SEPARATOR && *part_tmp != KEY_SEPARATOR && *part_tmp != '\0' && *part_tmp != END_SEPARATOR ; ++part_tmp);

        part_end = part_tmp;

        //ȥ������Ŀո�
        for (; isspace(static_cast<unsigned char>(*part_end)) && part_end != part_start; --part_end);

        //��������
        if (part_end <= part_start )
        {
            return -1;
        }

        //�õ�Key
        key.assign(part_start, part_end);

        //�����ѡ��
        if (*part_tmp == OPTION_SEPARATOR || *part_tmp == END_SEPARATOR || *part_tmp == '\0' )
        {
            mml_cmd_option_.insert(key);
            continue;
        }

        //����ǲ�����������������ƺ�ֵ
        if (*part_tmp == KEY_SEPARATOR)
        {
            //����KEY_SEPARATOR
            ++part_tmp;

            for (; isspace(static_cast<unsigned char>(*part_tmp)) && '\0' !=  *part_tmp  && END_SEPARATOR != *part_tmp ; ++part_tmp);

            part_start = part_tmp;

            //������ַ���,��������
            if (*part_tmp == STRING_SEPARATOR)
            {
                ++part_tmp;
                part_start = part_tmp;

                for (; *part_tmp != STRING_SEPARATOR && *part_tmp != '\0' && *part_tmp != END_SEPARATOR ;  ++part_tmp);

                part_end = part_tmp;

                //��������ָ������...
                if ( *part_end != STRING_SEPARATOR )
                {
                    return -1;
                }

                value.assign(part_start, part_end);
                mml_cmd_parameter_[key] = value;
                continue;
            }

            //�õ�value
            for (; *part_tmp != OPTION_SEPARATOR && *part_tmp != '\0' && *part_tmp != END_SEPARATOR ; ++part_tmp);

            part_end = part_tmp;

            //ȥ���ո�
            for (; isspace(static_cast<unsigned char>(*part_end)) && part_end != part_start; --part_end);

            if (part_end <= part_start )
            {
                return -1;
            }

            //�������
            value.assign(part_start, part_end);
            mml_cmd_parameter_[key] = value;
            continue;
        }
    }

    return 0;
}

