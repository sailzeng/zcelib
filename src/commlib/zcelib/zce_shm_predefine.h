/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_shm_predefine.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2005��10��21��
* @brief      ʵ��һ���ڹ����ڴ�ʹ�õ������࣬���ڷ������Ĵ��룬
*             �ҵ����Ŀ����
*             1.�����ڹ����ڴ�����ʹ�ã����ҿ��Թ������һ������ڷ������ָ���������
*             �������𣩵�ʱ���ָܻ�������ԭ����״̬������Ȼ�������е�״̬���Ҳ���
*             ������ҵ��
*             2.��Ч����������Ҫ�ܺ�STL�ȼ磬
*             3.����STL���﷨���������������ô�Ҹ��������ɱ���ͣ�
*
*             ���ڶ���̼乲�������ù�ACE��ʵ�֣�����ʵ�ֱȽϾ��ɣ�Ҳ���ӣ������Լ�
*             ������ڴ������ACE_Malloc_T���Լ�����λ���޹ص�ַ������ָ����;��ģ��
*             ACE_Based_Pointer_Basic��ͨ�������ڴ��ַ�������ַ���õ��ڲ�ָ����
*             �Զ����ַ��ƫ�ƣ��������ڹ����ڴ���ڲ���ֻ��ƫ���ˣ�
*             �����ʵ�ֹ��ڡ������ˡ�
*
*             BOOST��interprocessҲ����Ϊ�����ڴ����������ȷ�����������ڹ����ڴ�
*             ��ʹ�ã�������ÿ������Ҫ���з��䣬����û��Ԥ���ߴ���Щ������������ʵҲ
*             ��̫�����ڷ������������²�һ�£�interprocess���������Ӧ����2008�꣬
*             �ҵĴ�����ֱ������Ҷ��ˡ�
*
*             ����ѡ������ȫ���ڲ���ȫ��ʹ��ָ��ķ�ʽ���̶��������,���е����ݿ�ʼ��
*             ����ã�ͨ���±걣��ƫ��λ�õķ�ʽ��
*             ���������Ƕ�������ߣ����������ָ������ܸ�࣬��������Ҳ�ܺá�
*
* @details    ������������,������������. �����Ӧ���ŵĺ������һ�Σ���󷽱�����
*             �պ�ķ�����������������������������ģ���̵��š�
*             ��Ҫ����,��ѧ��͵�����˺ܶ�SGI STL�Ĵ���,
*             �����������ҵ��������벻һ��,��Ҫ��Ŀ���Ǳ��ֺ�STL�ĺ������һ��
*
*             �Ľ���ʷ
*             1.Date  : 2006XXXX
*             Author  : Sailzeng
*             Modification  :��һ����ɣ�����˿�����hash,��̭���ܵ�hash,˫������
*             ��list,vector, ��avl���Ĵ�����ʱ������,
*             ����̭���ܣ���ʱ�����HASH�����Ǻ��ã�
*
*             2.Date  : 2007XXXX
*             Author  : Scottxu��
*             Modification  :Scottxu������һ��rbtreee�Ĵ���⣬��С�Ӻܴ�����˵
*
*             3.Date  : 2008XXXX
*             Author  : Sailzeng
*             Modification  :������룬ͬʱ���������cache chunk�ȴ��롣
*             ͬʱ��ɵĻ���lock free ��dequeҲ��������ˣ�
*
*             4.Date  : 20110401
*             Author  : Sailzeng
*             Modification  :��ʼ�Դ������һ����������һЩ�ļ�����,�������
*             ͬʱ��ʼ�ڿ����Ƿ��ṩһЩ���ݻָ��Ĺ��������Լ��Ƿ���Ҫ�ṩ����Ȼ����
*             ��û��ѽ��
*
*             5.Date  : 20110412
*             Author  : Sailzeng
*             Modification  :������һ���µ�HASH ��REHASH�����ͻ�ķ�ʽ������ĳЩ��
*             ���£�����ֱ�ӱ������ݻ��߹����ڴ�(MMAP).�����ҳ�ΪSAFE����Ȼ��Ҳ����û
*             ��ȱ��ģ������ڼ�����һ̨������������ٵķ���ʵ���д洢�ָ����ݵĹ���,
*             ���ʵ��Ҫ���̼�ʱͨѶ�ļ���ͬ��,����˼·�������ǣ���Ȼ�ҵĸĶ�Ҳ��С�
*
*             6.Date  : 20110412
*             Author  : Sailzeng
*             Modification  :׼����placement new��дԭ���ĸ�ֵ������ͬʱ������������,
*             ��������Ŀ���ǣ�������������C++�Ĵ���,
*             ԭ���ǲ�֧�ִ��������ݵģ���������Է����ˣ�
*             ��ע�����������мǲ�ҪIPC������һ����ʹ��ʱ���ڻָ���ʱ����Ҫ���´���һ�¡�
*
*             7.Date  : 20140308
*             Author  : Sailzeng
*             Modification  ���Ľ���ԭ��HASH�㷨���������Ĺ���ʽ��STL�ķ�ʽ���ڴַţ�
*             ���������ʺϷ���������,�Ҵ���Ľ�һ�£�ֻ���ʵ��ķŴ���
*
*
* @note       ��һ�㣺
*             ������ݷ��ڹ����ڴ棬�ر��ǵ����ڶ�����̼乲���ʱ�򣬲������еĽṹ������
*             ������Щ��������û���POD�ṹ������û�����Ľṹ��ע��һ�¼���
*             1.����ָ�벻�����ڶ���̼乲��
*             2.��ò�Ҫ��ָ�����ݣ��������Լ���ȷ�����ǰ�ȫ��
*             3.�����Ҫʹ��������е������ڻָ���ʱ���������newһ�Σ��ָ����ָ�룬
*
*             �ڶ��㣺
*             zcelib�����ڴ��װ�ӿڲ���bool if_restore �Ľ���˵����
*             ������Ϊ��һ���Ƚϼ򵥵����⣬��������������������⣬�һ��ǽ���һ�£�
*             bool if_restore == false
*             ��ʾ�����лָ������������ؽ�
*             ���ڼ��������ڴ���࣬����ļ��Ѿ����ڻὫ�����ڴ��ļ�TRUNC����գ���Ȼ�����
*             �����ڴ��ؽ�������Ƕ��ڼ���ʹ�ã������ڴ��ģ����࣬�����¶��ڴ�����ݽ���
*             ��ʼ��������
*             bool if_restore == true
*             ��ʾ���лָ�����������ԭ�������ڴ棨����ӳ���ļ������ڴ��ڲ����ݵ���״��
*             ���ڼ��������ڴ���࣬����ļ��Ѿ����ڣ�������ļ������κβ���������Ȼ���ǻ����
*             ��С�ļ��ġ�������ʧ�ܻ᷵��ʧ�ܣ�����Ȼ����ļ������ڣ������´����ļ���
*             ����Ǽ���ʹ�ã������ڴ��ģ���࣬������ڴ�����κβ�����֪�������ͷ������
*             �Ƿ�һ�£����ʧ��Ҳ�᷵�ش��󣩡�
*             ͨ��������������Ӧ�������ˡ�
*             �󲿷�ʱ���㶼������ if_restore == true ���ַ�ʽ������Ϣ���ҰѺܶຯ����Ĭ
*             �ϲ��������false��Ϊ�˰�ȫ����
*             1.  ��������ڴ�û�н��������Զ����������ڴ档
*             2.  ��������ڴ��Ѿ���������������������ݴ��ڵĹ����ڴ���лָ���
*             3.  ���A,B����ʹ��һ�������ڴ棬A�����˹����ڴ棬B�Ϳ���ֱ��ʹ�á�
*             ���ַ�ʽ��֤�˴󲿷����������OK��
*
*             �����㣺
*             �ڲ���һЩ�ڲ�ƫ���õ�size_t��size_t��ȱ���������64λ������һ����˷�4���ֽڣ�
*             �ұ���������ĳЩ����£���uint32_t����size_t�������Լ�˼����һ�£������100��
*             �Ľڵ㣬�������hashtable,�˷ѵĿռ�Ҳ����12���ֽ�*100��=12M�����ˣ�������
*             ��ô����.��64λ��ϵͳ������Ϊ�˴��ڴ���
*
*/

#ifndef ZCE_LIB_SHARE_MEM_PRE_DEFINE_H_
#define ZCE_LIB_SHARE_MEM_PRE_DEFINE_H_

#include "zce_boost_non_copyable.h"
#include "zce_os_adapt_math.h"


namespace zce
{



//����һ�µĻָ�������һЩ���������Ļָ����ݣ�
//������֪�����Լ��ڸ������Ҫ���������
#define ALLOW_RESTORE_INCONFORMITY 0

//���ش���N��һ������,,Ϊʲô����STL�ķ�ʽ�أ���ΪSTL�ķ�ʽ���ڴֿ�
//STL���õķ�ʽһ�����ķ�ʽ
//��ԭ�����㷨���ǲ���STL���㷨�Ľ��ģ�
void hash_prime(const size_t node_num, size_t &real_num);


//���ش���N��һ������,,
void hash_prime_ary(const size_t node_num, size_t &real_num, size_t row, size_t prime_ary[]);


#define SHM_INDEX_T_LENGTH 32


//=============================================================================================
//����Hash�����ĺ�������(�º���),�����ҷ�Ҫ��"�������е���",��Ҫ��������STL,
//д��������ʱ��hashtable�����Ǳ�׼STL,
//Ϊ����ֱ����.net�±���,�Ҳ��ò����,��ʵ������Լ����巵��size_t����Key�ĺ�������

template <class keytpe> struct smem_hash {};

//���������ʵ��BKDRHash��������ԭ�е�STLPort�м�seed ��5��BKDRHash �Ƽ�ʹ��131��ֵ
inline size_t _shm_hash_string(const char *str)
{
    size_t hashval = 0;

    // 31 131 1313 13131 131313 etc..
    static const size_t BKDR_HASH_SEED = 131;
    for ( ; *str; ++str)
    {
        hashval = BKDR_HASH_SEED * hashval + *str;
    }

    return size_t(hashval);
}

template<> struct smem_hash<char *>
{
    size_t operator()(const char *str) const
    {
        return _shm_hash_string(str);
    }
};

template<> struct smem_hash<const char *>
{
    size_t operator()(const char *str) const
    {
        return _shm_hash_string(str);
    }
};

template<> struct smem_hash<std::string>
{
    size_t operator()(const std::string &str) const
    {
        return _shm_hash_string(str.c_str());
    }
};

template<> struct smem_hash<const std::string>
{
    size_t operator()(const std::string &str) const
    {
        return _shm_hash_string(str.c_str());
    }
};

template<> struct smem_hash<char>
{
    size_t operator()(char x) const
    {
        return static_cast<size_t>(x);
    }
};

template<> struct smem_hash<unsigned char>
{
    size_t operator()(unsigned char x) const
    {
        return static_cast<size_t>(x);
    }
};

template<> struct smem_hash<short>
{
    size_t operator()(short x) const
    {
        return static_cast<size_t>(x);
    }
};
template<> struct smem_hash<unsigned short>
{
    size_t operator()(unsigned short x) const
    {
        return static_cast<size_t>(x);
    }
};

template<> struct smem_hash<int>
{
    size_t operator()(int x) const
    {
        return static_cast<size_t>(x);
    }
};

template<> struct smem_hash<unsigned int>
{
    size_t operator()(unsigned int x) const
    {
        return static_cast<size_t>(x);
    }
};
template<> struct smem_hash<long>
{
    size_t operator()(long x) const
    {
        return static_cast<size_t>(x);
    }
};

template<> struct smem_hash<uint64_t>
{
    size_t operator()(uint64_t x) const
    {
#if defined (ZCE_LINUX64) || defined (ZCE_WIN64)
        // 64λϵͳ����ֱ�ӷ���
        return static_cast<size_t>(x);
#else
        // 32λ����ϵͳ, ǰ32λ+��32λ
        size_t r = (size_t)((x & 0xffffffff) + (x >> 32));
        return r;
#endif //
    }
};

//---------------------------------------------------------------------------------------------
//Ĭ�ϵ�һ����̭���������Զ���̭ʱ���ã��������һЩ����������������Ĵ���
template < class _value_type>
class _default_washout_fun
{
public:
    void operator()(_value_type & /*da*/)
    {
        return;
    }
};

//---------------------------------------------------------------------------------------------

//identityҲ���Ǳ�׼STL,͵,͵,͵, identity��ʵ������ȡ�Լ�
template <class T> struct smem_identity
{
    const T &operator()(const T &x) const
    {
        return x;
    }
};

//
template <class _Pair> struct mmap_select1st
{
    typename _Pair::first_type &
    operator()(_Pair &__x) const
    {
        return __x.first;
    }

    const typename _Pair::first_type &
    operator()(const _Pair &__x) const
    {
        return __x.first;
    }
};

//
template <class _Pair> struct mmap_select2st
{
    typename _Pair::second_type &
    operator()(_Pair &__x) const
    {
        return __x.second;
    }

    const typename _Pair::second_type &
    operator()(const _Pair &__x) const
    {
        return __x.second;
    }
};

//=============================================================================================
//
class _shm_memory_base : public ZCE_NON_Copyable
{
public:

    //�����ָ���ʾ,32λΪ0xFFFFFFFF,64λΪ0xFFFFFFFFFFFFFFFF
    static const size_t  _INVALID_POINT = static_cast<size_t>( -1);

protected:

    //�ڴ������ַ
    char                  *smem_base_;

    //���캯��
    _shm_memory_base():
        smem_base_(NULL)
    {
    };

    //���캯��
    _shm_memory_base(char *basepoint):
        smem_base_(basepoint)
    {
    };

    virtual ~_shm_memory_base()
    {
    }

};

//-----------------------------------------------------------------------------
///�ŵ��������ΪHASH��EXPIRE��LIST����������ṹ,
class _shm_list_index
{
public:
    ///LiST����������
    size_t  idx_next_;
    ///LiST��ǰ��������
    size_t  idx_prev_;

    _shm_list_index():
        idx_next_(_shm_memory_base::_INVALID_POINT),
        idx_prev_(_shm_memory_base::_INVALID_POINT)
    {
    }
    _shm_list_index(const size_t &nxt, const size_t &prv):
        idx_next_(nxt),
        idx_prev_(prv)
    {
    }
    ~_shm_list_index()
    {
    }
};

};



#endif //ZCE_LIB_SHARE_MEM_PRE_DEFINE_H_

