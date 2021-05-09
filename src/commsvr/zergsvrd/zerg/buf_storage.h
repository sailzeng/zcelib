#pragma once

namespace zerg
{
//===================================================================================================
/*!
* @brief
*
* @note
*/
class Buffer
{
public:
    //
    static const unsigned int CAPACITY_OF_BUFFER = soar::Zerg_Frame::MAX_LEN_OF_APPFRAME + 16;

public:

    //当前要使用的缓冲长度，当前处理的帧的长度,没有得到长度前填写0
    size_t      size_of_use_;

    //使用的尺寸
    size_t      size_of_buffer_;

    //数据缓冲区
    char       buffer_data_[CAPACITY_OF_BUFFER];

public:
    //
    Buffer();
    //
    ~Buffer();

    //填充数据
    void fill_write_data(const size_t szdata, const char* data);
    //读取数据
    void get_read_data(size_t& szdata, char* data);

    //
    inline char* get_use_point()
    {
        return buffer_data_ + size_of_buffer_;
    }
    //
    inline size_t get_leave_size()
    {
        return size_of_use_ - size_of_buffer_;
    }

    //
    inline bool is_full()
    {
        if (size_of_buffer_ >= size_of_use_)
        {
            return true;
        }
        return false;
    }

    //
    inline bool is_empty()
    {
        if (size_of_buffer_ == 0)
        {
            return true;
        }
        return false;
    };

    //
    void clear_buffer();
};

//===================================================================================================

/*!
* @brief
*
* @note
*/
class Buffer_Storage: public zce::NON_Copyable
{
public:
    //构造函数和析构函数
    Buffer_Storage();
    ~Buffer_Storage();

    //初始化
    void init_buffer_list(size_t szlist);

    //跟进外部的Handler的数量，初始化得到Buffer List的尺寸
    void init_buflist_by_hdlnum(size_t num_of_hdl);

    //反初始化,是否所有的申请空间,结束后一定要调用
    void close();

    //从队列分配一个Buffer出来
    zerg::Buffer* allocate_buffer();

    //释放一个Buffer到队列中
    void free_byte_buffer(zerg::Buffer* ptrbuf);

    //扩展Buffer队列
    void extend_bufferlist(size_t szlist = EXTEND_NUM_OF_LIST);

    //记录监控数据
    void monitor();

public:

    //单子实例函数
    static Buffer_Storage* instance();
    //清理单子实例
    static void clean_instance();

protected:
    //单子实例指针
    static Buffer_Storage* instance_;

protected:

    //每次如果BufferStroge不够用了，扩展的个数
    static const size_t EXTEND_NUM_OF_LIST = 1024;

protected:

    ///存放BUFF 的队列类型
    typedef zce::lordrings <zerg::Buffer*> ZBUFFER_RINGS;

    ///我担心内存泄露,所以加个总数计数
    size_t                size_of_bufferalloc_;

    ///BUFF队列
    ZBUFFER_RINGS         buffer_deque_;
};
}
