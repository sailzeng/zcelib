/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/shm_container/common.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2005年10月21日
* @brief      实现一套在共享内存使用的容器类，用于服务器的代码，
*             我的设计目标是
*             1.可以在共享内存在中使用，而且可以共享，而且还可以在服务器恢复（崩溃后
*             重新拉起）的时候还能恢复到服务原来的状态，（当然不是所有的状态，我不是
*             做银行业）
*             2.高效，性能至少要能和STL比肩，
*             3.兼容STL的语法，包括迭代器，让大家改造起来成本最低，
*
*             关于多进程间共享，我先用过ACE的实现，他的实现比较精巧（也复杂），他自己
*             设计了内存分配器ACE_Malloc_T，以及保存位置无关地址的类似指针用途的模版
*             ACE_Based_Pointer_Basic，通过共享内存地址，对象地址，得到内部指针相
*             对对象地址的偏移，这样，在共享内存的内部就只有偏移了，
*             但这个实现过于……重了。
*
*             BOOST的interprocess也可作为容器内存分配器，的确可以让容器在共享内存
*             中使用，但他的每个对象都要进行分配，而且没有预估尺寸这些方法，所以其实也
*             不太合适在服务器开发，吐槽一下，interprocess诞生的年代应该是2008年，
*             我的代码出现比他可找多了。
*
*             而我选择了完全在内部完全不使用指针的方式，固定最大数量,所有的数据开始都
*             分配好，通过下标保存偏移位置的方式。
*             这样无论是多个共享者，还是重启恢复都，能搞掂，而且性能也很好。
*
* @details    又想扰痒痒了,又想扰痒痒了. 但这次应该扰的很舒服的一次，大大方便了我
*             日后的服务器开发，而且让我真正进入了模版编程的门。
*             我要承认,我学（偷）用了很多SGI STL的代码,
*             这个定义风格和我的其他代码不一样,重要的目的是保持和STL的函数风格一致
*
*             改进历史
*             1.Date  : 2006XXXX
*             Author  : Sailzeng
*             Modification  :第一版完成，完成了开链的hash,淘汰功能的hash,双向链表
*             的list,vector, 但avl树的代码暂时放弃了,
*             带淘汰功能，超时管理的HASH，很是好用，
*
*             2.Date  : 2007XXXX
*             Author  : Scottxu，
*             Modification  :Scottxu增加了一个rbtreee的代码库，这小子很聪明的说
*
*             3.Date  : 2008XXXX
*             Author  : Sailzeng
*             Modification  :整理代码，同时还对完成了cache chunk等代码。
*             同时完成的还有lock free 的deque也整理过来了，
*
*             4.Date  : 20110401
*             Author  : Sailzeng
*             Modification  :开始对代码进行一次整理，改了一些文件名称,类的名称
*             同时开始在考虑是否提供一些备份恢复的功能需求，以及是否需要提供，当然现在
*             还没有呀。
*
*             5.Date  : 20110412
*             Author  : Sailzeng
*             Modification  :增加了一个新的HASH ，REHASH解决冲突的方式，用于某些情
*             况下，可以直接备份数据或者共享内存(MMAP).所以我称为SAFE，当然它也不是没
*             有缺点的，我现在急于在一台机器上用最快速的方法实现有存储恢复数据的功能,
*             这个实现要赞颂即时通讯的几个同事,基本思路来自他们，当然我的改动也不小喔
*
*             6.Date  : 20110412
*             Author  : Sailzeng
*             Modification  :准备用placement new改写原来的赋值函数，同时增加析构调用,
*             这样做的目的是，让这个代码更像C++的代码,
*             原来是不支持带虚表的数据的，现在你可以放入了，
*             【注】如果有虚表，切记不要IPC共享，而一个人使用时，在恢复的时候你要重新处理一下。
*
*             7.Date  : 20140308
*             Author  : Sailzeng
*             Modification  ：改进了原来HASH算法对于质数的管理方式，STL的方式过于粗放，
*             并不见得适合服务器开发,我打算改进一下，只做适当的放大处理
*
*
* @note       第一点：
*             如果数据放在共享内存，特别是当你在多个进程间共享的时候，不是所有的结构都合适
*             放入这些容器，最好还是POD结构，或者没有虚表的结构，注意一下几点
*             1.虚表的指针不可能在多进程间共享。
*             2.最好不要有指针数据，除非你自己能确保他们安全，
*             3.如果非要使用虚表，所有的数据在恢复的时候，最好重新new一次，恢复虚表指针，
*
*             第二点：
*             zcelib共享内存封装接口参数bool if_restore 的解释说明。
*             本来以为是一个比较简单的问题，但昨天佐助来问相关问题，我还是解释一下，
*             bool if_restore == false
*             表示不进行恢复操作，彻底重建
*             对于几个共享内存的类，如果文件已经存在会将共享内存文件TRUNC（晴空），然后进行
*             共享内存重建。如果是对于几个使用（共享）内存的模版的类，会重新对内存的数据进行
*             初始化操作。
*             bool if_restore == true
*             表示进行恢复操作，保持原来共享内存（包括映射文件），内存内部数据的现状。
*             对于几个共享内存的类，如果文件已经存在，不会对文件进行任何操作。（当然还是会进行
*             大小的检查的。如果检查失败会返回失败）。当然如果文件不存在，会重新创建文件。
*             如果是几个使用（共享）内存的模版类，不会对内存进行任何操作，知会检查各种头部数据
*             是否一致（检查失败也会返回错误）。
*             通过上面的描述大家应该明白了。
*             大部分时候，你都可以用 if_restore == true 这种方式（坏消息是我把很多函数的默
*             认参数搞成了false，为了安全）。
*             1.  如果共享内存没有建立，会自动建立共享内存。
*             2.  如果共享内存已经建立，程序重启，会根据存在的共享内存进行恢复。
*             3.  如果A,B两人使用一个共享内存，A创建了共享内存，B就可以直接使用。
*             这种方式保证了大部分情况都可以OK。
*
*             第三点：
*             内部的一些内部偏移用的size_t，size_t的缺点是如果在64位环境，一般会浪费4个字节，
*             我本来考虑在某些情况下，用uint32_t代替size_t，但我自己思考了一下，如果用100万
*             的节点，如果采用hashtable,浪费的空间也就是12个字节*100万，=12M，算了，不考虑
*             那么多了.用64位的系统不就是为了大内存吗？
*
*/

#ifndef ZCE_LIB_SHARE_MEM_PRE_DEFINE_H_
#define ZCE_LIB_SHARE_MEM_PRE_DEFINE_H_

#include "zce/util/non_copyable.h"
#include "zce/os_adapt/math.h"

namespace zce
{
//允许不一致的恢复，用于一些特殊调整后的恢复数据，
//除非你知道你自己在干嘛，否则不要调整这个宏
#define ALLOW_RESTORE_INCONFORMITY 0

//空序号指针标示,32位为0xFFFFFFFF,64位为0xFFFFFFFFFFFFFFFF CNTR = CONTAINER SHM = Share memory
const size_t  SHM_CNTR_INVALID_POINT = static_cast<size_t>(-1);

//返回大于N的一个质数,,为什么不用STL的方式呢，因为STL的方式过于粗狂，
//STL采用的方式一个查表的方式
//我原来的算法就是采用STL的算法改进的，
void hash_prime(const size_t node_num, size_t& real_num);

//返回大于N的一个质数,,
void hash_prime_ary(const size_t node_num, size_t& real_num, size_t row, size_t prime_ary[]);

#define SHM_INDEX_T_LENGTH 32

//=============================================================================================
//定义Hash函数的函数对象(仿函数),不是我非要当"特立独行的猪",非要抄而不用STL,
//写这个代码的时候hashtable还不是标准STL,
//为了能直接在.net下编译,我不得不如此,其实你可以自己定义返回size_t整数Key的函数对象

template <class keytpe> struct smem_hash {};

//这个函数其实是BKDRHash的描述，原有的STLPort中间seed 是5，BKDRHash 推荐使用131等值
inline size_t _shm_hash_string(const char* str)
{
    size_t hashval = 0;

    // 31 131 1313 13131 131313 etc..
    static const size_t BKDR_HASH_SEED = 131;
    for (; *str; ++str)
    {
        hashval = BKDR_HASH_SEED * hashval + *str;
    }

    return size_t(hashval);
}

template<> struct smem_hash<char*>
{
    size_t operator()(const char* str) const
    {
        return _shm_hash_string(str);
    }
};

template<> struct smem_hash<const char*>
{
    size_t operator()(const char* str) const
    {
        return _shm_hash_string(str);
    }
};

template<> struct smem_hash<std::string>
{
    size_t operator()(const std::string& str) const
    {
        return _shm_hash_string(str.c_str());
    }
};

template<> struct smem_hash<const std::string>
{
    size_t operator()(const std::string& str) const
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
        // 64位系统，则直接返回
        return static_cast<size_t>(x);
#else
        // 32位操作系统, 前32位+后32位
        size_t r = (size_t)((x & 0xffffffff) + (x >> 32));
        return r;
#endif //
    }
};

//---------------------------------------------------------------------------------------------
//默认的一个淘汰函数，在自动淘汰时调用，用于完成一些不便于用析构处理的代码
template < class T>
class _default_washout_fun
{
public:
    void operator()(T& /*da*/)
    {
        return;
    }
};

//---------------------------------------------------------------------------------------------

//identity也不是标准STL,偷,偷,偷, identity其实就是萃取自己
template <class T> struct smem_identity
{
    const T& operator()(const T& x) const
    {
        return x;
    }
};

//
template <class _Pair> struct mmap_select1st
{
    typename _Pair::first_type&
        operator()(_Pair& __x) const
    {
        return __x.first;
    }

    const typename _Pair::first_type&
        operator()(const _Pair& __x) const
    {
        return __x.first;
    }
};

//
template <class _Pair> struct mmap_select2st
{
    typename _Pair::second_type&
        operator()(_Pair& __x) const
    {
        return __x.second;
    }

    const typename _Pair::second_type&
        operator()(const _Pair& __x) const
    {
        return __x.second;
    }
};

//=============================================================================================
///放到这儿是因为HASH—EXPIRE和LIST都用了这个结构,
class _shm_list_index
{
public:
    ///LiST后驱索引，
    size_t  idx_next_;
    ///LiST的前驱索引，
    size_t  idx_prev_;

    _shm_list_index() :
        idx_next_(zce::SHM_CNTR_INVALID_POINT),
        idx_prev_(zce::SHM_CNTR_INVALID_POINT)
    {
    }
    _shm_list_index(const size_t& nxt, const size_t& prv) :
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
