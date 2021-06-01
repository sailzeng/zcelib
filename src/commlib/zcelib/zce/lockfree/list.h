#pragma once

#include "zce/shm_container/common.h"

namespace zce::lockfree
{
//
//
//�����������ʵ������Ҳ����

/*!
* @brief    ����ṹ�Ǳ���ABA����İ�ȫָ�룬�������ε��ǣ�����ṹ���
*           ��64bit�Ľṹ�£��󲿷��������lockfree�ġ��Գ���64bit
*           �Ľṹ��atomic is_lock_free���ԣ�������lock free��
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
            //��ȡһ��βָ���βָ���next
            auto tail = tail_;
            auto next = tail.point_->next_;

            //���βָ���Ѿ����ƶ��ˣ������¿�ʼ
            if (tail != tail_)
            {
                continue;
            }
            //���βָ��� next ��ΪNULL���� fetch ȫ��βָ�뵽next
            if (next.point_ != NULL)
            {
                continue;
            }
            n.next = tail.point_->next;
            //���������ɹ������˳�
            write_next.point_ = n;
            write_next.write_counter_ = next.write_counter_ + 1;
            if (CAS(tail.point_->next_, next, write_next) == true)
            {
                break;
            }
        }
        write_tail.point_ = n;
        write_tail.write_counter_ = tail.write_counter_ + 1;
        CAS(tail_, tail, write_tail); //��β���
    }

    bool dequeue(T *&data) //�����У��Ľ���
    {
        while (true) {
            //ȡ��ͷָ�룬βָ�룬�͵�һ��Ԫ�ص�ָ��
            head = Q->head;
            tail = Q->tail;
            next = head->next;

            // Q->head ָ�����ƶ�������ȡ headָ��
            if (head != Q->head) continue;

            // ����ǿն���
            if (head == tail && next == NULL) {
                return false;
            }

            //��� tail ָ�������
            if (head == tail && next == NULL) {
                CAS(Q->tail, tail, next);
                continue;
            }

            //�ƶ� head ָ��ɹ���ȡ������
            if (CAS(Q->head, head, next) == TRUE) {
                value = next->value;
                break;
            }
        }
        free(head); //�ͷ��ϵ�dummy���
        return value;
    }
protected:
    //
    std::atomic <safe_point<node> > head_;
    //
    std::atomic <safe_point<node> > tail_;
};
}
