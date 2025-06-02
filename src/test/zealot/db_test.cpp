#include "predefine.h"

namespace zeal
{
enum DBType
{
    DB_UNKNOWN = -1,
    DB_MYSQL = 0,
    DB_PQ = 1,
    DB_SQLITE = 2,
};

const char* DB_HOST[] =
{
    "127.0.0.1",
    "127.0.0.1",
    "localhost"
};

const char* DB_USER[] =
{
    "mysql",
    "postgres",
    "-"
};

const char* DB_PWD[] =
{
    "mysql",
    "postgres",
    "-"
};

int DB_PORT[] =
{
    3306, // MySQL
    5432, // PostgreSQL
    0     // SQLite does not use port
};

const std::string DB_URL[] =
{
    "mysql://mysql:mysql@127.0.0.1",
    "postgresql://postgres:postgres@127.0.0.1",
    "sqlite://localhost/db/134.db?createdb=true"
};

const std::string CREATE_DATABASE[] = {
    "CREATE DATABASE IF NOT EXISTS zce",
    "CREATE SCHEMA IF NOT EXISTS zce",
    "-"
};

const std::string DROP_TABLE[] = {
    R"(DROP TABLE IF EXISTS zce.test;)",
    R"(DROP TABLE IF EXISTS zce.test;)",
    R"(DROP TABLE IF EXISTS test;)",
};
const std::string CREATE_TABLE[] = {
R"(CREATE TABLE IF NOT EXISTS zce.test
(
    F1     INT UNSIGNED  NOT NULL AUTO_INCREMENT,
    F2     SMALLINT NOT NULL DEFAULT 0,
    F3     INT   NOT NULL DEFAULT 0,
    F4     INT   NOT NULL DEFAULT 0,
    F5     FLOAT(10,4) NOT NULL DEFAULT 0,
    F6     DOUBLE(10,4) NOT NULL DEFAULT 0,
    F7     VARCHAR(64) NOT NULL DEFAULT '',
    F8     DATETIME   NOT NULL DEFAULT '0',
    PRIMARY KEY (F1)
);)",
R"(CREATE TABLE IF NOT EXISTS zce.test (
    F1     SERIAL PRIMARY KEY,
    F2     SMALLINT NOT NULL DEFAULT 0,
    F3     INTEGER  NOT NULL DEFAULT 0,
    F4     INTEGER  NOT NULL DEFAULT 0,
    F5     NUMERIC(10, 4) NOT NULL DEFAULT 0,
    F6     DOUBLE PRECISION NOT NULL DEFAULT 0,
    F7     VARCHAR(64) NOT NULL DEFAULT '',
    F8     TIMESTAMP NOT NULL DEFAULT '1970-01-01 00:00:00'
);)",
R"(CREATE TABLE IF NOT EXISTS test (
    F1     INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    F2     INTEGER NOT NULL DEFAULT 0,
    F3     INTEGER NOT NULL DEFAULT 0,
    F4     INTEGER NOT NULL DEFAULT 0,
    F5     REAL NOT NULL DEFAULT 0.0,
    F6     REAL NOT NULL DEFAULT 0.0,
    F7     TEXT NOT NULL DEFAULT '',
    F8     TEXT NOT NULL DEFAULT '0000-00-00 00:00:00'
);)",
};

const std::string INSERT_TABLE[] = {
    R"(INSERT INTO zce.test(F2,F3,F4,F5,F6,F7,F8) VALUES(%d,%d,%d,%f,%f,'%s','%s');)",
    R"(INSERT INTO zce.test(F2,F3,F4,F5,F6,F7,F8) VALUES(%d,%d,%d,%f,%f,'%s','%s') RETURNING F1;)",
    R"(INSERT INTO test(F2,F3,F4,F5,F6,F7,F8) VALUES(%d,%d,%d,%f,%f,'%s','%s');)",
};

const std::string SELECT_ONE_TABLE[] = {
    R"(SELECT F1,F2,F3,F4,F5,F6,F7,F8 FROM zce.test WHERE F1=%d)",
    R"(SELECT F1,F2,F3,F4,F5,F6,F7,F8 FROM zce.test WHERE F1=%d)",
    R"(SELECT F1,F2,F3,F4,F5,F6,F7,F8 FROM test WHERE F1=%d)",
};

const std::string SELECT_ALL_TABLE[] = {
    R"(SELECT F1,F2,F3,F4,F5,F6,F7,F8 FROM zce.test )",
    R"(SELECT F1,F2,F3,F4,F5,F6,F7,F8 FROM zce.test )",
    R"(SELECT F1,F2,F3,F4,F5,F6,F7,F8 FROM test )",
};

const std::string DELETE_ALL_TABLE[] = {
    R"(DELETE FROM zce.test )",
    R"(DELETE FROM zce.test )",
    R"(DELETE FROM test )",
};

const std::string STMT_INSERT_TABLE[] = {
    R"(INSERT INTO zce.test(F2,F3,F4,F5,F6,F7,F8) VALUES($1,$2,$3,$4,$5,$6,$7);)",
    R"(INSERT INTO zce.test(F2,F3,F4,F5,F6,F7,F8) VALUES($1,$2,$3,$4,$5,$6,$7) RETURNING F1;)",
    R"(INSERT INTO test(F2,F3,F4,F5,F6,F7,F8) VALUES($1,$2,$3,$4,$5,$6,$7);)",
};

struct TEST_TABLE
{
    int f1_;
    short f2_;
    int f3_;
    int f4_;
    float f5_;
    double f6_;
    std::string f7_;
    zce::ztm f8_;
};

template<typename T>
concept DBExec = std::same_as<T, zce::db::my_exec> ||
std::same_as<T, zce::db::pq_exec> ||
std::same_as<T, zce::db::sqlite_exec>;

template<typename T>
constexpr DBType get_dbtype() {
    if constexpr (std::is_same_v<T, zce::db::my_exec>)
    {
        return DBType::DB_MYSQL;
    }
    else if constexpr (std::is_same_v<T, zce::db::pq_exec>)
    {
        return DBType::DB_PQ;
    }
    else if constexpr (std::is_same_v<T, zce::db::sqlite_exec>)
    {
        return DBType::DB_SQLITE;
    }
    else
    {
        return DBType::DB_UNKNOWN;
    }
}

template <DBExec dbt>
int db_connectdb(typename dbt::cnt& conn)
{
    DBType  db_type = get_dbtype<dbt>();
    int ret = dbt::connect(conn,
                           DB_HOST[db_type],
                           DB_PORT[db_type],
                           DB_USER[db_type],
                           DB_PWD[db_type]);
    EXPECT_EQ(ret, 0);
    if (ret != 0)
    {
        return ret;
    }
    return 0;
}

template <DBExec dbt>
int db_connectdb_url(typename dbt::cnt& conn)
{
    DBType  db_type = get_dbtype<dbt>();
    int ret = dbt::connect(conn,
                           DB_URL[db_type].c_str());
    EXPECT_EQ(ret, 0);
    if (ret != 0)
    {
        return ret;
    }
    return 0;
}

template <DBExec dbt>
int db_disconnectdb(typename dbt::cnt* conn)
{
    dbt::disconnect(conn);
    return 0;
}

template <DBExec dbt>
int db_createdatabase(typename dbt::cnt& conn)
{
    DBType  db_type = get_dbtype<dbt>();
    int ret = dbt::execute(conn, CREATE_DATABASE[db_type]);
    EXPECT_EQ(ret, 0);
    if (ret != 0)
    {
        return ret;
    }
    return 0;
}

template <DBExec dbt>
int db_createtable(typename dbt::cnt& conn)
{
    DBType  db_type = get_dbtype<dbt>();
    int ret = dbt::execute(conn, DROP_TABLE[db_type]);
    EXPECT_EQ(ret, 0);
    if (ret != 0)
    {
        return ret;
    }
    ret = dbt::execute(conn, CREATE_TABLE[db_type]);
    if (ret != 0)
    {
        return ret;
    }
    return 0;
}

template <DBExec dbt>
int db_insert_all(typename dbt::cnt& conn, size_t num)
{
    DBType  db_type = get_dbtype<dbt>();
    char sql[1024];
    for (size_t i = 0; i < num; ++i)
    {
        sprintf(sql, INSERT_TABLE[db_type].c_str(),
                (short)num,
                (int)num,
                (int)num,
                (float)num * 10.1,
                (double)num * 200.11,
                "GoogleTest - Google Testing and Mocking Framework",
                "2024-06-01 00:00:00");
        size_t num_affect = 0;
        UINT64 insert_id = 0;
        int ret = dbt::execute(conn, sql, num_affect, &insert_id);
        EXPECT_EQ(ret, 0);
        if (ret != 0)
        {
            return ret;
        }
        EXPECT_EQ(insert_id, i + 1);
        std::cout << "Insert row: " << insert_id << std::endl;
    }

    return 0;
}

template <DBExec dbt>
int db_select_one(typename dbt::cnt& conn, size_t num)
{
    DBType  db_type = get_dbtype<dbt>();
    char sql[1024];
    sprintf(sql, SELECT_ONE_TABLE[db_type].c_str(), (int)num);
    typename dbt::res db_result;
    size_t num_affect = 0;
    int ret = dbt::execute(conn, sql, num_affect, db_result);
    EXPECT_EQ(ret, 0);
    if (ret != 0)
    {
        return ret;
    }
    EXPECT_EQ(num_affect, 1);
    TEST_TABLE r1;
    bool has_one = db_result.cursor_next();
    EXPECT_TRUE(has_one);
    if (has_one)
    {
        db_result.cursor_field(0, r1.f1_);
        db_result.cursor_field(1, r1.f2_);
        db_result.cursor_field(2, r1.f3_);
        db_result.cursor_field(3, r1.f4_);
        db_result.cursor_field(4, r1.f5_);
        db_result.cursor_field(5, r1.f6_);
        db_result.cursor_field(6, r1.f7_);
        db_result.cursor_field(7, r1.f8_);
        EXPECT_EQ(r1.f1_, (int)num);

        auto [f1, f2, f3, f4, f5, f6, f7, f8] =
            db_result.make_tuple<int, short, int, int, float, double, std::string, zce::ztm>(0);

        std::tie(r1.f1_, r1.f2_, r1.f3_, r1.f4_, r1.f5_, r1.f6_, r1.f7_, r1.f8_) =
            db_result.make_tuple<int, short, int, int, float, double, std::string, zce::ztm>(0);
    }

    return 0;
}

template <DBExec dbt>
int db_select_all(typename dbt::cnt& conn, size_t count)
{
    DBType  db_type = get_dbtype<dbt>();
    char sql[1024];
    sprintf(sql, SELECT_ALL_TABLE[db_type].c_str());
    typename dbt::res db_result;
    size_t num_affect = 0;
    int ret = dbt::execute(conn, sql, num_affect, db_result);
    EXPECT_EQ(ret, 0);
    if (ret != 0)
    {
        return ret;
    }
    EXPECT_EQ(num_affect, count);
    TEST_TABLE r1;
    while (db_result.cursor_next())
    {
        db_result.cursor_field(0, r1.f1_);
        db_result.cursor_field(1, r1.f2_);
        db_result.cursor_field(2, r1.f3_);
        db_result.cursor_field(3, r1.f4_);
        db_result.cursor_field(4, r1.f5_);
        db_result.cursor_field(5, r1.f6_);
        db_result.cursor_field(6, r1.f7_);
        db_result.cursor_field(7, r1.f8_);
    }

    for (size_t i = 0; i < count; ++i)
    {
        db_result.cursor_seek(i);
        std::tie(r1.f1_, r1.f2_, r1.f3_, r1.f4_, r1.f5_, r1.f6_, r1.f7_, r1.f8_) =
            db_result.make_tuple<int, short, int, int, float, double, std::string, zce::ztm>(i);
    }
    // 注意下面 template 必须加
    using db_res_set = dbt::template res_set<int, short, int, int, float, double, std::string, zce::ztm>;
    db_res_set  rs(std::move(db_result));
    for (auto [f1, f2, f3, f4, f5, f6, f7, f8] : rs)
    {
    }
    return 0;
}

template <DBExec dbt>
int db_delete_all(typename dbt::cnt& conn, size_t count)
{
    DBType  db_type = get_dbtype<dbt>();
    char sql[1024];
    sprintf(sql, DELETE_ALL_TABLE[db_type].c_str());
    typename dbt::res db_result;
    size_t num_affect = 0;
    int ret = dbt::execute(conn, sql, num_affect, nullptr);
    EXPECT_EQ(ret, 0);
    if (ret != 0)
    {
        return ret;
    }
    EXPECT_EQ(num_affect, count);
    return 0;
}

template <DBExec dbt>
int db_stmt_insert(typename dbt::cmd& command, size_t count)
{
    DBType  db_type = get_dbtype<dbt>();
    char sql[1024];
    sprintf(sql, STMT_INSERT_TABLE[db_type].c_str());
    int ret = dbt::stmt_prepare(command, sql);
    EXPECT_EQ(ret, 0);
    if (ret != 0)
    {
        return ret;
    }
    for (size_t i = 0; i < count; ++i)
    {
        TEST_TABLE r1;
        r1.f2_ = (short)i;
        r1.f3_ = (int)i;
        r1.f4_ = (int)i;
        r1.f5_ = (float)(i * 10.1);
        r1.f6_ = (double)i * 202.22;
        r1.f7_ = "GoogleTest - Google Testing and Mocking Framework";
        zce::fuzzy_str_to_ztm("2024-06-01 00:00:00", &r1.f8_);
        command << r1.f2_;
        command << r1.f3_;
        command << r1.f4_;
        command << r1.f5_;
        command << r1.f6_;
        command << r1.f7_;
        //command << r1.f8_;

        size_t num_affect = 0;
        UINT64 insert_id = 0;
        ret = dbt::stmt_execute(command, num_affect, &insert_id);
        EXPECT_EQ(ret, 0);
        if (ret != 0)
        {
            return ret;
        }
        EXPECT_EQ(insert_id, i + 1);
        EXPECT_EQ(num_affect, 1);
        std::cout << "Insert row: " << insert_id << std::endl;
        command.bind_reset();
    }
    return 0;
}

TEST(MySQLTestSuite, TestAPI)
{
    zce::db::my_exec::cnt db_connect;
    EXPECT_EQ(db_connectdb<zce::db::my_exec>(db_connect), 0);
    EXPECT_EQ(db_createdatabase<zce::db::my_exec>(db_connect), 0);
    EXPECT_EQ(db_createtable<zce::db::my_exec>(db_connect), 0);
    EXPECT_EQ(db_insert_all<zce::db::my_exec>(db_connect, 20), 0);
    EXPECT_EQ(db_select_one<zce::db::my_exec>(db_connect, 20), 0);
    EXPECT_EQ(db_select_all<zce::db::my_exec>(db_connect, 20), 0);
    EXPECT_EQ(db_delete_all<zce::db::my_exec>(db_connect, 20), 0);
}

TEST(PQTestSuite, TestAPI)
{
    zce::db::pq_exec::cnt db_connect;
    EXPECT_EQ(db_connectdb<zce::db::pq_exec>(db_connect), 0);
    EXPECT_EQ(db_createdatabase<zce::db::pq_exec>(db_connect), 0);
    EXPECT_EQ(db_createtable<zce::db::pq_exec>(db_connect), 0);
    EXPECT_EQ(db_insert_all<zce::db::pq_exec>(db_connect, 20), 0);
    EXPECT_EQ(db_select_one<zce::db::pq_exec>(db_connect, 20), 0);
    EXPECT_EQ(db_select_all<zce::db::pq_exec>(db_connect, 20), 0);
    EXPECT_EQ(db_delete_all<zce::db::pq_exec>(db_connect, 20), 0);
}

TEST(SQLiteSuite, TestAPI)
{
    zce::db::sqlite_exec::cnt db_connect;
    EXPECT_EQ(db_connectdb_url<zce::db::sqlite_exec>(db_connect), 0);
    EXPECT_EQ(db_createtable<zce::db::sqlite_exec>(db_connect), 0);
    EXPECT_EQ(db_insert_all<zce::db::sqlite_exec>(db_connect, 20), 0);
    EXPECT_EQ(db_select_one<zce::db::sqlite_exec>(db_connect, 20), 0);
    EXPECT_EQ(db_select_all<zce::db::sqlite_exec>(db_connect, 20), 0);
    EXPECT_EQ(db_delete_all<zce::db::sqlite_exec>(db_connect, 20), 0);
    zce::db::sqlite_exec::cmd db_cmd(db_connect);
    EXPECT_EQ(db_stmt_insert<zce::db::sqlite_exec>(db_cmd, 20), 0);
}
}
