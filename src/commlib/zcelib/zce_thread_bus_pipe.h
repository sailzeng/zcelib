
/*!
* @copyright  2004-2019  Apache License, Version 2.0 FULLSAIL
* @filename   zce_thread_bus_pipe.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2012��9��18�����ϣ��й�������81��������գ�81��
* @brief
*
*
* @details
*
*
*
* @note
*
* ม��������ۿ�ս��ʮ�ꡡ�������䡡�Ҵ�Ӧ����һ�㡡�����
*
* ������ֿձ����Ǿ��Ͻ֡�ûʲô���ƺƴ�
* �����������󡡲������������±�̬
* ����һ�𾭹���ʱ���߽⡡ʮ��ִλ��ʮ��
* ·�Ϸ缱���һ���Ŵ������˾���
*
* Ӧ����ϧ�ġ���ʹ�����ˡ�����Ļ𻨡���Ȼ��ҫ

* ����ǹ�������˳�Ĭ������ʣ���ս���ζ��
* �����ٵ��ŭ����Щ�����ż��Ҳ����
* ���������������ͻ����ס�δ���õļ�����
* ���ű���������ĸ�����������ȥ����

* �����ߵ��硡���ഺ���ϡ���ɫ�Ĺ켣��ĥ��Ѫ·

* ม��������ۿ�ս��ʮ�ꡡ�������䡡��Զ������һ��
* ม�������ȥ��ս��ʮ�ꡡȥ����Զ����Ҳ��������㡡�����
*/

#ifndef ZCE_LIB_THREAD_BUS_PIPE_H_
#define ZCE_LIB_THREAD_BUS_PIPE_H_



#include "zce_boost_non_copyable.h"
#include "zce_lockfree_kfifo.h"
#include "zce_os_adapt_file.h"
#include "zce_os_adapt_error.h"
#include "zce_share_mem_mmap.h"
#include "zce_log_logging.h"
#include "zce_lock_null_lock.h"
#include "zce_lock_thread_mutex.h"

namespace zce::lockfree
{
class dequechunk_node;
class shm_dequechunk;
};

//�߳�ʹ�õ�˫��BUS�ܵ���
//��Ҫֱ��ʹ������࣬ʹ�����������typedef
template <typename ZCE_LOCK>
class ZCE_Thread_Bus_Pipe : public ZCE_NON_Copyable
{

protected:

    //�ڲ���ö�٣�PIPE�ı�ţ��ⲿ�����˽�
    enum ZCE_BUS_PIPE_ID
    {
        THR_RECV_PIPE_ID     = 0,
        THR_SEND_PIPE_ID     = 1,

        //���ȱ�ʾ,��Ҫ��������������,����������
        THR_NUM_OF_PIPE      = 2,
    };

protected:

    //�ܵ��õ��ڴ�ռ��ַ
    char                      *pipe_buffer_;

    //�ܵ����ó���,2���ܵ������ó���,
    size_t                     size_pipe_[THR_NUM_OF_PIPE];
    //���͹ܵ���ʵ�ʿռ䳤��
    size_t                     size_room_[THR_NUM_OF_PIPE];

    //N���ܵ�,������չܵ�,���͹ܵ�����,���MAX_NUMBER_OF_PIPE��
    zce::lockfree::shm_dequechunk  *bus_pipe_[THR_NUM_OF_PIPE];

    //��
    ZCE_LOCK                   bus_lock_[THR_NUM_OF_PIPE];

protected:
    //instance����ʹ�õĶ���
    static ZCE_Thread_Bus_Pipe *instance_;

public:
    //���캯��,�������ж��ʵ���Ŀ��ܣ���������
    ZCE_Thread_Bus_Pipe():
        pipe_buffer_(NULL)
    {
        for (size_t i = 0; i < THR_NUM_OF_PIPE; ++i )
        {
            size_pipe_[i] = 0;
            size_room_[i] = 0;
            bus_pipe_[i] = NULL;
        }

    }
    //��������
    ~ZCE_Thread_Bus_Pipe()
    {

    }

public:

    //-----------------------------------------------------------------
    //��ʼ�����ֲ���,�����ͺͽ��յĳ��Ȳ�һ��
    int initialize(size_t size_recv_pipe,
                   size_t size_send_pipe,
                   size_t max_frame_len)
    {
        ZCE_ASSERT(NULL == pipe_buffer_);


        size_pipe_[THR_RECV_PIPE_ID] = size_recv_pipe;
        size_pipe_[THR_SEND_PIPE_ID] = size_send_pipe;

        size_room_[THR_RECV_PIPE_ID] = zce::lockfree::shm_dequechunk::getallocsize(size_pipe_[THR_RECV_PIPE_ID]);
        size_room_[THR_SEND_PIPE_ID] = zce::lockfree::shm_dequechunk::getallocsize(size_pipe_[THR_SEND_PIPE_ID]);

        //
        const size_t FIXED_INTERVALS = 16;
        size_t sz_malloc = size_room_[THR_RECV_PIPE_ID] + size_room_[THR_SEND_PIPE_ID] + FIXED_INTERVALS * 2;

        pipe_buffer_ = new char [sz_malloc ];

        //��ʼ���ڴ�
        bus_pipe_[THR_RECV_PIPE_ID] = zce::lockfree::shm_dequechunk::initialize(size_pipe_[THR_RECV_PIPE_ID],
                                                                      max_frame_len,
                                                                      pipe_buffer_,
                                                                      false);

        bus_pipe_[THR_SEND_PIPE_ID] = zce::lockfree::shm_dequechunk::initialize(size_pipe_[THR_SEND_PIPE_ID],
                                                                      max_frame_len,
                                                                      pipe_buffer_ + size_room_[THR_RECV_PIPE_ID] + FIXED_INTERVALS,
                                                                      false);

        //�ܵ������Լ�Ҳ�����Ƿ��ָܻ�
        if ( NULL == bus_pipe_[THR_RECV_PIPE_ID]  || NULL == bus_pipe_[THR_SEND_PIPE_ID])
        {
            ZCE_LOG(RS_ERROR, "[zcelib] ZCE_Thread_Bus_Pipe::initialize pipe fail recv[%p]size[%u],send[%p],size[%u].",
                    bus_pipe_[THR_RECV_PIPE_ID],
                    size_pipe_[THR_RECV_PIPE_ID],
                    bus_pipe_[THR_SEND_PIPE_ID],
                    size_pipe_[THR_SEND_PIPE_ID]
                   );
            return -1;
        }

        return 0;
    }



    //-----------------------------------------------------------------
    //ע��

    //��RECV�ܵ���ȡ���ݣ�
    inline bool pop_front_recvpipe(zce::lockfree::dequechunk_node *&node)
    {
        ZCE_Lock_Guard<ZCE_LOCK> lock_guard(bus_lock_[THR_RECV_PIPE_ID]);
        return bus_pipe_[THR_RECV_PIPE_ID]->pop_front(node);
    }

    //��RECV�ܵ�д������
    inline bool push_back_recvpipe(const zce::lockfree::dequechunk_node *node)
    {
        ZCE_Lock_Guard<ZCE_LOCK> lock_guard(bus_lock_[THR_RECV_PIPE_ID]);
        return bus_pipe_[THR_RECV_PIPE_ID]->push_end(node);
    }


    //��SEND�ܵ���ȡ���ݣ�
    inline bool pop_front_sendpipe(zce::lockfree::dequechunk_node *&node)
    {
        ZCE_Lock_Guard<ZCE_LOCK> lock_guard(bus_lock_[THR_SEND_PIPE_ID]);
        return bus_pipe_[THR_SEND_PIPE_ID]->pop_front(node);
    }

    //��SEND�ܵ�д������
    inline bool push_back_sendpipe(const zce::lockfree::dequechunk_node *node)
    {
        ZCE_Lock_Guard<ZCE_LOCK> lock_guard(bus_lock_[THR_SEND_PIPE_ID]);
        return bus_pipe_[THR_SEND_PIPE_ID]->push_end(node);
    }


    //ȡRecv�ܵ�ͷ��֡��
    inline int get_frontsize_recvpipe(size_t &note_size)
    {
        ZCE_Lock_Guard<ZCE_LOCK> lock_guard(bus_lock_[THR_RECV_PIPE_ID]);

        if (bus_pipe_[THR_RECV_PIPE_ID] ->empty())
        {
            return -1;
        }

        note_size = bus_pipe_[THR_RECV_PIPE_ID]->get_front_len();
        return 0;
    }

    //ȡSend�ܵ�ͷ��֡��
    inline int get_frontsize_sendpipe(size_t &note_size)
    {
        ZCE_Lock_Guard<ZCE_LOCK> lock_guard(bus_lock_[THR_SEND_PIPE_ID]);

        if (bus_pipe_[THR_SEND_PIPE_ID] ->empty())
        {
            return -1;
        }

        note_size =  bus_pipe_[THR_SEND_PIPE_ID]->get_front_len();
        return 0;
    }



public:

    //Ϊ��SingleTon��׼��
    //�õ�Ψһ�ĵ���ʵ��
    ZCE_Thread_Bus_Pipe *instance()
    {
        if (instance_ == NULL)
        {
            instance_ = new ZCE_Thread_Bus_Pipe();
        }

        return instance_;
    }

    //��ֵΨһ�ĵ���ʵ��
    void instance(ZCE_Thread_Bus_Pipe *pinstatnce)
    {
        clean_instance();
        instance_ = pinstatnce;
        return;
    }

    //�������ʵ��
    void clean_instance()
    {
        if (instance_)
        {
            delete instance_;
        }

        instance_ = NULL;
        return;
    }

};




//--------------------------------------------------------------------------------------
//����һ���̶߳���һ���߳�д��BUS����Ȼ��ST������ʵ�����������̼߳���Ч��
//����һЩ�����ٶ��м���׷��ĵط�
typedef ZCE_Thread_Bus_Pipe<ZCE_Null_Mutex>   ZCE_Thread_ST_Bus_Pipe;

//������BUS,�����ж��˾���
typedef ZCE_Thread_Bus_Pipe<ZCE_Thread_Light_Mutex> ZCE_Thread_MT_Bus_Pipe;






#endif //ZCE_LIB_THREAD_BUS_PIPE_H_


