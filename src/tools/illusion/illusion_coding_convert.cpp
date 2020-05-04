锘?
#include "stdafx.h"

#include "illusion_coding_convert.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//澶勭悊镄勫崟瀛愬疄渚?Coding_Convert *Coding_Convert::instance_ = NULL;

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

///璁剧疆
void Coding_Convert::set_bytes_coding(CVT_CODING cvt_coding)
{
    cur_cvt_coding_ = cvt_coding;
}


//镙规嵁褰揿墠榛樿