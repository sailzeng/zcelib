/*!
* @copyright  2004-2014  Apache License, Version 2.0 FULLSAIL
* @filename   soar_frame_command.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2006年10月20日
* @brief
*
*
* @details    README:
* 拜托在改命令的时候先把目光投射过来
* 命令定义的规则
* 1.除了内部命名,所有的命令的请求命令为奇数,应答为偶数,请求就是客户端发送给服务器，应答就是服务器发送给客户端
* 2.如果没有一个流程没有请求，或者应答，请保持占位，
* 3.没有特殊原因请按照顺序编码
* 4.内部命令不会出现在网络上，
* 5.请求用REQ结尾,应答用RSP
* 6.每组服务器的命令都用一组_BEGIN,_END的宏屏蔽起来.
* 7.所有的命令字采用对应服务器的TYPE*10000+命令ID的方式定义.1-9999作为通讯服务器和业务服务器间的内部命令
* 8.主动发送给客户端的用RSP开头
* 9.麻烦对齐，我好不若能够以对齐的，你老搞坏了，我会画圈圈诅咒你的。
*
*
*
* @note
*
*/


#ifndef SOARING_LIB_APPFRAME_COMMAND_H_
#define SOARING_LIB_APPFRAME_COMMAND_H_

#if 1

enum PET_APPFRAME_CMD
{
    //------------------------------------------------------------------------------------------------
    CMD_INVALID_CMD                            = 0,               //无效的命令,用于初始化或者标示

    //------------------------------------------------------------------------------------------------
    //ZERG服务器和业务服务器通信内部的命令 内部命令，用于通讯服务器和业务进程间的处理。
    //------------------------------------------------------------------------------------------------
    INTERNAL_COMMAND_BEGIN                     = 1,            //开始
    INNER_REG_SOCKET_CLOSED                    = 3,            //内部处理,通知后面应用,某个Socket关闭
    INNER_RSP_SOCKET_CLOSED                    = 4,            //占位
    INNER_REQ_CLOSE_SOCKET                     = 5,            //占位
    INNER_RSP_CLOSE_SOCKET                     = 6,            //内部处理,后面的服务器通知接入服务器,关闭端口
    INNER_REQ_ERROR_SEND_SOCKET                = 7,            //内部处理,某个服务未能连接,发送错误同志后端服务，原来用过
    INNER_RSP_ERROR_SEND_SOCKET                = 8,            //占位
    INNER_REQ_ACCEPT_SOCKET                    = 9,            //内部处理,有PEER连接到我的服务端口，而且已经注册,未实现
    INNER_RSP_ACCEPT_SOCKET                    = 10,           //占位
    INNER_REQ_CONNECT_SOCKET                   = 11,           //内部处理,连接到对方的PEER,未实现
    INNER_RSP_CONNECT_SOCKET                   = 12,           //占位
    INNER_REQ_HANDSHAKE_TIMEOUT                = 13,           //内部处理,握手超时, 后端需要进行相应处理, 并指示通信服务器关闭连接,未实现
    INNER_RSP_HANDSHAKE_TIMEOUT                = 14,           //占位
    INNER_REQ_PEER_TIMEOUT                     = 15,           //内部处理,在接收应答超时构造内部超时消息
    INNER_RSP_PEER_TIMEOUT                     = 16,           //占位
    INNER_REQ_RECV_NAKED_UDP_FRAME             = 17,           //内部处理,收到一个UDP的裸数据，用APPFRAME包裹后传递给业务服务器，接受数据存在在DATA区
    INNER_RSP_RECV_NAKED_UDP_FRAME             = 18,           //占位
    INNER_REQ_SEND_NAKED_UDP_FRAME             = 19,           //占位
    INNER_RSP_SEND_NAKED_UDP_FRAME             = 20,           //内部处理,业务服务器要发送一个UDP的裸数据，其用APPFRAME包裹，发送数据存在在DATA区
    INTERNAL_COMMAND_END                       = 1000,         //

    //------------------------------------------------------------------------------------------------
    //通讯服务器的命令
    ZERG_COMMAND_BEGIN                        = 290000000,               //
    ZERG_CONNECT_REGISTER_REQ                 = 290000001,               //连接后，发送一个注册命令,
    ZERG_CONNECT_REGISTER_RSP                 = 290000002,               //占位
    ZERG_WITH_SINGATRUE_KEY_REQ               = 290000003,               //这个是FRAME带有SINGATRUE_KEY数据(QQ拉起登录)
    ZERG_WITH_SINGATRUE_KEY_RSP               = 290000004,               //占位
    ZERG_RESEND_SINGATRUE_KEY_REQ             = 290000005,               //这个是发送给客户端的，要求客户端重新发送SINGATRUE_KEY
    ZERG_RESEND_SINGATRUE_KEY_RSP             = 290000006,               //占位
    ZERG_REPEAT_LOGIN_KICK_OLD_REQ            = 290000007,               //
    ZERG_REPEAT_LOGIN_KICK_OLD_RSP            = 290000008,               //占位
    ZERG_WITH_64_SINGATRUE_KEY_REQ            = 290000009,               //这个是FRAME带有SINGATRUE_KEY数据
    ZERG_WITH_64_SINGATRUE_KEY_RSP            = 290000010,               //占位
    ZERG_HEART_BEAT_REQ                       = 290000011,               //心跳,内部心跳处理
    ZERG_HEART_BEAT_RSP                       = 290000012,               //占位
    ZERG_ERR_APPFRAME_SESSION_EXPIRE_REQ      = 290000013,               //超时的SESSION处理
    ZERG_ERR_APPFRAME_SESSION_EXPIRE_RSP      = 290000014,               //占位
    ZERG_WITH_VARIABLE_SINGATRUE_KEY_REQ      = 290000015,               //这个是FRAME带有SINGATRUE_KEY数据 不过不确定到底是多少位。 这下彻底上流了。(统一登录)
    ZERG_WITH_VARIABLE_SINGATRUE_KEY_RSP      = 290000016,               //占位
    ZERG_COMMAND_END                          = 290009999,               //
    //------------------------------------------------------------------------------------------------

};

#endif

#endif //#ifndef SOARING_LIB_APPFRAME_COMMAND_H_

