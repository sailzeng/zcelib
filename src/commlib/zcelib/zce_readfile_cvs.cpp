
#include "zce_predefine.h"
#include "zce_log_logging.h"
#include "zce_readfile_cvs.h"

ReadFile_CVS::ReadFile_CVS(const char *file_name,
                           size_t read_buf_len,
                           unsigned char line_delim,
                           unsigned char word_delim):
    readfile_name_(file_name),
    read_buf_len_(read_buf_len),
    line_delim_(line_delim),
    word_delim_(word_delim)
{
}


ReadFile_CVS::~ReadFile_CVS()
{
}

int ReadFile_CVS::readfile_cvs()
{
    //
    std::ifstream read_file(readfile_name_.c_str(), std::ios::in | std::ios::binary);
    std::string str_readbuf;
    std::istringstream  read_stream;


    //先读取文件，全部读取
    if (read_file)
    {

        ZCE_LOG(RS_INFO, "[framework] Batch process file [%s] open success.file .",
                readfile_name_.c_str());

        read_file.seekg(0, std::ios::end);
        std::streamsize size_of_file = read_file.tellg();
        read_file.seekg(0, std::ios::beg);

        //对不住您,俺不处理大于1G的文件，因为俺们都不富裕，
        ZCE_ASSERT (size_of_file < (0x40000000));

        //将所有的数据读入缓冲
        char *outbuf = new char[static_cast<size_t>(size_of_file + 16)];
        //读取所有的数据,文件的读取方式是二进制读取方式
        read_file.read(outbuf, size_of_file);
        outbuf[size_of_file] = '\0';

        //关闭文件
        read_file.close();

        str_readbuf.assign(outbuf, static_cast<size_t>(size_of_file));
        //这儿最好释放掉
        delete[] outbuf;

        //+1很重要,保证字符串的结尾是'\0'
        read_stream.str(str_readbuf);

        ZCE_LOG(RS_INFO, "[framework] Batch process file [%s] read success.file size [%u].",
                readfile_name_.c_str(),
                size_of_file);
    }
    else
    {
        ZCE_LOG(RS_ERROR, "[framework] Batch process file read fail[%s].", readfile_name_.c_str());
        return -1;
    }


    const size_t BUFF_LEN = 1024;
    char outbufline[BUFF_LEN + 1];
    outbufline[BUFF_LEN] = '\0';

    //如果文件状态正确,
    while (read_stream)
    {
        //
        read_stream.getline(outbufline, BUFF_LEN);
        std::string str_line(outbufline, strlen(outbufline));

        std::istringstream  line_stream;
        line_stream.str(str_line);


    }
    return 0;
}






