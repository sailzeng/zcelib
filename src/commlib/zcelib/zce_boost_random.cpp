#include "zce_predefine.h"
#include "zce_boost_random.h"

namespace zce
{

//����Ҫ������һ��������ַ�����
void random_base::get_string(random_base::RAND_STRING_TYPE str_type,
                             char *rand_str,
                             size_t str_len)
{

    uint32_t rand_data = 0;
    uint32_t rand_number = 0;

    for (size_t i = 0; i < str_len ; ++i )
    {

        //һ�������������2���ַ�
        if (0 == i % 2 )
        {
            rand_number = get_uint32();
            rand_data = rand_number & 0xFFFF;
        }
        else
        {
            rand_data = (rand_number & 0xFFFF0000) >> 16 ;
        }
        //�������������ַ�������һЩ�����������ö����ˣ���һ�������׵�
        switch (str_type)
        {
            //����
            case random_base::RAND_STR_NUMBER:
                rand_str [i] = static_cast<char>('0' + (rand_data % 10));
                break;
            //Сд
            case random_base::RAND_STR_LOWER:
                rand_str [i] = static_cast<char>('a' + (rand_data % 26));
                break;
            //��д
            case random_base::RAND_STR_UPPER:
                rand_str [i] = static_cast<char>('A' + (rand_data % 26));
                break;
            //����+Сд�ַ�
            case random_base::RAND_STR_NUMBER_LOWER:
                rand_data = rand_data % 36;
                if (rand_data < 10)
                {
                    rand_str [i] = '0' + static_cast<char>( rand_data );
                }
                else
                {
                    rand_str [i] = 'a' + static_cast<char>( rand_data ) ;
                }
                break;
            //����+��д�ַ�
            case random_base::RAND_STR_NUMBER_UPPER:
                rand_data = rand_data % 36;
                if (rand_data < 10)
                {
                    rand_str [i] = '0' + static_cast<char>( rand_data );
                }
                else
                {
                    rand_str [i] = 'A' + static_cast<char>( rand_data ) ;
                }
                break;
            //���֣���д��Сд��
            case random_base::RAND_STR_NUMBER_LOWER_UPPER:
                rand_data = rand_data % 62;
                if (rand_data < 10)
                {
                    rand_str [i] = '0' + static_cast<char>( rand_data );
                }
                else if (rand_data < 36)
                {
                    rand_str [i] = 'a' + static_cast<char>( rand_data ) ;
                }
                else
                {
                    rand_str [i] = 'A' + static_cast<char>( rand_data ) ;
                }
                break;

            ///����0-127��ASCII(����չ)�ַ���
            case random_base::RAND_STR_ASCII:
                rand_str [i] = static_cast<char>( rand_data & 0x7F ) ;
                break;
            ///����0-255�������ַ���
            case random_base::RAND_STR_BINARY:
                rand_str [i] = static_cast<char>( rand_data & 0xFF ) ;
                break;
            default:
                ZCE_ASSERT(false);
                break;
        }
    }
}

};

