#include "predefine.h"

namespace ztest
{
//社团数据库
const char* DB_MYSQL_HOST = "127.0.0.1";
//社团数据库用户
const char* DB_MYSQL_USER = "root";
////社团数据库用户密码
const char* DB_MYSQL_PASSWORD = "";

const std::string CREATE_DATABASE = "CREATE DATABASE IF NOT EXISTS zce";
const std::string DROP_TABLE = R"(DROP TABLE IF EXISTS zce.test;)";
const std::string CREATE_TABLE = R"(
CREATE TABLE IF NOT EXISTS zce.test
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
);)";

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
concept DBype = std::same_as<T, zce::db::my_exec> ||
std::same_as<T, zce::db::pq_exec>;

template <DBype dbt>
int db_createtable(typename dbt::cnt* conn)
{
    int ret = dbt::connect(conn,
                           DB_MYSQL_HOST,
                           MYSQL_PORT,
                           DB_MYSQL_USER,
                           DB_MYSQL_PASSWORD);
    EXPECT_EQ(ret, 0);
    if (ret != 0)
    {
        return ret;
    }
    ret = dbt::execute(conn, CREATE_DATABASE);
    EXPECT_EQ(ret, 0);
    if (ret != 0)
    {
        return ret;
    }
    ret = dbt::execute(conn, DROP_TABLE);
    EXPECT_EQ(ret, 0);
    if (ret != 0)
    {
        return ret;
    }
    ret = dbt::execute(conn, CREATE_TABLE);
    if (ret != 0)
    {
        return ret;
    }
    return 0;
}

template <DBype dbt>
int db_insert_all(typename dbt::cnt* conn, size_t num)
{
    char sql[1024];
    for (size_t i = 0; i < num; ++i)
    {
        sprintf(sql, "INSERT INTO zce.test(F2,F3,F4,F5,F6,F7,F8) VALUES(%d,%d,%d,%f,%f,'%s','%s')",
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
    }

    return 0;
}

template <DBype dbt>
int db_select_one(typename dbt::cnt* conn, size_t num)
{
    char sql[1024];
    sprintf(sql, "SELECT F1,F2,F3,F4,F5,F6,F7,F8 FROM zce.test WHERE F1=%d", (int)num);
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

template <DBype dbt>
int db_select_all(typename dbt::cnt* conn, size_t count)
{
    char sql[1024];
    sprintf(sql, "SELECT F1,F2,F3,F4,F5,F6,F7,F8 FROM zce.test ");
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

template <DBype dbt>
int db_delete_all(typename dbt::cnt* conn, size_t count)
{
    char sql[1024];
    sprintf(sql, "DELETE FROM zce.test ");
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

TEST(MySQLTestSuite, TestAPI)
{
    zce::db::my_exec::cnt db_connect;
    EXPECT_EQ(db_createtable<zce::db::my_exec>(&db_connect), 0);
    EXPECT_EQ(db_insert_all<zce::db::my_exec>(&db_connect, 20), 0);
    EXPECT_EQ(db_select_one<zce::db::my_exec>(&db_connect, 20), 0);
    EXPECT_EQ(db_select_all<zce::db::my_exec>(&db_connect, 20), 0);
    EXPECT_EQ(db_delete_all<zce::db::my_exec>(&db_connect, 20), 0);
}

TEST(PQTestSuite, TestAPI)
{
    zce::db::pq_exec::cnt db_connect;
    EXPECT_EQ(db_createtable<zce::db::pq_exec>(&db_connect), 0);
    EXPECT_EQ(db_insert_all<zce::db::pq_exec>(&db_connect, 20), 0);
    EXPECT_EQ(db_select_one<zce::db::pq_exec>(&db_connect, 20), 0);
    EXPECT_EQ(db_select_all<zce::db::pq_exec>(&db_connect, 20), 0);
    EXPECT_EQ(db_delete_all<zce::db::pq_exec>(&db_connect, 20), 0);
}
}
