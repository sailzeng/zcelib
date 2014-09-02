
#include "zce_predefine.h"
#include "zce_os_adapt_stdlib.h"
#include "zce_fmtstr_format.h"

#ifndef FMT_MIN
#define FMT_MIN(a,b) (((a)<(b))?(a):(b))
#endif

#ifndef FMT_MAX
#define FMT_MAX(a,b) (((a)>(b))?(a):(b))
#endif

//我为了追求速度，dopr_outch，将函数dopr_outch 改写成了宏，
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

//格式化double
void ZCE_LIB::fmt_double(char *buffer,
                         size_t max_len,
                         size_t &use_len,
                         double fvalue,
                         size_t width,
                         size_t precision,
                         int flags)
{
    //assert(buffer);

    buffer [0] = '\0';
    use_len = 0;

    //对于参数进行检查,
    if ( max_len == 0 )
    {
        return;
    }

    //如果你不关注精度，将精度调整成6，
    if (precision == 0 || precision > 64)
    {
        precision = DEFAULT_DOUBLE_PRECISION;
    }

    //double的最大长度E308
    const size_t LEN_OF_TMP_OUT_BUF = 512;
    char tmp_out_buf[LEN_OF_TMP_OUT_BUF + 1];
    tmp_out_buf[LEN_OF_TMP_OUT_BUF] = '\0';

    //输出要用的字符串
    int decimal = 0, sign = 0;

    if (flags & FMT_EXPONENT)
    {
        //指数ecvt的精度是输出数字的长度，不是小数点的长度，所以会有precision + 1
        ZCE_LIB::ecvt_r(fvalue,
                       static_cast<int>(precision + 1),
                       &decimal,
                       &sign,
                       tmp_out_buf,
                       LEN_OF_TMP_OUT_BUF);

    }
    else
    {
        ZCE_LIB::fcvt_r(fvalue,
                       static_cast<int>(precision),
                       &decimal,
                       &sign,
                       tmp_out_buf,
                       LEN_OF_TMP_OUT_BUF);
    }

    int cvt_str_len = static_cast<int>( strlen(tmp_out_buf));

    if (cvt_str_len <= 0)
    {
        return;
    }

    //计算各种方式下的字符串空间，看对齐方式下还要增加多少个空格
    //下面的部分数字应该用常量，但是我都解释了，不啰嗦了
    int sign_len = 0;

    if ( sign || flags & FMT_PLUS || flags & FMT_SPACE )
    {
        sign_len = 1;
    }

    int out_str_len = 0;

    if (flags & FMT_EXPONENT)
    {
        //7的来历是1.E+001,sign_len为符号位置占用的空间
        out_str_len = static_cast<int>(7 + precision + sign_len);
    }
    else
    {
        //tmp_out_buf
        if (decimal > 0)
        {
            //1为小数点.
            out_str_len = 1 + cvt_str_len + sign_len;
        }
        else
        {
            //2为"0."，
            out_str_len = static_cast<int>(2 + precision + sign_len);
        }
    }

    //要填补的空格长度
    int space_pad_len = static_cast<int>(  width - out_str_len );

    //如果要右对齐,
    if (!(flags & FMT_LEFT_ALIGN) && space_pad_len > 0 )
    {
        for (int i = 0; i < space_pad_len ; i++)
        {
            FMT_DOPR_OUTCH(buffer, use_len, max_len, ' ');
        }
    }

    //取出符号，进行判断，//如果浮点小于0，填写负数标示
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

    //用指数的方式输出
    if (flags & FMT_EXPONENT)
    {
        //输出第一个数字和.
        FMT_DOPR_OUTCH(buffer, use_len, max_len, tmp_out_buf[0]);
        FMT_DOPR_OUTCH(buffer, use_len, max_len, '.');

        //将小数部分输出
        for (size_t i = 0; i < precision; ++i)
        {
            FMT_DOPR_OUTCH(buffer, use_len, max_len, tmp_out_buf[i + 1]);
        }

        //输出指数部分，根据大小写输出指数E
        if (flags & FMT_UP)
        {
            FMT_DOPR_OUTCH(buffer, use_len, max_len, 'E');
        }
        else
        {
            FMT_DOPR_OUTCH(buffer, use_len, max_len, 'e');
        }

        //下面这段代码即使snprintf(bufbuffer+use_len,max_len-use_len,"%+0.3d",decimal);
        //是我有点走火入魔，我这样写无非是希望加快一点点速度,

        //将指数输出,使用"%+0.3d"的格式
        int i_exponent = decimal - 1;

        if (i_exponent >= 0)
        {
            FMT_DOPR_OUTCH(buffer, use_len, max_len, '+');
        }
        else
        {
            FMT_DOPR_OUTCH(buffer, use_len, max_len, '-');
        }

        //绝对值，因为我
        int u_exponent = i_exponent >= 0 ? i_exponent : -i_exponent ;
        int out_dec = 0;

        //输出百位
        if (u_exponent >= 100)
        {
            out_dec = u_exponent / 100;
            u_exponent = u_exponent - out_dec * 100;
        }

        FMT_DOPR_OUTCH(buffer, use_len, max_len, '0' + static_cast<char>( out_dec));

        //输出10位和个位
        out_dec = 0;

        if (u_exponent >= 10)
        {
            out_dec = u_exponent / 10;
            u_exponent = u_exponent - out_dec * 10;
        }

        FMT_DOPR_OUTCH(buffer, use_len, max_len, '0' + static_cast<char>( out_dec));
        FMT_DOPR_OUTCH(buffer, use_len, max_len, '0' + static_cast<char>( u_exponent));
    }
    else
    {
        //整数和小数混合输出
        if (decimal > 0)
        {
            //将小数部分输出
            for (size_t i = 0; i < cvt_str_len - precision; ++i)
            {
                FMT_DOPR_OUTCH(buffer, use_len, max_len, tmp_out_buf[i]);
            }

            FMT_DOPR_OUTCH(buffer, use_len, max_len, '.');

            //将小数部分输出
            for (size_t i = 0; i < precision; ++i)
            {
                FMT_DOPR_OUTCH(buffer, use_len, max_len, tmp_out_buf[i + cvt_str_len - precision]);
            }
        }
        //纯小数输出
        else
        {
            FMT_DOPR_OUTCH(buffer, use_len, max_len, '0');
            FMT_DOPR_OUTCH(buffer, use_len, max_len, '.');

            //补充好0
            for (size_t i = 0; i < precision - cvt_str_len; ++i)
            {
                FMT_DOPR_OUTCH(buffer, use_len, max_len, '0');
            }

            //将小数部分输出
            for (int i = 0; i < cvt_str_len; ++i)
            {
                FMT_DOPR_OUTCH(buffer, use_len, max_len, tmp_out_buf[i]);
            }
        }
    }

    //如果要左对齐，在尾巴上补气空格
    if ( (flags & FMT_LEFT_ALIGN) && space_pad_len > 0)
    {
        for (int i = 0; i < space_pad_len ; i++)
        {
            FMT_DOPR_OUTCH(buffer, use_len, max_len, ' ');
        }
    }
}

//用于int64的格式化输出，注意这个函数在BUFFER末尾不添加\0,从BSD的openssh snprintf代码移植
void ZCE_LIB::fmt_int64(char *buffer,
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
    //0x,或者0前缀的长度，#标识时使用
    int prefix_len = 0;

    //对符号位置进行处理
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

    //如果要添加0x之类的前缀
    if (flags & FMT_NUM)
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

    //如果是大写,16进制的转换全部用大写
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

    //计算要填补多少0或者空格
    zero_pad_len = static_cast<int>(precision - place);
    space_pad_len = static_cast<int>( width - FMT_MAX (precision, place) - (signvalue ? 1 : 0) - prefix_len);

    //如果精度空间有多的，但是没有要求填写0，那么仍然填写' '
    if (flags & FMT_ZERO)
    {
        if (zero_pad_len < 0)
        {
            zero_pad_len = 0;
        }

        zero_pad_len += space_pad_len;
        space_pad_len = 0;
    }

    //右对齐填补空格
    if (!(flags & FMT_LEFT_ALIGN) && space_pad_len > 0 )
    {
        for (int i = 0; i < space_pad_len ; i++)
        {
            FMT_DOPR_OUTCH(buffer, use_len, max_len, ' ');
        }
    }

    //填补符号
    if (signvalue)
    {
        FMT_DOPR_OUTCH(buffer, use_len, max_len, static_cast<char>(signvalue));
    }

    if (flags & FMT_NUM)
    {
        //十六进制添加0x
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

        //8进制添加0
        if (BASE_OCTAL == base)
        {
            FMT_DOPR_OUTCH(buffer, use_len, max_len, '0');
        }
    }

    //在精度范围输出0，如果没有0标识符号，填写' '
    if (zero_pad_len > 0)
    {
        for (int i = 0; i < zero_pad_len ; i++)
        {
            FMT_DOPR_OUTCH(buffer, use_len, max_len, '0');
        }
    }

    //输出数字
    while (place > 0)
    {
        --place;
        FMT_DOPR_OUTCH(buffer, use_len, max_len, convert[place]);
    }

    //左对齐，在末尾添加空格
    if ((flags & FMT_LEFT_ALIGN) && space_pad_len > 0 )
    {
        for (int i = 0; i < space_pad_len ; i++)
        {
            FMT_DOPR_OUTCH(buffer, use_len, max_len, ' ');
        }
    }
}

//用于字符串的格式化输出，注意这个函数末尾不添加\0
void ZCE_LIB::fmt_str(char *buffer,
                      size_t max_len,
                      size_t &use_len,
                      const char *value,
                      size_t str_len,
                      size_t width,
                      size_t precision,
                      int flags)
{
    use_len = 0;

    //不用处理的参数的情况
    if ( 0 == max_len || ( 0 == width && 0 == precision) )
    {
        return;
    }

    //对参数进行整理
    if (value == 0)
    {
        value = "<NULL>";
        str_len = 6;
    }

    size_t out_len = FMT_MIN(str_len, precision);
    //输出的空格数量
    int space_pad_len = static_cast<int>( width - out_len);

    //右对齐填补空格
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

    //左对齐，在末尾添加空格
    if ((flags & FMT_LEFT_ALIGN) && space_pad_len > 0 )
    {
        for (int i = 0; i < space_pad_len ; i++)
        {
            FMT_DOPR_OUTCH(buffer, use_len, max_len, ' ');
        }
    }
}

