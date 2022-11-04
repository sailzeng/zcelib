#pragma once

#include "zce/shm_container/common.h"

namespace zce::lockfree
{
/*!
* @brief      lockfree的Queue 队列，压入头部，从尾部取出
*
* @tparam     T,保存的是指针
* note        这个玩意测试性质非常大，主要就是为了满足我学习的爱好
*             这个代码可能存在ABA的问题
*             不要使用这个代码
*/
template <typename T>
class queue
{
    //数据都以指针存放，同时保留
    struct node
    {
        //指向数据的指针
        T *data_ = nullptr;
        //next的指针，指向后面的数据
        std::atomic <node *> next_ = nullptr;
    };

public:

    //
    queue() :
        head_(new node),
        tail_(head_.load()),
        queue_size_(0)
    {
        //头和尾都指向一个空的尾node
    }
    ~queue()
    {
        while (node* const old_head = head_.load())
        {
            head_.store(old_head->next_);
            delete old_head;
        }
    }

    queue(const queue& other) = delete;
    queue& operator=(const queue& other) = delete;

    //压入数据
    bool enqueue(T *const data)
    {
        auto new_tail = new node();
        new_tail->data_ = data;
        while (true)
        {
            //先取一下尾指针和尾指针的next
            node * old_tail = tail_;
            node * old_next = old_tail->next_;

            //如果尾指针已经被移动了，则重新开始
            if (old_tail != tail_)
            {
                continue;
            }
            //(1)如果tail_落后了，后移也可以，重新获取也可以
            if (old_next != nullptr)
            {
                //下面这行注释掉也可以，这儿只是加快移动tail_。再次循环tail也应该可以改变
                //tail_.compare_exchange_strong(old_tail, old_next);
                continue;
            }
            //如果加入新的尾结点成功，则退出
            node* write_null = nullptr;
            auto ret = old_tail->next_.compare_exchange_weak(write_null, new_tail);
            if (ret)
            {
                //（2）置尾结点, 这儿确实存在tail没有后移的情况，所以对应会有(1)(3)的处理
                tail_.compare_exchange_strong(old_tail, new_tail);
                ++queue_size_;
                break;
            }
            else
            {
                continue;
            }
        }
        return true;
    }

    //从队列提取数据
    bool dequeue(T *&data)
    {
        while (true)
        {
            //取出头指针，尾指针，和第一个元素的指针
            node * old_head = head_;
            node * old_tail = tail_;
            node * old_next = old_head->next_;

            // 头指针如果发生了变化，重来
            if (old_head != head_)
            {
                continue;
            }
            // 如果是空队列
            if (old_head == old_tail && old_next == nullptr)
            {
                return false;
            }
            //(3),
            if (old_head == old_tail && old_next != nullptr)
            {
                //下面这行注释掉也可以，这儿只是加快移动tail_。再次循环tail也应该可以改变
                //tail_.compare_exchange_strong(old_tail, old_next);
                continue;
            }
            auto ret = head_.compare_exchange_weak(old_head, old_next);
            if (ret)
            {
                data = old_head->data_;
                delete old_head;
                --queue_size_;
            }
            else
            {
                continue;
            }
        }
        return true;
    }

    size_t size()
    {
        return queue_size_;
    }

protected:
    ///队列的头部指针
    std::atomic <node *> head_;
    ///队列的尾部指针
    std::atomic <node *> tail_;
    ///队列的尺寸
    std::atomic<size_t> queue_size_;
};

/*!
* @brief    这个结构是避免ABA问题的安全指针，
*           write_counter_用于记录一些特殊的写入次数，避免ABA问题
*           但是无奈的是，这个结构如果在64bit的结构下，大部分情况不是lockfree的。
*           对超过64bit的结构用atomic is_lock_free测试，都不是lock free。
*
*/
template <typename N>
class safe_point
{
public:
    N *point_ = nullptr;
    int write_counter_ = 0;
};
}
