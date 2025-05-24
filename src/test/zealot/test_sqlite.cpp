#include "predefine.h"

int test_sqlite_handle(int /*argc*/, char* /*argv */[])
{
    zce::sqlite::connect hdl;
    int ret = 0;
    ret = hdl.connect_db("E:\\134.db", false, true);
    if (ret != 0)
    {
        return -1;
    }
    zce::sqlite::command cmd(&hdl);
    ret = cmd.execute("CREATE TABLE IF NOT EXISTS foo(id INTEGER PRIMARY KEY,name TEXT,age INTEGER,address TEXT,salary REAL)");
    if (ret != 0)
    {
        return -1;
    }
    ret = cmd.execute("REPLACE INTO foo(id,name,age,address,salary) VALUES(1,'aaa',22,'aaa.aaa',10000.01)");
    ret = cmd.execute("REPLACE INTO foo(id,name,age,address,salary) VALUES(2,'bbb',22,'bbb.bbb',20000.00)");
    ret = cmd.execute("REPLACE INTO foo(id,name,age,address,salary) VALUES(3,'ccc',22,'ccc.ccc',31000.03)");
    ret = cmd.execute("REPLACE INTO foo(id,name,age,address,salary) VALUES(4,'ddd',25,'ddd.ddd',34000.563)");
    ret = cmd.execute("REPLACE INTO foo(id,name,age,address,salary) VALUES(5,'eee',29,'eee.eee',40000.00)");
    zce::sqlite::result result;
    size_t num_affected = 0;
    ret = cmd.execute("SELECT id,name,age,address,salary FROM foo",num_affected, result);
    if (ret != 0)
    {
        return -1;
    }
    std::cout << result.num_of_columns();
    std::cout << result.num_of_rows();
    for (size_t i = 1; i <= (size_t)result.num_of_rows(); ++i)
    {
        int id = result.field<int>(i, 1);
        std::string name = result.field<std::string>(i, 2);
        int age = result.field<int>(i, 3);
        std::string address = result.field<std::string>(i, 4);
        double salary = result.field<double>(i, 5);
        std::cout << "1." << i
            << " id=" << id
            << " name=" << name
            << " age=" << age
            << " address=" << address
            << " salary=" << salary << std::endl;
    }
    hdl.close_db();

    return 0;
}

int test_sqlite_stmt(int /*argc*/, char* /*argv */[])
{
    zce::sqlite::connect hdl;
    int ret = 0;
    ret = hdl.connect_db("E:\\134.db", false, true);
    if (ret != 0)
    {
        return -1;
    }
    zce::sqlite::command cmd(&hdl);
    ret = cmd.execute("CREATE TABLE IF NOT EXISTS ofo(id INTEGER PRIMARY KEY,name TEXT,age INTEGER,address TEXT,salary REAL)");
    if (ret != 0)
    {
        return -1;
    }
    
    ret = cmd.stmt_prepare("REPLACE INTO ofo(id,name,age,address,salary) VALUES(?,?,?,?,?)");
    if (ret != 0)
    {
        return -1;
    }
    size_t id;
    std::string name;
    int age;
    std::string address;
    double salary;
    size_t num_affect = 0;
    uint64_t last_id = 0;
    for (size_t i = 0; i < 10; ++i)
    {
        id = 1 + i;
        name = "aaaa";
        name += std::to_string(i);
        age = (int)(22 + i);
        address = "aaaa.aaaa";
        address += std::to_string(i);
        salary = 1000.11 + i * 1000;

        cmd.reset_stmt();
        cmd << id;
        cmd << name;
        cmd << age;
        cmd << address;
        cmd  << salary;

        
        ret = cmd.stmt_execute(num_affect, &last_id);
        if (ret != 0)
        {
            return -1;
        }
        std::cout << "Last inserted ID: " << last_id << std::endl;
    }

    cmd.reset_stmt();
    ret = cmd.stmt_prepare("SELECT id,name,age,address,salary FROM ofo");
    if (ret != 0)
    {
        return -1;
    }
    zce::sqlite::stmt_result sq_stmt_res;
    ret = cmd.stmt_execute(num_affect, sq_stmt_res);
    if (ret != 0)
    {
        return -1;
    }
    std::cout << "Number of affected rows: " << num_affect << std::endl;
    while (sq_stmt_res.cursor_next())
    {
        std::tie(id, name, age, address, salary) =
            sq_stmt_res.make_tuple<size_t,std::string,int,std::string,double>();

        std::cout << " id=" << id
            << " name=" << name
            << " age=" << age
            << " address=" << address
            << " salary=" << salary << std::endl;
        //注意查询不能reset
    }

    return 0;
}
