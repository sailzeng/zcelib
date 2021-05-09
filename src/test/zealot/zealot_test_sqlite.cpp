#include "zealot_predefine.h"

int test_sqlite_handle(int /*argc*/, char* /*argv */[])
{
    zce::SQLite_Handler hdl;
    int ret = 0;
    ret = hdl.open_database("E:\\134.db", false, true);
    if (ret != 0)
    {
        return -1;
    }
    ret = hdl.execute("CREATE TABLE IF NOT EXISTS foo(id INTEGER PRIMARY KEY,name TEXT,age INTEGER,address TEXT,salary REAL)");
    if (ret != 0)
    {
        return -1;
    }
    ret = hdl.execute("REPLACE INTO foo(id,name,age,address,salary) VALUES(1,'aaa',22,'aaa.aaa',10000.01)");
    ret = hdl.execute("REPLACE INTO foo(id,name,age,address,salary) VALUES(2,'bbb',22,'bbb.bbb',20000.00)");
    ret = hdl.execute("REPLACE INTO foo(id,name,age,address,salary) VALUES(3,'ccc',22,'ccc.ccc',31000.03)");
    ret = hdl.execute("REPLACE INTO foo(id,name,age,address,salary) VALUES(4,'ddd',25,'ddd.ddd',34000.563)");
    zce::SQLite_Result result;
    ret = hdl.get_table("SELECT id,name,age,address,salary FROM foo", &result);
    if (ret != 0)
    {
        return -1;
    }
    std::cout << result.column_number();
    std::cout << result.row_number();
    for (size_t i = 1; i <= (size_t)result.row_number(); ++i)
    {
        int id = result.field_data<int>(i, 1);
        std::string name = result.field_data<std::string>(i, 2);
        int age = result.field_data<int>(i, 3);
        std::string address = result.field_data<std::string>(i, 4);
        double salary = result.field_data<double>(i, 5);
        std::cout << "1." << i
            << " id=" << id
            << " name=" << name
            << " age=" << age
            << " address=" << address
            << " salary=" << salary << std::endl;
    }
    hdl.close_database();

    return 0;
}

int test_sqlite_stmt(int /*argc*/, char* /*argv */[])
{
    zce::SQLite_Handler hdl;
    int ret = 0;
    ret = hdl.open_database("E:\\134.db", false, true);
    if (ret != 0)
    {
        return -1;
    }
    ret = hdl.execute("CREATE TABLE IF NOT EXISTS ofo(id INTEGER PRIMARY KEY,name TEXT,age INTEGER,address TEXT,salary REAL)");
    if (ret != 0)
    {
        return -1;
    }
    zce::SQLite_STMT stmt(&hdl);
    ret = stmt.prepare("REPLACE INTO ofo(id,name,age,address,salary) VALUES(?,?,?,?,?)");
    if (ret != 0)
    {
        return -1;
    }
    int id;
    std::string name;
    int age;
    std::string address;
    double salary;
    bool has_result = false;

    for (size_t i = 0; i < 10; ++i)
    {
        id = 1 + i;
        name = "aaaa";
        name += std::to_string(i);
        age = 22 + i;
        address = "aaaa.aaaa";
        address += std::to_string(i);
        salary = 1000.11 + i * 1000;

        stmt.reset();
        stmt << id;
        stmt << name;
        stmt << age;
        stmt << address;
        stmt << salary;

        ret = stmt.step(has_result);
        if (ret != 0)
        {
            return -1;
        }
    }

    stmt.reset();
    ret = stmt.prepare("SELECT id,name,age,address,salary FROM ofo");
    if (ret != 0)
    {
        return -1;
    }

    while (true)
    {
        ret = stmt.step(has_result);
        if (ret != 0)
        {
            return -1;
        }
        if (has_result == false)
        {
            break;
        }

        stmt >> id;
        stmt >> name;
        stmt >> age;
        stmt >> address;
        stmt >> salary;
        std::cout << " id=" << id
            << " name=" << name
            << " age=" << age
            << " address=" << address
            << " salary=" << salary << std::endl;
        //注意查询不能reset
    }

    return 0;
}