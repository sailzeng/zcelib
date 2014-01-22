/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_boost_close_assist.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2011年7月1日
* @brief      
*             
*
* @details
*
* @note
*
*/

#ifndef ZCE_LIB_BOOST_CLOSE_ASSIST_H_
#define ZCE_LIB_BOOST_CLOSE_ASSIST_H_

namespace ZCE_LIB
{

/*!
* @brief     用于文件处理过程的自动释放
*/
template <class _value_type >
class close_assist
{
public:

    ///构造函得到文件句柄
    close_assist(const _value_type to_close) :
        to_close_(to_close)
    {
    }

    //利用析构函数关闭文件
    ~close_assist();

protected:

    ///自动处理的文件句柄
    _value_type    to_close_;

};



};

#endif //ZCE_LIB_BOOST_CLOSE_ASSIST_H_

