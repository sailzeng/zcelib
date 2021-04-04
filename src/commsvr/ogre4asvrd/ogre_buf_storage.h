

#ifndef OGRE_BUFFER_STORAGE_H_
#define OGRE_BUFFER_STORAGE_H_

/****************************************************************************************************
class Ogre_Buffer_Storage BUFFER�洢��
****************************************************************************************************/
class Ogre_Buffer_Storage
{

    //
    typedef zce::lordrings <Ogre4a_App_Frame *> SAIL_FRAME_BUFFER_POOL;



protected:

    //ֻ������ʵ��,�������
    Ogre_Buffer_Storage(const Ogre_Buffer_Storage &);
    //ֻ������ʵ��,�������
    Ogre_Buffer_Storage &operator =(const Ogre_Buffer_Storage &);

public:
    //���캯������������
    Ogre_Buffer_Storage();
    ~Ogre_Buffer_Storage();

    //��ʼ��
    void init_buffer_list(size_t szlist );
    //����ʼ��,�Ƿ����е�����ռ�,������һ��Ҫ����
    void uninit_buffer_list();

    //����һ��Buffer��List��
    Ogre4a_App_Frame *allocate_byte_buffer();
    //�ͷ�һ��Buffer��List��
    void free_byte_buffer(Ogre4a_App_Frame *ptrbuf );

    //��չBufferList
    void extend_buffer_list(size_t szlist = EXTEND_NUM_OF_LIST);

public:

    //Ϊ��SingleTon��׼��
    //ʵ���ĸ�ֵ
    static void instance(Ogre_Buffer_Storage *);
    //ʵ���Ļ��
    static Ogre_Buffer_Storage *instance();
    //���ʵ��
    static void clean_instance();

protected:

    //ÿ�����BufferStroge�������ˣ���չ�ĸ���
    static const size_t EXTEND_NUM_OF_LIST = 256;

protected:

    //�ҵ����ڴ�й¶,���ԼӸ���������
    size_t                    size_buffer_alloc_;

    //BUFFER�ĳ���
    SAIL_FRAME_BUFFER_POOL    frame_buffer_ary_;

protected:

    //instance����ʹ�õĶ���
    static Ogre_Buffer_Storage *instance_;

};

#endif //OGRE_BUFFER_STORAGE_H_

