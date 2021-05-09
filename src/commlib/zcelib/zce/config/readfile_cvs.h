#ifndef ZCE_LIB_READFILE_CVS_H_
#define ZCE_LIB_READFILE_CVS_H_

class ReadFile_CVS
{
public:

public:

    //构造函数
    ReadFile_CVS(const char* file_name,
                 size_t read_buf_len = 50 * 1024 * 1024,
                 unsigned char line_delim = '\n',
                 unsigned char word_delim = '|');
    ~ReadFile_CVS();

    //处理批量文件,得到列表
    int readfile_cvs();

protected:

    ///批处理文件名称
    std::string   readfile_name_;

    ///
    size_t        read_buf_len_;
    ///
    unsigned char line_delim_;
    ///
    unsigned char word_delim_;
};

#endif //#ifndef ZCE_LIB_READFILE_CVS_H_
