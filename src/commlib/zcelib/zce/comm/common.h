#pragma once

///////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace zce
{
///IO触发事件, 这儿不要改造成enum class
enum RECTOR_EVENT
{
    NULL_MASK = 0,

    ///SOCKET读取事件，但句柄变为可读是，调用read_event
    READ_MASK = (1 << 1),
    ///SOCKET写事件，当句柄变为可写是，write_event
    WRITE_MASK = (1 << 2),
    ///SOCKET异常事件，触发后调用exception_event
    EXCEPTION_MASK = (1 << 3),
    ///SOCKET异步CONNECT成功后，调用connect_event，异步CONNECT失败后，
    CONNECT_MASK = (1 << 4),
    ///SOCKET ACCEPT事件，当一个ACCEPT端口可以链接后，调用accept_event
    ACCEPT_MASK = (1 << 5),

    ///iNotify通知事件，文件系统的改变通知,调用read_event，
    INOTIFY_MASK = (1 << 9),
};

//SLECT函数处理的3个事件
enum class SELECT_EVENT
{
    SE_READ,
    SE_WRITE,
    SE_EXCEPTION,
};

//! message queue wait model,
enum MQW_WAIT_MODEL
{
    //不用等待，尝试一下
    MQW_NO_WAIT,
    //一直等待
    MQW_WAIT_FOREVER,
    //在超时时间内等待
    MQW_WAIT_TIMEOUT,
};

static const char* DAY_OF_WEEK_NAME[] =
{
    ("Sun"),
    ("Mon"),
    ("Tue"),
    ("Wed"),
    ("Thu"),
    ("Fri"),
    ("Sat")
};

static const char* MONTH_NAME[] =
{
    ("Jan"),
    ("Feb"),
    ("Mar"),
    ("Apr"),
    ("May"),
    ("Jun"),
    ("Jul"),
    ("Aug"),
    ("Sep"),
    ("Oct"),
    ("Nov"),
    ("Dec")
};

//! 为什么最后还是搞了一个自己的时间定义，因为所有的结构都有短板。
//! tm 没有时区和微妙
//! time_t 没有微秒
//! MYSQL_TIME结构只能在MySQL环境下使用
struct ztm
{
    int tz_ = 0;    // time zone
    int year_ = 0;  // years since 0
    int mon_ = 0;   // months since January - [1, 12]
    int day_ = 0;   // day of the month - [1, 31]
    int hour_ = 0;  // hours since midnight - [0, 23]
    int min_ = 0;   // minutes after the hour - [0, 59]
    int sec_ = 0;   // seconds after the minute - [0, 60] including leap second
    int usec_ = 0;  // microseconds after the second - [0, 999999]
};
}
