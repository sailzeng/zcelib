#pragma once

namespace ogre
{
class buffer_storage
{
    //
    typedef zce::lord_rings <soar::ogre4a_frame*> SAIL_FRAME_BUFFER_POOL;

protected:

    //只声明不实现,避免错误
    buffer_storage(const buffer_storage&);
    //只声明不实现,避免错误
    buffer_storage& operator =(const buffer_storage&);

public:
    //构造函数和析构函数
    buffer_storage();
    ~buffer_storage();

    //初始化
    void init_buffer_list(size_t szlist);
    //反初始化,是否所有的申请空间,结束后一定要调用
    void uninit_buffer_list();

    //分配一个Buffer到List中
    soar::ogre4a_frame* allocate_byte_buffer();
    //释放一个Buffer到List中
    void free_byte_buffer(soar::ogre4a_frame* ptrbuf);

    //扩展BufferList
    void extend_buffer_list(size_t szlist = EXTEND_NUM_OF_LIST);

public:

    //为了SingleTon类准备
    //实例的赋值
    static void instance(buffer_storage*);
    //实例的获得
    static buffer_storage* instance();
    //清除实例
    static void clear_inst();

protected:

    //每次如果BufferStroge不够用了，扩展的个数
    static const size_t EXTEND_NUM_OF_LIST = 256;

protected:

    //我担心内存泄露,所以加个总数计数
    size_t                    size_buffer_alloc_;

    //BUFFER的池子
    SAIL_FRAME_BUFFER_POOL    frame_buffer_ary_;

protected:

    //instance函数使用的东西
    static buffer_storage* instance_;
};
}//namespace ogre
