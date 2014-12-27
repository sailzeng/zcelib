
// illusion_main.h : main header file for the illusion application
//
#pragma once


class Coding_Convert
{


    enum CVT_CODING
    {
        CVT_UTF8,
        CVT_UTF16,
        CVT_MBCS,
    };

protected:
    Coding_Convert();
    ~Coding_Convert();

public:

    /*!
    * @brief      根据当前默认的字符编码方式，转换为UTF8
    * @return     int == 0表示转换成功
    * @param      src 源字符串，CString结构，根据MFC的字符集编码集决定
    * @param      dst 转后的的字符串，这个函数默认转换为UTF8的字符集合
    * @note
    */
    int default_to_utf8(const CString &src, std::string &dst);

    ///默认编码转换为UTF16
    int default_to_utf16(const CString &src, std::string &dst);
    //默认编码转换为MBCS
    int default_to_mbcs(const CString &src, std::string &dst);

    ///MFC默认的字符集转换为bytes需要的字符集
    int default_to_bytescoding(const CString &src, std::string &dst);
    ///
    int bytescoding_to_mbcs(const std::string &src, std::string &dst);

    ///
    int utf8_to_mbcs(const std::string &src, std::string &dst);
    ///
    int utf16_to_mbcs(const std::string &src, std::string &dst);

    ///设置bytes 转换的的CODING，允许bytes设置不同的字符集
    void set_bytes_coding(CVT_CODING cvt_coding);

public:

    static Coding_Convert *instance();

    static void clean_instance();


protected:
    //
    static const size_t CONVERT_BUFFER_LEN = 64 * 1024 - 1;

protected:

    //单子实例
    static Coding_Convert  *instance_;

protected:

    ///
    wchar_t *cvt_utf16_buf_ = NULL;
    ///
    char *cvt_utf8_buf_ = NULL;
    ///
    char *cvt_mbcs_buf_ = NULL;


    ///
    CVT_CODING cur_cvt_coding_ = CVT_MBCS;
};



