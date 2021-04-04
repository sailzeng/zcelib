/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_boost_singleton.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2013��1��1��
* @brief      signleton��ģ��ʵ�֡�
*
* @details
*
* @note       ����ʵ�����ر���ͬʹ��signleton�ĵ�ģ�棬�ر�����ν��С���ɵ�signleton�ĵ�ģ
*             �档����Կ����ҵĴ�������󲿷ֶ����Լ�ʵ�ֵ�signleton instance������
*             ����signleton�ĵ�ģ��ʵ�֣�������2���ô���һ�Ǳ����ظ���ʼ����һ����д���롣
*             �����Ҹ�����Ϊsignleton�Ķ�γ�ʼ��������һ�����Ŵ�����������Ū���ɵĵ����⣬
*             ��ʵ�õĴ��붼Ӧ�ñ�֤�Լ��ĳ�ʼ�������ǵ��߳�״̬�������������������߼����ӣ�
*             �ܶ���붼�����Ⱥ��ʼ��˳��ģ�������ʵ��A�����ܳ�ʼ��B����������������˳��
*             ��������˳����ܲ�һ�������෴����ʼ��A��B��C������C��B��A����
*
*             �������������������һ�仰��ʱ����ڱ���Ĳõ�С�������������,ĳ�̶ֳ�������Ϊ
*             �������������һ��������
*
*             ������һƫ�����ǣ�����������signleton�ĵ�ģ��
*             http://leoxiang.com/dev/different-cpp-singleton-implementaion
*             ���Լ����������Ҳ����������
*             http://www.cnblogs.com/fullsail/archive/2013/01/03/2842618.html
*/

#ifndef ZCE_LIB_BOOST_SINGLETON_H_
#define ZCE_LIB_BOOST_SINGLETON_H_

//========================================================================================================
//

/*!
* @brief      ��ν����ͨ����ĵ���ģ�棬���ڵ���ģʽ�Ŀ���Ӧ�ã�
*             �ŵ㣺����������������ȷ������˳��ɿأ�����˳�������෴��
*             ��C++�󲿷�����һ�£�
*             ȱ�㣺���߳��Լ�����Ĺ�������������������Ȼ��֧�֣�
*             ���˸о���������ϵ���󣬲���Ҫ������ν���̵߳ĳ�ʼ������
*             ����ʹ��
* @tparam     T   ϣ��ʹ��singletonģʽ�����ݽṹ
*/
template <typename T>
class ZCE_Singleton
{

public:
    ///ʵ������
    inline static T *instance()
    {
        static T obj;
        return &obj;
    }

private:
    /// ctor is hidden
    ZCE_Singleton();
    /// copy ctor is hidden
    ZCE_Singleton(ZCE_Singleton const &);

};

//========================================================================================================

/*!
* @brief      ���׼ȷ˵������BOOST��singletonʵ�֣�����BOOST��POOL���
*             singletonʵ�֡����ڿ��ٴ���SingleTonģʽ�Ĵ��롣
*             �ŵ㣺���̰߳�ȫ�����ڣ����죩������ϵ�£����Ա�֤���찲ȫ
*             ȱ�㣺����������ѣ�����������ڸ��ӵ��Ⱥ����ɹ�ϵ����˳����Ȼ
*             �����⣨��ȫ�־�̬�����ĳ�ʼ��˳����Ȼ�ǲ��ɿصģ���
*             ���˲������ر��Ƽ�ʹ�����ģ�棬��Ȼʹ���ྲ̬�������ֲ��ɿ���
*             ���ڵķ�ʽ�������м�ļ��ɣ��Ǻǡ������һ�ξͿ������ˣ���
* @tparam     T    ϣ��ʹ��singletonģʽ�����ݽṹ
*/
template <typename T>
class ZCE_Boost_Singleton
{
private:

    struct object_creator
    {
        object_creator()
        {
            ZCE_Boost_Singleton<T>::instance();
        }
        inline void do_nothing() const {}
    };

    //������ľ�̬����object_creator�Ĺ����ʼ��,�ڽ���main֮ǰ�Ѿ�������instance
    //�Ӷ������˶�γ�ʼ��������
    static object_creator create_object_;
public:

    //
    static T *instance()
    {
        static T obj;
        //do_nothing �Ǳ�Ҫ�ģ�do_nothing�������е���˼��
        //�������create_object_.do_nothing();��仰����main����ǰ��
        //create_object_�Ĺ��캯�������ᱻ���ã�instance��ȻҲ���ᱻ���ã�
        //�ҵĹ�����ģ����ӳ�ʵ�ֵ���Ч���£����û����仰��������Ҳ����ʵ��
        // Singleton_WY<T>::object_creator,���Ծͻᵼ���������
        create_object_.do_nothing();
        return &obj;
    }
};
//��Ϊcreate_object_����ľ�̬������������һ��ͨ�õ�����
template <typename T>  typename ZCE_Boost_Singleton<T>::object_creator ZCE_Boost_Singleton<T>::create_object_;

//========================================================================================================

#endif

