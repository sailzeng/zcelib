#pragma once

#include "zce/shm_container/common.h"

namespace zce::lockfree
{
/*!
* @brief      lockfree的
*
* @tparam     T
* note        这个玩意测试性质非常大，主要就是为了满足我学习的爱好
*/
template <typename T>
class queue
{
    struct node
    {
        T *data_ = nullptr;

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

    //
    bool enqueue(T *const data)
    {
        auto new_tail = new node();
        while (true)
        {
            //先取一下尾指针和尾指针的next
            node * old_tail = tail_;
            node * old_next = old_tail->next_;

            //如果尾指针已经被移动了，则重新开始
            if (old_tail != tail_ || )
            {
                continue;
            }
            //(1)如果tail_落后了，后移也可以，重新获取也可以
            if (old_next != nullptr)
            {
                //下面这行注释掉也可以，这儿只是加快移动tail_。再次循环tail也应该可以改变
                tail_.compare_exchange_strong(old_tail, old_next);
                continue;
            }
            //如果加入新的尾结点成功，则退出
            node* write_null = nullptr;
            auto ret = old_tail->next_.compare_exchange_weak(write_null, new_tail);
            if (ret)
            {
                old_tail->data_ = data;
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
            if (old_head == old_tail && old_next == NULL)
            {
                return false;
            }
            //(3),
            if (old_head == old_tail && old_next != NULL)
            {
                //下面这行注释掉也可以，这儿只是加快移动tail_。再次循环tail也应该可以改变
                tail_.compare_exchange_strong(old_tail, old_next);
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
protected:
    ///
    std::atomic <node *> head_;
    ///
    std::atomic <node *> tail_;
    ///
    size_t queue_size_;
};

/*!
* @brief    这个结构是避免ABA问题的安全指针，但是无奈的是，这个结构如果
*           在64bit的结构下，大部分情况不是lockfree的。对超过64bit
*           的结构用atomic is_lock_free测试，都不是lock free。
*/
template <typename N>
class safe_point
{
public:
    N *point_ = nullptr;
    int write_counter_ = 0;
};
}
