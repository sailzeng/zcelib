-- 对象测试
tb_ptr_2.b_array_[1] = 10000

tb_ptr_1.a_ = 1111
tb_ptr_2:set_a(2222)


tb_val:set_b1(1)

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

