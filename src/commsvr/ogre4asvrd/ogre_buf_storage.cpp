
#include "ogre_predefine.h"
#include "ogre_buf_storage.h"
#include "ogre_configure.h"

//instance函数使用的东西
Ogre_Buffer_Storage *Ogre_Buffer_Storage::instance_ = NULL;

/****************************************************************************************************
class Ogre_Buffer_Storage
****************************************************************************************************/
Ogre_Buffer_Storage::Ogre_Buffer_Storage():
    size_buffer_alloc_(0)
{
    //extend_buffer_list(szlist);
}
//
Ogre_Buffer_Storage::~Ogre_Buffer_Storage()
{

}

//
void Ogre_Buffer_Storage::init_buffer_list(size_t szlist)
{

    extend_buffer_list(szlist);
}

//
void Ogre_Buffer_Storage::uninit_buffer_list()
{

    //少使用函数，懒得注意效率
    size_t sz_of_buffer = frame_buffer_ary_.size();

    for (size_t i = 0; i < sz_of_buffer; ++i)
    {
        delete frame_buffer_ary_[i];
        frame_buffer_ary_[i] = NULL;
    }

    frame_buffer_ary_.clear();
    size_buffer_alloc_ = 0;
}

/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2005年11月27日
Function        : Ogre_Buffer_Storage::allocate_byte_buffer
Return          : Ogre4a_App_Frame*
Parameter List  : NULL
Description     :
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
Ogre4a_App_Frame *Ogre_Buffer_Storage::allocate_byte_buffer()
{

    //缓冲区使用完了,扩展
    if (true == frame_buffer_ary_.empty() )
    {
        extend_buffer_list();
    }

    Ogre4a_App_Frame *tmppr = frame_buffer_ary_[0];
    frame_buffer_ary_[0] = NULL;
    frame_buffer_ary_.pop_front();
    return tmppr;
}

//
void Ogre_Buffer_Storage::free_byte_buffer(Ogre4a_App_Frame *ptrbuf)
{
    ptrbuf->reset_framehead();
    frame_buffer_ary_.push_back(ptrbuf);
}

//
/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2005年11月27日
Function        : Ogre_Buffer_Storage::extend_buffer_list
Return          : void
Parameter List  :
  Param1: size_t szlist
Description     : 扩展了缓冲区数量，
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
void Ogre_Buffer_Storage::extend_buffer_list(size_t szlist)
{
    //
    ZLOG_INFO( "extend_buffer_list size:%d total:%d need memory [%u] ,total use memory [%u].\n",
               szlist,
               size_buffer_alloc_,
               szlist * (Ogre4a_App_Frame::MAX_OF_OGRE_FRAME_LEN + sizeof(size_t)),
               size_buffer_alloc_ * (Ogre4a_App_Frame::MAX_OF_OGRE_FRAME_LEN + sizeof(size_t))
             );

    //重新扩展一下空间
    frame_buffer_ary_.resize(size_buffer_alloc_ + szlist);

    //将新的NEW数据装载进去
    for (size_t i = 0; i < szlist; ++i)
    {
        Ogre4a_App_Frame *tmppr = new(Ogre4a_App_Frame::MAX_OF_OGRE_FRAME_LEN) Ogre4a_App_Frame();
        tmppr->reset_framehead();
        frame_buffer_ary_.push_back(tmppr);
    }

    //
    size_buffer_alloc_ += szlist;

}

//为了SingleTon类准备
//实例的赋值
void Ogre_Buffer_Storage::instance(Ogre_Buffer_Storage *instance)
{
    clean_instance();
    instance_ = instance;
}
//实例的获得
Ogre_Buffer_Storage *Ogre_Buffer_Storage::instance()
{
    if (NULL == instance_)
    {
        clean_instance();
        instance_ = new Ogre_Buffer_Storage();
    }

    return instance_;
}
//清除实例
void Ogre_Buffer_Storage::clean_instance()
{
    if (instance_)
    {
        delete instance_;
        instance_ = NULL;
    }

}
