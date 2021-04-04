/*!
* @copyright  2004-2014  Apache License, Version 2.0 FULLSAIL
* @filename   soar_frame_malloc.cpp
* @author     Sailzeng <sailerzeng@gmail.com>,Scottxu,
* @version
* @date       2008��2��20��
* @brief
*
*
* @details
*
*
*
* @note
*
*/

#ifndef SOARING_LIB_APPFRAME_MALLOCOR_H_
#define SOARING_LIB_APPFRAME_MALLOCOR_H_

#include "soar_zerg_frame.h"

template <typename ZCE_LOCK >
class AppFrame_Mallocor_Mgr
{


public:

    //���캯��
    AppFrame_Mallocor_Mgr();
    //��������
    ~AppFrame_Mallocor_Mgr();

public:

    /*!
    * @brief      ��ʼ��
    * @param      init_num   ÿ��POOL�ڲ���FRAME����
    * @param      max_frame_len ����FRAME�ĳ��ȣ����������������
    * @note
    */
    void initialize(size_t init_num = NUM_OF_ONCE_INIT_FRAME,
                    size_t max_frame_len = Zerg_App_Frame::MAX_LEN_OF_APPFRAME);


    /*!
    * @brief      ����Ҫ��ĵ�FRAME�ߴ��С������һ��FRAME
    * @return     Zerg_App_Frame*
    * @param      frame_len
    */
    Zerg_App_Frame *alloc_appframe(size_t frame_len);

    /*!
    * @brief      �ͷ�һ��APPFRAME������
    * @param      proc_frame �����frame
    */
    void free_appframe(Zerg_App_Frame *proc_frame);

    /*!
    * @brief      ����һ��APPFRAME
    * @param      model_freame  ģ��FRAME
    * @param      cloned_frame  ����¡��FRAME
    * @note
    */
    void clone_appframe(const Zerg_App_Frame *model_freame, Zerg_App_Frame *&cloned_frame);

    /*!
    * @brief      ���������Է����FRAME�ĳ���
    * @return     size_t
    */
    size_t get_max_framelen();

    //�������ӵ�����,���ʵ��ü�����
    void adjust_pool_capacity();

protected:


    /*!
    * @brief      ��չPOOL��ĳ��LIST������
    * @param      list_no  LiST���±�
    * @param      extend_num ��չ������
    */
    void extend_list_capacity(size_t list_no, size_t extend_num);


    /*!
    * @brief
    * @return     size_t   ������ص�LiST���±���
    * @param      sz_frame ��Ҫ�����frame�ĳ���
    */
    inline size_t get_roundup(size_t sz_frame);

public:

    //�õ�SINGLETON��ʵ��
    static AppFrame_Mallocor_Mgr *instance();
    //����SINGLETON��ʵ��
    static void clean_instance();


protected:
    //Ͱ���еĸ���
    static const size_t  NUM_OF_FRAMELIST = 10;

    //ÿ��Ͱ��ʼ����FRAME�ĸ���
    static const size_t NUM_OF_ONCE_INIT_FRAME = 512;

    //������е�NUM_OF_ALLOW_LIST_IDLE_FRAME
    static const size_t NUM_OF_ALLOW_LIST_IDLE_FRAME = 1024;

    //
    typedef zce::lordrings <Zerg_App_Frame *>     LIST_OF_APPFRAME;
    //
    typedef std::vector< LIST_OF_APPFRAME > APPFRAME_MEMORY_POOL;

protected:

    //GCC�İ汾���С��4���᲻֧��ģ���е�static�����Ա�ĳ�����const static��Ա���塣
    //�����Ͽ�����#if (__GNUC__ < 4)���Σ�����ʵ��̫æ������������鳤��NUM_OF_FRAMELIST
    //����������Ҫ���塣
    size_t                  size_appframe_[NUM_OF_FRAMELIST];

    //FRAME���ڴ����
    APPFRAME_MEMORY_POOL    frame_pool_;
    //���ӵ���
    ZCE_LOCK                zce_lock_;

protected:

    //����ʵ��
    static AppFrame_Mallocor_Mgr    *instance_;

};

//��ʼ��
template <typename ZCE_LOCK >
void AppFrame_Mallocor_Mgr<ZCE_LOCK>::initialize(size_t init_num,
                                                 size_t max_frame_len)
{
    ZCE_ASSERT(max_frame_len > 2048 && init_num > 8);

    ZCE_LOG(RS_INFO, "[framework] AppFrame_Mallocor_Mgr::AppFrame_Mallocor_Mgr init num=%u,max_frame_len=%u.",
            init_num,
            max_frame_len);

    //�õ�����
    size_t sz_frame = max_frame_len;

    for (size_t i = 0; i < NUM_OF_FRAMELIST; ++i)
    {
        size_appframe_[NUM_OF_FRAMELIST - i - 1] = sz_frame;
        sz_frame = sz_frame / 2;
    }

    frame_pool_.resize(NUM_OF_FRAMELIST);

    for (size_t i = 0; i < NUM_OF_FRAMELIST; ++i)
    {
        extend_list_capacity(i, init_num);
    }

}

//�����Է����FRAME�ĳ���
template <typename ZCE_LOCK >
size_t AppFrame_Mallocor_Mgr<ZCE_LOCK>::get_max_framelen()
{
    return size_appframe_[NUM_OF_FRAMELIST - 1];
}


//����Ҫ��ĵ�FRAME�ߴ��С������һ��FRAME
template <typename ZCE_LOCK >
inline size_t AppFrame_Mallocor_Mgr<ZCE_LOCK>::get_roundup(size_t sz_frame)
{
    //Ҳ��ѭ���һ���
    for (size_t i = 0; i < NUM_OF_FRAMELIST; ++i)
    {
        if (sz_frame <= size_appframe_[i])
        {
            return i;
        }
    }

    //���˺�
    ZCE_ASSERT(false);
    return static_cast<size_t>(-1);
}

//���캯��
template <typename ZCE_LOCK >
AppFrame_Mallocor_Mgr<ZCE_LOCK>::AppFrame_Mallocor_Mgr()
{
    memset(size_appframe_, 0, sizeof(size_appframe_));
}

//��������
template <typename ZCE_LOCK >
AppFrame_Mallocor_Mgr<ZCE_LOCK>::~AppFrame_Mallocor_Mgr()
{
    //
    ZCE_LOG(RS_INFO, "[framework] AppFrame_Mallocor_Mgr::~AppFrame_Mallocor_Mgr.");

    //���Ӧ��size == capacity , freesize==0
    for (size_t i = 0; i < NUM_OF_FRAMELIST; ++i)
    {
        //����ڴ�ȫ���黹
        if (frame_pool_[i].freesize() == 0)
        {
            //
            ZCE_LOG(RS_INFO, "[framework] List %u(frame size:%u):,free node:%u,capacity node:%u,list node:%u.Ok.",
                    i,
                    size_appframe_[i],
                    frame_pool_[i].freesize(),
                    frame_pool_[i].capacity(),
                    frame_pool_[i].size());
        }
        //��������ڴ�
        else
        {
            //
            ZCE_LOG(RS_ERROR, "[framework] List %u(frame size:%u):,free node:%u,capacity node:%u,list node:%u.Have memory leak.Please check your code.",
                    i,
                    size_appframe_[i],
                    frame_pool_[i].freesize(),
                    frame_pool_[i].capacity(),
                    frame_pool_[i].size());
        }

        //�ͷŵ�����Ŀռ�
        size_t frame_pool_len = frame_pool_[i].size();

        for (size_t j = 0; j < frame_pool_len; ++j)
        {
            Zerg_App_Frame *proc_frame = NULL;
            frame_pool_[i].pop_front(proc_frame);
            Zerg_App_Frame::delete_frame(proc_frame);
        }
    }
}


//������Ҫ���ȣ��ӳ��ӷ���һ��APPFRAME
template <typename ZCE_LOCK >
Zerg_App_Frame *AppFrame_Mallocor_Mgr<ZCE_LOCK>::alloc_appframe(size_t frame_len)
{
    typename ZCE_LOCK::LOCK_GUARD tmp_guard(zce_lock_);
    size_t hk = get_roundup(frame_len);

    //
    if (frame_pool_[hk].size() <= 0 )
    {
        extend_list_capacity(hk, NUM_OF_ONCE_INIT_FRAME);
    }

    //
    Zerg_App_Frame *new_frame = NULL;
    frame_pool_[hk].pop_front(new_frame);

    new_frame->init_framehead(static_cast<unsigned int>(frame_len));

    return new_frame;
}


//��¡һ��APPFAME
//�������û�м�������Ϊ�о�����Ҫ��alloc_appframe�������������������ظ�����
template <typename ZCE_LOCK >
void AppFrame_Mallocor_Mgr<ZCE_LOCK>::clone_appframe(const Zerg_App_Frame *model_freame,
                                                     Zerg_App_Frame *&cloned_frame)
{
    //
    size_t frame_len = model_freame->frame_length_;
    cloned_frame = alloc_appframe(frame_len);
    model_freame->clone(cloned_frame);
}


//�ͷ�һ��APPFRAME������
template <typename ZCE_LOCK >
void AppFrame_Mallocor_Mgr<ZCE_LOCK>::free_appframe(Zerg_App_Frame *proc_frame)
{
    ZCE_ASSERT(proc_frame);
    typename ZCE_LOCK::LOCK_GUARD tmp_guard(zce_lock_);
    size_t hk = get_roundup(proc_frame->frame_length_);
    frame_pool_[hk].push_back(proc_frame);
}

//�������ӵ�����
template <typename ZCE_LOCK >
void AppFrame_Mallocor_Mgr<ZCE_LOCK>::adjust_pool_capacity()
{
    typename ZCE_LOCK::LOCK_GUARD tmp_guard(zce_lock_);

    for (size_t i = 0; i < NUM_OF_FRAMELIST; ++i)
    {
        //���ʣ�������
        if (frame_pool_[i].size() > 2 * NUM_OF_ALLOW_LIST_IDLE_FRAME)
        {
            size_t free_sz = frame_pool_[i].size() - 2 * NUM_OF_ALLOW_LIST_IDLE_FRAME;

            for (size_t j = 0; j < free_sz; ++j)
            {
                Zerg_App_Frame *new_frame = NULL;
                frame_pool_[i].pop_front(new_frame);
                delete new_frame;
            }
        }
    }
}

//��չLIST������
template <typename ZCE_LOCK >
void AppFrame_Mallocor_Mgr<ZCE_LOCK>::extend_list_capacity(size_t list_no, size_t extend_num)
{
    size_t old_capacity =  frame_pool_[list_no].capacity();
    frame_pool_[list_no].resize(old_capacity + extend_num);

    for (size_t j = 0; j < extend_num; ++j)
    {
        Zerg_App_Frame *proc_frame =  Zerg_App_Frame::new_frame(size_appframe_[list_no] + 1);
        frame_pool_[list_no].push_back(proc_frame);
    }
}

//�õ�SINGLETON��ʵ��
template <typename ZCE_LOCK >
AppFrame_Mallocor_Mgr<ZCE_LOCK> *AppFrame_Mallocor_Mgr<ZCE_LOCK>::instance()
{
    if (instance_ == NULL)
    {
        instance_ = new AppFrame_Mallocor_Mgr();
    }

    return instance_;
}

//����SINGLETON��ʵ��
template <typename ZCE_LOCK >
void AppFrame_Mallocor_Mgr<ZCE_LOCK>::clean_instance()
{
    if (instance_)
    {
        delete instance_;
        instance_ = NULL;
    }
}

//
typedef AppFrame_Mallocor_Mgr<ZCE_Null_Mutex> NULLMUTEX_APPFRAME_MALLOCOR ;
typedef AppFrame_Mallocor_Mgr<ZCE_Thread_Light_Mutex> THREADMUTEX_APPFRAME_MALLOCOR ;

#endif //#ifndef SOARING_LIB_APPFRAME_MALLOCOR_H_

