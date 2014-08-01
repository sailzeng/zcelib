
#include "zce_predefine.h"
#include "zce_os_adapt_string.h"
#include "zce_cfg_ini_pasttime.h"

//
ZCE_CfgFile_INI_Pt::ZCE_CfgFile_INI_Pt(const char *szfname)
{
    file_name_.assign(szfname);
}

ZCE_CfgFile_INI_Pt::~ZCE_CfgFile_INI_Pt()
{

}

//得到key值的String
size_t ZCE_CfgFile_INI_Pt::get_private_str(const char *sec_name,
                                           const char *key_name,
                                           const char *default_str,
                                           char *return_str,
                                           const size_t size_ret_str)
{
    //调用static get_privateprofile_string函数完成,注意参数
    return get_privateprofile_string(sec_name,
                                     key_name,
                                     default_str,
                                     return_str,
                                     size_ret_str, file_name_.c_str());
}

//得到所有的section,用'\0'分隔，以'\0\0'结束
size_t ZCE_CfgFile_INI_Pt::get_private_allsection(char *return_str,
                                                  const size_t size_ret_str)
{
    //调用static get_privateprofile_string函数完成,注意参数
    return get_privateprofile_string(NULL,
                                     NULL,
                                     NULL,
                                     return_str,
                                     size_ret_str, file_name_.c_str());
}

//得到某个Section下所有的Key = string值 用'\0'分隔，以'\0\0'结束
size_t ZCE_CfgFile_INI_Pt::getprivate_allkey(const char *sec_name,
                                             char *return_str, const size_t size_ret_str)
{
    //调用static get_privateprofile_string函数完成,注意参数
    return get_privateprofile_string(sec_name, NULL, NULL, return_str, size_ret_str, file_name_.c_str());
}

//得到Int的 Key值
int ZCE_CfgFile_INI_Pt::get_private_int(const char *sec_name,
                                        const char *key_name,
                                        const int default_int)
{
    //调用static get_privateprofile_string函数完成,注意参数
    const size_t TMP_BUFFER_LEN = 64;
    char default_str[TMP_BUFFER_LEN], return_str[TMP_BUFFER_LEN];
    snprintf(default_str, TMP_BUFFER_LEN, "%d", default_int);
    get_privateprofile_string(sec_name,
                              key_name,
                              default_str,
                              return_str,
                              64,
                              file_name_.c_str());
    return atoi(return_str);
}

//得到bool的 Key值
bool ZCE_CfgFile_INI_Pt::get_private_bool(const char *sec_name,
                                          const char *key_name,
                                          bool bDefault)
{
    //调用static get_privateprofile_string函数完成,注意参数
    const size_t TMP_BUFFER_LEN = 64;
    char default_str[TMP_BUFFER_LEN + 1], return_str[TMP_BUFFER_LEN + 1];

    if (bDefault)
    {
        snprintf(default_str, TMP_BUFFER_LEN, "%s", "TRUE");
    }
    else
    {
        snprintf(default_str, TMP_BUFFER_LEN, "%s", "FALSE");
    }

    get_privateprofile_string(sec_name, key_name, default_str, return_str, 64, file_name_.c_str());

    if (ZCE_OS::strcasecmp(return_str, "TRUE"))
    {
        return false;
    }
    else
    {
        return true;
    }
}

//写入String
bool ZCE_CfgFile_INI_Pt::write_private_string(const char *sec_name,
                                              const char *key_name,
                                              const char *write_string)
{
    //调用static WritePrivateProfileString函数完成,注意参数
    return write_privateprofile_string(sec_name, key_name, write_string, file_name_.c_str());
}

//修改Section
bool ZCE_CfgFile_INI_Pt::write_private_section(const char *sec_name,
                                               const char *write_string)
{
    //调用static WritePrivateProfileSection函数完成,注意参数
    return   write_privateprofile_section(sec_name, write_string, file_name_.c_str());
}

//写入Int
bool ZCE_CfgFile_INI_Pt::write_private_int(const char *sec_name,
                                           const char *key_name,
                                           const int nvalue)
{
    //调用static write_privateprofile_string函数完成,注意参数
    const size_t TMP_BUFFER_LEN = 64;
    char write_string[TMP_BUFFER_LEN + 1];
    snprintf(write_string, TMP_BUFFER_LEN, "%d", nvalue);
    return write_privateprofile_string(sec_name,
                                       key_name,
                                       write_string,
                                       file_name_.c_str());
}

//写入Bool变量
bool ZCE_CfgFile_INI_Pt::write_private_bool(const char *sec_name,
                                            const char *key_name,
                                            const bool bvalue)
{
    //调用static write_privateprofile_string函数完成,注意参数
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

    return write_privateprofile_string(sec_name, key_name, write_string, file_name_.c_str());
}

//删除Key
bool ZCE_CfgFile_INI_Pt::del_private_key(const char *sec_name, const char *key_name)
{

    return write_privateprofile_string(sec_name, key_name, NULL, file_name_.c_str());
}

//删除Section
bool ZCE_CfgFile_INI_Pt::del_private_section(const char *sec_name)
{

    return write_privateprofile_string(sec_name,
                                       NULL,
                                       NULL,
                                       file_name_.c_str());
}

size_t ZCE_CfgFile_INI_Pt::get_privateprofile_string(
    const char  *sec_name,
    const char  *key_name,
    const char  *default_str,
    char  *return_str,
    const size_t size_ret_str,
    const char  *file_name)
{
    //返回字符串的指针为空,返回错误
    if (return_str == NULL || file_name == NULL)
    {
        assert(0);
    }

    *return_str = '\0';

    //返回字符串的尺寸小于1,
    if (size_ret_str <= 1)
    {
        *return_str = '\0';
        return size_ret_str;
    }

    int operate = 0;

    //如果App和Key全部为空，则返回所有的Section，用'\0'分隔，最后一个用两个'\0'标识
    if (sec_name == NULL && key_name == NULL)
    {
        operate = 1;
    }

    //如果Key为空，App不空，则返回所有的Section下的Key值，用'\0'分隔，最后一个用两个'\0'标识
    if (sec_name != NULL && key_name == NULL)
    {
        operate = 2;
    }

    //App为空，Key不为空,则检查所有的Key，将第一个匹配的键值返回
    if (sec_name == NULL && key_name != NULL)
    {
        operate = 3;
    }

    //App，Key部不为空，这返回App，Key都匹配的键值
    if (sec_name != NULL && key_name != NULL)
    {
        operate = 4;
    }

    std::ifstream cfgfile(file_name);

    //文件打不开，返回默认值
    if (!cfgfile)
    {
        strncpy(return_str, default_str, size_ret_str - 1);
        *(return_str + size_ret_str - 1) = '\0';
        return strlen(return_str);
    }

    //8*1024,1行的最大值
    char choneline[LINE_BUFFER_LEN + 1], chkey[LINE_BUFFER_LEN + 1], chstring[LINE_BUFFER_LEN + 1];
    choneline[LINE_BUFFER_LEN] = '\0';
    chkey[LINE_BUFFER_LEN] = '\0';
    chstring[LINE_BUFFER_LEN] = '\0';

    char     *pstrtmp;
    size_t ntmp;
    bool bApp;

    //如果App和Key全部为空，则返回所有的Section，用'\0'分隔，最后一个用两个'\0'标识
    if (operate == 1)
    {
        //如果返回字符串长度小于2,全部返回空字符
        if (size_ret_str <= 2)
        {
            *return_str = '\0';
            *(return_str + 1) = '\0';
            return size_ret_str;
        }

        bApp = false;
        pstrtmp = return_str;
        ntmp = size_ret_str - 1;

        while (cfgfile)
        {
            cfgfile.getline(choneline, LINE_BUFFER_LEN);
            //fgets(choneline,LINE_BUFFER_LEN,pfile);
            //整理，
            ZCE_OS::strtrim(choneline);

            //注释行
            if (choneline[0] == ';' || choneline[0] == '#')
            {
                continue;
            }

            if (choneline[0] == '[' && choneline[strlen(choneline) - 1] == ']')
            {
                //去掉'[',']'
                memmove(choneline, choneline + 1, strlen(choneline) - 1);
                choneline[strlen(choneline) - 2] = '\0';

                strncpy(pstrtmp, choneline, ntmp - 1);
                *(pstrtmp + ntmp - 1) = '\0';

                ntmp = ntmp - strlen(pstrtmp) - 1;

                //同时考虑结束字符用两个'\0'
                if (ntmp > 1)
                {
                    //跳到下一个写的地方、长度包括一个'\0',
                    pstrtmp += strlen(pstrtmp) + 1;
                }
                //
                else
                {
                    *(pstrtmp + strlen(pstrtmp)) = '\0';
                    //结束的字符要用双'\0'
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
            //返回默认值
            if (default_str == NULL)
            {
                return_str[0] = '\0';
                return 0;
            }
            else
            {
                strncpy(return_str, default_str, size_ret_str - 2);
                *(return_str + strlen(return_str)) = '\0';
                *(return_str + strlen(return_str) + 1) = '\0';
                return strlen(return_str);
            }
        }

    }

    //如果Key为空，App不空，则返回所有的Section下的Key值，用'\0'分隔，最后一个用两个'\0'标识
    if (operate == 2)
    {
        //如果返回字符串长度小于2,全部返回空字符
        if (size_ret_str <= 2)
        {
            *return_str = '\0';
            *(return_str + 1) = '\0';
            return size_ret_str;
        }

        bApp = false;
        pstrtmp = return_str;
        ntmp = size_ret_str - 1;

        while (cfgfile)
        {

            cfgfile.getline(choneline, LINE_BUFFER_LEN);
            //整理，
            ZCE_OS::strtrim(choneline);

            //注释行
            if (choneline[0] == ';' || choneline[0] == '#')
            {
                continue;
            }

            //找匹配的Section
            if (choneline[0] == '[' && choneline[strlen(choneline) - 1] == ']')
            {
                //已经找到下一个Section,没有发现相关的Key，返回查询的所有Key值
                if (bApp == true)
                {
                    *pstrtmp = '\0';
                    return size_ret_str - ntmp - 2;
                }

                //去掉'[',']'
                memmove(choneline, choneline + 1, strlen(choneline) - 1);
                choneline[strlen(choneline) - 2] = '\0';
                //整理，
                ZCE_OS::strtrim(choneline);

                if (ZCE_OS::strcasecmp(choneline, sec_name) == 0)
                {
                    bApp = true;
                    pstrtmp = return_str;
                    ntmp = size_ret_str - 1;
                    continue;
                }
            }

            //找key
            if (bApp == true)
            {
                char *str = strstr(choneline, "=");

                if (str != NULL)
                {

                    strncpy(pstrtmp, choneline, ntmp - 1);
                    //添加结束符
                    *(pstrtmp + strlen(pstrtmp)) = '\0';
                    //长度包括一个'\0'，
                    ntmp = ntmp - strlen(pstrtmp) - 1;

                    //同时考虑结束字符用两个'\0'
                    if (ntmp > 1)
                    {
                        //跳到下一个写的地方、长度包括一个'\0',
                        pstrtmp += strlen(pstrtmp) + 1;
                    }
                    //
                    else
                    {
                        *(pstrtmp + strlen(pstrtmp)) = '\0';
                        //结束的字符要用双'\0'
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
            //返回默认值
            if (default_str == NULL)
            {
                return_str[0] = '\0';
                return 0;
            }
            else
            {
                strncpy(return_str, default_str, size_ret_str - 2);
                *(return_str + strlen(return_str)) = '\0';
                *(return_str + strlen(return_str) + 1) = '\0';
                return strlen(return_str);
            }
        }

    }

    //App为空，Key不为空,则检查所有的Key，将第一个匹配的键值返回
    if (operate == 3)
    {
        while (cfgfile)
        {
            cfgfile.getline(choneline, LINE_BUFFER_LEN);
            //整理
            ZCE_OS::strtrim(choneline);

            //注释行
            if (choneline[0] == ';' || choneline[0] == '#')
            {
                continue;
            }

            char *str = strstr(choneline, "=");

            if (str != NULL)
            {
                char *snext = str + 1;
                *str = '\0';
                strncpy(chkey, choneline, LINE_BUFFER_LEN);
                strncpy(chstring, snext, LINE_BUFFER_LEN);

                ////
                //找到返回。
                if (ZCE_OS::strcasecmp(chkey, key_name) == 0)
                {
                    strncpy(return_str, chstring, size_ret_str - 1);
                    *(return_str + size_ret_str - 1) = '\0';
                    return strlen(return_str);
                }
            }
        }

        //返回默认值
        if (default_str == NULL)
        {
            return_str = '\0';
            return 0;
        }
        else
        {
            strncpy(return_str, default_str, size_ret_str - 1);
            *(return_str + size_ret_str - 1) = '\0';
            return strlen(return_str);
        }
    }

    //App，Key部不为空，这返回App，Key都匹配的键值
    if (operate == 4)
    {
        bApp = false;

        while (cfgfile)
        {

            cfgfile.getline(choneline, LINE_BUFFER_LEN);
            //整理
            ZCE_OS::strtrim(choneline);

            //注释行
            if (choneline[0] == ';' || choneline[0] == '#')
            {
                continue;
            }

            if (choneline[0] == '[' && choneline[strlen(choneline) - 1] == ']')
            {
                //已经找到下一个Section,没有发现相关的Key，返回默认值
                if (bApp == true)
                {
                    //返回默认值
                    if (default_str == NULL)
                    {
                        return_str = '\0';
                        return 0;
                    }
                    else
                    {
                        strncpy(return_str, default_str, size_ret_str - 1);
                        *(return_str + size_ret_str - 1) = '\0';
                        return strlen(return_str);
                    }
                }

                //去掉'[',']'
                memmove(choneline, choneline + 1, strlen(choneline) - 1);
                choneline[strlen(choneline) - 2] = '\0';
                //
                ZCE_OS::strtrim(choneline);

                //
                if (ZCE_OS::strcasecmp(choneline, sec_name) == 0)
                {
                    bApp = true;
                    continue;
                }
            }

            if (bApp == true)
            {
                char *str = strstr(choneline, "=");

                if (str != NULL)
                {
                    char *snext = str + 1;
                    *str = '\0';
                    strncpy(chkey, choneline, LINE_BUFFER_LEN);
                    strncpy(chstring, snext, LINE_BUFFER_LEN);
                    ////
                    ZCE_OS::strtrim(chkey);
                    ZCE_OS::strtrim(chstring);

                    //找到返回。
                    if (ZCE_OS::strcasecmp(chkey, key_name) == 0)
                    {
                        strncpy(return_str, chstring, size_ret_str - 1);
                        *(return_str + size_ret_str - 1) = '\0';
                        return strlen(return_str);
                    }
                }
            }
        }

        //
        if (default_str == NULL)
        {
            return_str = '\0';
            return 0;
        }
        else
        {
            strncpy(return_str, default_str, size_ret_str - 1);
            *(return_str + size_ret_str - 1) = '\0';
            return strlen(return_str);
        }
    }

    //No rearch
    return 0;
}

//读取INI文件的谋个字段作为整数返回
int ZCE_CfgFile_INI_Pt::get_privateprofile_int(
    const char *sec_name,
    const char *key_name,
    int default_int,
    const char *file_name)
{
    const size_t TMP_BUFFER_LEN = 64;
    char default_str[TMP_BUFFER_LEN + 1], return_str[TMP_BUFFER_LEN + 1];
    snprintf(default_str, TMP_BUFFER_LEN, "%d", default_int);
    ZCE_CfgFile_INI_Pt::get_privateprofile_string(sec_name, key_name, default_str, return_str, 64, file_name);
    return atoi(return_str);
}

//写INI文件的谋个字段，
bool ZCE_CfgFile_INI_Pt::write_privateprofile_string(
    const char *sec_name,
    const char *key_name,
    const char *write_string,
    const char *file_name)
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
    pfile = fopen(file_name, "r+t");

    //文件打不开，
    if (pfile == NULL)
    {
        pfile = fopen(file_name, "w+t");

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

    //1行的最大值
    char choneline[LINE_BUFFER_LEN + 1], line[LINE_BUFFER_LEN + 1], chkey[LINE_BUFFER_LEN + 1], chstring[LINE_BUFFER_LEN + 1];

    if (operate == 1)
    {
        //如果没有相关的App，将section=string 写在末尾
        fseek(pfile, 0, SEEK_END);
        fprintf(pfile, "%s", "\n");
        fprintf(pfile, "%s=%s", key_name, write_string);
        return true;
    }

    if (operate == 2)
    {
        //新文件保存数据区
        std::vector<std::string> strarytmp;
        std::string strtmp;
        line[0] = '\0';
        //表示是否找到App,key
        bool bApp = false, bkey = false;

        while (!feof(pfile))
        {
            //填写到临时文件
            if (line[0] != '\0')
            {
                std::string strtmp(line);
                strarytmp.push_back(line);
            }

            if (fgets(choneline, LINE_BUFFER_LEN, pfile) == NULL)
            {
                break;
            }

            memmove(line, choneline, strlen(choneline) + 1);
            //整理
            ZCE_OS::strtrim(choneline);

            //注释行,空行
            if (choneline[0] == ';' || choneline[0] == '#' || choneline[0] == '\0')
            {
                continue;
            }

            //[Section]
            if (choneline[0] == '[' && choneline[strlen(choneline) - 1] == ']')
            {
                //已经找到下一个Section,没有发现相关的Key，返回默认值
                if (bApp == true)
                {
                    if (key_name != NULL && write_string != NULL)
                    {
                        snprintf(choneline, LINE_BUFFER_LEN, "%s=%s\n", key_name, write_string);
                        choneline[LINE_BUFFER_LEN] = '\0';
                        std::string strtmp(choneline);
                        strarytmp.push_back(choneline);
                    }

                    break;
                }

                choneline[strlen(choneline) - 1] = '\0';
                //规整
                ZCE_OS::strtrimleft(choneline);
                //去掉'[',']'
                memmove(choneline, choneline + 1, strlen(choneline));
                ZCE_OS::strtrimright(choneline);

                //比较Section部分
                if (ZCE_OS::strcasecmp(choneline, sec_name) == 0)
                {
                    bApp = true;

                    //要删除这个App
                    if (key_name == NULL)
                    {
                        line[0] = '\0';
                    }

                    continue;
                }
            }

            //如果[Section] 已经找到
            if (bApp == true)
            {
                //如果Key==NULL,不用查询，继续
                if (key_name == NULL)
                {
                    line[0] = '\0';
                    continue;
                }

                //找到第一个'='
                char *str = strstr(choneline, "=");

                //如果找到
                if (str != NULL)
                {
                    char *snext = str + 1;
                    *str = '\0';
                    strncpy(chkey, choneline, LINE_BUFFER_LEN);
                    strncpy(chstring, snext, LINE_BUFFER_LEN);
                    //规整
                    ZCE_OS::strtrim(chkey);
                    ZCE_OS::strtrim(chstring);

                    //找到对应的Key
                    if (ZCE_OS::strcasecmp(chkey, key_name) == 0)
                    {
                        //表示找到键值
                        bkey = true;

                        //如果Keystring==NULL则删除Key，如果Keystring!=NULL,则写入。
                        if (write_string != NULL)
                        {
                            snprintf(line, LINE_BUFFER_LEN, "%s=%s\n", key_name, write_string);
                            break;
                        }
                        else
                        {
                            line[0] = '\0';
                            break;
                        }
                    }
                }
            }
        }

        //
        if (line[0] != '\0')
        {
            std::string strtmp(line);
            strarytmp.push_back(line);
        }

        //如果找到App ,并且找到Key或者key == NUL ,表示工作已经完成,
        if (bApp == true)
        {

            if (key_name != NULL && write_string != NULL && feof(pfile) && bkey != true)
            {
                snprintf(line, LINE_BUFFER_LEN, "\n%s=%s", key_name, write_string);
                std::string strtmp(line);
                strarytmp.push_back(line);
            }

            while (!feof(pfile))
            {

                if (fgets(line, LINE_BUFFER_LEN, pfile) == NULL)
                {
                    break;
                }

                std::string strtmp(line);
                strarytmp.push_back(line);

            }

            fclose(pfile);
            //将信息写会文件
            pfile = fopen(file_name, "w+t");

            for (size_t j = 0; j < strarytmp.size(); ++j)
            {
                fputs(strarytmp[j].c_str(), pfile);
            }

            //关闭文件
            fclose(pfile);
            return true;
        }
        else if (bApp == false)
        {
            //将App，Key String全部写入
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

bool ZCE_CfgFile_INI_Pt::write_privateprofile_section(
    const char *sec_name,
    const char *write_string,
    const char *file_name)
{

    if (sec_name == NULL || write_string == NULL || write_string == NULL)
    {
        return false;
    }

    FILE *pfile;
    pfile = fopen(file_name, "r+t");

    //文件打不开，
    if (pfile == NULL)
    {
        return false;
    }

    //8*1024,1行的最大值

    char choneline[LINE_BUFFER_LEN + 1], line[LINE_BUFFER_LEN + 1], *read_ret = NULL;

    //新文件保存数据区
    std::vector<std::string> strarytmp;
    std::string strtmp;
    line[0] = '\0';

    //表示是否找到App,key
    bool bApp = false;

    while (!feof(pfile))
    {
        //填写到临时数据区
        if (line[0] != '\0')
        {
            std::string strtmp(line);
            strarytmp.push_back(line);
        }

        read_ret = fgets(choneline, LINE_BUFFER_LEN, pfile);
        if (NULL == read_ret)
        {
            return false;
        }
        memmove(line, choneline, strlen(choneline) + 1);
        //整理
        ZCE_OS::strtrim(choneline);

        //注释行,空行
        if (choneline[0] == ';' || choneline[0] == '#' || choneline[0] == '\0')
        {
            continue;
        }

        //[Section]
        if (choneline[0] == '[' && choneline[strlen(choneline) - 1] == ']')
        {
            //去掉'[',']'
            memmove(choneline, choneline + 1, strlen(choneline) - 1);
            choneline[strlen(choneline) - 2] = '\0';
            //规整
            ZCE_OS::strtrim(choneline);

            //比较Section部分
            if (ZCE_OS::strcasecmp(choneline, sec_name) == 0)
            {
                bApp = true;
                //要修改这个App
                snprintf(line, LINE_BUFFER_LEN, "[%s]\n", write_string);
                break;
            }
        }
    }

    //
    if (line[0] != '\0')
    {
        std::string strtmp(line);
        strarytmp.push_back(line);
    }

    //如果找到App ,并且找到Key或者key == NUL ,表示工作已经完成,
    if (bApp == true)
    {

        while (!feof(pfile))
        {
            read_ret = fgets(line, LINE_BUFFER_LEN, pfile);
            if (NULL == read_ret)
            {
                return false;
            }
            std::string strtmp(line);
            strarytmp.push_back(line);
        }

        fclose(pfile);
        //将信息写会文件
        pfile = fopen(file_name, "w+t");

        for (size_t j = 0; j < strarytmp.size(); ++j)
        {
            fputs(strarytmp[j].c_str(), pfile);
        }

        //关闭文件
        fclose(pfile);
        return true;
    }

    return false;
}