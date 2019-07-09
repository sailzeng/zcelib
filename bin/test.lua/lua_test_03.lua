--测试构造函数
temp_3c = T3C(1024,"2048",4096)
print(temp_3c)
print("temp_3c.c1_ = "..temp_3c.c1_)
print("temp_3c.c2_ = "..temp_3c.c2_)
print("temp_3c.c3_ = "..temp_3c.c3_)

temp_3c:set_c1(1111,2222)
print("temp_3c.c1_ = "..temp_3c.c1_)
temp_3c:set_c2(1111,2222.3333,"I love beijing TianAnMen!!!")
print("temp_3c.c2_ = "..temp_3c.c2_)
temp_3c:set_c3(1000.1111,2000.2222,3000.3333,55,"1000.1111")
print("temp_3c.c3_ = "..temp_3c.c3_)

-- 对象测试
print("start lua test.----------------------------------")
-- 测试使用继承的数据
tb_ptr_2.a_ = 100
tb_ptr_2.b_array_[1] = 10000
tb_ptr_1.b1_ = 1111

tb_ptr_2:set_b2(2222,3333)

tb_val:set_b1(1)

print("ta ptr val ref ")
print("ta_ptr.a_ = "..ta_ptr.a_)
print("ta_ref.a_ = "..ta_ref.a_)
print("ta_val.a_ = "..ta_val.a_)

ta_ptr.a_ = 20000
ta_ref.a_ = 30000
ta_val.a_ = 40000


print("tb_ptr_1.b1_ = "..tb_ptr_1.b1_)
print("tb_ptr_1.b2_ = "..tb_ptr_1.b2_)
print("tb_ptr_1.b3_ = "..tb_ptr_1.b3_)


tb_ptr_1:set_b1(1)
tb_ptr_1:set_b2(2,3)
tb_ptr_1:set_b3(4,5,6)

--对象数组
tb_ptr_2.b_array_[1] = 10000
tb_ptr_2.b_array_[2] = 20000
tb_ptr_2.b_array_[3] = 30000
tb_ptr_2.b_array_[4] = 40000
tb_ptr_2.b_array_[5] = 50000
tb_ptr_2.b_array_[6] = 60000
tb_ptr_2.b_array_[7] = 70000
tb_ptr_2.b_array_[8] = 80000
tb_ptr_2.b_array_[9] = 90000
tb_ptr_2.b_array_[10] = 100000
tb_ptr_2.b_array_[11] = 110000
tb_ptr_2.b_array_[12] = 120000
tb_ptr_2.b_array_[13] = 130000
tb_ptr_2.b_array_[14] = 140000
tb_ptr_2.b_array_[15] = 150000
tb_ptr_2.b_array_[16] = 160000
tb_ptr_2.b_array_[17] = 170000
tb_ptr_2.b_array_[18] = 180000
tb_ptr_2.b_array_[19] = 190000
tb_ptr_2.b_array_[20] = 200000


