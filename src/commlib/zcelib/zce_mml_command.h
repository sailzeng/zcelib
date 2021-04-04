/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_mml_command.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       Monday, December 30, 2013
* @brief      �˻��������Է�װ������һЩͨ�õĿ��Ʒ�����������ı�����
*             MML Module Management Language,  ����һ�����ܵ�����.Ҳ���Է���Ϊ������
*             һ��Ϊ�ı��������з�ʽ,����UNIX��������,���Ƿָ����һ��Ϊ','��
              ��������Ƽ������������ܿ���������,�����ܶ������������,������
* @details    ֧����2��MML���ķ�װ
*             ֧�ֵķ����������ʽ,��ο�MML_STRING_PATTERN��˵��
* @note
*
*/

#ifndef ZCE_LIB_MML_COMMAND_H_
#define ZCE_LIB_MML_COMMAND_H_

//���Ӵ�Сд
class LessofCaseString
{
public:
    bool operator()(const std::string &src, const std::string &dst) const
    {
        return ( strcasecmp(src.c_str(), dst.c_str()) < 0);
    }
};


/*!
* @brief      MML ����̨����
*             ����һЩ�ı���������������GM�����
*/
class MML_Console_Command
{

    ///�����ѡ��
    typedef std::set <std::string, LessofCaseString>               MMLCMD_OPTION;
    ///����Ĳ���
    typedef std::map <std::string, std::string, LessofCaseString > MMLCMD_PARAMETER;

public:

    ///MML���ĸ�ʽ��
    enum MML_STRING_PATTERN
    {
        ///��һ�֣���Ӧ����parse_mml_cnd_string1
        ///CMD SVR : A = 1, B = 2, C = " i love c++!", D;
        ///:ǰ���������֣������ֿ������������ʣ������ǲ���������ѡ�֮����,�������ַ���������""����������;��β��
        ///û�п���ת�⣬
        MML_STRING_PATTERN_1,
        ///�ڶ��֣���Ӧ����parse_mml_cnd_string1
        ///CMD_1 A = 1 B = 2 C = " i love c++!" D
        ///��һ�������������֣�������ѡ��Ͳ�����֮���ÿո����, һ��ֻ����һ������
        MML_STRING_PATTERN_2,
    };

public:

    /*!
    * @brief      ���캯��
    * @param      mml_string MML������ַ���
    * @param      pattern    MML�����ʽ
    */
    MML_Console_Command(const char *mml_string,
                        MML_Console_Command::MML_STRING_PATTERN pattern);
    ///Ĭ�Ϲ��캯��
    MML_Console_Command();
    ///��������
    ~MML_Console_Command();


protected:

    ///��ʽ1��MML����������
    int parse_mml_cnd_string1(const char *mml_string);

    ///��ʽ2��MML����������
    int parse_mml_cnd_string2(const char *mml_string);

public:



    /*!
    * @brief      ���������в���,֧�ֵķ����������ʽ���� CMD SVR :A=1,B=2,C=" i love c++!",D;
    * @return     int            �����Ƿ�ɹ�
    * @param[in]  mml_string MML���
    * @param[in]  pattern    �����ʽ
    */
    int parse_mml_cnd_string(const char *mml_string,
                             MML_Console_Command::MML_STRING_PATTERN pattern);

    /*!
    * @brief      ȡ�ã�������MML�����
    * @return     int �����MML��䣬����0�����򷵻�-1
    * @param[out] std::string& ȡ����������
    */
    int get_mml_string(std::string &mml_string) const;

    /*!
    * @brief      ���أ�������MML�����
    * @return     const char* ���ص�MML��䣬���û�з���""
    */
    const char *get_mml_string() const;

    /*!
    * @brief      �õ�MML������
    * @return     int �����MML�����֣�����0�����򷵻�-1
    * @param      mml_cmd ȡ����������
    */
    int get_mml_command(std::string &mml_cmd) const;

    /*!
    * @brief      ����MML������
    * @return     const char* ���ص�MML�����֣����û�з���""
    */
    const char *get_mml_command() const;


    /*!
    * @brief      �Ƿ����������ѡ��
    * @return     int ��������ѡ���0�����û�����ѡ���-1
    * @param      mml_option ����ѡ��
    */
    int have_cmd_option(const std::string &mml_option) const;


    /*!
    * @brief      ���ݲ������ƣ��õ�����Ĳ�����ֵ
    * @return     int  ����������������0�����û�����ѡ���-1
    * @param      para_key   ��������
    * @param      para_value ������ֵ
    */
    int get_cmd_parameter(const std::string &para_key, std::string &para_value)  const;


    /*!
    * @brief      ���ݲ������ƣ��õ�����Ĳ�����ֵ
    * @return     const char * ����ȡ�õ��������ֵ�����û�ж�Ӧ�Ĳ������ƣ�����""
    * @param      para_key   ��������
    */
    const char *get_cmd_parameter(const std::string &para_key)  const;

protected:

    ///MML ���
    std::string             mml_string_;
    ///MML����
    std::string             mml_command_;

    ///MML����ѡ��
    MMLCMD_OPTION           mml_cmd_option_;
    ///MML�������
    MMLCMD_PARAMETER        mml_cmd_parameter_;
};

#endif //_COMM_LIB_MML_COMMAND_H_

