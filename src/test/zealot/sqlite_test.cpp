#include "predefine.h"

namespace zeal
{
int test_sqlite_cmd()
{
	ZCE_TRACE_FUNCTION(RS_DEBUG);
	zce::mkdir("./db/");
	zce::sqlite::connect hdl;
	int ret = 0;
	ret = hdl.connect_db("./db/134.db", false, true);
	EXPECT_EQ(ret, 0);
	if (ret != 0)
	{
		return -1;
	}

	zce::sqlite::command cmd(&hdl);
	ret = cmd.execute("CREATE TABLE IF NOT EXISTS foo(id INTEGER PRIMARY KEY,name TEXT,age INTEGER,address TEXT,salary REAL)");
	EXPECT_EQ(ret, 0);
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
	ret = cmd.execute("SELECT id,name,age,address,salary FROM foo", num_affected, result);
	EXPECT_EQ(ret, 0);
	if (ret != 0)
	{
		return -1;
	}
	std::cout << result.num_of_columns() << std::endl;
	std::cout << result.num_of_rows() << std::endl;
	int id = 0,age=0;
	std::string name, address;
	double salary = 0.0;
	for (size_t i = 0; i < (size_t)result.num_of_rows(); ++i)
	{
		id = result.field<int>(i, 0);
		name = result.field<std::string>(i, 1);
		age = result.field<int>(i, 2);
		address = result.field<std::string>(i, 3);
		salary = result.field<double>(i, 4);
		std::cout << "1." << i
			<< " id=" << id
			<< " name=" << name
			<< " age=" << age
			<< " address=" << address
			<< " salary=" << salary << std::endl;
	}

	for (size_t i = 0; i < (size_t)result.num_of_rows(); ++i)
	{
		std::tie(id, name, age, address, salary) =
			result.make_tuple<int, std::string, int, std::string, double>(i);
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

int test_sqlite_stmt()
{
	zce::sqlite::connect hdl;
	int ret = 0;
	ret = hdl.connect_db("./db/134.db", false, true);
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
	size_t id = 0;
	std::string name;
	int age = 0;
	std::string address;
	double salary = 0.0;
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
		cmd << id;
		cmd << name;
		cmd << age;
		cmd << address;
		cmd << salary;

		ret = cmd.stmt_execute(num_affect, &last_id);
		EXPECT_EQ(ret, 0);
		if (ret != 0)
		{
			std::cerr << "Error executing statement: " << ret << std::endl;
			return -1;
		}
		std::cout << "Last inserted ID: " << last_id << std::endl;
	}

	ret = cmd.stmt_prepare("SELECT id,name,age,address,salary FROM ofo");
	EXPECT_EQ(ret, 0);
	if (ret != 0)
	{
		return -1;
	}
	zce::sqlite::stmt_result sq_stmt_res;
	ret = cmd.stmt_execute(num_affect, sq_stmt_res);
	EXPECT_EQ(ret, 0);
	if (ret != 0)
	{
		return -1;
	}
	std::cout << "Number of affected rows: " << num_affect << std::endl;
	while (sq_stmt_res.cursor_next())
	{
		std::tie(id, name, age, address, salary) =
			sq_stmt_res.make_tuple<size_t, std::string, int, std::string, double>();

		std::cout << " id=" << id
			<< " name=" << name
			<< " age=" << age
			<< " address=" << address
			<< " salary=" << salary << std::endl;
	}

	return 0;
}

int test_sqlite_url()
{
	ZCE_TRACE_FUNCTION(RS_DEBUG);
	zce::mkdir("./db/");
	zce::sqlite::connect hdl;
	int ret = 0;
	ret = hdl.connect_url("sqlite://localhost/./db/134.db?createdb=true");
	EXPECT_EQ(ret, 0);
	if (ret != 0)
	{
		return -1;
	}
	return 0;
}

TEST(SQLiteTestSuite, TestAPI)
{
	EXPECT_EQ(test_sqlite_cmd(), 0);
	EXPECT_EQ(test_sqlite_stmt(), 0);
	EXPECT_EQ(test_sqlite_url(), 0);
}
}
