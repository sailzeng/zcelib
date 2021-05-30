#include "zce/predefine.h"
#include "zce/util/random.h"

namespace zce
{
//根据要求，生成一个随机数字符串，
void random_base::get_string(RAND_STRING str_type,
                             char* rand_str,
                             size_t str_len)
{
    uint32_t rand_data = 0;
    uint32_t rand_number = 0;

    for (size_t i = 0; i < str_len; ++i)
    {
        //一个随机数字生成2个字符
        if (0 == i % 2)
        {
            rand_number = get_uint32();
            rand_data = rand_number & 0xFFFF;
        }
        else
        {
            rand_data = (rand_number & 0xFFFF0000) >> 16;
        }
        //根据类型生成字符串，有一些常量，但懒得定义了，你一看就明白的
        switch (str_type)
        {
            //数字
            case RAND_STRING::NUMBER:
                rand_str[i] = static_cast<char>('0' + (rand_data % 10));
                break;
                //小写
            case RAND_STRING::LOWER:
                rand_str[i] = static_cast<char>('a' + (rand_data % 26));
                break;
                //大写
            case RAND_STRING::UPPER:
                rand_str[i] = static_cast<char>('A' + (rand_data % 26));
                break;
                //数字+小写字符
            case RAND_STRING::NUMBER_LOWER:
                rand_data = rand_data % 36;
                if (rand_data < 10)
                {
                    rand_str[i] = '0' + static_cast<char>(rand_data);
                }
                else
                {
                    rand_str[i] = 'a' + static_cast<char>(rand_data);
                }
                break;
                //数字+大写字符
            case RAND_STRING::NUMBER_UPPER:
                rand_data = rand_data % 36;
                if (rand_data < 10)
                {
                    rand_str[i] = '0' + static_cast<char>(rand_data);
                }
                else
                {
                    rand_str[i] = 'A' + static_cast<char>(rand_data);
                }
                break;
                //数字，大写，小写，
            case RAND_STRING::NUMBER_LOWER_UPPER:
                rand_data = rand_data % 62;
                if (rand_data < 10)
                {
                    rand_str[i] = '0' + static_cast<char>(rand_data);
                }
                else if (rand_data < 36)
                {
                    rand_str[i] = 'a' + static_cast<char>(rand_data);
                }
                else
                {
                    rand_str[i] = 'A' + static_cast<char>(rand_data);
                }
                break;

                ///产生0-127的ASCII(非扩展)字符串
            case RAND_STRING::ASCII:
                rand_str[i] = static_cast<char>(rand_data & 0x7F);
                break;
                ///产生0-255二进制字符串
            case RAND_STRING::BINARY:
                rand_str[i] = static_cast<char>(rand_data & 0xFF);
                break;
            default:
                ZCE_ASSERT(false);
                break;
        }
    }
}
};