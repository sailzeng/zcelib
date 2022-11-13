/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2021年5月
* @brief
*             queue_buffer :
*             一个只能向尾部添加数据的BUFFER，
*             数据能多次放入，但取出数据要全部取出。
*             读取数据可以直接使用point指针。
* @details
*
* @note
*
*/

#pragma once

namespace zce
{
//================================================================================
//!一个只能向尾部添加数据的BUFFER
class queue_buffer
{
public:
    //!构造，析构，赋值函数，为了加速，写了右值处理的函数
    queue_buffer() = default;
    queue_buffer(size_t buf_size);
    ~queue_buffer();
    queue_buffer(const queue_buffer& others);
    queue_buffer(queue_buffer&& others) noexcept;
    queue_buffer& operator=(const queue_buffer& others);
    queue_buffer& operator=(queue_buffer&& others) noexcept;

    //!初始化
    bool initialize(size_t capacity_size);

    static queue_buffer* new_self(size_t capacity_size)
    {
        return new queue_buffer(capacity_size);
    }

    //!容量
    inline size_t capacity()
    {
        return size_of_capacity_;
    }

    //!已经使用的空间
    inline size_t size()
    {
        return end_point_;
    }

    //!剩余的空间
    inline size_t free()
    {
        return size_of_capacity_ - end_point_;
    }

    //!是否已经满了
    inline bool full()
    {
        if (end_point_ >= size_of_capacity_)
        {
            return true;
        }
        return false;
    }

    //!是否为空
    inline bool empty()
    {
        if (end_point_ == 0)
        {
            return true;
        }
        return false;
    };

    inline size_t start_point()
    {
        return start_point_;
    }
    inline size_t end_point()
    {
        return end_point_;
    }
    //将start_point归零，特殊地方使用
    inline void clear_start_point()
    {
        start_point_ = 0;
    }

    //数据长度
    inline size_t data_size()
    {
        return end_point_ - start_point_;
    }

    //!继续在尾部增加数据
    bool push_back(const char* data,
                   size_t szdata);

    //!仅仅移动尾部指针，（你可以先用data_point拷贝进去数据，再改变end_point）
    bool push_back(size_t szdata);

    /*!
     * @brief 从头部取出数据数据，
     * @param[out]    data 取出的数据
     * @param[in,out] szdata 数据长度，输入时表示data长度，返回时标示取出长度
     * @param[in]     squeeze_front  是否
    */
    void pop_front(char* data,
                   size_t& szdata,
                   bool squeeze_front = false);

    void pop_front(size_t& szdata,
                   bool squeeze_front = false);

    /// 挤出空间，如果start_point_>0,将数据前移，让start_point_归零
    void squeeze_front();

    /**
     * @brief             从start_point_+ offset偏移开始，填充数据data,长度为szdata,
     * @param[out]    data   填充的数据
     * @param[in,out] szdata 填充数据长度
     * @param[in] offset     偏移长度，从这个位置开始填充
     * @return               返回true标识填充成功
    */
    bool set(const char* data,
             size_t szdata,
             size_t offset = 0);

    /**
     * @brief         尝试读取所有数据
     * @param[out]    data   读取的数据
     * @param[in,out] szdata 作为输入参数表述data长度，作为输出参数表述读取长度
     * @param[in]     whole  是否要求完整取出
     * @return bool   读取成功返回true，如果长度不够（且要求完成取出），返回false
    */
    bool get(char* data,
             size_t& szdata,
             size_t offset = 0,
             bool whole = true) const;

    //!清理
    void clear();

    //!直接使用数据
    inline char* data_point(size_t offset = 0)
    {
        return buffer_data_ + start_point_ + offset;
    }

    //给你直接使用的能力
public:

    ///当前要使用的缓冲长度，
    size_t  size_of_capacity_ = 0;

    ///数据开始的位置
    size_t  start_point_ = 0;

    ///数据结束的位置
    size_t  end_point_ = 0;

    ///数据缓冲区
    char* buffer_data_ = nullptr;
};
}