
#include "zce_predefine.h"
#include "zce_os_adapt_string.h"
#include "zce_cfg_ini_pasttime.h"

//
ZCE_INI_Pt::ZCE_INI_Pt(const char *szfname)
{
    one_line_ = new char[LINE_BUFFER_LEN + 1];
    str_key_ = new char[LINE_BUFFER_LEN + 1];
    str_value_ = new char[LINE_BUFFER_LEN + 1];
    write_line_ = new char[LINE_BUFFER_LEN + 1];

    //8*1024 -1,1�е����ֵ
    one_line_[LINE_BUFFER_LEN] = '\0';
    write_line_[LINE_BUFFER_LEN] = '\0';
    str_key_[LINE_BUFFER_LEN] = '\0';
    str_value_[LINE_BUFFER_LEN] = '\0';

    file_name_.assign(szfname);
}

ZCE_INI_Pt::~ZCE_INI_Pt()
{
    if (one_line_)
    {
        delete one_line_;
        one_line_ = NULL;
    }
    if (str_key_)
    {
        delete str_key_;
        str_key_ = NULL;
    }
    if (str_value_)
    {
        delete str_value_;
        str_value_ = NULL;
    }
    if (write_line_)
    {
        delete write_line_;
        write_line_ = NULL;
    }
}

//�õ�keyֵ��String
size_t ZCE_INI_Pt::get_private_str(const char *sec_name,
                                   const char *key_name,
                                   const char *default_str,
                                   char *return_str,
                                   const size_t size_ret_str)
{
    //�����ַ�����ָ��Ϊ��,���ش���
    assert(return_str);
    if (!return_str)
    {
        return 0;
    }

    *return_str = '\0';

    //�����ַ����ĳߴ�С��1,
    if (size_ret_str <= 1)
    {
        *return_str = '\0';
        return size_ret_str;
    }

    int operate = 0;

    //���App��Keyȫ��Ϊ�գ��򷵻����е�Section����'\0'�ָ������һ��������'\0'��ʶ
    if (sec_name == NULL && key_name == NULL)
    {
        operate = 1;
    }

    //���KeyΪ�գ�App���գ��򷵻����е�Section�µ�Keyֵ����'\0'�ָ������һ��������'\0'��ʶ
    if (sec_name != NULL && key_name == NULL)
    {
        operate = 2;
    }

    //AppΪ�գ�Key��Ϊ��,�������е�Key������һ��ƥ��ļ�ֵ����
    if (sec_name == NULL && key_name != NULL)
    {
        operate = 3;
    }

    //App��Key����Ϊ�գ��ⷵ��App��Key��ƥ��ļ�ֵ
    if (sec_name != NULL && key_name != NULL)
    {
        operate = 4;
    }

    std::ifstream cfgfile(file_name_.c_str());

    //�ļ��򲻿�������Ĭ��ֵ
    if (!cfgfile)
    {
        strncpy(return_str, default_str, size_ret_str - 1);
        return_str[size_ret_str - 1] = '\0';
        return strlen(return_str);
    }

    char *pstrtmp;
    size_t ntmp;
    bool if_app;

    //���App��Keyȫ��Ϊ�գ��򷵻����е�Section����'\0'�ָ������һ��������'\0'��ʶ
    if (operate == 1)
    {
        //��������ַ�������С��2,ȫ�����ؿ��ַ�
        if (size_ret_str <= 2)
        {
            *return_str = '\0';
            *(return_str + 1) = '\0';
            return size_ret_str;
        }

        if_app = false;
        pstrtmp = return_str;
        ntmp = size_ret_str - 1;

        while (cfgfile)
        {
            cfgfile.getline(one_line_, LINE_BUFFER_LEN);
            //fgets(choneline,LINE_BUFFER_LEN,pfile);
            //����
            zce::strtrim(one_line_);

            //ע����
            if (one_line_[0] == ';' || one_line_[0] == '#')
            {
                continue;
            }

            if (one_line_[0] == '[' && one_line_[strlen(one_line_) - 1] == ']')
            {
                //ȥ��'[',']'
                memmove(one_line_, one_line_ + 1, strlen(one_line_) - 1);
                one_line_[strlen(one_line_) - 2] = '\0';

                strncpy(pstrtmp, one_line_, ntmp - 1);
                *(pstrtmp + ntmp - 1) = '\0';

                ntmp = ntmp - strlen(pstrtmp) - 1;

                //ͬʱ���ǽ����ַ�������'\0'
                if (ntmp > 1)
                {
                    //������һ��д�ĵط������Ȱ���һ��'\0',
                    pstrtmp += strlen(pstrtmp) + 1;
                }
                //
                else
                {
                    *(pstrtmp + strlen(pstrtmp)) = '\0';
                    //�������ַ�Ҫ��˫'\0'
                    *(return_str + strlen(pstrtmp) + 1) = '\0';
                    return size_ret_str - ntmp - 2;
                }
            }
        }

        //
        if (*return_str != '\0')
        {
            *pstrtmp = '\0';
            return size_ret_str - ntmp - 2;
        }
        else
        {
            //����Ĭ��ֵ
            if (default_str == NULL)
            {
                return_str[0] = '\0';
                return 0;
            }
            else
            {
                strncpy(return_str, default_str, size_ret_str - 2);
                return_str[size_ret_str - 2] = '\0';
                *(return_str + strlen(return_str)) = '\0';
                *(return_str + strlen(return_str) + 1) = '\0';
                return strlen(return_str);
            }
        }

    }

    //���KeyΪ�գ�App���գ��򷵻����е�Section�µ�Keyֵ����'\0'�ָ������һ��������'\0'��ʶ
    if (operate == 2)
    {
        //��������ַ�������С��2,ȫ�����ؿ��ַ�
        if (size_ret_str <= 2)
        {
            *return_str = '\0';
            *(return_str + 1) = '\0';
            return size_ret_str;
        }

        if_app = false;
        pstrtmp = return_str;
        ntmp = size_ret_str - 1;

        while (cfgfile)
        {

            cfgfile.getline(one_line_, LINE_BUFFER_LEN);
            //����
            zce::strtrim(one_line_);

            //ע����
            if (one_line_[0] == ';' || one_line_[0] == '#')
            {
                continue;
            }

            //��ƥ���Section
            if (one_line_[0] == '[' && one_line_[strlen(one_line_) - 1] == ']')
            {
                //�Ѿ��ҵ���һ��Section,û�з�����ص�Key�����ز�ѯ������Keyֵ
                if (if_app == true)
                {
                    *pstrtmp = '\0';
                    return size_ret_str - ntmp - 2;
                }

                //ȥ��'[',']'
                memmove(one_line_, one_line_ + 1, strlen(one_line_) - 1);
                one_line_[strlen(one_line_) - 2] = '\0';
                //����
                zce::strtrim(one_line_);

                if (zce::strcasecmp(one_line_, sec_name) == 0)
                {
                    if_app = true;
                    pstrtmp = return_str;
                    ntmp = size_ret_str - 1;
                    continue;
                }
            }

            //��key
            if (if_app == true)
            {
                char *str = strstr(one_line_, "=");

                if (str != NULL)
                {

                    strncpy(pstrtmp, one_line_, ntmp - 1);
                    //��ӽ�����
                    *(pstrtmp + strlen(pstrtmp)) = '\0';
                    //���Ȱ���һ��'\0'��
                    ntmp = ntmp - strlen(pstrtmp) - 1;

                    //ͬʱ���ǽ����ַ�������'\0'
                    if (ntmp > 1)
                    {
                        //������һ��д�ĵط������Ȱ���һ��'\0',
                        pstrtmp += strlen(pstrtmp) + 1;
                    }
                    //
                    else
                    {
                        *(pstrtmp + strlen(pstrtmp)) = '\0';
                        //�������ַ�Ҫ��˫'\0'
                        *(return_str + strlen(pstrtmp) + 1) = '\0';
                        return size_ret_str - ntmp - 2;
                    }
                }
            }
        }

        //
        if (*return_str != '\0')
        {
            *pstrtmp = '\0';
            return size_ret_str - ntmp - 2;
        }
        else
        {
            //����Ĭ��ֵ
            if (default_str == NULL)
            {
                return_str[0] = '\0';
                return 0;
            }
            else
            {
                strncpy(return_str, default_str, size_ret_str - 2);
                return_str[size_ret_str - 2] = '\0';
                *(return_str + strlen(return_str)) = '\0';
                *(return_str + strlen(return_str) + 1) = '\0';
                return strlen(return_str);
            }
        }

    }

    //AppΪ�գ�Key��Ϊ��,�������е�Key������һ��ƥ��ļ�ֵ����
    if (operate == 3)
    {
        while (cfgfile)
        {
            cfgfile.getline(one_line_, LINE_BUFFER_LEN);
            //����
            zce::strtrim(one_line_);

            //ע����
            if (one_line_[0] == ';' || one_line_[0] == '#')
            {
                continue;
            }

            char *str = strstr(one_line_, "=");

            if (str != NULL)
            {
                char *snext = str + 1;
                *str = '\0';
                strncpy(str_key_, one_line_, LINE_BUFFER_LEN);
                strncpy(str_value_, snext, LINE_BUFFER_LEN);
                str_value_[LINE_BUFFER_LEN] = '\0';

                ////
                //�ҵ����ء�
                if (zce::strcasecmp(str_key_, key_name) == 0)
                {
                    strncpy(return_str, str_value_, size_ret_str - 1);
                    return_str[size_ret_str - 1] = '\0';
                    return strlen(return_str);
                }
            }
        }

        //����Ĭ��ֵ
        if (default_str == NULL)
        {
            *return_str = '\0';
            return 0;
        }
        else
        {
            strncpy(return_str, default_str, size_ret_str - 1);
            return_str [size_ret_str - 1] = '\0';
            return strlen(return_str);
        }
    }

    //App��Key����Ϊ�գ��ⷵ��App��Key��ƥ��ļ�ֵ
    if (operate == 4)
    {
        if_app = false;

        while (cfgfile)
        {

            cfgfile.getline(one_line_, LINE_BUFFER_LEN);
            //����
            zce::strtrim(one_line_);

            //ע����
            if (one_line_[0] == ';' || one_line_[0] == '#')
            {
                continue;
            }

            if (one_line_[0] == '[' && one_line_[strlen(one_line_) - 1] == ']')
            {
                //�Ѿ��ҵ���һ��Section,û�з�����ص�Key������Ĭ��ֵ
                if (if_app == true)
                {
                    //����Ĭ��ֵ
                    if (default_str == NULL)
                    {
                        *return_str = '\0';
                        return 0;
                    }
                    else
                    {
                        strncpy(return_str, default_str, size_ret_str - 1);
                        *(return_str + size_ret_str - 1) = '\0';
                        return strlen(return_str);
                    }
                }

                //ȥ��'[',']'
                memmove(one_line_, one_line_ + 1, strlen(one_line_) - 1);
                one_line_[strlen(one_line_) - 2] = '\0';
                //
                zce::strtrim(one_line_);

                //
                if (zce::strcasecmp(one_line_, sec_name) == 0)
                {
                    if_app = true;
                    continue;
                }
            }

            if (if_app == true)
            {
                char *str = strstr(one_line_, "=");

                if (str != NULL)
                {
                    char *snext = str + 1;
                    *str = '\0';
                    strncpy(str_key_, one_line_, LINE_BUFFER_LEN);
                    strncpy(str_value_, snext, LINE_BUFFER_LEN);
                    ////
                    zce::strtrim(str_key_);
                    zce::strtrim(str_value_);

                    //�ҵ����ء�
                    if (zce::strcasecmp(str_key_, key_name) == 0)
                    {
                        strncpy(return_str, str_value_, size_ret_str - 1);
                        *(return_str + size_ret_str - 1) = '\0';
                        return strlen(return_str);
                    }
                }
            }
        }

        //
        if (default_str == NULL)
        {
            *return_str = '\0';
            return 0;
        }
        else
        {
            strncpy(return_str, default_str, size_ret_str - 1);
            return_str[size_ret_str - 1] = '\0';
            return strlen(return_str);
        }
    }

    //No rearch
    return 0;
}

//�õ����е�section,��'\0'�ָ�����'\0\0'����
size_t ZCE_INI_Pt::get_private_allsection(char *return_str,
                                          const size_t size_ret_str)
{
    //����static get_privateprofile_string�������,ע�����
    return get_private_str(NULL,
                           NULL,
                           NULL,
                           return_str,
                           size_ret_str);
}

//�õ�ĳ��Section�����е�Key = stringֵ ��'\0'�ָ�����'\0\0'����
size_t ZCE_INI_Pt::getprivate_allkey(const char *sec_name,
                                     char *return_str, const size_t size_ret_str)
{
    //����static get_privateprofile_string�������,ע�����
    return get_private_str(sec_name,
                           NULL,
                           NULL,
                           return_str,
                           size_ret_str);
}

//�õ�Int�� Keyֵ
int ZCE_INI_Pt::get_private_int(const char *sec_name,
                                const char *key_name,
                                const int default_int)
{
    //����static get_privateprofile_string�������,ע�����
    const size_t TMP_BUFFER_LEN = 64;
    char default_str[TMP_BUFFER_LEN], return_str[TMP_BUFFER_LEN];
    snprintf(default_str, TMP_BUFFER_LEN - 1, "%d", default_int);
    get_private_str(sec_name,
                    key_name,
                    default_str,
                    return_str,
                    TMP_BUFFER_LEN - 1);
    return atoi(return_str);
}

//�õ�bool�� Keyֵ
bool ZCE_INI_Pt::get_private_bool(const char *sec_name,
                                  const char *key_name,
                                  bool bDefault)
{
    //����static get_privateprofile_string�������,ע�����
    const size_t TMP_BUFFER_LEN = 64;
    char default_str[TMP_BUFFER_LEN], return_str[TMP_BUFFER_LEN];

    if (bDefault)
    {
        snprintf(default_str, TMP_BUFFER_LEN - 1, "%s", "TRUE");
    }
    else
    {
        snprintf(default_str, TMP_BUFFER_LEN - 1, "%s", "FALSE");
    }

    get_private_str(sec_name,
                    key_name,
                    default_str,
                    return_str,
                    TMP_BUFFER_LEN - 1);

    if (zce::strcasecmp(return_str, "TRUE"))
    {
        return false;
    }
    else
    {
        return true;
    }
}


//�޸�Section
bool ZCE_INI_Pt::write_private_section(const char *sec_name,
                                       const char *write_string)
{
    if (sec_name == NULL || write_string == NULL )
    {
        return false;
    }

    FILE *pfile = fopen(file_name_.c_str(), "r+t");

    //�ļ��򲻿���
    if (pfile == NULL)
    {
        return false;
    }

    //8*1024,1�е����ֵ
    char *read_ret = NULL;


    //���ļ�����������
    std::vector<std::string> strarytmp;
    write_line_[0] = '\0';

    //��ʾ�Ƿ��ҵ�App,key
    bool bApp = false;

    while (!feof(pfile))
    {
        //��д����ʱ������
        if (write_line_[0] != '\0')
        {
            std::string strtmp(write_line_);
            strarytmp.push_back(write_line_);
        }

        read_ret = fgets(one_line_, LINE_BUFFER_LEN, pfile);
        if (NULL == read_ret)
        {
            return false;
        }
        memmove(write_line_, one_line_, strlen(one_line_) + 1);
        //����
        zce::strtrim(one_line_);

        //ע����,����
        if (one_line_[0] == ';' || one_line_[0] == '#' || one_line_[0] == '\0')
        {
            continue;
        }

        //[Section]
        if (one_line_[0] == '[' && one_line_[strlen(one_line_) - 1] == ']')
        {
            //ȥ��'[',']'
            memmove(one_line_, one_line_ + 1, strlen(one_line_) - 1);
            one_line_[strlen(one_line_) - 2] = '\0';
            //����
            zce::strtrim(one_line_);

            //�Ƚ�Section����
            if (zce::strcasecmp(one_line_, sec_name) == 0)
            {
                bApp = true;
                //Ҫ�޸����App
                snprintf(write_line_, LINE_BUFFER_LEN, "[%s]\n", write_string);
                break;
            }
        }
    }

    //
    if (write_line_[0] != '\0')
    {
        std::string strtmp(write_line_);
        strarytmp.push_back(write_line_);
    }

    //����ҵ�App ,�����ҵ�Key����key == NUL ,��ʾ�����Ѿ����,
    if (bApp == true)
    {

        while (!feof(pfile))
        {
            read_ret = fgets(write_line_, LINE_BUFFER_LEN, pfile);
            if (NULL == read_ret)
            {
                return false;
            }
            std::string strtmp(write_line_);
            strarytmp.push_back(write_line_);
        }

        fclose(pfile);
        //����Ϣд���ļ�
        pfile = fopen(file_name_.c_str(), "w+t");

        for (size_t j = 0; j < strarytmp.size(); ++j)
        {
            fputs(strarytmp[j].c_str(), pfile);
        }

        //�ر��ļ�
        fclose(pfile);
        return true;
    }

    return false;
}

//д��Int
bool ZCE_INI_Pt::write_private_int(const char *sec_name,
                                   const char *key_name,
                                   const int nvalue)
{
    //����static write_privateprofile_string�������,ע�����
    const size_t TMP_BUFFER_LEN = 64;
    char write_string[TMP_BUFFER_LEN + 1];
    snprintf(write_string, TMP_BUFFER_LEN, "%d", nvalue);
    return write_private_string(sec_name,
                                key_name,
                                write_string);
}

//д��Bool����
bool ZCE_INI_Pt::write_private_bool(const char *sec_name,
                                    const char *key_name,
                                    const bool bvalue)
{
    //����static write_privateprofile_string�������,ע�����
    const size_t TMP_BUFFER_LEN = 64;
    char  write_string[TMP_BUFFER_LEN + 1];

    if (bvalue)
    {
        strncpy(write_string, "TRUE", TMP_BUFFER_LEN);
    }
    else
    {
        strncpy(write_string, "FASLE", TMP_BUFFER_LEN);
    }

    return write_private_string(sec_name, key_name, write_string);
}

//ɾ��Key
bool ZCE_INI_Pt::del_private_key(const char *sec_name, const char *key_name)
{

    return write_private_string(sec_name, key_name, NULL);
}

//ɾ��Section
bool ZCE_INI_Pt::del_private_section(const char *sec_name)
{

    return write_private_string(sec_name,
                                NULL,
                                NULL);
}


//дINI�ļ���ı���ֶΣ�
bool ZCE_INI_Pt::write_private_string(const char *sec_name,
                                      const char *key_name,
                                      const char *write_string)
{

    int operate(0);

    if (sec_name == NULL && key_name == NULL && write_string == NULL)
    {
        return false;
    }

    if (sec_name == NULL)
    {
        operate = 1;
    }
    else
    {
        operate = 2;
    }

    FILE *pfile;
    pfile = fopen(file_name_.c_str(), "r+t");

    //�ļ��򲻿���
    if (pfile == NULL)
    {
        pfile = fopen(file_name_.c_str(), "w+t");

        if (pfile == NULL)
        {
            return false;
        }

        if (sec_name != NULL)
        {
            fprintf(pfile, "[%s]\n", sec_name);
        }

        fprintf(pfile, "%s=%s", key_name, write_string);
        fclose(pfile);
        return true;
    }

    if (operate == 1)
    {
        //���û����ص�App����section=string д��ĩβ
        fseek(pfile, 0, SEEK_END);
        fprintf(pfile, "%s", "\n");
        fprintf(pfile, "%s=%s", key_name, write_string);
        return true;
    }

    if (operate == 2)
    {
        //���ļ�����������
        std::vector<std::string> strarytmp;
        write_line_[0] = '\0';
        //��ʾ�Ƿ��ҵ�App,key
        bool bApp = false, bkey = false;

        while (!feof(pfile))
        {
            //��д����ʱ�ļ�
            if (write_line_[0] != '\0')
            {
                std::string strtmp(write_line_);
                strarytmp.push_back(write_line_);
            }

            if (fgets(one_line_, LINE_BUFFER_LEN, pfile) == NULL)
            {
                break;
            }

            ::memcpy(write_line_, one_line_, strlen(one_line_) + 1);
            //����
            zce::strtrim(one_line_);

            //ע����,����
            if (one_line_[0] == ';' || one_line_[0] == '#' || one_line_[0] == '\0')
            {
                continue;
            }

            //[Section]
            if (one_line_[0] == '[' && one_line_[strlen(one_line_) - 1] == ']')
            {
                //�Ѿ��ҵ���һ��Section,û�з�����ص�Key������Ĭ��ֵ
                if (bApp == true)
                {
                    if (key_name != NULL && write_string != NULL)
                    {
                        snprintf(one_line_, LINE_BUFFER_LEN, "%s=%s\n", key_name, write_string);
                        one_line_[LINE_BUFFER_LEN] = '\0';
                        std::string strtmp(one_line_);
                        strarytmp.push_back(one_line_);
                    }

                    break;
                }

                one_line_[strlen(one_line_) - 1] = '\0';
                //����
                zce::strtrimleft(one_line_);
                //ȥ��'[',']'
                memmove(one_line_, one_line_ + 1, strlen(one_line_));
                zce::strtrimright(one_line_);

                //�Ƚ�Section����
                if (zce::strcasecmp(one_line_, sec_name) == 0)
                {
                    bApp = true;

                    //Ҫɾ�����App
                    if (key_name == NULL)
                    {
                        write_line_[0] = '\0';
                    }

                    continue;
                }
            }

            //���[Section] �Ѿ��ҵ�
            if (bApp == true)
            {
                //���Key==NULL,���ò�ѯ������
                if (key_name == NULL)
                {
                    write_line_[0] = '\0';
                    continue;
                }

                //�ҵ���һ��'='
                char *str = strstr(one_line_, "=");

                //����ҵ�
                if (str != NULL)
                {
                    char *snext = str + 1;
                    *str = '\0';
                    strncpy(str_key_, one_line_, LINE_BUFFER_LEN);
                    strncpy(str_value_, snext, LINE_BUFFER_LEN);
                    //����
                    zce::strtrim(str_key_);
                    zce::strtrim(str_value_);

                    //�ҵ���Ӧ��Key
                    if (zce::strcasecmp(str_key_, key_name) == 0)
                    {
                        //��ʾ�ҵ���ֵ
                        bkey = true;

                        //���Keystring==NULL��ɾ��Key�����Keystring!=NULL,��д�롣
                        if (write_string != NULL)
                        {
                            snprintf(write_line_, LINE_BUFFER_LEN, "%s=%s\n", key_name, write_string);
                            break;
                        }
                        else
                        {
                            write_line_[0] = '\0';
                            break;
                        }
                    }
                }
            }
        }

        //
        if (write_line_[0] != '\0')
        {
            std::string strtmp(write_line_);
            strarytmp.push_back(write_line_);
        }

        //����ҵ�App ,�����ҵ�Key����key == NUL ,��ʾ�����Ѿ����,
        if (bApp == true)
        {

            if (key_name != NULL && write_string != NULL && feof(pfile) && bkey != true)
            {
                snprintf(write_line_, LINE_BUFFER_LEN, "\n%s=%s", key_name, write_string);
                std::string strtmp(write_line_);
                strarytmp.push_back(write_line_);
            }

            while (!feof(pfile))
            {

                if (fgets(write_line_, LINE_BUFFER_LEN, pfile) == NULL)
                {
                    break;
                }

                std::string strtmp(write_line_);
                strarytmp.push_back(write_line_);

            }

            fclose(pfile);
            //����Ϣд���ļ�
            pfile = fopen(file_name_.c_str(), "w+t");

            for (size_t j = 0; j < strarytmp.size(); ++j)
            {
                fputs(strarytmp[j].c_str(), pfile);
            }

            //�ر��ļ�
            fclose(pfile);
            return true;
        }
        else if (bApp == false)
        {
            //��App��Key Stringȫ��д��
            if (bkey == false && key_name != NULL && write_string != NULL)
            {
                fseek(pfile, 0, SEEK_END);
                fprintf(pfile, "\n[%s]", sec_name);
                fprintf(pfile, "\n%s=%s", key_name, write_string);
                fclose(pfile);
                return true;
            }
            else
            {
                fclose(pfile);
                return false;
            }
        }

        fclose(pfile);
        return false;
    }

    //No rearch
    return false;
}
