/*!
* @copyright  2004-2019  Apache License, Version 2.0 FULLSAIL
* @filename   zce_boost_non_copyable.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2011��7��1��
* @brief      �ο�boostд��һ���࣬���ڲ��ɸ��Ƶ����ͣ�ֻҪ�������̳о�OK��
*             ���ϣ��һ���಻�ܿ������̳�ZCE_NON_Copyable�Ϳ����ˡ�
*
* @details
*
* @note
*
*/

#ifndef ZCE_LIB_BOOST_NON_COPYABLE_H_
#define ZCE_LIB_BOOST_NON_COPYABLE_H_

/*!
* @brief      ���ܸ��Ƶ�ͨ����������ʵ�֣��������캯���͸�ֵ�������ﵽ�޷����Ƶ�Ŀ��
*/
class ZCE_NON_Copyable
{
protected:
    ///���캯����������������������֤���Լ̳�
    ZCE_NON_Copyable()
    {

    }
    ~ZCE_NON_Copyable()
    {
    }
    // emphasize the following members are private
private:

    ///�������캯������������ʵ��
    ZCE_NON_Copyable(const ZCE_NON_Copyable & );
    ///��ֵ��������������ʵ��
    const ZCE_NON_Copyable &operator=( const ZCE_NON_Copyable & );
};

#endif //ZCE_LIB_BOOST_NON_COPYABLE_H_

