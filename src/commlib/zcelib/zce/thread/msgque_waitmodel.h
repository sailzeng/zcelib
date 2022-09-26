#pragma once

namespace zce
{

enum MQW_WAIT_MODEL
{
    //不用等待，尝试一下
    MQW_NO_WAIT,
    //一直等待
    MQW_WAIT_FOREVER,
    //在超时时间内等待
    MQW_WAIT_TIMEOUT,
};

}