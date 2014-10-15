/*!
* @copyright  2004-2014  Apache License, Version 2.0 FULLSAIL
* @filename   soar_svrd_timer_base.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       Monday, September 08, 2014
* @brief
*
*
* @details    所以你按赞因为你孤单,所以你按赞好像你存在
*
*
*
* @note
*      赞
*      演唱：陶晶莹
*      有上山下海有侧拍自拍
*      有宠物有小孩
*      有粉专社团最爱
*      有奇闻怪谭有上帝有撒旦
*      所以你分享因为你感慨
*      所以你打卡好像你精彩
*      有男人女人
*      有家人敌人有些话不能说
*      有嘴炮依然臭跩
*      有假掰瞎掰有的没的动态
*      所以你按赞因为你孤单
*      所以你按赞好像你存在
*      所以我按赞
*
*      有人晒恩爱有人耍白烂
*      有小菜有大餐
*      有前男友前女友
*      有加了没有有故事有片段
*      所以你分享因为你感慨
*      所以你打卡好像你精彩
*      有潜水偷看有大惊小怪
*      有黑特有公关
*      有摸不到的云端
*
*      所以你按赞因为你孤单
*      所以你按赞好像你存在
*      所以我按赞
*
*      游乐园幻化成数位源
*      在你的屏幕前取代了全世界
*      刷了又刷不重要的近况
*      夺走你的现实到底有多疯狂
*      所以你按赞因为你孤单
*      所以你按赞好像你存在
*      所以我按赞
*
*/

#ifndef SOARING_LIB_TIMER_HANDLER_H_
#define SOARING_LIB_TIMER_HANDLER_H_


class Soar_Stat_Monitor;
class Server_Config_Base;


/*!
* @brief      服务器框架的定时器处理类
*             可以从其得到当前的时钟
* @note
*/
class Server_Timer_Base : public ZCE_Timer_Handler
{

    friend class Soar_Svrd_Appliction;
public:

    ///构造函数,因为框架的设计构造的时候不初始化timer queue，
    Server_Timer_Base();
    ///析构函数
    ~Server_Timer_Base();

protected:

    // 检查监控数据
    void check_monitor(const ZCE_Time_Value &now_time);

    // 系统及进程状态采样
    void report_status();

    ///取得当前的时间，用于对时间精度要求不高的场合
    ZCE_Time_Value gettimeofday();


protected:


    /*!
    * @brief      初始化，如果希望增加APP的定时器或者调整心跳进度，请在调用这个函数前完成
    * @return     virtual int
    * @param      queue
    */
    virtual int initialize(ZCE_Timer_Queue *queue);


    /// 定时处理监控数据
    virtual int timer_timeout(const ZCE_Time_Value &now_time,
                              const void *act = 0);

    ///定时器关闭
    virtual int timer_close();

    ///设置心跳定时器的进度，默认是0.5s一次，如果觉得不够，在initialize前重新设置
    ///精度USEC
    void set_heart_precision(const ZCE_Time_Value &precision);


    /*!
    * @brief      增加一个APP的定时器
    * @param[in]  interval 增加的定时器的间隔 
    * @param[in]  act      增加的定时器的标示
    */
    void add_app_timer(const ZCE_Time_Value &interval, const void *act);

    
protected:

    ///服务器定时器ID, 
    static const  int  SERVER_TIMER_ID[];

    ///APP Timer的最大数量，
    static const size_t MAX_APP_TIMER_NUMBER = 6;

    ///默认心跳的精度
    static const time_t DEF_TIMER_INTERVAL_USEC = 500000;

protected:

    ///心跳的精度，
    ZCE_Time_Value heart_precision_ = ZCE_Time_Value(0, DEF_TIMER_INTERVAL_USEC);

    ///心跳计数器，heartbeat_counter_不从0开始计数是避免第一次模除的时候就发生事情
    uint64_t  heartbeat_counter_ = 1;

    ///从开始心跳到现在的毫秒数，这个数值是通过heartbeat_counter_和精度得到的，
    ///并不准确，用于各种初略计算
    uint64_t  heart_total_mesc_ = 0;

    time_t last_check_ = 0;

    // 监控的实例
    Soar_Stat_Monitor *stat_monitor_ = NULL;

    
    ///非心跳以外，还可以设置N个APP定时器，你自己配置,我为你点个赞
    size_t zan_timer_num_ = 0;
    ///
    ZCE_Time_Value zan_timer_internal_[MAX_APP_TIMER_NUMBER];
    ///
    const void *zan_timer_act_[MAX_APP_TIMER_NUMBER];
    

public:
    ///当前时间
    static ZCE_Time_Value now_time_;

};

#endif // SOARING_LIB_TIMER_HANDLER_H_
