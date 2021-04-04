
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
    close();
}

//�����ⲿ��Handler����������ʼ���õ�Buffer List�ĳߴ�
void ZBuffer_Storage::init_buflist_by_hdlnum(size_t num_of_hdl)
{
    size_t szlist = (num_of_hdl / 100);


    //��С���߽�
    const size_t MIN_HDL_LIST_NUMBER = 128;
    const size_t MAX_HDL_LIST_NUMBER = 12800;

    //���С����Сֵ������Ϊ��Сֵ
    if (szlist < MIN_HDL_LIST_NUMBER)
    {
        szlist = MIN_HDL_LIST_NUMBER;
    }

    //���������Сֵ������Ϊ���ֵ
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
void ZBuffer_Storage::close()
{
    //
    //��ʹ�ú���������ע��Ч��
    size_t sz_of_buffer = buffer_deque_.size();

    for (size_t i = 0; i < sz_of_buffer; ++i)
    {
        delete buffer_deque_[i];
        buffer_deque_[i] = NULL;
    }

    buffer_deque_.clear();
    size_of_bufferalloc_ = 0;
}

//����һ��Buffer����
Zerg_Buffer *ZBuffer_Storage::allocate_buffer()
{
    //������ʹ������,��չ
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

//��չBuffer����
void ZBuffer_Storage::extend_bufferlist(size_t szlist)
{
    //��ӡռ�õ��ڴ�����
    ZCE_LOG(RS_INFO, "[zergsvr] extend_bufferlist size:[%u] total:[%d] need memory [%u] ,total use memory [%u].",
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

    Soar_Stat_Monitor::instance()->set_by_statid(ZERG_BUFFER_STORAGE_NUMBER,
                                                 0,
                                                 0,
                                                 size_of_bufferalloc_);
}

//�õ�Ψһ�ĵ���ʵ��
ZBuffer_Storage *ZBuffer_Storage::instance()
{
    if (instance_ == NULL)
    {
        instance_ = new ZBuffer_Storage();
    }

    return instance_;
}

//�������ʵ��
void ZBuffer_Storage::clean_instance()
{
    if (instance_)
    {
        delete instance_;
    }

    instance_ = NULL;
    return;
}

