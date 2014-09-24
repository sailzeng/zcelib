

#ifndef OGRE_BUFFER_STORAGE_H_
#define OGRE_BUFFER_STORAGE_H_

/****************************************************************************************************
class Ogre_Buffer_Storage BUFFER存储的
****************************************************************************************************/
class Ogre_Buffer_Storage
{

    //
    typedef ZCE_LIB::lordrings <Ogre4a_AppFrame *> SAIL_FRAME_BUFFER_POOL;

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
    static Ogre_Buffer_Storage *instance_;

protected:

    //只声明不实现,避免错误
    Ogre_Buffer_Storage(const Ogre_Buffer_Storage &);
    //只声明不实现,避免错误
    Ogre_Buffer_Storage &operator =(const Ogre_Buffer_Storage &);

public:
    //构造函数和析构函数
    Ogre_Buffer_Storage();
    ~Ogre_Buffer_Storage();

    //初始化
    void init_buffer_list(size_t szlist );
    //反初始化,是否所有的申请空间,结束后一定要调用
    void uninit_buffer_list();

    //分配一个Buffer到List中
    Ogre4a_AppFrame *allocate_byte_buffer();
    //释放一个Buffer到List中
    void free_byte_buffer(Ogre4a_AppFrame *ptrbuf );

    //扩展BufferList
    void extend_buffer_list(size_t szlist = EXTEND_NUM_OF_LIST);

public:

    //为了SingleTon类准备
    //实例的赋值
    static void instance(Ogre_Buffer_Storage *);
    //实例的获得
    static Ogre_Buffer_Storage *instance();
    //清除实例
    static void clean_instance();

};

#endif //_OGRE_BUFFER_STORAGE_H_

