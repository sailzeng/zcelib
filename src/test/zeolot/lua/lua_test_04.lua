-- 调用函数1
function test_error()
	
	print("test_error() called !!")
	
	test_error_1()
end

-- 调用函数2
function test_error_1()

	print("test_error_1() called !!")
	
	test_error_2()
end

-- 调用函数3，
function test_error_2()

	print("test_error_2() called !!")
	
	-- 其不存在
	test_error_3()
end