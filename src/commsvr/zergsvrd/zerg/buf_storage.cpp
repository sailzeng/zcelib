#include "zerg/predefine.h"
#include "zerg/buf_storage.h"
#include "zerg/stat_define.h"

namespace zerg
{
/****************************************************************************************************
class ZByteBuffer
****************************************************************************************************/
Buffer::Buffer() :
    size_of_buffer_(0),
    size_of_use_(0)
{
}

//
Buffer::~Buffer()
{
}

//
void Buffer::fill_write_data(const size_t szdata, const char* data)
{
    memcpy(buffer_data_, data, szdata);
    size_of_use_ += szdata;
    //
}
//
void Buffer::get_read_data(size_t& szdata, char* data)
{
    memcpy(data, buffer_data_, szdata);
}

void Buffer::clear()
{
    size_of_use_ = 0;
    size_of_buffer_ = 0;
}

/****************************************************************************************************
class Buffer_Storage
****************************************************************************************************/

Buffer_Storage* Buffer_Storage::instance_ = NULL;

Buffer_Storage::Buffer_Storage() :
    size_of_bufferalloc_(0)
{
    //extend_bufferlist(szlist);
}
//
Buffer_Storage::~Buffer_Storage()
{
    close();
}

//跟进外部的Handler的数量，初始化得到Buffer List的尺寸
void Buffer_Storage::init_buflist_by_hdlnum(size_t num_of_hdl)
{
    size_t szlist = (num_of_hdl / 100);

    //最小最大边界
    const size_t MIN_HDL_LIST_NUMBER = 128;
    const size_t MAX_HDL_LIST_NUMBER = 12800;

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
void Buffer_Storage::init_buffer_list(size_t szlist)
{
    extend_bufferlist(szlist);
}

//
void Buffer_Storage::close()
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

//分配一个Buffer出来
Buffer* Buffer_Storage::allocate_buffer()
{
    //缓冲区使用完了,扩展
    if (true == buffer_deque_.empty())
    {
        extend_bufferlist();
    }

    Buffer* tmppr = buffer_deque_[0];
    buffer_deque_[0] = NULL;
    buffer_deque_.pop_front();
    return tmppr;
}

//
void Buffer_Storage::free_byte_buffer(Buffer* ptrbuf)
{
    ZCE_ASSERT(ptrbuf);
    ptrbuf->clear();
    buffer_deque_.push_back(ptrbuf);
}

//扩展Buffer队列
void Buffer_Storage::extend_bufferlist(size_t szlist)
{
    //打印占用的内存数量
    ZCE_LOG(RS_INFO, "[zergsvr] extend_bufferlist size:[%u] total:[%d] need memory [%u] ,total use memory [%u].",
            szlist,
            size_of_bufferalloc_,
            szlist * Buffer::CAPACITY_OF_BUFFER,
            size_of_bufferalloc_ * Buffer::CAPACITY_OF_BUFFER
    );
    buffer_deque_.resize(size_of_bufferalloc_ + szlist);

    for (size_t i = 0; i < szlist; ++i)
    {
        Buffer* tmppr = new Buffer();
        tmppr->clear();
        buffer_deque_.push_back(tmppr);
    }

    size_of_bufferalloc_ += szlist;
}

void Buffer_Storage::monitor()
{
    soar::stat_monitor::instance()->set_counter(ZERG_BUFFER_STORAGE_NUMBER,
                                                0,
                                                0,
                                                size_of_bufferalloc_);
    soar::stat_monitor::instance()->set_counter(ZERG_BUFFER_STORAGE_FREE,
                                                0,
                                                0,
                                                buffer_deque_.free());
    soar::stat_monitor::instance()->set_counter(ZERG_BUFFER_STORAGE_NUMBER,
                                                0,
                                                0,
                                                buffer_deque_.size());
}

//得到唯一的单子实例
Buffer_Storage* Buffer_Storage::instance()
{
    if (instance_ == NULL)
    {
        instance_ = new Buffer_Storage();
    }

    return instance_;
}

//清除单子实例
void Buffer_Storage::clear_inst()
{
    if (instance_)
    {
        delete instance_;
    }

    instance_ = NULL;
    return;
}
}