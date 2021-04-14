/*
DROP TABLE IF EXISTS benchmark.test1;
CREATE TABLE IF NOT EXISTS benchmark.test1
(
F1      INT   NOT NULL ,
F2      INT   NOT NULL DEFAULT 0,
F3      INT   NOT NULL DEFAULT 0,
F4      INT   NOT NULL DEFAULT 0,
F5      INT   NOT NULL DEFAULT 0,
F6      INT   NOT NULL DEFAULT 0,
F7      INT   NOT NULL DEFAULT 0,
F8      INT   NOT NULL DEFAULT 0,
F9      INT   NOT NULL DEFAULT 0,
F10     INT   NOT NULL DEFAULT 0,
F11     FLOAT(10,4) NOT NULL DEFAULT 0,
F12     DOUBLE(10,4) NOT NULL DEFAULT 0,
F13     VARCHAR(64) NOT NULL DEFAULT '',
F14     DATETIME   NOT NULL DEFAULT '0',
PRIMARY KEY (F1)
);
*/


#include "zealot_predefine.h"


const static int  RETURN_SUCC     = 0;
const static int  RETURN_DB_ERROR = -1;

//社团数据库
const char *DB_BENCHMARK_IP       = "172.16.55.40";
//社团数据库用户
const char *DB_BENCHMARK_USER     = "root";
////社团数据库用户密码
const char *DB_BENCHMARK_PASSWORD = "";

//MYSQL数据库连接对象
static ZCE_Mysql_Connect g_db_connect;

//MYSQL命令执行对象
static ZCE_Mysql_Command g_db_command;



int benchmark_db_query(const char *sql, uint64_t &numaffect, uint64_t &insertid, char *szErr)
{
    int ret = 0;
    //连接数据库
    if (g_db_connect.is_connected() == false)
    {
        ret = g_db_connect.connect_by_host(DB_BENCHMARK_IP, DB_BENCHMARK_USER, DB_BENCHMARK_PASSWORD);

        //如果错误
        if (ret != 0)
        {
            sprintf(szErr, "[%d]:%s", g_db_connect.get_error_no(), g_db_connect.get_error_message());
            return RETURN_DB_ERROR;
        }
        //
        g_db_command.set_connection(&g_db_connect);
    }
    //如果已经连接过数据库,则不用再次连接,ping一次就OK了,成本低
    else
    {
        g_db_connect.ping();
    }

    g_db_command.set_sql_command(sql, strlen(sql));
    ret = g_db_command.execute(numaffect, insertid);
    //如果错误
    if (ret != 0)
    {
        sprintf(szErr, "[%d]:%s", g_db_connect.get_error_no(), g_db_connect.get_error_message());
        return RETURN_DB_ERROR;
    }

    //成功
    return  RETURN_SUCC;
}


int benchmark_db_query(const char *sql, uint64_t &numaffect, ZCE_Mysql_Result &dbresult, char *szErr)
{
    int ret = 0;
    //连接数据库
    if (g_db_connect.is_connected() == false)
    {
        ret = g_db_connect.connect_by_host(DB_BENCHMARK_IP, DB_BENCHMARK_USER, DB_BENCHMARK_PASSWORD);

        //如果错误
        if (ret != 0)
        {
            sprintf(szErr, "[%d]:%s", g_db_connect.get_error_no(), g_db_connect.get_error_message());
            return RETURN_DB_ERROR;
        }
        //
        g_db_command.set_connection(&g_db_connect);
    }
    //如果已经连接过数据库,则不用再次连接,ping一次就OK了,成本低
    else
    {
        g_db_connect.ping();
    }
    g_db_command.set_sql_command(sql, strlen(sql));
    ret = g_db_command.execute(numaffect, dbresult);
    //如果错误
    if (ret != 0)
    {
        sprintf(szErr, "[%d]:%s", g_db_connect.get_error_no(), g_db_connect.get_error_message());
        return RETURN_DB_ERROR;
    }

    //成功
    return  RETURN_SUCC;
}

int benchmark_db_query(const char *sql, ZCE_Mysql_Result &dbresult, char *szErr)
{
    int ret = 0;
    //连接数据库
    if (g_db_connect.is_connected() == false)
    {
        ret = g_db_connect.connect_by_host(DB_BENCHMARK_IP, DB_BENCHMARK_USER, DB_BENCHMARK_PASSWORD);

        //如果错误
        if (ret != 0)
        {
            sprintf(szErr, "[%d]:%s", g_db_connect.get_error_no(), g_db_connect.get_error_message());
            return RETURN_DB_ERROR;
        }
        //
        g_db_command.set_connection(&g_db_connect);
    }
    //如果已经连接过数据库,则不用再次连接,ping一次就OK了,成本低
    else
    {
        g_db_connect.ping();
    }

    g_db_command.set_sql_command(sql, strlen(sql));
    ret = g_db_command.execute(dbresult);
    //如果错误
    if (ret != 0)
    {
        sprintf(szErr, "[%d]:%s", g_db_connect.get_error_no(), g_db_connect.get_error_message());
        return RETURN_DB_ERROR;
    }

    //成功
    return  RETURN_SUCC;
}



int benchmark_insert_record(int table_id, int id, bool bexcutesql, char *szErr)
{
    char tmpsql[1024];

    sprintf(tmpsql, "INSERT INTO benchmark.test%d SET "
            "F1=%d,"
            "F2=%d,"
            "F3=%d,"
            "F4=4,"
            "F5=5,"
            "F6=6,"
            "F7=7,"
            "F8=8,"
            "F9=9,"
            "F9=10,"
            "F11=11.11,"
            "F12=12.12,"
            "F13='%015d%015d%015d',"
            "F14='2006-06-01 00:00:00' ",
            table_id,
            id,
            id,
            id,
            id,
            id,
            id
           );
    //是否执行SQL,拼
    if (bexcutesql)
    {
        uint64_t numaffect, insertid;
        int ret = benchmark_db_query(tmpsql, numaffect, insertid, szErr);
        if (ret != RETURN_SUCC)
        {
            return ret;
        }

    }
    return RETURN_SUCC;
}

int benchmark_delete_record(int table_id, int id, bool bexcutesql, char *szErr)
{
    char tmpsql[1024];

    sprintf(tmpsql, "DELETE FROM benchmark.test%d WHERE F1=%d ",
            table_id,
            id
           );
    //是否执行SQL,拼
    if (bexcutesql)
    {
        uint64_t numaffect, insertid;
        int ret = benchmark_db_query(tmpsql, numaffect, insertid, szErr);
        if (ret != RETURN_SUCC)
        {
            return ret;
        }

    }
    return RETURN_SUCC;
}

int benchmark_select_record(int table_id, int id, bool bexcutesql, char *szErr)
{
    char tmpsql[1024];

    sprintf(tmpsql, "SELECT F1,F2,F3,F4,F5,F6,F7,F8,F9,F10,F11,F12,F13,F14 FROM benchmark.test%d "
            "WHERE F1=%d ",
            table_id,
            id
           );
    if (bexcutesql)
    {
        ZCE_Mysql_Result dbresult;
        uint64_t numaffect;
        //进行查询,
        int ret = benchmark_db_query(tmpsql, numaffect, dbresult, szErr);
        if (ret != RETURN_SUCC )
        {
            return ret;
        }
        //一个也没有查询到
        if (numaffect == 0)
        {
        }
    }

    return RETURN_SUCC;
}

int benchmark_cache_record(int table_id, int id, int numruncache, bool bexcutesql, char *szErr )
{
    for (int i = 0; i < numruncache; i++ )
    {
        int ret = benchmark_select_record(table_id, id, bexcutesql, szErr);
        if (ret != RETURN_SUCC)
        {
            return ret;
        }
    }
    return RETURN_SUCC;
}

enum BENCHMARK_TYPE
{
    BENCHMARK_INSERT,
    BENCHMARK_SELECT,
    BENCHMARK_CACHE,
    BENCHMARK_DELETE,
};

int test_sql_main (int argc, char *argv[])
{

    char szErr[256];
    BENCHMARK_TYPE benchmarktype = BENCHMARK_SELECT;

    int numprocess = 0;
    int numruncache = 0;
    int num_sleep  = 0;
    int table_id = 0;
    bool bexecute = false;


    ZCE_Get_Option get_opt (argc, argv, "dicsn:r:p:t:");
    int c;
    //Process the scanned options with the help of the overloaded ()
    //operator.

    while ((c = get_opt ()) != EOF)
    {
        switch (c)
        {
            case 'e':
                bexecute = false;
                break;
            case 'd':
                benchmarktype = BENCHMARK_DELETE;
                break;
            case 'i':
                benchmarktype = BENCHMARK_INSERT;
                break;
            case 'c':
                benchmarktype = BENCHMARK_CACHE;
                break;
            case 's':
                benchmarktype = BENCHMARK_SELECT;
                break;
            case 'n':
                numprocess = atoi(get_opt.optarg) ;
                break;
            case 'r':
                numruncache = atoi(get_opt.optarg) ;
                break;
            case 'p':
                num_sleep  = atoi(get_opt.optarg) ;
                break;
            case 't':
                table_id =  atoi(get_opt.optarg) ;
                break;
            default:
                break;
        }
    }

    if (numprocess == 0 || (numruncache == 0 && benchmarktype == BENCHMARK_CACHE))
    {
        return 0;
    }

    switch (benchmarktype)
    {
        case BENCHMARK_DELETE:
            for (int i = 0; i < numprocess; i++)
            {
                int ret = benchmark_delete_record(table_id, i, bexecute, szErr);
                if (ret > 0)
                {
                    return ret;
                }
            }
            break;
        case BENCHMARK_INSERT:
            for (int i = 0; i < numprocess; i++)
            {
                int ret = benchmark_insert_record(table_id, i, bexecute, szErr);
                if (ret > 0)
                {
                    return ret;
                }
            }
            break;
        case BENCHMARK_SELECT:
            for (int i = 0; i < numprocess; i++)
            {
                int ret = benchmark_select_record(table_id, i, bexecute, szErr);
                if (ret > 0)
                {
                    return ret;
                }
            }
            break;
        case BENCHMARK_CACHE:
            for (int i = 0; i < numprocess; i++)
            {
                int ret = benchmark_cache_record(table_id, i, numruncache, bexecute, szErr);
                if (ret > 0)
                {
                    return ret;
                }
            }
            break;
        default :
            break;
    }

    zce::sleep(num_sleep);

    return 0;
}