
#ifndef ZCE_LIB_BUS_MMAP_PIPE_H_
#define ZCE_LIB_BUS_MMAP_PIPE_H_

//美国著名计算机专家、C语言及Unix之父、图灵奖获得者丹尼斯·里奇（Dennis Ritchie）于2011年10日12日于家中去世，享年70岁。

#include "zce_boost_non_copyable.h"

namespace ZCE_LIB
{
class dequechunk_node;
class shm_dequechunk;
};

//MMAP的管道，你要初始化几条就初始化几条
class ZCE_Bus_MMAPPipe : public ZCE_NON_Copyable
{

public:

    //最大的PIPE的数量，暂时写完512条,感觉大部分时候比较浪费，呵呵，不过算了，也就几K空间
    static const size_t MAX_NUMBER_OF_PIPE = 512;

protected:

    //
    struct ZCE_BUS_PIPE_HEAD
    {
        ZCE_BUS_PIPE_HEAD();
        ~ZCE_BUS_PIPE_HEAD();

        //机器字长
        uint32_t            size_of_sizet_;
        //
        //管道数量
        size_t              number_of_pipe_;
        //管道配置长度,2个管道的配置长度,
        size_t              size_of_pipe_[MAX_NUMBER_OF_PIPE];
        //
        size_t              size_of_room_[MAX_NUMBER_OF_PIPE];
    };

public:

    //构造函数,
    ZCE_Bus_MMAPPipe();
    //析购函数
    ~ZCE_Bus_MMAPPipe();

public:


    /*!
    * @brief      初始化，根据参数，
    * @return     int
    * @param      bus_mmap_name
    * @param      number_of_pipe  管道的数量
    * @param      size_of_pipe    管道的大小
    * @param      max_frame_len
    * @param      if_restore      是否进行恢复
    * @note       
    */
    int initialize(const char *bus_mmap_name,
                   size_t number_of_pipe,
                   size_t size_of_pipe[],
                   size_t max_frame_len,
                   bool if_restore);

    //初始化，只根据文件进行初始化，用于某些工具对MMAP文件进行处理的时候
    int initialize(const char *bus_mmap_name,
                   size_t max_frame_len);


    //-----------------------------------------------------------------
    //怀疑TMD我有强迫症倾向，提供这么多接口干嘛，下面一组足够用了。
    bool is_exist_bus(size_t pipe_id);
    //向管道写入帧
    inline int push_back_bus(size_t pipe_id, const ZCE_LIB::dequechunk_node *node);
    //从管道POP读取帧，拷贝后删除
    inline int pop_front_bus(size_t pipe_id, ZCE_LIB::dequechunk_node *node);
    //从管道拷贝复制一个帧出来
    inline int read_front_bus(size_t pipe_id, ZCE_LIB::dequechunk_node *&node);
    //抛弃一个帧
    inline int pop_front_bus(size_t pipe_id);
    //取管道头的帧长
    inline int get_front_nodesize(size_t pipe_id, size_t &note_size);


    //管道为满
    inline bool is_full_bus(size_t pipe_id);
    //管道是否为空
    inline bool is_empty_bus(size_t pipe_id);
    //管道的空余空间,
    inline void get_bus_freesize(size_t pipe_id, size_t &pipe_size, size_t &free_size);

    //-----------------------------------------------------------------
protected:

    //初始化所有的数据管道
    int init_all_pipe(size_t max_frame_len,
                      bool if_restore);

public:

    ///为了SingleTon类准备
    ///实例的赋值
    static void instance(ZCE_Bus_MMAPPipe *);
    ///实例的获得
    static ZCE_Bus_MMAPPipe *instance();
    ///清除实例
    static void clean_instance();



protected:

    ///BUS的名字，也就是MMAP映射文件名称
    char                       bus_mmap_name_[PATH_MAX + 1];

    ///BUS文件的头部信息
    ZCE_BUS_PIPE_HEAD          bus_head_;

    ///N个管道,比如接收管道,发送管道……,最大MAX_NUMBER_OF_PIPE个
    ZCE_LIB::shm_dequechunk  *bus_pipe_pointer_[MAX_NUMBER_OF_PIPE];

    ///MMAP内存文件，
    ZCE_ShareMem_Mmap          mmap_file_;


protected:
    //instance函数使用的东西
    static ZCE_Bus_MMAPPipe *instance_;
};


//管道是否存在
inline bool ZCE_Bus_MMAPPipe::is_exist_bus(size_t pipe_id)
{
    return bus_pipe_pointer_[pipe_id] == NULL ? false : true;
}
//管道为满
inline bool ZCE_Bus_MMAPPipe::is_full_bus(size_t pipe_id)
{
    return bus_pipe_pointer_[pipe_id]->full();
}

//管道是否为空
inline bool ZCE_Bus_MMAPPipe::is_empty_bus(size_t pipe_id)
{
    return bus_pipe_pointer_[pipe_id]->empty();
}

//管道的空余空间,
inline void ZCE_Bus_MMAPPipe::get_bus_freesize(size_t pipe_id, size_t &pipe_size, size_t &free_size)
{
    pipe_size = bus_head_.size_of_pipe_[pipe_id];
    free_size = bus_pipe_pointer_[pipe_id]->freesize();
    return;
}


//向管道写入帧
inline int ZCE_Bus_MMAPPipe::push_back_bus(size_t pipe_id, const ZCE_LIB::dequechunk_node *node)
{

    //取出一个帧
    bool bret = bus_pipe_pointer_[pipe_id]->push_end(node);

    //
    if (!bret)
    {
        ZLOG_ALERT("[zcelib] %u Pipe is full or data small?,Some data can't put to pipe. Please increase and check. nodesize=%u, freesize=%u",
                   pipe_id, node->size_of_node_,
                   (unsigned int)bus_pipe_pointer_[pipe_id]->freesize());
        return -1;
    }

    return 0;
}

//取管道头的帧长
inline int ZCE_Bus_MMAPPipe::get_front_nodesize(size_t pipe_id, size_t &note_size)
{
    if (bus_pipe_pointer_[pipe_id]->empty())
    {
        return -1;
    }

    note_size = bus_pipe_pointer_[pipe_id]->get_front_len();
    return 0;
}




/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2006年4月14日
Function        : ZCE_Bus_MMAPPipe::pop_front_bus
Return          : inline int
Parameter List  :
  Param1: ZERG_PIPE_ID pipe_id 管道ID
  Param2: dequechunk_node* const pframe 准备复制node指针，指针是不变的
Description     : 从管道读取帧
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
inline int ZCE_Bus_MMAPPipe::pop_front_bus(size_t pipe_id, ZCE_LIB::dequechunk_node *node)
{
    if (bus_pipe_pointer_[pipe_id]->empty())
    {
        return -1;
    }

    //取出一个帧
    bus_pipe_pointer_[pipe_id]->pop_front(node);

    return 0;
}

//从管道拷贝复制一个帧出来
inline int ZCE_Bus_MMAPPipe::read_front_bus(size_t pipe_id, ZCE_LIB::dequechunk_node *&node)
{
    if (bus_pipe_pointer_[pipe_id]->empty())
    {
        return -1;
    }

    //取出一个帧
    bus_pipe_pointer_[pipe_id]->read_front(node);

    return 0;
}

//抛弃一个帧
inline int ZCE_Bus_MMAPPipe::pop_front_bus(size_t pipe_id)
{
    if (bus_pipe_pointer_[pipe_id]->empty())
    {
        return -1;
    }

    //取出一个帧
    bus_pipe_pointer_[pipe_id]->discard_frond();
    return 0;
}









#endif //ZCE_LIB_BUS_MMAP_PIPE_H_





