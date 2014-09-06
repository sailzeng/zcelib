
#include "zerg_predefine.h"
#include "zerg_buf_storage.h"
#include "zerg_stat_define.h"

/****************************************************************************************************
class ZByteBuffer
****************************************************************************************************/

ZBuffer_Storage *ZBuffer_Storage::instance_ = NULL;

Zerg_Buffer::Zerg_Buffer():
    size_of_use_(0),
    size_of_buffer_(0)
{
}

//
Zerg_Buffer::~Zerg_Buffer()
{
}

//
void Zerg_Buffer::fill_write_data(const size_t szdata, const char *data)
{
    memcpy(buffer_data_, data, szdata);
    size_of_buffer_ += szdata;
    //
}
//
void Zerg_Buffer::get_read_data(size_t &szdata, char *data)
{
    memcpy(data, buffer_data_, szdata);
}

void Zerg_Buffer::clear_buffer()
{
    size_of_buffer_ = 0;
    size_of_use_  = 0;
}

/****************************************************************************************************
class ZBuffer_Storage
****************************************************************************************************/
ZBuffer_Storage::ZBuffer_Storage():
    size_of_bufferalloc_(0)
{
    //extend_bufferlist(szlist);
}
//
ZBuffer_Storage::~ZBuffer_Storage()
{
    uninit_buffer_list();
}

//跟进外部的Handler的数量，初始化得到Buffer List的尺寸
void ZBuffer_Storage::init_buflist_by_hdlnum(size_t num_of_hdl)
{
    size_t szlist = (num_of_hdl / 100);

    //如果小于最小值，调整为最小值
    if (szlist < MIN_HDL_LIST_NUMBER)
    {
        szlist = MIN_HDL_LIST_NUMBER;
    }

    //如果大于最小值，调整为最大值
    if (szlist > MAX_HDL_LIST_NUMBER)
    {
        szlist = MAX_HDL_LIST_NUMBER;
    }

    init_buffer_list(szlist);
}

//
void ZBuffer_Storage::init_buffer_list(size_t szlist)
{
    extend_bufferlist(szlist);
}

//
void ZBuffer_Storage::uninit_buffer_list()
{
    //
    //少使用函数，懒得注意效率
    size_t sz_of_buffer = buffer_deque_.size();

    for (size_t i = 0; i < sz_of_buffer; ++i)
    {
        delete buffer_deque_[i];
        buffer_deque_[i] = NULL;
    }

    buffer_deque_.clear();
    size_of_bufferalloc_ = 0;
}

/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2005年11月27日
Function        : ZBuffer_Storage::allocate_buffer
Return          : ZByteBuffer*
Parameter List  : NULL
Description     :
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
Zerg_Buffer *ZBuffer_Storage::allocate_buffer()
{
    //缓冲区使用完了,扩展
    if ( true == buffer_deque_.empty() )
    {
        extend_bufferlist();
    }

    Zerg_Buffer *tmppr = buffer_deque_[0];
    buffer_deque_[0] = NULL;
    buffer_deque_.pop_front();
    return tmppr;
}

//
void ZBuffer_Storage::free_byte_buffer(Zerg_Buffer *ptrbuf)
{
    ZCE_ASSERT(ptrbuf);
    ptrbuf->clear_buffer();
    buffer_deque_.push_back(ptrbuf);
}

//
/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2005年11月27日
Function        : ZBuffer_Storage::extend_bufferlist
Return          : void
Parameter List  :
  Param1: size_t szlist
Description     :
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
void ZBuffer_Storage::extend_bufferlist(size_t szlist)
{
    //打印占用的内存数量
    ZLOG_INFO("[zergsvr] extend_bufferlist size:[%u] total:[%d] need memory [%u] ,total use memory [%u].",
              szlist,
              size_of_bufferalloc_,
              szlist * Zerg_Buffer::CAPACITY_OF_BUFFER,
              size_of_bufferalloc_ * Zerg_Buffer::CAPACITY_OF_BUFFER
             );
    buffer_deque_.resize(size_of_bufferalloc_ + szlist);

    for (size_t i = 0; i < szlist; ++i)
    {
        Zerg_Buffer *tmppr = new Zerg_Buffer();
        tmppr->clear_buffer();
        buffer_deque_.push_back(tmppr);
    }

    size_of_bufferalloc_ += szlist;

    Comm_Stat_Monitor::instance()->set_by_statid(ZERG_BUFFER_STORAGE_NUMBER,
                                                 0,
                                                 0,
                                                 size_of_bufferalloc_);
}

//得到唯一的单子实例
ZBuffer_Storage *ZBuffer_Storage::instance()
{
    if (instance_ == NULL)
    {
        instance_ = new ZBuffer_Storage();
    }

    return instance_;
}

//清除单子实例
void ZBuffer_Storage::clean_instance()
{
    if (instance_)
    {
        delete instance_;
    }

    instance_ = NULL;
    return;
}

