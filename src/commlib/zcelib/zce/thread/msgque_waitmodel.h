#pragma once

namespace zce
{

enum MQW_WAIT_MODEL
{
    //���õȴ�������һ��
    MQW_NO_WAIT,
    //һֱ�ȴ�
    MQW_WAIT_FOREVER,
    //�ڳ�ʱʱ���ڵȴ�
    MQW_WAIT_TIMEOUT,
};

}