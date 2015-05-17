/// @file       zerg_stat_define.h
/// @date       2012/07/21 14:02
///
/// @author     yunfeiyang
///
/// @brief      小虫监控项定义
///
#ifndef ZERG_STAT_DEFINE_H_
#define ZERG_STAT_DEFINE_H_

//监控的FEATURE_ID
enum ZERG_MONITOR_FEATURE_ID
{
    ZERG_SERVICE_STAT_BEGIN =  9000,
    ZERG_ACCEPT_PEER_COUNTER,               //五分钟内已经Accept的PEER计数
    ZERG_ACCEPT_PEER_NUMBER,                //正在Accept的PEER数量,绝对值
    ZERG_CONNECT_PEER_COUNTER,              //五分钟内已经Connect的PEER计数
    ZERG_CONNECT_PEER_NUMBER,               //正在Connect的PEER数量,绝对值
    ZERG_BUFFER_STORAGE_NUMBER,             //BUFFER缓冲区的CHUNK个数,绝对值
    ZERG_SEND_FRAME_COUNTER,                //五分钟内从发送管道取出放入发送队列的帧总数，
    ZERG_RECV_FRAME_COUNTER,                //五分钟内成功接收放入接收管道的帧总数
    ZERG_RECV_BYTES_COUNTER,                //五分钟内成功收到字节数总数,非精确值,因为每个PEER都是隔一段时间统计一次
    ZERG_SEND_BYTES_COUNTER,                //五分钟内成功发送的字节总数,非精确值
    ZERG_RECV_SUCC_COUNTER,                 //五分钟内接受成功帧总数,,非精确值
    ZERG_SEND_SUCC_COUNTER,                 //五分钟内发送成功帧总数,非精确值
    ZERG_RECV_FAIL_COUNTER,                 //五分钟内接收失败的数据帧总数
    ZERG_SEND_FAIL_COUNTER,                 //五分钟内发送数据失败的总数
    ZERG_RECV_BLOCK_COUNTER,                //五分钟内接收数据发生阻塞的总数
    ZERG_SEND_BLOCK_COUNTER,                //五分钟内发送数据发生阻塞的总数
    ZERG_UDP_RECV_COUNTER,                  //五分钟内收到UDP数据的次数
    ZERG_UDP_SEND_COUNTER,                  //五分钟内发送UDP数据的次数
    ZERG_UDP_RECV_BYTES_COUNTER,            //五分钟内收到UDP数据字节的总数
    ZERG_UDP_SEND_BYTES_COUNTER,            //五分钟内内发送UDP数据字节的总数
    ZERG_UDP_RECV_FAIL_COUNTER,             //五分钟内收到UDP数据错误次数
    ZERG_UDP_SEND_FAIL_COUNTER,             //五分钟内发送UDP数据错误次数
    ZERG_RECV_PIPE_FULL_COUNTER,            //五分钟内内接收数据管道满错误的计数,接收丢失数据个数
    ZERG_RECV_PIPE_FULL_NUMBER,             //接收数据管道满错误的累计值
    ZERG_SEND_LIST_FULL_COUNTER,            //五分钟内 TCP的发送队列满了，发送丢失数据个数
    ZERG_SEND_LIST_FULL_NUMBER,             //TCP的发送队列满了的累计值

    // 请不要在心跳监控中间插监控项
    ZERG_HEART_BEAT_RECV_TIME_GAP,          // 五分钟心跳包总收发时间间隔
    ZERG_HEART_BEAT_RECV_COUNT,             // 五分钟心跳包总收发次数
    ZERG_HEART_BEAT_LESS_FIFTY_COUNT,          // 五分钟心跳包总收发时间间隔少于50ms次数
    ZERG_HEART_BEAT_LESS_HNDRD_COUNT,          // 五分钟心跳包总收发时间间隔少于100ms次数
    ZERG_HEART_BEAT_LESS_FIFHNDRD_COUNT,          // 五分钟心跳包总收发时间间隔少于500ms次数
    ZERG_HEART_BEAT_MORE_FIFHNDRD_COUNT,          // 五分钟心跳包总收发时间间隔大于等于500ms次数

    ZERG_HEART_BEAT_APP_RECV_TIME_GAP,          // 五分钟心跳包APP收发时间间隔
    ZERG_HEART_BEAT_APP_RECV_COUNT,             // 五分钟心跳包APP收发次数
    ZERG_HEART_BEAT_APP_LESS_FIFTY_COUNT,          // 五分钟心跳包APP收发时间间隔少于50ms次数
    ZERG_HEART_BEAT_APP_LESS_HNDRD_COUNT,          // 五分钟心跳包APP收发时间间隔少于100ms次数
    ZERG_HEART_BEAT_APP_LESS_FIFHNDRD_COUNT,          // 五分钟心跳包APP收发时间间隔少于500ms次数
    ZERG_HEART_BEAT_APP_MORE_FIFHNDRD_COUNT,          // 五分钟心跳包APP收发时间间隔大于等于500ms次数
    ZERG_SEND_FRAME_COUNTER_BY_SVR_TYPE,                //五分钟内从发送管道取出放入发送队列的帧总数,分服务类型
    ZERG_RECV_FRAME_COUNTER_BY_SVR_TYPE,                //五分钟内成功接收放入接收管道的帧总数,分服务类型
    ZERG_SEND_FRAME_COUNTER_BY_CMD,                     //五分钟内从发送管道取出放入发送队列的帧总数,分命令字
    ZERG_RECV_FRAME_COUNTER_BY_CMD,                     //五分钟内成功接收放入接收管道的帧总数,分命令字
    ZERG_SEND_FAIL_COUNTER_BY_SVR_TYPE,
    ZERG_SEND_LIST_FULL_COUNTER_BY_SVR_TYPE,            // 发送队列满导致导致丢包的数量，分服务ID

    // 后面随便加
    ZERG_SERVICE_STAT_END,                  //
};

#endif // ZERG_STAT_DEFINE_H_
