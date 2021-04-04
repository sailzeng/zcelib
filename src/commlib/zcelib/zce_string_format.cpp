
#include "zce_predefine.h"
#include "zce_os_adapt_stdlib.h"
#include "zce_string_format.h"

#ifndef FMT_MIN
#define FMT_MIN(a,b) (((a)<(b))?(a):(b))
#endif

#ifndef FMT_MAX
#define FMT_MAX(a,b) (((a)>(b))?(a):(b))
#endif

//��Ϊ��׷���ٶȣ�dopr_outch��������dopr_outch ��д���˺꣬
#ifndef FMT_DOPR_OUTCH
#define FMT_DOPR_OUTCH(a,b,c,d)   if (((b)) < (c)) \
        (a)[((b))++] = d;
#endif

//static void dopr_outch(char *buffer, size_t &use_len, size_t max_len, char c)
//{
//    if (use_len < max_len)
//    {
//        buffer[(use_len)++] = c;
//    }
//}

//��ʽ��double
void zce::fmt_double(char *buffer,
                     size_t max_len,
                     size_t &use_len,
                     double fvalue,
                     size_t width,
                     size_t precision,
                     int flags)
{
    //ZCE_ASSERT(buffer);
    buffer [0] = '\0';
    use_len = 0;

    //���ڲ������м��,
    if ( max_len == 0 )
    {
        return;
    }

    //����㲻��ע���ȣ������ȵ�����6��
    if (precision == 0 || precision > 64)
    {
        precision = DEFAULT_DOUBLE_PRECISION;
    }

    //double����󳤶�E308
    const size_t LEN_OF_TMP_OUT_BUF = 512;
    char tmp_out_buf[LEN_OF_TMP_OUT_BUF + 1];
    tmp_out_buf[LEN_OF_TMP_OUT_BUF] = '\0';

    //���Ҫ�õ��ַ���
    int decimal = 0, sign = 0;

    if (flags & FMT_EXPONENT)
    {
        //ָ��ecvt�ľ�����������ֵĳ��ȣ�����С����ĳ��ȣ����Ի���precision + 1
        zce::ecvt_r(fvalue,
                    static_cast<int>(precision + 1),
                    &decimal,
                    &sign,
                    tmp_out_buf,
                    LEN_OF_TMP_OUT_BUF);

    }
    else
    {
        zce::fcvt_r(fvalue,
                    static_cast<int>(precision),
                    &decimal,
                    &sign,
                    tmp_out_buf,
                    LEN_OF_TMP_OUT_BUF);
    }
    int cvt_str_len = static_cast<int>(strlen(tmp_out_buf));
    if (cvt_str_len <= 0)
    {
        return;
    }

    //������ַ�ʽ�µ��ַ����ռ䣬�����뷽ʽ�»�Ҫ���Ӷ��ٸ��ո�
    //����Ĳ�������Ӧ���ó����������Ҷ������ˣ���������
    int sign_len = 0;

    if ( sign || flags & FMT_PLUS || flags & FMT_SPACE )
    {
        sign_len = 1;
    }

    int out_str_len = 0;

    if (flags & FMT_EXPONENT)
    {
        //7��������1.E+001,sign_lenΪ����λ��ռ�õĿռ�
        out_str_len = static_cast<int>(7 + precision + sign_len);
    }
    else
    {
        //tmp_out_buf
        if (decimal > 0)
        {
            //1ΪС����.
            out_str_len = 1 + cvt_str_len + sign_len;
        }
        else
        {
            //2Ϊ"0."��
            out_str_len = static_cast<int>(2 + precision + sign_len);
        }
    }

    //Ҫ��Ŀո񳤶�
    int space_pad_len = static_cast<int>(  width - out_str_len );

    //���Ҫ�Ҷ���,
    if (!(flags & FMT_LEFT_ALIGN) && space_pad_len > 0 )
    {
        for (int i = 0; i < space_pad_len ; i++)
        {
            FMT_DOPR_OUTCH(buffer, use_len, max_len, ' ');
        }
    }

    //ȡ�����ţ������жϣ�//�������С��0����д������ʾ
    if (sign)
    {
        FMT_DOPR_OUTCH(buffer, use_len, max_len, '-');
    }
    else if (flags & FMT_PLUS)
    {
        FMT_DOPR_OUTCH(buffer, use_len, max_len, '+');
    }
    else if (flags & FMT_SPACE)
    {
        FMT_DOPR_OUTCH(buffer, use_len, max_len, ' ');
    }

    //��ָ���ķ�ʽ���
    if (flags & FMT_EXPONENT)
    {
        //�����һ�����ֺ�.
        FMT_DOPR_OUTCH(buffer, use_len, max_len, tmp_out_buf[0]);
        FMT_DOPR_OUTCH(buffer, use_len, max_len, '.');

        //��С���������
        for (size_t i = 0; i < precision; ++i)
        {
            FMT_DOPR_OUTCH(buffer, use_len, max_len, tmp_out_buf[i + 1]);
        }

        //���ָ�����֣����ݴ�Сд���ָ��E
        if (flags & FMT_UP)
        {
            FMT_DOPR_OUTCH(buffer, use_len, max_len, 'E');
        }
        else
        {
            FMT_DOPR_OUTCH(buffer, use_len, max_len, 'e');
        }

        //������δ��뼴ʹsnprintf(bufbuffer+use_len,max_len-use_len,"%+0.3d",decimal);
        //�����е��߻���ħ��������д�޷���ϣ���ӿ�һ����ٶ�,

        //��ָ�����,ʹ��"%+0.3d"�ĸ�ʽ
        int i_exponent = decimal - 1;

        if (i_exponent >= 0)
        {
            FMT_DOPR_OUTCH(buffer, use_len, max_len, '+');
        }
        else
        {
            FMT_DOPR_OUTCH(buffer, use_len, max_len, '-');
        }

        //����ֵ����Ϊ��
        int u_exponent = i_exponent >= 0 ? i_exponent : -i_exponent ;
        int out_dec = 0;

        //�����λ
        if (u_exponent >= 100)
        {
            out_dec = u_exponent / 100;
            u_exponent = u_exponent - out_dec * 100;
            FMT_DOPR_OUTCH(buffer, use_len, max_len, '0' + static_cast<char>(out_dec));
        }

        //���10λ�͸�λ
        out_dec = 0;

        if (u_exponent >= 10)
        {
            out_dec = u_exponent / 10;
            u_exponent = u_exponent - out_dec * 10;
            FMT_DOPR_OUTCH(buffer, use_len, max_len, '0' + static_cast<char>(out_dec));
        }
        FMT_DOPR_OUTCH(buffer, use_len, max_len, '0' + static_cast<char>( u_exponent));
    }
    else
    {
        //������С��������
        if (decimal > 0)
        {
            //�������������
            for (size_t i = 0; i < cvt_str_len - precision; ++i)
            {
                FMT_DOPR_OUTCH(buffer, use_len, max_len, tmp_out_buf[i]);
            }

            FMT_DOPR_OUTCH(buffer, use_len, max_len, '.');

            //��С���������
            for (size_t i = 0; i < precision; ++i)
            {
                FMT_DOPR_OUTCH(buffer, use_len, max_len, tmp_out_buf[i + cvt_str_len - precision]);
            }
        }
        //��С�����
        else
        {
            FMT_DOPR_OUTCH(buffer, use_len, max_len, '0');
            FMT_DOPR_OUTCH(buffer, use_len, max_len, '.');

            //�����0
            for (size_t i = 0; i < precision - cvt_str_len; ++i)
            {
                FMT_DOPR_OUTCH(buffer, use_len, max_len, '0');
            }

            //��С���������
            for (int i = 0; i < cvt_str_len; ++i)
            {
                FMT_DOPR_OUTCH(buffer, use_len, max_len, tmp_out_buf[i]);
            }
        }
    }

    //���Ҫ����룬��β���ϲ����ո�
    if ( (flags & FMT_LEFT_ALIGN) && space_pad_len > 0)
    {
        for (int i = 0; i < space_pad_len ; i++)
        {
            FMT_DOPR_OUTCH(buffer, use_len, max_len, ' ');
        }
    }
}

void zce::fmt_double(std::string &stdstr,
                     double fvalue,
                     size_t width,
                     size_t precision,
                     int flags)
{
    //����㲻��ע���ȣ������ȵ�����6��
    if (precision == 0 || precision > 64)
    {
        precision = DEFAULT_DOUBLE_PRECISION;
    }

    //double����󳤶�E308
    const size_t LEN_OF_TMP_OUT_BUF = 512;
    char tmp_out_buf[LEN_OF_TMP_OUT_BUF + 1];
    tmp_out_buf[LEN_OF_TMP_OUT_BUF] = '\0';

    //���Ҫ�õ��ַ���
    int decimal = 0, sign = 0;

    if (flags & FMT_EXPONENT)
    {
        //ָ��ecvt�ľ�����������ֵĳ��ȣ�����С����ĳ��ȣ����Ի���precision + 1
        zce::ecvt_r(fvalue,
                    static_cast<int>(precision + 1),
                    &decimal,
                    &sign,
                    tmp_out_buf,
                    LEN_OF_TMP_OUT_BUF);

    }
    else
    {
        zce::fcvt_r(fvalue,
                    static_cast<int>(precision),
                    &decimal,
                    &sign,
                    tmp_out_buf,
                    LEN_OF_TMP_OUT_BUF);
    }
    int cvt_str_len = static_cast<int>(strlen(tmp_out_buf));
    if (cvt_str_len <= 0)
    {
        return;
    }

    //������ַ�ʽ�µ��ַ����ռ䣬�����뷽ʽ�»�Ҫ���Ӷ��ٸ��ո�
    //����Ĳ�������Ӧ���ó����������Ҷ������ˣ���������
    int sign_len = 0;

    if (sign || flags & FMT_PLUS || flags & FMT_SPACE)
    {
        sign_len = 1;
    }

    int out_str_len = 0;

    if (flags & FMT_EXPONENT)
    {
        //7��������1.E+001,sign_lenΪ����λ��ռ�õĿռ�
        out_str_len = static_cast<int>(7 + precision + sign_len);
    }
    else
    {
        //tmp_out_buf
        if (decimal > 0)
        {
            //1ΪС����.
            out_str_len = 1 + cvt_str_len + sign_len;
        }
        else
        {
            //2Ϊ"0."��
            out_str_len = static_cast<int>(2 + precision + sign_len);
        }
    }

    //Ҫ��Ŀո񳤶�
    int space_pad_len = static_cast<int>(width - out_str_len);

    //���Ҫ�Ҷ���,
    if (!(flags & FMT_LEFT_ALIGN) && space_pad_len > 0)
    {
        stdstr.append(space_pad_len, ' ');
    }

    //ȡ�����ţ������жϣ�//�������С��0����д������ʾ
    if (sign)
    {
        stdstr.append(1, '-');
    }
    else if (flags & FMT_PLUS)
    {
        stdstr.append(1, '+');
    }
    else if (flags & FMT_SPACE)
    {
        stdstr.append(1, ' ');
    }

    //��ָ���ķ�ʽ���
    if (flags & FMT_EXPONENT)
    {
        //�����һ�����ֺ�.
        stdstr.append(1, tmp_out_buf[0]);
        stdstr.append(1, '.');

        //��С���������
        for (size_t i = 0; i < precision; ++i)
        {
            stdstr.append(1, tmp_out_buf[i + 1]);
        }

        //���ָ�����֣����ݴ�Сд���ָ��E
        if (flags & FMT_UP)
        {
            stdstr.append(1, 'E');
        }
        else
        {
            stdstr.append(1, 'e');
        }

        //������δ��뼴ʹsnprintf(bufbuffer+use_len,max_len-use_len,"%+0.3d",decimal);
        //�����е��߻���ħ��������д�޷���ϣ���ӿ�һ����ٶ�,

        //��ָ�����,ʹ��"%+0.3d"�ĸ�ʽ
        int i_exponent = decimal - 1;

        if (i_exponent >= 0)
        {
            stdstr.append(1, '+');
        }
        else
        {
            stdstr.append(1, '-');
        }

        //����ֵ����Ϊ��
        int u_exponent = i_exponent >= 0 ? i_exponent : -i_exponent;
        int out_dec = 0;

        //�����λ
        if (u_exponent >= 100)
        {
            out_dec = u_exponent / 100;
            u_exponent = u_exponent - out_dec * 100;
            stdstr.append(1, '0' + static_cast<char>(out_dec));
        }

        //���10λ�͸�λ
        out_dec = 0;

        if (u_exponent >= 10)
        {
            out_dec = u_exponent / 10;
            u_exponent = u_exponent - out_dec * 10;
            stdstr.append(1, '0' + static_cast<char>(out_dec));
        }
        stdstr.append(1, '0' + static_cast<char>(u_exponent));
    }
    else
    {
        //������С��������
        if (decimal > 0)
        {
            //�������������
            for (size_t i = 0; i < cvt_str_len - precision; ++i)
            {
                stdstr.append(1, tmp_out_buf[i]);
            }
            stdstr.append(1, '.');

            //��С���������
            for (size_t i = 0; i < precision; ++i)
            {
                stdstr.append(1, tmp_out_buf[i + cvt_str_len - precision]);
            }
        }
        //��С�����
        else
        {
            stdstr.append(1, '0');
            stdstr.append(1, '.');

            //�����0
            for (size_t i = 0; i < precision - cvt_str_len; ++i)
            {
                stdstr.append(1, '0');
            }

            //��С���������
            for (int i = 0; i < cvt_str_len; ++i)
            {
                stdstr.append(1, tmp_out_buf[i]);
            }
        }
    }

    //���Ҫ����룬��β���ϲ����ո�
    if ((flags & FMT_LEFT_ALIGN) && space_pad_len > 0)
    {
        stdstr.append(space_pad_len, ' ');
    }
}

//--------------------------------------------------------------------------------------------------------------------
//����int64�ĸ�ʽ�������ע�����������BUFFERĩβ�����\0,��BSD��openssh snprintf������ֲ
void zce::fmt_int64(char *buffer,
                    size_t max_len,
                    size_t &use_len,
                    int64_t value,
                    BASE_NUMBER_SYSTEM base,
                    size_t width,
                    size_t precision,
                    int flags)
{
    use_len = 0;

    if ( 0 == max_len )
    {
        return;
    }

    uint64_t uvalue = value;

    const size_t MAX_OUT_LEN = 64;
    char convert[MAX_OUT_LEN + 1];
    convert[MAX_OUT_LEN] = '\0';

    int signvalue = 0;
    size_t place = 0;
    // amount to space pad
    int space_pad_len = 0;
    // amount to zero pad
    int zero_pad_len = 0;
    //0x,����0ǰ׺�ĳ��ȣ�#��ʶʱʹ��
    int prefix_len = 0;

    //�Է���λ�ý��д���
    if (!(flags & FMT_UNSIGNED))
    {
        if (value < 0)
        {
            signvalue = '-';
            uvalue = -value;
        }
        // Do a sign (+/i)
        else if (flags & FMT_PLUS)
        {
            signvalue = '+';
        }
        else if (flags & FMT_SPACE)
        {
            signvalue = ' ';
        }
    }

    //���Ҫ���0x֮���ǰ׺
    if (flags & FMT_PREFIX)
    {
        if (BASE_HEXADECIMAL == base )
        {
            prefix_len = 2;
        }

        if (BASE_OCTAL  == base )
        {
            prefix_len = 1;
        }
    }

    const char BASE_UPPERCASE_OUTCHAR[] = {"0123456789ABCDEF"};
    const char BASE_LOWERCASE_OUTCHAR[] = {"0123456789abcdef"};

    //����Ǵ�д,16���Ƶ�ת��ȫ���ô�д
    const char *use_char_ary = BASE_LOWERCASE_OUTCHAR;

    if (flags & FMT_UP)
    {
        use_char_ary = BASE_UPPERCASE_OUTCHAR;
    }

    do
    {
        convert[place++] = use_char_ary [uvalue % base];
        uvalue = (uvalue / base );
    }
    while (uvalue && (place < MAX_OUT_LEN));

    //����Ҫ�����0���߿ո�
    zero_pad_len = static_cast<int>(precision - place);
    space_pad_len = static_cast<int>( width - FMT_MAX (precision, place) - (signvalue ? 1 : 0) - prefix_len);

    //������ȿռ��ж�ģ�����û��Ҫ����д0����ô��Ȼ��д' '
    if (flags & FMT_ZERO)
    {
        if (zero_pad_len < 0)
        {
            zero_pad_len = 0;
        }

        zero_pad_len += space_pad_len;
        space_pad_len = 0;
    }

    //�Ҷ�����ո�
    if (!(flags & FMT_LEFT_ALIGN) && space_pad_len > 0 )
    {
        for (int i = 0; i < space_pad_len ; i++)
        {
            FMT_DOPR_OUTCH(buffer, use_len, max_len, ' ');
        }
    }

    //�����
    if (signvalue)
    {
        FMT_DOPR_OUTCH(buffer, use_len, max_len, static_cast<char>(signvalue));
    }

    if (flags & FMT_PREFIX)
    {
        //ʮ���������0x
        if (BASE_HEXADECIMAL == base)
        {
            FMT_DOPR_OUTCH(buffer, use_len, max_len, '0');

            if (flags & FMT_UP)
            {
                FMT_DOPR_OUTCH(buffer, use_len, max_len, 'X');
            }
            else
            {
                FMT_DOPR_OUTCH(buffer, use_len, max_len, 'x');
            }
        }

        //8�������0
        if (BASE_OCTAL == base)
        {
            FMT_DOPR_OUTCH(buffer, use_len, max_len, '0');
        }
    }

    //�ھ��ȷ�Χ���0�����û��0��ʶ���ţ���д' '
    if (zero_pad_len > 0)
    {
        for (int i = 0; i < zero_pad_len ; i++)
        {
            FMT_DOPR_OUTCH(buffer, use_len, max_len, '0');
        }
    }

    //�������
    while (place > 0)
    {
        --place;
        FMT_DOPR_OUTCH(buffer, use_len, max_len, convert[place]);
    }

    //����룬��ĩβ��ӿո�
    if ((flags & FMT_LEFT_ALIGN) && space_pad_len > 0 )
    {
        for (int i = 0; i < space_pad_len ; i++)
        {
            FMT_DOPR_OUTCH(buffer, use_len, max_len, ' ');
        }
    }
}

void zce::fmt_int64(std::string &stdstr,
                    int64_t value,
                    BASE_NUMBER_SYSTEM base,
                    size_t width,
                    size_t precision,
                    int flags)
{

    uint64_t uvalue = value;

    const size_t MAX_OUT_LEN = 64;
    char convert[MAX_OUT_LEN + 1];
    convert[MAX_OUT_LEN] = '\0';

    char signvalue = 0;
    size_t place = 0;
    // amount to space pad
    int space_pad_len = 0;
    // amount to zero pad
    int zero_pad_len = 0;
    //0x,����0ǰ׺�ĳ��ȣ�#��ʶʱʹ��
    int prefix_len = 0;

    //�Է���λ�ý��д���
    if (!(flags & FMT_UNSIGNED))
    {
        if (value < 0)
        {
            signvalue = '-';
            uvalue = -value;
        }
        // Do a sign (+/i)
        else if (flags & FMT_PLUS)
        {
            signvalue = '+';
        }
        else if (flags & FMT_SPACE)
        {
            signvalue = ' ';
        }
    }

    //���Ҫ���0x֮���ǰ׺
    if (flags & FMT_PREFIX)
    {
        if (BASE_HEXADECIMAL == base)
        {
            prefix_len = 2;
        }

        if (BASE_OCTAL == base)
        {
            prefix_len = 1;
        }
    }

    const char BASE_UPPERCASE_OUTCHAR[] = { "0123456789ABCDEF" };
    const char BASE_LOWERCASE_OUTCHAR[] = { "0123456789abcdef" };

    //����Ǵ�д,16���Ƶ�ת��ȫ���ô�д
    const char *use_char_ary = BASE_LOWERCASE_OUTCHAR;

    if (flags & FMT_UP)
    {
        use_char_ary = BASE_UPPERCASE_OUTCHAR;
    }

    //ע��õ����ַ����Ƿ�ת��
    do
    {
        convert[place++] = use_char_ary[uvalue % base];
        uvalue = (uvalue / base);
    }
    while (uvalue && (place < MAX_OUT_LEN));

    //����Ҫ�����0���߿ո�
    zero_pad_len = static_cast<int>(precision - place);
    space_pad_len = static_cast<int>(width - FMT_MAX(precision, place) - (signvalue ? 1 : 0) - prefix_len);

    //������ȿռ��ж�ģ�����û��Ҫ����д0����ô��Ȼ��д' '
    if (flags & FMT_ZERO)
    {
        if (zero_pad_len < 0)
        {
            zero_pad_len = 0;
        }

        zero_pad_len += space_pad_len;
        space_pad_len = 0;
    }

    //�Ҷ�����ո�
    if (!(flags & FMT_LEFT_ALIGN) && space_pad_len > 0)
    {
        stdstr.append(space_pad_len, ' ');
    }

    //�����
    if (signvalue)
    {
        stdstr.append(1, signvalue);
    }

    if (flags & FMT_PREFIX)
    {
        //ʮ���������0x
        if (BASE_HEXADECIMAL == base)
        {
            stdstr.append(1, '0');
            if (flags & FMT_UP)
            {
                stdstr.append(1, 'X');
            }
            else
            {
                stdstr.append(1, 'x');
            }
        }

        //8�������0
        if (BASE_OCTAL == base)
        {
            stdstr.append(1, '0');
        }
    }

    //�ھ��ȷ�Χ���0�����û��0��ʶ���ţ���д' '
    if (zero_pad_len > 0)
    {
        stdstr.append(zero_pad_len, '0');
    }

    //�������
    while (place > 0)
    {
        --place;
        stdstr.append(1, convert[place]);
    }

    //����룬��ĩβ��ӿո�
    if ((flags & FMT_LEFT_ALIGN) && space_pad_len > 0)
    {
        stdstr.append(space_pad_len, ' ');
    }
}

//--------------------------------------------------------------------------------------------------------------------
//�����ַ����ĸ�ʽ�������ע���������ĩβ�����\0
void zce::fmt_str(char *buffer,
                  size_t max_len,
                  size_t &use_len,
                  const char *value,
                  size_t str_len,
                  size_t width,
                  size_t precision,
                  int flags)
{
    use_len = 0;

    //���ô���Ĳ��������
    if ( 0 == max_len || ( 0 == width && 0 == precision) )
    {
        return;
    }

    //�Բ�����������
    if (value == nullptr)
    {
        value = "<NULL>";
        str_len = 6;
    }

    size_t out_len = FMT_MIN(str_len, precision);
    //����Ŀո�����
    int space_pad_len = static_cast<int>( width - out_len);

    //�Ҷ�����ո�(�������)
    if (!(flags & FMT_LEFT_ALIGN) && space_pad_len > 0 )
    {
        for (int i = 0; i < space_pad_len ; i++)
        {
            FMT_DOPR_OUTCH(buffer, use_len, max_len, ' ');
        }
    }

    size_t copy_len = FMT_MIN(out_len, max_len);
    memcpy(buffer + use_len, value, copy_len);
    use_len += copy_len;
    max_len -= copy_len;
    /*while (out_cnt < out_len)
    {
        FMT_DOPR_OUTCH(buffer, use_len, max_len, *value++);
        ++out_cnt;
    }*/

    //����룬��ĩβ��ӿո�
    if ((flags & FMT_LEFT_ALIGN) && space_pad_len > 0 )
    {
        for (int i = 0; i < space_pad_len ; i++)
        {
            FMT_DOPR_OUTCH(buffer, use_len, max_len, ' ');
        }
    }
}

void zce::fmt_str(std::string &stdstr,
                  const char *value,
                  size_t str_len,
                  size_t width,
                  size_t precision,
                  int flags)
{
    //���ô���Ĳ��������
    if (0 == width && 0 == precision)
    {
        return;
    }

    //�Բ�����������
    if (value == 0)
    {
        value = "<NULL>";
        str_len = 6;
    }
    size_t out_len = FMT_MIN(str_len, precision);
    //����Ŀո�����
    int space_pad_len = static_cast<int>(width - out_len);

    //�Ҷ�����ո�(�������)
    if (!(flags & FMT_LEFT_ALIGN) && space_pad_len > 0)
    {
        stdstr.append(space_pad_len, ' ');
    }
    stdstr.append(value, out_len);

    //����룬��ĩβ��ӿո�
    if ((flags & FMT_LEFT_ALIGN) && space_pad_len > 0)
    {
        stdstr.append(space_pad_len, ' ');
    }
}