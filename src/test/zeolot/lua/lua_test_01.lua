function lua_func(arg1, arg2, arg3, arg4)
	return arg1 + arg2, arg3 + arg4, arg1+ arg2 + arg3 +arg4
end


result = add2_fun(1, 2)

print("add2_fun(1,2) = "..result)


result = add3_fun(1, 2,3)

print("add3_fun(1,2,3) = "..result)