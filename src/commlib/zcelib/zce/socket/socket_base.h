#ifndef ZCE_LIB_SOCKET_BASE_H_
#define ZCE_LIB_SOCKET_BASE_H_

//SOCKET��ַ�Ļ���
class ZCE_Sockaddr;
class ZCE_Time_Value;

//SOCKET�Ļ���
class ZCE_Socket_Base
{
protected:

    //SOCKET�����LINUX����int���ļ������WINDOWS��һ��SOCKET���ͣ���ʵ����һ��WINDOWS�����Ҳ����ָ��
    ZCE_SOCKET     socket_handle_;

    //ZCE_Socket_Base�����ṩ���ⲿ�ã����Բ�׼��
protected:

    /*!
    * @brief      ���캯��
    */
    ZCE_Socket_Base();

    /*!
    * @brief      �������캯��
    * @param      socket_hanle
    */
    explicit ZCE_Socket_Base(const ZCE_SOCKET &socket_hanle);


    /*!
    * @brief      ���������������close��
    * @note       �������˵��һ�£�ACE�����socket��װ��ʱ����������������
    *             û�е���close�����ĳ������Ǳ����ڲ���(value)���ݵ�ʱ������
    *             �ˣ����Ҿ������õĲ������Ӧ�ø��ã��������޴���֪���������û
    *             ���ͷ���Դ������������ѡ�
    */
    ~ZCE_Socket_Base();

public:

    //���þ��
    void set_handle(const ZCE_SOCKET &socket_hanle);
    //��ȡ���
    ZCE_SOCKET get_handle() const;

    //Open SOCK�������BIND���ص�ַ�ķ�ʽ
    int open(int type,
             int protocol_family = AF_INET,
             int protocol = 0,
             bool reuse_addr = true);

    //Open SOCK�����BIND���ص�ַ�ķ�ʽ
    int open(int type,
             const ZCE_Sockaddr *local_addr,
             int protocol_family = AF_INET,
             int protocol = 0,
             bool reuse_addr = false);

    ///�ر�֮,һ������£��һ�����ͷ���Դ�Ǳ�Ȼ�ɹ���һ����
    int close();


    /*!
    * @brief      �ͷŶԾ���Ĺ����������Ϊ��Ч���ѣ���������������Ϊ��������ʱ
    *             Ϊ�˱����������������ر�close�˾��ʱ��������ǰʹ���������
    * @note       ��ο�����������˵��
    */
    void release_noclose();

    /*!
    * @brief      ��ĳ�����ص�IP��ַ��
    * @return     int
    * @param      add_name
    */
    int bind(const ZCE_Sockaddr *add_name) const;

    ///��ĳЩѡ�WIN32Ŀǰֻ֧��O_NONBLOCK
    int sock_enable (int value) const;

    ///�ر�ĳЩѡ�WIN32Ŀǰֻ֧��O_NONBLOCK
    int sock_disable(int value) const;

    //��ȡSocket��ѡ��
    int getsockopt (int level,
                    int optname,
                    void *optval,
                    socklen_t *optlen)  const;

    //����Socket��ѡ��
    int setsockopt (int level,
                    int optname,
                    const void *optval,
                    int optlen) const;

    //ȡ�öԶ˵ĵ�ַ��Ϣ
    int getpeername (ZCE_Sockaddr *addr)  const;

    //ȡ�ñ��صĵ�ַ��Ϣ
    int getsockname (ZCE_Sockaddr *addr)  const;

    //��Ҫ˵�����ǣ�UDPҲ������connect������UDP��connect������������,ֻ�Ǽ�¼ͨ�ŵ�ַ����Ȼ�����ֱ�ӵ���send or recv��������ȷҪͨ�ŵĵ�ַ
    //���Խ�connect��send��recv 3�������ŵ���base���棬��Ҷ�����ʹ��

    //connectĳ����ַ
    int connect(const ZCE_Sockaddr *addr) const;

    //�������ݣ���������״̬������Ϊ
    ssize_t recv (void *buf,
                  size_t len,
                  int flags = 0) const;

    //�������ݣ���������״̬������Ϊ
    ssize_t send (const void *buf,
                  size_t len,
                  int flags = 0) const;

};

#endif //#ifndef ZCE_LIB_SOCKET_BASE_H_

