/*!
* @copyright  2004-2019  Apache License, Version 2.0 FULLSAIL
* @filename   zce/util/non_copyable.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2011��7��1��
* @brief      �ο�boostд��һ���࣬���ڲ��ɸ��Ƶ����ͣ�ֻҪ�������̳о�OK��
*             ���ϣ��һ���಻�ܿ������̳�zce::NON_Copyable�Ϳ����ˡ�
*
* @details
*
* @note
*
*/

#pragma once

namespace zce
{
/*!
* @brief      ���ܸ��Ƶ�ͨ����������ʵ�֣��������캯���͸�ֵ�������ﵽ�޷����Ƶ�Ŀ��
*/
class NON_Copyable
{
protected:
    ///���캯����������������������֤���Լ̳�
    NON_Copyable()
    {

    }
    ~NON_Copyable()
    {
    }
    // emphasize the following members are private
private:

    ///�������캯������������ʵ��
    NON_Copyable(const zce::NON_Copyable&) = delete;
    ///��ֵ��������������ʵ��
    const NON_Copyable& operator=(const zce::NON_Copyable&) = delete;
};

}
