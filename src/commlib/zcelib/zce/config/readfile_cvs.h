#pragma once

class ReadFile_CVS
{
public:

public:

    //构造函数
    ReadFile_CVS() = default;
    ~ReadFile_CVS() = default;

    //处理批量文件,得到列表
    int readfile_cvs(const char* file_name,
                     size_t read_buf_len = 50 * 1024 * 1024,
                     unsigned char line_delim = '\n',
                     unsigned char word_delim = '|');

protected:

    ///批处理文件名称
    std::string   readfile_name_;

    //! 读取的buf的尺寸
    size_t        read_buf_len_;
    //! 行结束符
    unsigned char line_delim_ = '\n';
    //! 每段的结束符
    unsigned char word_delim_ = '|';
};
