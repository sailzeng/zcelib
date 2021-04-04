
#include "ogre_predefine.h"
#include "ogre_buf_storage.h"
#include "ogre_configure.h"

//instance����ʹ�õĶ���
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

    //��ʹ�ú���������ע��Ч��
    size_t sz_of_buffer = frame_buffer_ary_.size();

    for (size_t i = 0; i < sz_of_buffer; ++i)
    {
        Ogre4a_App_Frame::delete_ogre(frame_buffer_ary_[i]);
        frame_buffer_ary_[i] = NULL;
    }

    frame_buffer_ary_.clear();
    size_buffer_alloc_ = 0;
}


Ogre4a_App_Frame *Ogre_Buffer_Storage::allocate_byte_buffer()
{

    //������ʹ������,��չ
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


//��չ�˻���������
void Ogre_Buffer_Storage::extend_buffer_list(size_t szlist)
{
    //
    ZCE_LOG(RS_INFO, "extend_buffer_list size:%d total:%d need memory [%u] ,total use memory [%u].\n",
            szlist,
            size_buffer_alloc_,
            szlist * (Ogre4a_App_Frame::MAX_OF_OGRE_FRAME_LEN + sizeof(size_t)),
            size_buffer_alloc_ * (Ogre4a_App_Frame::MAX_OF_OGRE_FRAME_LEN + sizeof(size_t))
           );

    //������չһ�¿ռ�
    frame_buffer_ary_.resize(size_buffer_alloc_ + szlist);

    //���µ�NEW����װ�ؽ�ȥ
    for (size_t i = 0; i < szlist; ++i)
    {
        Ogre4a_App_Frame *tmppr = Ogre4a_App_Frame::new_ogre(Ogre4a_App_Frame::MAX_OF_OGRE_FRAME_LEN);
        tmppr->reset_framehead();
        frame_buffer_ary_.push_back(tmppr);
    }

    //
    size_buffer_alloc_ += szlist;

}

//Ϊ��SingleTon��׼��
//ʵ���ĸ�ֵ
void Ogre_Buffer_Storage::instance(Ogre_Buffer_Storage *instance)
{
    clean_instance();
    instance_ = instance;
}
//ʵ���Ļ��
Ogre_Buffer_Storage *Ogre_Buffer_Storage::instance()
{
    if (NULL == instance_)
    {
        clean_instance();
        instance_ = new Ogre_Buffer_Storage();
    }

    return instance_;
}
//���ʵ��
void Ogre_Buffer_Storage::clean_instance()
{
    if (instance_)
    {
        delete instance_;
        instance_ = NULL;
    }

}
