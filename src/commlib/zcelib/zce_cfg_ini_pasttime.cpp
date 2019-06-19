
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

	//8*1024 -1,1行的最大值
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

//得到key值的String
size_t ZCE_INI_Pt::get_private_str(const char *sec_name,
                                   const char *key_name,
                                   const char *default_str,
                                   char *return_str,
                                   const size_t size_ret_str)
{
	//返回字符串的指针为空,返回错误
    assert(return_str);
    if (!return_str)
    {
        return 0;
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

	std::ifstream cfgfile(file_name_.c_str());

	//文件打不开，返回默认值
	if (!cfgfile)
	{
		strncpy(return_str, default_str, size_ret_str - 1);
		return_str[size_ret_str - 1] = '\0';
		return strlen(return_str);
	}

	char* pstrtmp;
	size_t ntmp;
	bool if_app;

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

		if_app = false;
		pstrtmp = return_str;
		ntmp = size_ret_str - 1;

		while (cfgfile)
		{
			cfgfile.getline(one_line_, LINE_BUFFER_LEN);
			//fgets(choneline,LINE_BUFFER_LEN,pfile);
			//整理，
			ZCE_LIB::strtrim(one_line_);

			//注释行
			if (one_line_[0] == ';' || one_line_[0] == '#')
			{
				continue;
			}

			if (one_line_[0] == '[' && one_line_[strlen(one_line_) - 1] == ']')
			{
				//去掉'[',']'
				memmove(one_line_, one_line_ + 1, strlen(one_line_) - 1);
				one_line_[strlen(one_line_) - 2] = '\0';

				strncpy(pstrtmp, one_line_, ntmp - 1);
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
				return_str[size_ret_str - 2] = '\0';
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

		if_app = false;
		pstrtmp = return_str;
		ntmp = size_ret_str - 1;

		while (cfgfile)
		{

			cfgfile.getline(one_line_, LINE_BUFFER_LEN);
			//整理，
			ZCE_LIB::strtrim(one_line_);

			//注释行
			if (one_line_[0] == ';' || one_line_[0] == '#')
			{
				continue;
			}

			//找匹配的Section
			if (one_line_[0] == '[' && one_line_[strlen(one_line_) - 1] == ']')
			{
				//已经找到下一个Section,没有发现相关的Key，返回查询的所有Key值
				if (if_app == true)
				{
					*pstrtmp = '\0';
					return size_ret_str - ntmp - 2;
				}

				//去掉'[',']'
				memmove(one_line_, one_line_ + 1, strlen(one_line_) - 1);
				one_line_[strlen(one_line_) - 2] = '\0';
				//整理，
				ZCE_LIB::strtrim(one_line_);

				if (ZCE_LIB::strcasecmp(one_line_, sec_name) == 0)
				{
					if_app = true;
					pstrtmp = return_str;
					ntmp = size_ret_str - 1;
					continue;
				}
			}

			//找key
			if (if_app == true)
			{
				char* str = strstr(one_line_, "=");

				if (str != NULL)
				{

					strncpy(pstrtmp, one_line_, ntmp - 1);
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
				return_str[size_ret_str - 2] = '\0';
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
			cfgfile.getline(one_line_, LINE_BUFFER_LEN);
			//整理
			ZCE_LIB::strtrim(one_line_);

			//注释行
			if (one_line_[0] == ';' || one_line_[0] == '#')
			{
				continue;
			}

			char* str = strstr(one_line_, "=");

			if (str != NULL)
			{
				char* snext = str + 1;
				*str = '\0';
				strncpy(str_key_, one_line_, LINE_BUFFER_LEN);
				strncpy(str_value_, snext, LINE_BUFFER_LEN);
				str_value_[LINE_BUFFER_LEN] = '\0';

				////
				//找到返回。
				if (ZCE_LIB::strcasecmp(str_key_, key_name) == 0)
				{
					strncpy(return_str, str_value_, size_ret_str - 1);
					return_str[size_ret_str - 1] = '\0';
					return strlen(return_str);
				}
			}
		}

		//返回默认值
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

	//App，Key部不为空，这返回App，Key都匹配的键值
	if (operate == 4)
	{
		if_app = false;

		while (cfgfile)
		{

			cfgfile.getline(one_line_, LINE_BUFFER_LEN);
			//整理
			ZCE_LIB::strtrim(one_line_);

			//注释行
			if (one_line_[0] == ';' || one_line_[0] == '#')
			{
				continue;
			}

			if (one_line_[0] == '[' && one_line_[strlen(one_line_) - 1] == ']')
			{
				//已经找到下一个Section,没有发现相关的Key，返回默认值
				if (if_app == true)
				{
					//返回默认值
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

				//去掉'[',']'
				memmove(one_line_, one_line_ + 1, strlen(one_line_) - 1);
				one_line_[strlen(one_line_) - 2] = '\0';
				//
				ZCE_LIB::strtrim(one_line_);

				//
				if (ZCE_LIB::strcasecmp(one_line_, sec_name) == 0)
				{
					if_app = true;
					continue;
				}
			}

			if (if_app == true)
			{
				char* str = strstr(one_line_, "=");

				if (str != NULL)
				{
					char* snext = str + 1;
					*str = '\0';
					strncpy(str_key_, one_line_, LINE_BUFFER_LEN);
					strncpy(str_value_, snext, LINE_BUFFER_LEN);
					////
					ZCE_LIB::strtrim(str_key_);
					ZCE_LIB::strtrim(str_value_);

					//找到返回。
					if (ZCE_LIB::strcasecmp(str_key_, key_name) == 0)
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

//得到所有的section,用'\0'分隔，以'\0\0'结束
size_t ZCE_INI_Pt::get_private_allsection(char *return_str,
                                          const size_t size_ret_str)
{
    //调用static get_privateprofile_string函数完成,注意参数
	return get_private_str(NULL,
						   NULL,
						   NULL,
						   return_str,
						   size_ret_str);
}

//得到某个Section下所有的Key = string值 用'\0'分隔，以'\0\0'结束
size_t ZCE_INI_Pt::getprivate_allkey(const char *sec_name,
                                     char *return_str, const size_t size_ret_str)
{
    //调用static get_privateprofile_string函数完成,注意参数
    return get_private_str(sec_name,
						   NULL, 
						   NULL, 
						   return_str, 
						   size_ret_str);
}

//得到Int的 Key值
int ZCE_INI_Pt::get_private_int(const char *sec_name,
                                const char *key_name,
                                const int default_int)
{
    //调用static get_privateprofile_string函数完成,注意参数
    const size_t TMP_BUFFER_LEN = 64;
    char default_str[TMP_BUFFER_LEN], return_str[TMP_BUFFER_LEN];
    snprintf(default_str, TMP_BUFFER_LEN-1, "%d", default_int);
	get_private_str(sec_name,
					key_name,
					default_str,
					return_str,
					TMP_BUFFER_LEN-1);
    return atoi(return_str);
}

//得到bool的 Key值
bool ZCE_INI_Pt::get_private_bool(const char *sec_name,
                                  const char *key_name,
                                  bool bDefault)
{
    //调用static get_privateprofile_string函数完成,注意参数
    const size_t TMP_BUFFER_LEN = 64;
    char default_str[TMP_BUFFER_LEN], return_str[TMP_BUFFER_LEN];

    if (bDefault)
    {
        snprintf(default_str, TMP_BUFFER_LEN - 1, "%s", "TRUE");
    }
    else
    {
        snprintf(default_str, TMP_BUFFER_LEN-1, "%s", "FALSE");
    }

	get_private_str(sec_name,
					key_name,
					default_str, 
					return_str,
					TMP_BUFFER_LEN-1);

    if (ZCE_LIB::strcasecmp(return_str, "TRUE"))
    {
        return false;
    }
    else
    {
        return true;
    }
}


//修改Section
bool ZCE_INI_Pt::write_private_section(const char *sec_name,
                                       const char *write_string)
{
	if (sec_name == NULL || write_string == NULL )
	{
		return false;
	}

	FILE* pfile = fopen(file_name_.c_str(), "r+t");

	//文件打不开，
	if (pfile == NULL)
	{
		return false;
	}

	//8*1024,1行的最大值
	char *read_ret = NULL;


	//新文件保存数据区
	std::vector<std::string> strarytmp;
	write_line_[0] = '\0';

	//表示是否找到App,key
	bool bApp = false;

	while (!feof(pfile))
	{
		//填写到临时数据区
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
		//整理
		ZCE_LIB::strtrim(one_line_);

		//注释行,空行
		if (one_line_[0] == ';' || one_line_[0] == '#' || one_line_[0] == '\0')
		{
			continue;
		}

		//[Section]
		if (one_line_[0] == '[' && one_line_[strlen(one_line_) - 1] == ']')
		{
			//去掉'[',']'
			memmove(one_line_, one_line_ + 1, strlen(one_line_) - 1);
			one_line_[strlen(one_line_) - 2] = '\0';
			//规整
			ZCE_LIB::strtrim(one_line_);

			//比较Section部分
			if (ZCE_LIB::strcasecmp(one_line_, sec_name) == 0)
			{
				bApp = true;
				//要修改这个App
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

	//如果找到App ,并且找到Key或者key == NUL ,表示工作已经完成,
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
		//将信息写会文件
		pfile = fopen(file_name_.c_str(), "w+t");

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

//写入Int
bool ZCE_INI_Pt::write_private_int(const char *sec_name,
                                   const char *key_name,
                                   const int nvalue)
{
    //调用static write_privateprofile_string函数完成,注意参数
    const size_t TMP_BUFFER_LEN = 64;
    char write_string[TMP_BUFFER_LEN + 1];
    snprintf(write_string, TMP_BUFFER_LEN, "%d", nvalue);
    return write_private_string(sec_name,
                                key_name,
                                write_string);
}

//写入Bool变量
bool ZCE_INI_Pt::write_private_bool(const char *sec_name,
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

    return write_private_string(sec_name, key_name, write_string);
}

//删除Key
bool ZCE_INI_Pt::del_private_key(const char *sec_name, const char *key_name)
{

    return write_private_string(sec_name, key_name, NULL);
}

//删除Section
bool ZCE_INI_Pt::del_private_section(const char *sec_name)
{

    return write_private_string(sec_name,
                                NULL,
                                NULL);
}


//写INI文件的谋个字段，
bool ZCE_INI_Pt::write_private_string(const char* sec_name,
                                      const char* key_name,
                                      const char* write_string)
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

    //文件打不开，
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
        write_line_[0] = '\0';
        //表示是否找到App,key
        bool bApp = false, bkey = false;

        while (!feof(pfile))
        {
            //填写到临时文件
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
            //整理
            ZCE_LIB::strtrim(one_line_);

            //注释行,空行
            if (one_line_[0] == ';' || one_line_[0] == '#' || one_line_[0] == '\0')
            {
                continue;
            }

            //[Section]
            if (one_line_[0] == '[' && one_line_[strlen(one_line_) - 1] == ']')
            {
                //已经找到下一个Section,没有发现相关的Key，返回默认值
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
                //规整
                ZCE_LIB::strtrimleft(one_line_);
                //去掉'[',']'
                memmove(one_line_, one_line_ + 1, strlen(one_line_));
                ZCE_LIB::strtrimright(one_line_);

                //比较Section部分
                if (ZCE_LIB::strcasecmp(one_line_, sec_name) == 0)
                {
                    bApp = true;

                    //要删除这个App
                    if (key_name == NULL)
                    {
                        write_line_[0] = '\0';
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
                    write_line_[0] = '\0';
                    continue;
                }

                //找到第一个'='
                char *str = strstr(one_line_, "=");

                //如果找到
                if (str != NULL)
                {
                    char *snext = str + 1;
                    *str = '\0';
                    strncpy(str_key_, one_line_, LINE_BUFFER_LEN);
                    strncpy(str_value_, snext, LINE_BUFFER_LEN);
                    //规整
                    ZCE_LIB::strtrim(str_key_);
                    ZCE_LIB::strtrim(str_value_);

                    //找到对应的Key
                    if (ZCE_LIB::strcasecmp(str_key_, key_name) == 0)
                    {
                        //表示找到键值
                        bkey = true;

                        //如果Keystring==NULL则删除Key，如果Keystring!=NULL,则写入。
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

        //如果找到App ,并且找到Key或者key == NUL ,表示工作已经完成,
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
            //将信息写会文件
            pfile = fopen(file_name_.c_str(), "w+t");

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
