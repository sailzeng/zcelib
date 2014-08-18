/*!
* @copyright  2004-2014  Apache License, Version 2.0 FULLSAIL
* @filename   zce_event_reactor_wfmo.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version    
* @date       Sunday, August 17, 2014
* @brief      WFMO 是WaitForMultipleObject的缩写。这个事Windows专有的东东。
*             
*             
* @details    
*             
*             
*             
* @note       
*             
*/

#ifndef ZCE_LIB_EVENT_REACTOR_WFMO_H_
#define ZCE_LIB_EVENT_REACTOR_WFMO_H_

#include "zce_event_reactor_base.h"

/*!
* @brief      WaitforMu 的IO反应器，IO多路复用模型
*
*/
class ZCE_WFMO_Reactor : public ZCE_Reactor
{

public:

    /*!
    * @brief    构造函数
    */
    ZCE_WFMO_Reactor();


    /*!
    * @brief      和析构函数 virtual的喔。
    */
    virtual ~ZCE_WFMO_Reactor();

public:

    
};

#endif //ZCE_LIB_EVENT_REACTOR_WFMO_H_

