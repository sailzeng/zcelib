#pragma once

#include "zce/shm_container/common.h"

namespace zce::lockfree
{
//
//
//所以这个玩意实验性质也更大

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

/*!
* @brief
*
* @tparam     T
* note
*/
template <typename T>
class queue
{
    struct node
    {
        T *data_ = nullptr;

        std::atomic <safe_point<node> > next_;
    };

public:

    //
    bool enqueue(const T *data)
    {
        auto n = new node();
        n->data_ = data;

        while (true)
        {
            //先取一下尾指针和尾指针的next
            auto tail = tail_;
            auto next = tail.point_->next_;

            //如果尾指针已经被移动了，则重新开始
            if (tail != tail_)
            {
                continue;
            }
            //如果尾指针的 next 不为NULL，则 fetch 全局尾指针到next
            if (next.point_ != NULL)
            {
                continue;
            }
            n.next = tail.point_->next;
            //如果加入结点成功，则退出
            write_next.point_ = n;
            write_next.write_counter_ = next.write_counter_ + 1;
            if (CAS(tail.point_->next_, next, write_next) == true)
            {
                break;
            }
        }
        write_tail.point_ = n;
        write_tail.write_counter_ = tail.write_counter_ + 1;
        CAS(tail_, tail, write_tail); //置尾结点
    }

    bool dequeue(T *&data) //出队列，改进版
    {
        while (true) {
            //取出头指针，尾指针，和第一个元素的指针
            head = Q->head;
            tail = Q->tail;
            next = head->next;

            // Q->head 指针已移动，重新取 head指针
            if (head != Q->head) continue;

            // 如果是空队列
            if (head == tail && next == NULL) {
                return false;
            }

            //如果 tail 指针落后了
            if (head == tail && next == NULL) {
                CAS(Q->tail, tail, next);
                continue;
            }

            //移动 head 指针成功后，取出数据
            if (CAS(Q->head, head, next) == TRUE) {
                value = next->value;
                break;
            }
        }
        free(head); //释放老的dummy结点
        return value;
    }
protected:
    //
    std::atomic <safe_point<node> > head_;
    //
    std::atomic <safe_point<node> > tail_;
};
}
