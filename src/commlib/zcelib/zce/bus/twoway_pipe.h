/*!
* @copyright  2004-2019  Apache License, Version 2.0 FULLSAIL
* @filename
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2012年9月18日晚上，中国人民抗日81周年纪念日，81年
* @brief
*
*
* @details
*
*
*
* @note
*
* 喔　你我霎眼抗战二十年　世界怎变　我答应你那一点　不会变
*
* 当天空手空臂我们就上街　没什么声势浩大
* 但被不安养大　不足养大　哪里怕表态
* 当中一起经过了时代瓦解　十大执位再十大
* 路上风急雨大　一起吓大　听惯了警戒
*
* 应该珍惜的　即使牺牲了　激起的火花　仍然照耀

* 几响枪火敲破了沉默领土　剩下烧焦了味道
* 现在少点愤怒　多些厚道　偶尔也很躁
* 不管这种争拗有型或老套　未做好的继续做
* 活着必须革命　心高气傲　哪里去不到

* 他虽走得早　他青春不老　灰色的轨迹　磨成血路

* 喔　你我霎眼抗战二十年　世界怎变　永远企你这一边
* 喔　哪怕再去抗战二十年　去到多远　我也铭记我起点　不会变
*/

#pragma once

#include "zce/bus/mmap_pipe.h"

namespace zce
{
class node;
class kfifo;
};

namespace zce
{
//双行道
class twoway_buspipe : public mmap_buspipe<2>
{
public:

    //PIPE的编号
    enum ZCE_BUS_PIPE_ID
    {
        RECV_PIPE_ID = 0,
        SEND_PIPE_ID = 1,
        NUM_OF_PIPE = 2,      //长度标示,不要用于做函数参数,否则会有溢出
    };

protected:

    //ZERG管道的名称,放在这儿，因为大家都用
    static const char          BUS_PIPE_NAME[NUM_OF_PIPE][16];

protected:
    //instance函数使用的东西
    static twoway_buspipe* two_way_instance_;

public:

    //构造函数,
    twoway_buspipe();
    //析购函数
    ~twoway_buspipe();

public:

    //初始化部分参数,
    int initialize(const char* bus_mmap_name,
                   size_t size_recv_pipe,
                   size_t size_send_pipe,
                   size_t max_frame_len,
                   bool if_restore);

public:

    //实例的赋值
    static void instance(twoway_buspipe*);
    //实例的获得
    static twoway_buspipe* instance();
    //清除实例
    static void clear_inst();
};
}
