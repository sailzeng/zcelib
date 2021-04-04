
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


    //�ȶ�ȡ�ļ���ȫ����ȡ
    if (read_file)
    {

        ZCE_LOG(RS_INFO, "[framework] Batch process file [%s] open success.file .",
                readfile_name_.c_str());

        read_file.seekg(0, std::ios::end);
        std::streamsize size_of_file = read_file.tellg();
        read_file.seekg(0, std::ios::beg);

        //�Բ�ס��,�����������1G���ļ�����Ϊ���Ƕ�����ԣ��
        ZCE_ASSERT (size_of_file < (0x40000000));

        //�����е����ݶ��뻺��
        char *outbuf = new char[static_cast<size_t>(size_of_file + 16)];
        //��ȡ���е�����,�ļ��Ķ�ȡ��ʽ�Ƕ����ƶ�ȡ��ʽ
        read_file.read(outbuf, size_of_file);
        outbuf[size_of_file] = '\0';

        //�ر��ļ�
        read_file.close();

        str_readbuf.assign(outbuf, static_cast<size_t>(size_of_file));
        //�������ͷŵ�
        delete[] outbuf;

        //+1����Ҫ,��֤�ַ����Ľ�β��'\0'
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

    //����ļ�״̬��ȷ,
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






