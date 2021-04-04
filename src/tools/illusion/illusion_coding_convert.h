
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
    * @brief      ���ݵ�ǰĬ�ϵ��ַ����뷽ʽ��ת��ΪUTF8
    * @return     int == 0��ʾת���ɹ�
    * @param      src Դ�ַ�����CString�ṹ������MFC���ַ������뼯����
    * @param      dst ת��ĵ��ַ������������Ĭ��ת��ΪUTF8���ַ�����
    * @note
    */
    int default_to_utf8(const CString &src, std::string &dst);

    ///Ĭ�ϱ���ת��ΪUTF16
    int default_to_utf16(const CString &src, std::string &dst);
    //Ĭ�ϱ���ת��ΪMBCS
    int default_to_mbcs(const CString &src, std::string &dst);

    ///MFCĬ�ϵ��ַ���ת��Ϊbytes��Ҫ���ַ���
    int default_to_bytescoding(const CString &src, std::string &dst);
    ///
    int bytescoding_to_mbcs(const std::string &src, std::string &dst);

    ///
    int utf8_to_mbcs(const std::string &src, std::string &dst);
    ///
    int utf16_to_mbcs(const std::string &src, std::string &dst);

    ///����bytes ת���ĵ�CODING������bytes���ò�ͬ���ַ���
    void set_bytes_coding(CVT_CODING cvt_coding);

public:

    static Coding_Convert *instance();

    static void clean_instance();


protected:
    //
    static const size_t CONVERT_BUFFER_LEN = 64 * 1024 - 1;

protected:

    //����ʵ��
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



