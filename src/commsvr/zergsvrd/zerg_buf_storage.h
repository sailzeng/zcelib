
#ifndef ZERG_BUFFER_STORAGE_H_
#define ZERG_BUFFER_STORAGE_H_

//===================================================================================================

/*!
* @brief
*
* @note
*/
class Zerg_Buffer
{
public:
    //
    static const unsigned int CAPACITY_OF_BUFFER = Zerg_App_Frame::MAX_LEN_OF_APPFRAME + 16;

public:

    //��ǰҪʹ�õĻ��峤�ȣ���ǰ�����֡�ĳ���,û�еõ�����ǰ��д0
    size_t      size_of_use_;

    //ʹ�õĳߴ�
    size_t      size_of_buffer_;

    //���ݻ�����
    char       buffer_data_[CAPACITY_OF_BUFFER];

public:
    //
    Zerg_Buffer();
    //
    ~Zerg_Buffer();

    //�������
    void fill_write_data(const size_t szdata, const char *data);
    //��ȡ����
    void get_read_data(size_t &szdata, char *data);

    //
    inline char *get_use_point()
    {
        return buffer_data_ + size_of_buffer_;
    }
    //
    inline size_t get_leave_size()
    {
        return size_of_use_ - size_of_buffer_;
    }

    //
    inline bool is_full()
    {
        if (size_of_buffer_ >= size_of_use_)
        {
            return true;
        }

        return false;
    }

    //
    inline bool is_empty()
    {
        if (size_of_buffer_ == 0)
        {
            return true;
        }

        return false;
    };

    //
    void clear_buffer();

};


//===================================================================================================

/*!
* @brief
*
* @note
*/
class ZBuffer_Storage : public ZCE_NON_Copyable
{
public:
    //���캯������������
    ZBuffer_Storage();
    ~ZBuffer_Storage();

    //��ʼ��
    void init_buffer_list(size_t szlist);

    //�����ⲿ��Handler����������ʼ���õ�Buffer List�ĳߴ�
    void init_buflist_by_hdlnum(size_t num_of_hdl);

    //����ʼ��,�Ƿ����е�����ռ�,������һ��Ҫ����
    void close();

    //�Ӷ��з���һ��Buffer����
    Zerg_Buffer *allocate_buffer();

    //�ͷ�һ��Buffer��������
    void free_byte_buffer(Zerg_Buffer *ptrbuf );

    //��չBuffer����
    void extend_bufferlist(size_t szlist = EXTEND_NUM_OF_LIST);

public:

    //����ʵ������
    static ZBuffer_Storage *instance();
    //������ʵ��
    static void clean_instance();

protected:
    //����ʵ��ָ��
    static ZBuffer_Storage *instance_;

protected:

    //ÿ�����BufferStroge�������ˣ���չ�ĸ���
    static const size_t EXTEND_NUM_OF_LIST = 1024;

protected:

    ///���BUFF �Ķ�������
    typedef zce::lordrings <Zerg_Buffer *> ZBUFFER_RINGS;

    ///�ҵ����ڴ�й¶,���ԼӸ���������
    size_t                size_of_bufferalloc_;

    ///BUFF����
    ZBUFFER_RINGS         buffer_deque_;



};

#endif //_ZERG_BUFFER_STORAGE_H_

