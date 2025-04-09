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

#include "predefine.h"

const static int  RETURN_SUCC = 0;
const static int  RETURN_DB_ERROR = -1;

//社团数据库
const char* DB_BENCHMARK_IP = "127.0.0.1";
//社团数据库用户
const char* DB_BENCHMARK_USER = "root";
////社团数据库用户密码
const char* DB_BENCHMARK_PASSWORD = "";

struct TEST1_TABLE
{
    int f1_;
    int f2_;
    int f3_;
    int f4_;
    int f5_;
    int f6_;
    int f7_;
    int f8_;
    int f9_;
    int f10_;
    float f11_;
    double f12_;
    std::string f13_;
    MYSQL_TIME f14_;
};

//MYSQL数据库连接对象
static zce::mysql::connect g_db_connect;

int benchmark_db_query(const char* sql, size_t& num_affect, uint64_t* insert_id, char* szErr)
{
    int ret = 0;
    //连接数据库
    if (g_db_connect.is_connected() == false)
    {
        ret = g_db_connect.connect_by_host(DB_BENCHMARK_IP,
                                           MYSQL_PORT,
                                           DB_BENCHMARK_USER,
                                           DB_BENCHMARK_PASSWORD);

        //如果错误
        if (ret != 0)
        {
            sprintf(szErr, "[%d]:%s", g_db_connect.error_no(), g_db_connect.error_message());
            return RETURN_DB_ERROR;
        }
    }
    //如果已经连接过数据库,则不用再次连接,ping一次就OK了,成本低
    else
    {
        g_db_connect.ping();
    }
    std::string_view sql_str(sql);
    zce::mysql::command cmd(g_db_connect);
    ret = cmd.execute(sql_str, num_affect, insert_id);
    //如果错误
    if (ret != 0)
    {
        sprintf(szErr, "[%d]:%s", g_db_connect.error_no(), g_db_connect.error_message());
        return RETURN_DB_ERROR;
    }

    //成功
    return  RETURN_SUCC;
}

int benchmark_db_query(const char* sql,
                       uint64_t& num_affect,
                       zce::mysql::result& db_result,
                       char* szErr)
{
    int ret = 0;
    //连接数据库
    if (g_db_connect.is_connected() == false)
    {
        ret = g_db_connect.connect_by_host(DB_BENCHMARK_IP,
                                           MYSQL_PORT,
                                           DB_BENCHMARK_USER,
                                           DB_BENCHMARK_PASSWORD);

        //如果错误
        if (ret != 0)
        {
            sprintf(szErr, "[%d]:%s", g_db_connect.error_no(), g_db_connect.error_message());
            return RETURN_DB_ERROR;
        }
        //
    }
    //如果已经连接过数据库,则不用再次连接,ping一次就OK了,成本低
    else
    {
        g_db_connect.ping();
    }
    std::string_view sql_str(sql);
    zce::mysql::command cmd(g_db_connect);
    ret = cmd.execute(sql_str, num_affect, db_result);
    //如果错误
    if (ret != 0)
    {
        sprintf(szErr, "[%d]:%s", g_db_connect.error_no(), g_db_connect.error_message());
        return RETURN_DB_ERROR;
    }

    //成功
    return  RETURN_SUCC;
}

int benchmark_db_query(const char* sql,
                       zce::mysql::result& db_result,
                       char* szErr)
{
    int ret = 0;
    //连接数据库
    if (g_db_connect.is_connected() == false)
    {
        ret = g_db_connect.connect_by_host(DB_BENCHMARK_IP,
                                           MYSQL_PORT,
                                           DB_BENCHMARK_USER,
                                           DB_BENCHMARK_PASSWORD);

        //如果错误
        if (ret != 0)
        {
            sprintf(szErr, "[%d]:%s", g_db_connect.error_no(), g_db_connect.error_message());
            return RETURN_DB_ERROR;
        }
    }
    //如果已经连接过数据库,则不用再次连接,ping一次就OK了,成本低
    else
    {
        g_db_connect.ping();
    }
    std::string_view sql_str(sql);
    zce::mysql::command cmd(g_db_connect);
    ret = cmd.execute(sql_str);
    //如果错误
    if (ret != 0)
    {
        sprintf(szErr, "[%d]:%s", g_db_connect.error_no(), g_db_connect.error_message());
        return RETURN_DB_ERROR;
    }
    ret = cmd.get_result(0, 0, &db_result, true);
    if (ret != 0)
    {
        sprintf(szErr, "[%d]:%s", g_db_connect.error_no(), g_db_connect.error_message());
        return RETURN_DB_ERROR;
    }
    return  RETURN_SUCC;
}

int benchmark_insert_record(int table_id, int id, bool bexcutesql, char* szErr)
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
        size_t num_affect = 0; uint64_t insert_id = 0;
        int ret = benchmark_db_query(tmpsql, num_affect, &insert_id, szErr);
        if (ret != RETURN_SUCC)
        {
            return ret;
        }
    }
    return RETURN_SUCC;
}

int benchmark_delete_record(int table_id, int id, bool bexcutesql, char* szErr)
{
    char tmpsql[1024];

    sprintf(tmpsql, "DELETE FROM benchmark.test%d WHERE F1=%d ",
            table_id,
            id
    );
    //是否执行SQL,拼
    if (bexcutesql)
    {
        size_t num_affect = 0;
        uint64_t insert_id = 0;
        int ret = benchmark_db_query(tmpsql, num_affect, &insert_id, szErr);
        if (ret != RETURN_SUCC)
        {
            return ret;
        }
    }
    return RETURN_SUCC;
}

int benchmark_select_record(int table_id,
                            int id,
                            bool bexcutesql,
                            char* szErr)
{
    char tmpsql[1024];

    sprintf(tmpsql, "SELECT F1,F2,F3,F4,F5,F6,F7,F8,F9,F10,F11,F12,F13,F14 FROM benchmark.test%d "
            "WHERE F1=%d ",
            table_id,
            id
    );
    if (bexcutesql)
    {
        zce::mysql::result db_result;
        uint64_t num_affect;
        //进行查询,
        int ret = benchmark_db_query(tmpsql, num_affect, db_result, szErr);
        if (ret != RETURN_SUCC)
        {
            return ret;
        }
        //一个也没有查询到
        if (num_affect == 0)
        {
            return -1;
        }
        size_t rowid = 0;
        while (db_result.cursor_fetch())
        {
            TEST1_TABLE r1;
            db_result.field(rowid, 0, r1.f1_);
            db_result.field(rowid, 1, r1.f2_);
            db_result.field(rowid, 2, r1.f3_);
            db_result.field(rowid, 3, r1.f4_);
            db_result.field(rowid, 4, r1.f5_);
            db_result.field(rowid, 5, r1.f6_);
            db_result.field(rowid, 6, r1.f7_);
            db_result.field(rowid, 7, r1.f8_);
            db_result.field(rowid, 8, r1.f9_);
            db_result.field(rowid, 9, r1.f10_);
            db_result.field(rowid, 10, r1.f11_);
            db_result.field(rowid, 11, r1.f12_);
            db_result.field(rowid, 12, r1.f13_);
            db_result.field(rowid, 13, r1.f14_);
            rowid++;

            db_result.cursor_field(0, r1.f1_);
            db_result.cursor_field(1, r1.f2_);
            db_result.cursor_field(2, r1.f3_);
            db_result.cursor_field(3, r1.f4_);
            db_result.cursor_field(4, r1.f5_);
            db_result.cursor_field(5, r1.f6_);
            db_result.cursor_field(6, r1.f7_);
            db_result.cursor_field(7, r1.f8_);
            db_result.cursor_field(8, r1.f9_);
            db_result.cursor_field(9, r1.f10_);
            db_result.cursor_field(10, r1.f11_);
            db_result.cursor_field(11, r1.f12_);
            db_result.cursor_field(12, r1.f13_);
            db_result.cursor_field(13, r1.f14_);

            //[r1.f1_, r1.f2_, r1.f3_, r1.f4_, r1.f5_, r1.f6_, r1.f7_, r1.f8_, r1.f9_, r1.f10_, r1.f11_, r1.f12_, r1.f13_, r1.f14_]
            auto tuple_r = db_result.make_tuple<int, int, int, int, int, int, int, int, int, int, float, double, std::string, MYSQL_TIME>(rowid);
        }
    }
    return RETURN_SUCC;
}

int benchmark_cache_record(int table_id,
                           int id,
                           int numruncache,
                           bool bexcutesql,
                           char* szErr)
{
    for (int i = 0; i < numruncache; i++)
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

int test_sql_main(int argc, char* argv[])
{
    char sz_err[256];
    BENCHMARK_TYPE benchmarktype = BENCHMARK_SELECT;

    int numprocess = 0;
    int numruncache = 0;
    int num_sleep = 0;
    int table_id = 0;
    bool bexecute = false;

    zce::get_option get_opt(argc, argv, "dicsn:r:p:t:");
    int c;
    //Process the scanned options with the help of the overloaded ()
    //operator.

    while ((c = get_opt()) != EOF)
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
            numprocess = atoi(get_opt.optarg);
            break;
        case 'r':
            numruncache = atoi(get_opt.optarg);
            break;
        case 'p':
            num_sleep = atoi(get_opt.optarg);
            break;
        case 't':
            table_id = atoi(get_opt.optarg);
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
            int ret = benchmark_delete_record(table_id, i, bexecute, sz_err);
            if (ret > 0)
            {
                return ret;
            }
        }
        break;
    case BENCHMARK_INSERT:
        for (int i = 0; i < numprocess; i++)
        {
            int ret = benchmark_insert_record(table_id, i, bexecute, sz_err);
            if (ret > 0)
            {
                return ret;
            }
        }
        break;
    case BENCHMARK_SELECT:
        for (int i = 0; i < numprocess; i++)
        {
            int ret = benchmark_select_record(table_id, i, bexecute, sz_err);
            if (ret > 0)
            {
                return ret;
            }
        }
        break;
    case BENCHMARK_CACHE:
        for (int i = 0; i < numprocess; i++)
        {
            int ret = benchmark_cache_record(table_id,
                                             i,
                                             numruncache,
                                             bexecute,
                                             sz_err);
            if (ret > 0)
            {
                return ret;
            }
        }
        break;
    default:
        break;
    }

    zce::sleep(num_sleep);

    return 0;
}
