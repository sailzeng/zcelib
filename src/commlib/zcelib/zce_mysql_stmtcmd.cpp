#include "zce_predefine.h"
#include "zce_mysql_stmtcmd.h"
#include "zce_mysql_stmtbind.h"
#include "zce_mysql_result.h"

//这些函数都是4.1.2后的版本功能
#if MYSQL_VERSION_ID >= 40100

ZCE_Mysql_STMT_Command::ZCE_Mysql_STMT_Command():
    mysql_connect_(NULL),
    mysql_stmt_(NULL),
    is_bind_result_(false)
{
    //保留INITBUFSIZE的空间
    stmt_command_.reserve(INITBUFSIZE);
}

//指定一个connect
ZCE_Mysql_STMT_Command::ZCE_Mysql_STMT_Command(ZCE_Mysql_Connect *conn ):
    mysql_connect_(NULL),
    mysql_stmt_(NULL),
    is_bind_result_(false)
{

    //assert(conn != NULL);
    if (conn != NULL && conn->is_connected())
    {
        mysql_connect_ = conn;
        //处理
        mysql_stmt_ = mysql_stmt_init(mysql_connect_->get_mysql_handle());
    }

    //保留INITBUFSIZE的空间
    stmt_command_.reserve(INITBUFSIZE);
}

ZCE_Mysql_STMT_Command::~ZCE_Mysql_STMT_Command()
{
    if ( NULL  != mysql_stmt_)
    {
        int tmpret = mysql_stmt_free_result(mysql_stmt_);
        tmpret = mysql_stmt_close(mysql_stmt_);

    }
}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2005年10月17日
Function        : ZCE_Mysql_STMT_Command::set_connection
Return          : int
Parameter List  :
  Param1: ZCE_Mysql_Connect* conn 设置的链接
Description     :
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
int ZCE_Mysql_STMT_Command::set_connection(ZCE_Mysql_Connect *conn)
{
    //检查参数
    if (conn != NULL && conn->is_connected())
    {
        mysql_connect_ = conn;

        if (mysql_stmt_ != NULL)
        {
            int tmpret = mysql_stmt_close(mysql_stmt_);
            MYSQLSTMTCMDCHECK(tmpret == 0);
        }

        mysql_stmt_ = mysql_stmt_init(mysql_connect_->get_mysql_handle());

        return MYSQL_RETURN_FAIL;
    }

    //
    return MYSQL_RETURN_OK;
}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2005年10月17日
Function        : ZCE_Mysql_STMT_Command::stmt_prepare_bind
Return          : int
Parameter List  :
  Param1: ZCE_Mysql_STMT_Bind* bindparam  绑定的参数
  Param2: ZCE_Mysql_STMT_Bind* bindresult 绑定的结果
Description     : 准备SQL,并且分析绑定的变量
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
int ZCE_Mysql_STMT_Command::stmt_prepare_bind(ZCE_Mysql_STMT_Bind *bindparam, ZCE_Mysql_STMT_Bind *bindresult)
{

    int tmpret = mysql_stmt_prepare(mysql_stmt_, stmt_command_.c_str(), static_cast<unsigned long>(stmt_command_.size()));
    MYSQLSTMTCMDCHECK(tmpret == 0);

    is_bind_result_ = false;

    //原打算检查语句的绑定变量个数,后来决定还是让MySQL自己检查,
    //unsigned long paramcount = mysql_stmt_param_count(mysql_stmt_);
    //ZASSERT(paramcount > 0 &&  bindparam!=NULL || paramcount == 0 && bindparam == NULL);

    //绑定的参数
    if ( bindparam != NULL)
    {
        tmpret = mysql_stmt_bind_param(mysql_stmt_, bindparam->get_stmt_bind_handle());
        MYSQLSTMTCMDCHECK(tmpret == 0);
    }

    //绑定的结果
    if (bindresult)
    {
        tmpret = mysql_stmt_bind_result(mysql_stmt_, bindresult->get_stmt_bind_handle());
        //出错返回,或者处理
        MYSQLSTMTCMDCHECK(tmpret == 0);

        is_bind_result_ = true;
    }

    return MYSQL_RETURN_OK;
}

//设置SQL Command语句,为BIN型的SQL语句准备,同时绑定参数,结果
int ZCE_Mysql_STMT_Command::set_stmt_command(const std::string &sqlcmd, ZCE_Mysql_STMT_Bind *bindparam, ZCE_Mysql_STMT_Bind *bindresult)
{
    stmt_command_ = sqlcmd;
    return stmt_prepare_bind(bindparam, bindresult);
}

//设置SQL Command语句,为BIN型的SQL语句准备,用于要帮定变量的SQL,结果
int ZCE_Mysql_STMT_Command::set_stmt_command(const char *sqlcmd, size_t szsql, ZCE_Mysql_STMT_Bind *bindparam, ZCE_Mysql_STMT_Bind *bindresult)
{
    ZCE_ASSERT(sqlcmd != NULL);
    //
    stmt_command_.assign(sqlcmd, szsql);

    return stmt_prepare_bind(bindparam, bindresult);
}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2005年10月17日
Function        : ZCE_Mysql_STMT_Command::execute
Return          : int == MYSQL_RETURN_OK 0,表示成功
Parameter List  :
  Param1: unsigned int* num_affect    返回的影响记录条数
  Param2: unsigned int* lastid       返回的LASTID
  Param3: ZCE_Mysql_STMT_Bind* bindresult  绑定的返回结果,用于有查询结果的语句
Description     : 执行STMT SQL语句,基础函数,让大家共同调用的基础函数
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
int ZCE_Mysql_STMT_Command::execute(unsigned int *num_affect , unsigned int *lastid)
{
    int tmpret = 0;

    //执行
    tmpret = mysql_stmt_execute(mysql_stmt_);

    //检查命令执行结果,返回错误，或者抛出异常
    MYSQLSTMTCMDCHECK(tmpret == 0);

    //如果要返回结果,进行转储
    if (is_bind_result_)
    {
        tmpret = mysql_stmt_store_result(mysql_stmt_);
        //出错返回,或者处理
        MYSQLSTMTCMDCHECK(tmpret == 0);
    }

    //执行SQL命令影响了多少行,mysql_affected_rows 必须在转储结果集后,所以你要注意输入的参数
    if (num_affect)
    {
        *num_affect = (unsigned int) mysql_stmt_affected_rows(mysql_stmt_);
    }

    if (lastid)
    {
        *lastid = (unsigned int) mysql_stmt_insert_id(mysql_stmt_);
    }

    //成功
    return MYSQL_RETURN_OK;
}

//执行SQL语句,不用输出结果集合的那种
int ZCE_Mysql_STMT_Command::execute(unsigned int &num_affect, unsigned int &lastid)
{
    return execute(&num_affect , &lastid);
}

//执行SQL语句,SELECT语句,转储结果集合的那种,
int ZCE_Mysql_STMT_Command::execute(unsigned int &num_affect)
{
    return execute(&num_affect, NULL);
}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2005年10月17日
Function        : ZCE_Mysql_STMT_Command::fetch_row_next
Return          : int
Parameter List  : NULL
Description     :
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
int ZCE_Mysql_STMT_Command::fetch_row_next() const
{
    int tmpret = mysql_stmt_fetch(mysql_stmt_);

    if (tmpret != 0)
    {
        return MYSQL_RETURN_FAIL;
    }

    return MYSQL_RETURN_OK;

}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2005年10月17日
Function        : ZCE_Mysql_STMT_Command::fetch_column
Return          : int
Parameter List  :
  Param1: MYSQL_BIND* bind
  Param2: unsigned int column
  Param3: unsigned int offset
Description     :
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
int  ZCE_Mysql_STMT_Command::fetch_column(MYSQL_BIND *bind, unsigned int column, unsigned int offset) const
{
    int tmpret = mysql_stmt_fetch_column(mysql_stmt_, bind, column, offset);

    if (tmpret != 0)
    {
        return MYSQL_RETURN_FAIL;
    }

    return MYSQL_RETURN_OK;
}

//
int ZCE_Mysql_STMT_Command::SeekResultRow(unsigned int nrow) const
{
    //检查结果集合为空,或者参数row错误
    mysql_stmt_data_seek(mysql_stmt_, nrow);
    int tmpret  = mysql_stmt_fetch(mysql_stmt_);

    MYSQLSTMTCMDCHECK(tmpret == 0);

    return MYSQL_RETURN_OK;
}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2005年10月17日
Function        : ZCE_Mysql_STMT_Command::ParamToMetadata
Return          : void
Parameter List  :
  Param1: ZCE_Mysql_Result* tmpres
Description     :
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
void ZCE_Mysql_STMT_Command::ParamToMetadata(ZCE_Mysql_Result *tmpres) const
{
    MYSQL_RES *myres =  mysql_stmt_param_metadata(mysql_stmt_);
    tmpres->set_mysql_result(myres);
    return ;
}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2005年10月17日
Function        : ZCE_Mysql_STMT_Command::ResultToMetadata
Return          : void
Parameter List  :
  Param1: ZCE_Mysql_Result* tmpres
Description     :
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
void ZCE_Mysql_STMT_Command::ResultToMetadata(ZCE_Mysql_Result *tmpres) const
{
    MYSQL_RES *myres =  mysql_stmt_result_metadata(mysql_stmt_);
    tmpres->set_mysql_result(myres);
    return ;
}

#endif //MYSQL_VERSION_ID >= 40100

