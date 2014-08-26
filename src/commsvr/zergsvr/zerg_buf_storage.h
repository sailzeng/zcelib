
#ifndef ZERG_BUFFER_STORAGE_H_
#define ZERG_BUFFER_STORAGE_H_

/****************************************************************************************************
class Zerg_Buffer
****************************************************************************************************/
class Zerg_Buffer
{
public:
    //
    static const unsigned int CAPACITY_OF_BUFFER = Zerg_App_Frame::MAX_LEN_OF_APPFRAME + 16;

public:

    //当前要使用的缓冲长度，当前处理的帧的长度,没有得到长度前填写0
    size_t      size_of_use_;

    //使用的尺寸
    size_t      size_of_buffer_;

    //数据缓冲区
    char       buffer_data_[CAPACITY_OF_BUFFER];

public:
    //
    Zerg_Buffer();
    //
    ~Zerg_Buffer();

    //填充数据
    void fill_write_data(const size_t szdata, const char *data);
    //读取数据
    void get_read_data(size_t &szdata, char *data);

    //
    inline char *get_use_point()
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

/****************************************************************************************************
class ZBuffer_Storage
****************************************************************************************************/
class ZBuffer_Storage
{

protected:

    //每次如果BufferStroge不够用了，扩展的个数
    static const size_t EXTEND_NUM_OF_LIST = 1024;

    //最小最大边界
    static const size_t MIN_HDL_LIST_NUMBER = 128;
    //
    static const size_t MAX_HDL_LIST_NUMBER = 12800;

protected:
    //
    typedef ZCE_LIB::lordrings <Zerg_Buffer *> ZListBufferAry;

    //我担心内存泄露,所以加个总数计数
    size_t                size_of_bufferalloc_;

    //
    ZListBufferAry        buffer_deque_;

protected:
    //单子实例指针
    static ZBuffer_Storage *instance_;

protected:

    //只声明不实现,避免错误
    ZBuffer_Storage(const ZBuffer_Storage &);
    //只声明不实现,避免错误
    ZBuffer_Storage &operator =(const ZBuffer_Storage &);

public:
    //构造函数和析构函数
    ZBuffer_Storage();
    ~ZBuffer_Storage();

    //初始化
    void init_buffer_list(size_t szlist);

    //跟进外部的Handler的数量，初始化得到Buffer List的尺寸
    void init_buflist_by_hdlnum(size_t num_of_hdl);

    //反初始化,是否所有的申请空间,结束后一定要调用
    void uninit_buffer_list();

    //分配一个Buffer到List中
    Zerg_Buffer *allocate_buffer();

    //释放一个Buffer到List中
    void free_byte_buffer(Zerg_Buffer *ptrbuf );

    //扩展BufferList
    void extend_bufferlist(size_t szlist = EXTEND_NUM_OF_LIST);

public:

    //单子实例函数
    static ZBuffer_Storage *instance();
    //清理单子实例
    static void clean_instance();

};

#endif //_ZERG_BUFFER_STORAGE_H_

