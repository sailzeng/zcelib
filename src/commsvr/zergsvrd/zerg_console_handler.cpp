
#include "zerg_predefine.h"
//#include "zerg_console_handler.h"
//#include "zerg_application.h"
//#include "zerg_configure.h"
//
//Zerg_Console_Handler::Zerg_Console_Handler(ZCE_Reactor *reactor_inst):
//    MML_Console_Handler(reactor_inst)
//{
//}
//
//Zerg_Console_Handler::~Zerg_Console_Handler()
//{
//}
//
///******************************************************************************************
//Author          : Sail ZENGXING  Date Of Creation: 2006��10��16��
//Function        : Zerg_Console_Handler::process_mml_command
//Return          : int
//Parameter List  :
//  Param1: char* ret_string ���ص�����
//  Param2: size_t& str_len  �����������,�����ʾbuf�ĳ���,�����ʾ��������ݳ���
//Description     :
//Calls           :
//Called By       :
//Other           : str_len���뱻�ı䣬�м�.
//Modify Record   :
//******************************************************************************************/
//int Zerg_Console_Handler::process_mml_command(char *ret_string, size_t &str_len)
//{
//    int ret = 0;
//
//    std::string mml_cmd;
//    ret = console_command_.GetMMLCommand(mml_cmd);
//
//    if (ret != 0)
//    {
//        return ret;
//    }
//
//    //��ѯ������״̬
//    if ( strcasecmp(mml_cmd.c_str(), "QUERY_STATS") == 0 )
//    {
//        ret = cmd_get_zergstats(ret_string, str_len);
//
//        if (ret != 0)
//        {
//            return ret;
//        }
//    }
//    else if ( strcasecmp(mml_cmd.c_str(), "QUERY_PEER") == 0 )
//    {
//        ret = cmd_get_peer_stats(ret_string, str_len);
//
//        if (ret != 0)
//        {
//            return ret;
//        }
//    }
//    //�رն˿�
//    else if (strcasecmp(mml_cmd.c_str(), "CLOSE_PEER") == 0)
//    {
//        ret = cmd_close_socketpeer(ret_string, str_len);
//
//        if (ret != 0)
//        {
//            return ret;
//        }
//    }
//
//    //�޸���־���������
//    else if (strcasecmp(mml_cmd.c_str(), "MODIFY_LOG") == 0)
//    {
//        ret = cmd_modify_logpriority(ret_string, str_len);
//
//        if (ret != 0)
//        {
//            return ret;
//        }
//    }
//    //�رշ�����
//    else if (strcasecmp(mml_cmd.c_str(), "CLOSE_SERVICE") == 0)
//    {
//        ret = cmd_close_services(ret_string, str_len);
//
//        if (ret != 0)
//        {
//            return ret;
//        }
//    }
//
//    //To Do:��,������Լ�ϲ���ļ��
//    else
//    {
//        ZCE_LOG(RS_ERROR,"[zergsvr] Can't Process this command :%s.", mml_cmd.c_str());
//        str_len = 0;
//    }
//
//    //����
//    ZCE_LOG(RS_INFO,"[zergsvr] Process Command %s, Ret:%u.", mml_cmd.c_str(), ret);
//    return 0;
//}
//
///******************************************************************************************
//Author          : Sail ZENGXING  Date Of Creation: 2006��10��17��
//Function        : Zerg_Console_Handler::cmd_get_zergstats
//Return          : int
//Parameter List  :
//  Param1: char* ret_string
//  Param2: size_t& str_len
//Description     : �õ�ZERG��������״̬
//Calls           :
//Called By       :
//Other           : һ���ʱ��
//Modify Record   :
//******************************************************************************************/
//int Zerg_Console_Handler::cmd_get_zergstats(char *ret_string, size_t &str_len)
//{
//    size_t tmp_size = 0;
//
//    char day_and_time[35];
//    zce::timestamp (day_and_time, sizeof(day_and_time));
//
//    tmp_size += snprintf(ret_string + tmp_size, str_len - tmp_size, "Console Get Zerg(Tcpsvrd) services info.");
//    tmp_size += snprintf(ret_string + tmp_size, str_len - tmp_size, "Server Time %s.", day_and_time);
//
//    //�õ�ͳ������
//    std::ostringstream tmp_ostrstm;
//
//    //DUMP��Ϣ
//    Zerg_Server_Config::instance()->dump_status_info(tmp_ostrstm);
//    Soar_Stat_Monitor::instance()->dump_status_info(tmp_ostrstm);
//    TCP_Svc_Handler::dump_status_staticinfo(tmp_ostrstm);
//
//    tmp_size += snprintf(ret_string + tmp_size, str_len - tmp_size, "%s", tmp_ostrstm.str().c_str());
//    tmp_size += snprintf(ret_string + tmp_size, str_len - tmp_size, "Have fun my brother.Author: Pet2 Team .");
//    //
//    str_len = tmp_size;
//
//    return 0;
//}
//
///******************************************************************************************
//Author          : Sail ZENGXING  Date Of Creation: 2006��10��17��
//Function        : Zerg_Console_Handler::cmd_get_peer_stats
//Return          : int
//Parameter List  :
//  Param1: char* ret_string
//  Param2: size_t& str_len
//Description     : ��ѯ���ӵ�Peer״̬,��һ����ȫ������,���ұȽϺ�ʱ
//Calls           :
//Called By       :
//Other           : ʵ������QUERY PEER:STARTNO=?,NUMQUERY=?;
//Modify Record   :
//******************************************************************************************/
//int Zerg_Console_Handler::cmd_get_peer_stats(char *ret_string, size_t &str_len)
//{
//    int ret = 0;
//    size_t tmp_size = 0;
//    tmp_size += snprintf(ret_string + tmp_size, str_len - tmp_size, "Console Get Zerg(Tcpsvrd) services info.");
//
//    size_t startno = 0;
//    size_t numquery = 0;
//
//    //��ȡSTARTNO����
//    std::string tmpstr;
//    ret = console_command_.GetCmdParameter("STARTNO", tmpstr);
//
//    //ʹ��Ĭ�ϲ���
//    if (ret != 0)
//    {
//        startno = 0;
//    }
//
//    startno = atoi(tmpstr.c_str());
//
//    //��ȡNUMQUERY����
//    const size_t MAX_QUERY_NUMBER = 1024;
//    ret = console_command_.GetCmdParameter("NUMQUERY", tmpstr);
//
//    if (ret != 0)
//    {
//        numquery = MAX_QUERY_NUMBER;
//    }
//
//    numquery = atoi(tmpstr.c_str());
//
//    if (numquery == 0 || numquery > MAX_QUERY_NUMBER)
//    {
//        numquery = MAX_QUERY_NUMBER;
//    }
//
//    std::ostringstream ostr_stream;
//    TCP_Svc_Handler::dump_svcpeer_info(ostr_stream, startno, numquery);
//    tmp_size += snprintf(ret_string + tmp_size, str_len - tmp_size, "%s", ostr_stream.str().c_str());
//
//    str_len = tmp_size;
//
//    return 0;
//}
//
////ֹͣ����
//int Zerg_Console_Handler::cmd_close_services(char *ret_string, size_t &str_len)
//{
//    Zerg_Service_App::instance()->set_run_sign(false);
//
//    size_t tmp_size = snprintf(ret_string, str_len, "Console close services. Server exit at immediately,Please wait.");
//    str_len = tmp_size;
//    ZCE_LOG(RS_INFO,"[zergsvr] %s", ret_string);
//
//    return 0;
//}
//
////
//int Zerg_Console_Handler::cmd_close_socketpeer(char *ret_string, size_t &str_len)
//{
//    int ret = 0;
//    size_t tmp_size = 0;
//    SERVICES_ID svr_info;
//    //��ȡSTARTNO����
//    std::string tmpstr;
//    ret = console_command_.GetCmdParameter("SVCTYPE", tmpstr);
//
//    //ʹ��Ĭ�ϲ���
//    if (ret != 0)
//    {
//        return ret;
//    }
//
//    svr_info.services_type_ = static_cast<unsigned short>( atoi(tmpstr.c_str()));
//
//    ret = console_command_.GetCmdParameter("SVCID", tmpstr);
//
//    //ʹ��Ĭ�ϲ���
//    if (ret != 0)
//    {
//        return ret;
//    }
//
//    svr_info.services_id_ = atoi(tmpstr.c_str());
//
//    ZCE_LOG(RS_INFO,"[zergsvr] Console To close One Services:%u|%u", svr_info.services_id_, svr_info.services_type_);
//    ret = TCP_Svc_Handler::close_connect_services_peer(svr_info);
//
//    tmp_size += snprintf(ret_string + tmp_size, str_len - tmp_size, "Console To close One Services: %u|%u %s.", svr_info.services_type_, svr_info.services_id_,
//                         (ret == 0) ? "Success" : "Fail" );
//
//    str_len = tmp_size;
//
//    return 0;
//}
//
///******************************************************************************************
//Author          : Sail ZENGXING  Date Of Creation: 2006��10��17��
//Function        : Zerg_Console_Handler::cmd_modify_logpriority
//Return          : int
//Parameter List  :
//  Param1: char* ret_string
//  Param2: size_t& str_len
//Description     :
//Calls           :
//Called By       :
//Other           : MODIFY LOG:PRIORITY=?;  PRIORITYΪ��־�����ȼ���,
//Modify Record   :
//******************************************************************************************/
//int Zerg_Console_Handler::cmd_modify_logpriority(char *ret_string, size_t &str_len)
//{
//    int ret = 0;
//    std::string tmpstr;
//    ret = console_command_.GetCmdParameter("PRIORITY", tmpstr);
//
//    if (ret != 0)
//    {
//        return ret;
//    }
//
//    ZCE_LOG_PRIORITY new_log_priority =  zce_LogTrace_Basic::log_priorities(tmpstr.c_str());
//    ZCE_LOG_PRIORITY old_log_priority = Zerg_Service_App::instance()->get_log_priority();
//    //�޸�APPL����־����
//    Zerg_Service_App::instance()->set_log_priority(new_log_priority);
//
//    size_t tmp_size = 0;
//    tmp_size += snprintf(ret_string + tmp_size, str_len - tmp_size, "Console modify log priority.Old log priority %u, New log priority %u .", old_log_priority, new_log_priority);
//    str_len = tmp_size;
//    ZCE_LOG(RS_INFO,"[zergsvr] %s", ret_string);
//
//    return 0;
//}
//
