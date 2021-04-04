

#include "stdafx.h"

#include "illusion_coding_convert.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//����ĵ���ʵ��
Coding_Convert *Coding_Convert::instance_ = NULL;

//
Coding_Convert::Coding_Convert()
{
    cvt_utf16_buf_ = new wchar_t[CONVERT_BUFFER_LEN + 1];
    cvt_utf8_buf_ = new char[CONVERT_BUFFER_LEN + 1];
    cvt_mbcs_buf_ = new char[CONVERT_BUFFER_LEN + 1];

    cvt_mbcs_buf_[CONVERT_BUFFER_LEN] = '\0';
    cvt_utf8_buf_[CONVERT_BUFFER_LEN] = '\0';
    cvt_utf16_buf_[CONVERT_BUFFER_LEN] = 0;
}


Coding_Convert::~Coding_Convert()
{
    if (cvt_utf16_buf_)
    {
        delete cvt_utf16_buf_;
        cvt_utf16_buf_ = NULL;
    }
    if (cvt_utf8_buf_)
    {
        delete cvt_utf8_buf_;
        cvt_utf8_buf_ = NULL;
    }
    if (cvt_mbcs_buf_)
    {
        delete cvt_mbcs_buf_;
        cvt_mbcs_buf_ = NULL;
    }
}


Coding_Convert *Coding_Convert::instance()
{
    if (instance_ == NULL)
    {
        instance_ = new Coding_Convert();
    }
    return instance_;
}

void Coding_Convert::clean_instance()
{
    if (instance_)
    {
        delete instance_;
        instance_ = NULL;
    }
}

///����
void Coding_Convert::set_bytes_coding(CVT_CODING cvt_coding)
{
    cur_cvt_coding_ = cvt_coding;
}


//���ݵ�ǰĬ�ϵ��ַ����뷽ʽ��ת��ΪUTF8
int Coding_Convert::default_to_utf8(const CString &src, std::string &dst)
{
#if defined UNICODE || defined _UNICODE

    DWORD ret = 0;
    ret = ::WideCharToMultiByte(CP_UTF8,
                                NULL,
                                (LPCTSTR)src,
                                src.GetLength() + 1,
                                cvt_utf8_buf_,
                                CONVERT_BUFFER_LEN,
                                NULL,
                                0);
    if (ret == 0)
    {
        return -1;
    }
    dst = cvt_utf8_buf_;
    return 0;

#else
    // MBCS ===> UTF16 ===> UTF8

    // ��һ���Ȱ�MBCS��ת����UTF-16
    DWORD ret = 0;
    ret = ::MultiByteToWideChar(CP_ACP,
                                0,
                                (LPCTSTR)src,
                                src.GetLength() + 1,
                                cvt_utf16_buf_,
                                CONVERT_BUFFER_LEN);
    if (ret == 0)
    {
        return -1;
    }
    int u16_buf_len = ret;
    // �ڶ����ٰ�UTF-16����ת��ΪUTF-8����
    ret = ::WideCharToMultiByte(CP_UTF8,
                                NULL,
                                cvt_utf16_buf_,
                                u16_buf_len,
                                cvt_utf8_buf_,
                                CONVERT_BUFFER_LEN,
                                NULL,
                                0);
    if (ret == 0)
    {
        return -1;
    }
    dst = cvt_utf8_buf_;

    return 0;
#endif
}

//���ݵ�ǰĬ�ϵ��ַ����뷽ʽ��ת��ΪUTF16(UNICODE)
int Coding_Convert::default_to_utf16(const CString &src, std::string &dst)
{
#if defined UNICODE || defined _UNICODE
    // UTF16 == UTF16
    if (src.GetLength() > CONVERT_BUFFER_LEN)
    {
        return -1;
    }
    dst.assign(((const char *)((LPCTSTR)src)), ((src.GetLength() + 1) * (sizeof(wchar_t))));
    return 0;
#else
    // MBCS ===> UTF16
    DWORD ret = 0;
    ret = ::MultiByteToWideChar(CP_ACP,
                                0,
                                (LPCTSTR)src,
                                src.GetLength() + 1,
                                cvt_utf16_buf_,
                                CONVERT_BUFFER_LEN);
    if (ret == 0)
    {
        return -1;
    }
    dst.assign(((const char *)(cvt_utf16_buf_)), (ret * (sizeof(wchar_t))));
    return 0;
#endif
}

//���ݵ�ǰĬ�ϵ��ַ����뷽ʽ��ת��ΪMBCS
int Coding_Convert::default_to_mbcs(const CString &src, std::string &dst)
{
#if defined UNICODE || defined _UNICODE
    //UTF16 == > MBCS
    DWORD ret = 0;
    ret = ::WideCharToMultiByte(CP_ACP,
                                NULL,
                                (LPCTSTR)src,
                                src.GetLength() + 1,
                                cvt_mbcs_buf_,
                                CONVERT_BUFFER_LEN,
                                NULL,
                                0);
    if (ret == 0)
    {
        return -1;
    }
    dst = cvt_mbcs_buf_;
    return 0;
#else
    // MBCS ===> MBCS
    if (src.GetLength() > CONVERT_BUFFER_LEN)
    {
        return -1;
    }
    dst = ((LPCTSTR)src);
    return 0;
#endif
}


///
int Coding_Convert::utf8_to_mbcs(const std::string &src, std::string &dst)
{
    // ��һ���Ȱ�UTF8��ת����UTF-16
    DWORD ret = 0;
    ret = ::MultiByteToWideChar(CP_UTF8,
                                0,
                                src.c_str(),
                                src.length() + 1,
                                cvt_utf16_buf_,
                                CONVERT_BUFFER_LEN);
    if (ret == 0)
    {
        return -1;
    }
    int u16_buf_len = ret;
    // �ڶ����ٰ�UTF-16����ת��ΪMBCS����
    ret = ::WideCharToMultiByte(CP_ACP,
                                NULL,
                                cvt_utf16_buf_,
                                u16_buf_len,
                                cvt_mbcs_buf_,
                                CONVERT_BUFFER_LEN,
                                NULL,
                                0);
    if (ret == 0)
    {
        return -1;
    }
    dst = cvt_mbcs_buf_;

    return 0;
}


//
int Coding_Convert::utf16_to_mbcs(const std::string &src, std::string &dst)
{
    memcpy((void *)cvt_utf16_buf_, src.c_str(), src.length());
    //UTF16 == > MBCS
    DWORD ret = 0;
    ret = ::WideCharToMultiByte(CP_ACP,
                                NULL,
                                cvt_utf16_buf_,
                                src.length() / sizeof(wchar_t),
                                cvt_mbcs_buf_,
                                CONVERT_BUFFER_LEN,
                                NULL,
                                0);
    if (ret == 0)
    {
        return -1;
    }
    dst = cvt_mbcs_buf_;
    return 0;
}


//
int Coding_Convert::default_to_bytescoding(const CString &src, std::string &dst)
{
    int ret = 0;
    if (cur_cvt_coding_ == CVT_UTF8)
    {
        ret = default_to_utf8(src, dst);
    }
    else if (cur_cvt_coding_ == CVT_UTF16)
    {
        ret = default_to_utf16(src, dst);
    }
    else if (cur_cvt_coding_ == CVT_MBCS)
    {
        ret = default_to_mbcs(src, dst);
    }
    else
    {
        ZCE_ASSERT(false);
    }
    return ret;
}


///
int Coding_Convert::bytescoding_to_mbcs(const std::string &src, std::string &dst)
{
    int ret = 0;
    if (cur_cvt_coding_ == CVT_UTF8)
    {
        ret = utf8_to_mbcs(src, dst);
    }
    else if (cur_cvt_coding_ == CVT_UTF16)
    {
        ret = utf16_to_mbcs(src, dst);
    }
    else if (cur_cvt_coding_ == CVT_MBCS)
    {
        dst = src;
    }
    else
    {
        ZCE_ASSERT(false);
    }
    return 0;
}