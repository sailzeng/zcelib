#include "ogre_predefine.h"
#include "ogre_comm_manager.h"

#include "ogre_udp_ctrl_handler.h"
#include "ogre_accept_handler.h"
#include "ogre_tcp_ctrl_handler.h"
#include "ogre_buf_storage.h"
#include "ogre_configure.h"
#include "ogre_ip_restrict.h"

Ogre_Comm_Manger *Ogre_Comm_Manger::instance_ = NULL;

Ogre_Comm_Manger::Ogre_Comm_Manger():
    ogre_config_(NULL)
{
}

Ogre_Comm_Manger::~Ogre_Comm_Manger()
{

}

//���һ���˿��Ƿ�ȫ.Ȼ��������ý��д���
int Ogre_Comm_Manger::check_safe_port(ZCE_Sockaddr_In &inetadd)
{
    const size_t IP_ADDR_LEN = 31;
    char ip_addr_str[IP_ADDR_LEN + 1];
    size_t use_len = 0;

    //������˱��ռ��,������õĶ˿�
    if ( false == inetadd.check_safeport() )
    {
        //���ʹ�ñ��մ�(TRUE)
        if (ogre_config_->ogre_cfg_data_.ogre_insurance_)
        {
            ZCE_LOG(RS_ERROR, "Unsafe port [%s],if you need to open this port,please close insurance. \n",
                    inetadd.to_string(ip_addr_str,IP_ADDR_LEN,use_len));
            return SOAR_RET::ERR_OGRE_UNSAFE_PORT_WARN;
        }
        //�����ʹ�ñ���(FALSE)
        else
        {
            //��������
            ZCE_LOG(RS_INFO, "Warn!Warn! Unsafe port [%s] listen.Please notice! \n",
                    inetadd.to_string(ip_addr_str,IP_ADDR_LEN,use_len));
        }
    }
    //
    return 0;
}

//�õ�����
int Ogre_Comm_Manger::get_config(const Ogre_Server_Config *config)
{
    int ret = 0;

    //���ô����֡����󳤶�
    Ogre4a_App_Frame::set_max_framedata_len(config ->ogre_cfg_data_.max_data_len_);

    //IP����,
    ret = Ogre_IPRestrict_Mgr::instance()->get_config(config);

    if (0 != ret )
    {
        return ret;
    }

    //TCP ��ȡ����
    ret = Ogre_TCP_Svc_Handler::get_config(config);
    if (ret != 0)
    {
        return ret;
    }
    ogre_config_ = config;
    return 0;
}


//�����еĶ����е����ݷ��ͣ���SEND�ܵ��ҵ����е�����ȥ����,
//�����룬���Ǽ���һ����෢�͵�֡���޶�
int Ogre_Comm_Manger::get_all_senddata_to_write(size_t &procframe)
{
    int ret = 0;

    //
    for (procframe = 0;
         Soar_MMAP_BusPipe::instance()->is_empty_bus(Soar_MMAP_BusPipe::SEND_PIPE_ID) == false &&
         procframe < MAX_ONCE_SEND_FRAME; ++procframe)
    {
        Ogre4a_App_Frame *send_frame = Ogre_Buffer_Storage::instance()->allocate_byte_buffer();

        //
        ret = Soar_MMAP_BusPipe::instance()->pop_front_bus(Soar_MMAP_BusPipe::SEND_PIPE_ID,
                                                           reinterpret_cast< zce::lockfree::dequechunk_node *&>(send_frame));

        if (ret != 0)
        {
            //�黹����
            Ogre_Buffer_Storage::instance()->free_byte_buffer(send_frame);
            continue;
        }

        //���FRAME�ĳ���
        if (send_frame->ogre_frame_len_ > Ogre4a_App_Frame::MAX_OF_OGRE_FRAME_LEN)
        {
            ZCE_LOG(RS_ALERT, "Ogre_Comm_Manger::get_all_senddata_to_write len %u\n",
                    send_frame->ogre_frame_len_);
            DEBUGDUMP_OGRE_HEAD(send_frame, "Ogre_Comm_Manger::get_all_senddata_to_write", RS_ALERT);
            ZCE_ASSERT(false);
            return SOAR_RET::ERR_OGRE_SEND_FRAME_TOO_LEN;
        }

        //�����TCP
        if (send_frame->ogre_frame_option_ & Ogre4a_App_Frame::OGREDESC_PEER_TCP )
        {
            ret = Ogre_TCP_Svc_Handler::process_send_data(send_frame);

            if (ret != 0)
            {
                //�黹����
                Ogre_Buffer_Storage::instance()->free_byte_buffer(send_frame);
                continue;
            }
        }

        //�����UDP
        else if (send_frame->ogre_frame_option_ & Ogre4a_App_Frame::OGREDESC_PEER_UDP )
        {
            //��������
            Ogre_UDPSvc_Hdl::send_alldata_to_udp(send_frame);
            Ogre_Buffer_Storage::instance()->free_byte_buffer(send_frame);
        }
        //�㶼����д������η��ͣ�
        else
        {
            ZCE_LOG(RS_ERROR, "Ogre frame have not send option,Please Check you code.\n");
            Ogre_Buffer_Storage::instance()->free_byte_buffer(send_frame);
        }
    }

    return 0;
}

//��ʼ��ͨѶ������
int Ogre_Comm_Manger::init_comm_manger()
{
    int ret = 0;

    //��ʼ�����еļ����˿�

    for (unsigned int i = 0; i < ogre_config_->ogre_cfg_data_.accept_peer_num_; ++i)
    {
        Ogre_TCPAccept_Hdl *accpet_hd = new Ogre_TCPAccept_Hdl(
            ogre_config_->ogre_cfg_data_.accept_peer_ary_[i]);

        ret = accpet_hd->create_listenpeer();
        if (0 != ret)
        {
            return ret;
        }
    }


    for (unsigned int i = 0; i <= ogre_config_->ogre_cfg_data_.udp_peer_num_; ++i)
    {
        Ogre_UDPSvc_Hdl *udp_hd = new Ogre_UDPSvc_Hdl(
            ogre_config_->ogre_cfg_data_.udp_peer_ary_[i]);

        ret = udp_hd->init_udp_peer();
        if (0 != ret)
        {
            return ret;
        }
    }

    //��ʼ����̬����
    ret = Ogre_TCP_Svc_Handler::init_all_static_data();

    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

//ע��ͨѶ������
int Ogre_Comm_Manger::uninit_comm_manger()
{
    //
    Ogre_TCP_Svc_Handler::unInit_all_static_data();
    //
    Ogre_IPRestrict_Mgr::clean_instance();

    return 0;
}

//�õ����ӵ�ʵ��
Ogre_Comm_Manger *Ogre_Comm_Manger::instance()
{
    if (instance_ == NULL)
    {
        instance_ = new Ogre_Comm_Manger();
    }

    return instance_;
}

//�����ӵ�ʵ��
void Ogre_Comm_Manger::clean_instance()
{
    if (instance_)
    {
        delete instance_;
        instance_ = NULL;
    }
}

