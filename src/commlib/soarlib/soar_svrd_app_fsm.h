
#ifndef SOARING_LIB_SVRD_APP_TRNAS_H_
#define SOARING_LIB_SVRD_APP_TRNAS_H_

#include "soar_svrd_app_base.h"

#define REGISTER_TRANSACTION_CMD(trans_class, cmd_word ) \
    { \
        int __ret_register_ = Transaction_Manager::instance()->register_trans_cmd(cmd_word, \
                                                                                  new trans_class(ZCE_Timer_Queue::instance(),Transaction_Manager::instance())); \
        if (__ret_register_ !=  0) \
        { \
            return __ret_register_; \
        } \
    }



/*!
* @brief      ״̬����APP
*
* @note
*/
class Comm_SvrdApp_FSM : public Soar_Svrd_Appliction
{

protected:

    //ʹ���������Application
    Comm_SvrdApp_FSM();
    virtual ~Comm_SvrdApp_FSM();

public:

    //���ӵ���register_func_cmd
    virtual int app_start(int argc, const char *argv[]);

    //���д���,
    virtual int app_run();

    //�˳�����
    virtual int app_exit();

protected:


    virtual int register_trans_cmd() = 0;

};

#endif //#ifndef SOARING_LIB_SVRD_APP_TRNAS_H_

