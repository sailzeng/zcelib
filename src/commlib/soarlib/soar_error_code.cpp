#include "soar_predefine.h"
#include "soar_error_code.h"

///
const char *SOAR_RET::error_string(int error_id)
{
    switch (error_id)
    {
        case FAIL_INTERNAL_ERORR:
            return "Internal system error.";
        case ERROR_PROCESS_MYSQLDB:
            return  "Commlib: ERROR_PROCESS_MYSQLDB.";
        case ERROR_PROCESS_SQLITE:
            return  "Commlib: ERROR_PROCESS_SQLITE.";
        case ERROR_INIT_RECEIVE_PIPE:
            return  "Commlib: Create Receive Pipe Error.";
        case ERROR_INIT_SEND_PIPE:
            return  "Commlib: Create Send Pipe Error.";
        case ERROR_INIT_ERROR_PIPE:
            return  "Commlib: Create Error Pipe Error.";
        case ERROR_PIPE_IS_FULL:
            return  "Commlib: Pipe is full.";
        case ERROR_PIPE_IS_EMPTY:
            return  "Commlib: Pipe is empty.";
        case ERROR_GET_CFGFILE_CONFIG_FAIL:
            return  "Commlib: Get Config file configure error.";
        case ERROR_GET_AUTO_CONN_CONFIG_FAIL:
            return  "Commlib: Get Auto Connect config error.";
        case ERROR_NO_FOUND_DATA_FROM_DB:
            return  "Commlib: Not found any data from db.";
        case ERROR_MMAP_MALLOC_FAIL:
            return  "Commlib: MMAP malloc fail.";
        case ERROR_FIND_REPEAT_CMD_ID:
            return  "Commlib: Transaction Manager Find repeat command id.";
        case ERROR_FIND_REPEAT_TRANSACTION_ID:
            return  "Commlib: Find Repeat Transaction ID.";
        case ERROR_CANNOT_FIND_TRANSACTION_ID:
            return  "Commlib: Can't Find Transaction ID.";
        case ERROR_APPFRAME_BUFFER_SHORT:
            return  "Commlib: APPFRAME buffer too short.";
        case ERROR_CDR_ENCODE_FAIL:
            return  "Commlib: CDR encode fail.";
        case ERROR_CDR_DECODE_FAIL:
            return  "Commlib: CDR decode fail.";
        case ERROR_INVALID_DATA_ID:
            return  "Commlib: Invalid data id.";
        case ERROR_SERVICES_INFO_CONFIG:
            return  "Commlib: Can't find Services info.";
        case ERROR_MML_ERROR:
            return  "Commlib: MML error.";
        case ERROR_MML_STRING_PARSE_FAIL:
            return  "Commlib: Parse MML String fail.";
        case ERROR_CONSOLE_IS_CLOSED:
            return  "Commlib: Console is closed.";
        case ERROR_CONSOLE_START_FAIL:
            return  "Commlib: Console start fail.";
        case ERROR_EVENT_NOTIFY_FAIL:
            return  "Commlib: Event notify fail.";
        case ERROR_EVENT_PROCESS_FAIL:
            return  "Commlib: Event Process fail.";
        case ERROR_FRAME_DATA_IS_ERROR:
            return  "Commlib: Frame Data is error.";
        case ERROR_QQUIN_INCERTITUDE:
            return  "Commlib: QQUin incertitude.";
        case ERROR_FRAME_NOT_TAIL_IPINFO:
            return  "Commlib: Appframe don't have ip info.";
        case ERROR_APPFRAME_ERROR:
            return  "Commlib: Appframe error.";
        case ERROR_APPFRAME_ENCRYPT_DECRYPT:
            return  "Commlib: Appframe encrypt or decrypt error.";
        case ERROR_OCCUR_EXCEPTION:
            return  "Commlib: ERROR_OCCUR_EXCEPTION.";
        case ERROR_FORK_FAIL:
            return  "Commlib: ERROR_FORK_FAIL";
        case ERROR_STARTUP_TIP:
            return  "Commlib: ERROR_STARTUP_TIP";
        case ERROR_GET_STARTUP_CONFIG_FAIL:
            return  "Commlib: ERROR_GET_STARTUP_CONFIG_FAIL";
        case ERROR_WRITE_ERROR_PIDFILE:
            return  "Commlib: ERROR_WRITE_ERROR_PIDFILE";
        case ERROR_INIT_TIMER_QUEUE:
            return  "Commlib: ERROR_INIT_TIMER_QUEUE";
        case ERROR_NOT_FIND_SERVICES_INFO:
            return  "Commlib: ERROR_NOT_FIND_SERVICES_INFO";
        case ERROR_INIT_DB_CONFIG:
            return  "Commlib: ERROR_INIT_DB_CONFIG";
        case ERROR_GET_CONFIG_FROM_DB:
            return  "Commlib: ERROR_CET_CONFIG_FROM_DB";
        case ERROR_WRITE_PID_FILE_FAIL:
            return  "Commlib: ERROR_WRITE_PID_FILE_FAIL";
        case ERROR_DIR_REPORT_INIT_ERROR:
            return  "Commlib: ERROR_DIR_REPORT_INIT_ERROR";
        case ERROR_NOT_FIND_FUNC:
            return  "Commlib: ERROR_NOT_FIND_FUNC";
        case ERROR_RECORD_LOG_FAIL_BUFFER_LACK:
            return  "Commlib: ERROR_RECORD_LOG_FAIL_BUFFER_LACK";
        case ERROR_NAVIGATION_TREE_XML_ERROR:
            return  "Commlib: ERROR_NAVIGATION_TREE_XML_ERROR";
        case ERROR_NAVIGATION_TREE_PARSE_FAIL:
            return  "Commlib: ERROR_NAVIGATION_TREE_PARSE_FAIL";
        case ERROR_NOT_FIND_NAVTREE_NODE:
            return  "Commlib: ERROR_NOT_FIND_NAVTREE_NODE";
        case ERROR_APPFRAME_SESSION:
            return  "Commlib: Appframe is session frame";
        case ERROR_DIRTY_KEYWORD:
            return  "Commlib: ERROR_DIRTY_KEYWORD";
        case ERROR_MMAP_CACHE_ERROR:
            return  "Commlib: ERROR_MMAP_CACHE_ERROR";
        case ERROR_REPEAT_PROCESS_START:
            return  "Commlib: ERROR_REPEAT_PROCESS_START";
        case ERROR_PROCESS_MUTEX_ERROR:
            return  "Commlib: ERROR_PROCESS_MUTEX_ERROR";
        case ERROR_NAVTREE_XML_FILE_FAIL:
            return  "Commlib:ERROR_NAVTREE_XML_FILE_FAIL";
        case ERROR_INVALID_QQ_NUMBER:
            return  "Commlib:ERROR_INVALID_QQ_NUMBER";
        case ERROR_INVALID_SOARING_LIB_TYPE:
            return  "Commlib:ERROR_INVALID_SOARING_LIB_TYPE";
        case ERROR_INVALID_INTERNAL_IPADDRESS:
            return  "Commlib:ERROR_INVALID_INTERNAL_IPADDRESS";
        case ERROR_MML_CONSOLE_FAIL:
            return  "ERROR_MML_CONSOLE_FAIL";
        case ERROR_LOAD_DLL_OR_SO_FAIL:
            return  "ERROR_LOAD_DLL_OR_SO_FAIL";
        case ERROR_APPFRAME_SESSION_EXPIRE:
            return  "ERROR_APPFRAME_SESSION_EXPIRE";
        case ERROR_OGRE_FRAME_ERROR:
            return  "ERROR_OGRE_FRAME_ERROR";
        case ERROR_TRANS_NO_CLONE_COMMAND:
            return  "ERROR_TRANS_NO_CLONE_COMMAND";
        case ERROR_TRANS_HAS_FINISHED:
            return  "ERROR_TRANS_HAS_FINISHED";
        case ERROR_NOTIFY_SEND_QUEUE_DEQUEUE_FAIL:
            return  "ERROR_NOTIFY_SEND_QUEUE_DEQUEUE_FAIL";
        case ERROR_NOTIFY_RECV_QUEUE_DEQUEUE_FAIL:
            return  "ERROR_NOTIFY_RECV_QUEUE_DEQUEUE_FAIL";
        case ERROR_NOTIFY_SEND_QUEUE_ENQUEUE_FAIL:
            return  "ERROR_NOTIFY_SEND_QUEUE_ENQUEUE_FAIL";
        case ERROR_NOTIFY_RECV_QUEUE_ENQUEUE_FAIL:
            return  "ERROR_NOTIFY_RECV_QUEUE_ENQUEUE_FAIL";
        case ERROR_ZULU_INPUT_IPADDRESS_ERROR:
            return  "ERROR_ZULU_INPUT_IPADDRESS_ERROR";
        case ERROR_ZULU_CONNECT_SERVER_FAIL:
            return  "ERROR_ZULU_CONNECT_SERVER_FAIL";
        case ERROR_ZULU_SEND_PACKAGE_FAIL:
            return  "ERROR_ZULU_SEND_PACKAGE_FAIL";
        case ERROR_ZULU_RECEIVE_PACKAGE_FAIL:
            return  "ERROR_ZULU_RECEIVE_PACKAGE_FAIL";
        case ERROR_ZULU_RECEIVE_OTHERS_COMMAND:
            return  "ERROR_ZULU_RECEIVE_OTHERS_COMMAND";
        case ERROR_MML_COMMAND_INVALID:
            return  "ERROR_MML_COMMAND_INVALID";
        case ERROR_LOTTERY_RATIO_TOTAL:
            return  "ERROR_LOTTERY_RATIO_TOTAL";
        case ERROR_TRANSACTION_NEED_CMD_ERROR:
            return  "ERROR_TRANSACTION_NEED_CMD_ERROR";
        case ERROR_SQLITE3_SQL_SYNTAX_ERROR:
            return  "ERROR_SQLITE3_SQL_SYNTAX_ERROR";
        case ERROR_SQLITE3_OPEN_DATABASE:
            return  "ERROR_SQLITE3_OPEN_DATABASE";
        case ERROR_BOOL_EXPRESSION_ERROR:
            return  "ERROR_BOOL_EXPRESSION_ERROR";
        case ERROR_BOOL_EXPRESSION_HAVE_ERR_CHAR:
            return  "ERROR_BOOL_EXPRESSION_HAVE_ERR_CHAR";
        case ERROR_TASKID_PHASEID_NOT_MATCHING:
            return  "ERROR_TASKID_PHASEID_NOT_MATCHING";
        case ERROR_SINGLE_SELECT_USER_INPUT_ERROR:
            return  "ERROR_SINGLE_SELECT_USER_INPUT_ERROR";
        case ERROR_TASK_ID_EQUAL_ZERO:
            return  "ERROR_TASK_ID_EQUAL_ZERO";
        case ERROR_CHECK_CONDITION_ERROR:
            return  "ERROR_CHECK_CONDITION_ERROR";
        case ERROR_GETTASK_CONDITION_RRROR:
            return  "ERROR_GETTASK_CONDITION_RRROR";
        case ERROR_ILLUSION_STRING_TO_ID_FAIL:
            return  "ERROR_ILLUSION_STRING_TO_ID_FAIL";
        case ERROR_ILLUSION_CFG_COULD_HAVE_MODIFY:
            return  "ERROR_ILLUSION_CFG_COULD_HAVE_MODIFY";
        case ERROR_ILLUSION_EXCEL_FILE_ERROR:
            return  "ERROR_ILLUSION_EXCEL_FILE_ERROR";
        case ERROR_ZULU_LINK_DISCONNECT:
            return  "ERROR_ZULU_LINK_DISCONNECT";
        case ERROR_LOLO_SEND_RECV_CONN_NOT_SAME:
            return  "ERROR_LOLO_SEND_RECV_CONN_NOT_SAME";
        case ERROR_COMMLIB_END:
            return  "";

            // proxy
        case ERR_PROXY_BEGIN:
            return  "";
        case ERR_PROXY_APPFRAME_CMD_ERROR:
            return  "ERR_PROXY_APPFRAME_CMD_ERROR";
        case ERR_PROXY_SEND_PIPE_IS_FULL:
            return  "ERR_PROXY_SEND_PIPE_IS_FULL";
        case ERR_PROXY_RECEIVE_ERROR_FRAME:
            return  "ERR_PROXY_RECEIVE_ERROR_FRAME";
        case ERR_PROXY_END:
            return  "";

            // zergsvr
        case ERR_ZERG_FAIL_SOCKET_OP_ERROR:
            return  "Socket return a error.";
        case ERR_ZERG_GREATER_MAX_LEN_FRAME:
            return  "Appframe length is greater than max length.";
        case ERR_ZERG_GREATER_DEFINE_LEN_FRAME:
            return  "Appframe length is greater than define length.";
        case ERR_ZERG_SOCKET_CLOSE:
            return  "Socket is closed";
        case ERR_ZERG_SERVER_ALREADY_LONGIN:
            return  "Services already login in this server.";
        case ERR_ZERG_CONNECT_NO_FIND_SVCINFO:
            return  "Connect not Find this services info.";
        case ERR_ZERG_INIT_RECEIVE_PIPE:
            return  "Create Receive Pipe Error.";
        case ERR_ZERG_INIT_SEND_PIPE:
            return  "Create Send Pipe Error.";
        case ERR_ZERG_INIT_ERR_ZERG_PIPE:
            return  "Create Error Pipe Error.";
        case ERR_ZERG_NO_FIND_EVENT_HANDLE:
            return  "Not Find Handle with services info.";
        case ERR_ZERG_RECEIVE_PIPE_IS_FULL:
            return  "Recveive Pipe full.";
        case ERR_ZERG_SEND_PIPE_IS_FULL:
            return  "Send Pipe full.";
        case ERR_ZERG_ERROR_PIPE_IS_FULL:
            return  "Error Pipe full.";
        case ERR_ZERG_GREAT_THAN_ERROR_TRY:
            return  "Up to Max error try send.";
        case ERR_ZERG_MMAP_MALLOC_FAIL:
            return  "Alloc MMAP memory Fail.";
        case ERR_ZERG_INIT_ACCEPT_SOCKET_FAIL:
            return  "Init Accept Socket Fail.";
        case ERR_ZERG_GET_STARTUP_CONFIG_FAIL:
            return  "Get Startup Config Fail.";
        case ERR_ZERG_GET_CFGFILE_CONFIG_FAIL:
            return  "Get Config file configure error.";
        case ERR_ZERG_GET_AUTO_CONN_CONFIG_FAIL:
            return  "Get Auto Connect config error.";
        case ERR_ZERG_OCCUR_EXCEPTION:
            return  "Occur exception.";
        case ERR_ZERG_APPFRAME_ERROR:
            return  "Appframe error.";
        case ERR_ZERG_INIT_LISTEN_PORT_FAIL:
            return  "Init Accept Socket Peer fail.";
        case ERR_ZERG_UNSAFE_PORT_WARN:
            return  "Unsafe port,if you need to open this port,please close insurance.";
        case ERR_ZERG_INIT_UPD_PORT_FAIL:
            return  "Init UDP Port fail.";
        case ERR_ZERG_IP_RESTRICT_CHECK_FAIL:
            return  "IP Restrict check fail";
        case ERR_ZERG_UDPSESSION_NOFOUND:
            return  "ERR_ZERG_UDPSESSION_NOFOUND";
        case ERR_ZERG_UDPSESSION_FULL:
            return  "ERR_ZERG_UDPSESSION_FULL";
        case ERR_ZERG_LOAD_OPERATION_KEY_FAIL:
            return  "ERR_ZERG_LOAD_OPERATION_KEY_FAIL";

        default:
            return "";
    }
}