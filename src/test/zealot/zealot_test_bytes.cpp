


#include "zealot_predefine.h"
#include "zealot_test_function.h"

int test_bytes_ntoh(int /*argc*/, char * /*argv*/[])
{
    uint16_t a_16 = 0x1234;
    uint16_t b_16 = ZCE_HTONS(a_16);

    uint32_t a_32 = 0x11223344;
    uint32_t b_32 = ZCE_HTONL(a_32);


    uint64_t a_64 = 0x1122334455667788ULL;
    uint64_t b_64 = ZCE_HTONLL(a_64);

    printf("%x %x  %llx", b_16, b_32, b_64);

    return 0;
}





int test_bytes_hash(int /*argc*/, char * /*argv*/[])
{

    test_bytes_encode(0, NULL);

    test_bytes_ntoh(0, NULL);


    int ret = 0;
    const size_t TEST_ARRAY_NUMBER = 8;
    static unsigned char test_buf[TEST_ARRAY_NUMBER][81] =
    {
        { "" },
        { "a" },
        { "abc" },
        { "message digest" },
        { "abcdefghijklmnopqrstuvwxyz" },
        { "12345678901234567890123456789012345678901234567890123456" },
        { "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789" },
        { "12345678901234567890123456789012345678901234567890123456789012345678901234567890" }
    };

    static const size_t test_buflen[TEST_ARRAY_NUMBER] =
    {
        0, 1, 3, 14, 26,56,62, 80
    };

    static const unsigned char md5_test_sum[TEST_ARRAY_NUMBER][16] =
    {
        { 0xD4, 0x1D, 0x8C, 0xD9, 0x8F, 0x00, 0xB2, 0x04,  0xE9, 0x80, 0x09, 0x98, 0xEC, 0xF8, 0x42, 0x7E },
        { 0x0C, 0xC1, 0x75, 0xB9, 0xC0, 0xF1, 0xB6, 0xA8,  0x31, 0xC3, 0x99, 0xE2, 0x69, 0x77, 0x26, 0x61 },
        { 0x90, 0x01, 0x50, 0x98, 0x3C, 0xD2, 0x4F, 0xB0,  0xD6, 0x96, 0x3F, 0x7D, 0x28, 0xE1, 0x7F, 0x72 },
        { 0xF9, 0x6B, 0x69, 0x7D, 0x7C, 0xB7, 0x93, 0x8D,  0x52, 0x5A, 0x2F, 0x31, 0xAA, 0xF1, 0x61, 0xD0 },
        { 0xC3, 0xFC, 0xD3, 0xD7, 0x61, 0x92, 0xE4, 0x00,  0x7D, 0xFB, 0x49, 0x6C, 0xCA, 0x67, 0xE1, 0x3B },
        { 0x49, 0xF1, 0x93, 0xAD, 0xCE, 0x17, 0x84, 0x90,  0xE3, 0x4D, 0x1B, 0x3A, 0x4E, 0xC0, 0x06, 0x4C },
        { 0xD1, 0x74, 0xAB, 0x98, 0xD2, 0x77, 0xD9, 0xF5,  0xA5, 0x61, 0x1C, 0x2C, 0x9F, 0x41, 0x9D, 0x9F },
        { 0x57, 0xED, 0xF4, 0xA2, 0x2B, 0xE3, 0xC9, 0x55,  0xAC, 0x49, 0xDA, 0x2E, 0x21, 0x07, 0xB6, 0x7A }
    };
    unsigned char result[32] = {0};

    ZCE_LIB::md5(test_buf[2], test_buflen[2], result);

    ZCE_LIB::sha1(test_buf[2], test_buflen[2], result);
    ZCE_LIB::md5(test_buf[5], test_buflen[5], result);
    ZCE_LIB::md5(test_buf[6], test_buflen[6], result);
    ZCE_LIB::sha1(test_buf[6], test_buflen[6], result);


    for (size_t i = 0; i < TEST_ARRAY_NUMBER; ++i)
    {
        ZCE_LIB::md5(test_buf[i], test_buflen[i], result);
        ret = memcmp(result, md5_test_sum[i], 16);
        if (ret != 0)
        {
            abort();
        }
    }

    //ZCE_LIB::md5((unsigned char *)"abcd",4,result);
    //ZCE_LIB::md5((unsigned char *)"dcba",4,result);


    static const unsigned char sha1_test_sum[TEST_ARRAY_NUMBER][20] =
    {

        { 0xda, 0x39, 0xa3, 0xee, 0x5e, 0x6b, 0x4b, 0x0d, 0x32, 0x55, 0xbf, 0xef, 0x95, 0x60, 0x18, 0x90, 0xaf, 0xd8, 0x07, 0x09 },
        { 0x86, 0xf7, 0xe4, 0x37, 0xfa, 0xa5, 0xa7, 0xfc, 0xe1, 0x5d, 0x1d, 0xdc, 0xb9, 0xea, 0xea, 0xea, 0x37, 0x76, 0x67, 0xb8 },
        { 0xa9, 0x99, 0x3e, 0x36, 0x47, 0x06, 0x81, 0x6a, 0xba, 0x3e, 0x25, 0x71, 0x78, 0x50, 0xc2, 0x6c, 0x9c, 0xd0, 0xd8, 0x9d },
        { 0xc1, 0x22, 0x52, 0xce, 0xda, 0x8b, 0xe8, 0x99, 0x4d, 0x5f, 0xa0, 0x29, 0x0a, 0x47, 0x23, 0x1c, 0x1d, 0x16, 0xaa, 0xe3 },
        { 0x32, 0xd1, 0x0c, 0x7b, 0x8c, 0xf9, 0x65, 0x70, 0xca, 0x04, 0xce, 0x37, 0xf2, 0xa1, 0x9d, 0x84, 0x24, 0x0d, 0x3a, 0x89 },
        { 0x0a, 0x84, 0x66, 0x6b, 0x66, 0xe8, 0x43, 0xa4, 0x14, 0x60, 0x88, 0xfb, 0x46, 0xaa, 0xba, 0xa9, 0x98, 0xb4, 0xc2, 0xb1 },
        { 0x76, 0x1c, 0x45, 0x7b, 0xf7, 0x3b, 0x14, 0xd2, 0x7e, 0x9e, 0x92, 0x65, 0xc4, 0x6f, 0x4b, 0x4d, 0xda, 0x11, 0xf9, 0x40 },
        { 0x50, 0xab, 0xf5, 0x70, 0x6a, 0x15, 0x09, 0x90, 0xa0, 0x8b, 0x2c, 0x5e, 0xa4, 0x0f, 0xa0, 0xe5, 0x85, 0x55, 0x47, 0x32 },
    };
    for (size_t i = 0; i < TEST_ARRAY_NUMBER; ++i)
    {
        ZCE_LIB::sha1(test_buf[i], test_buflen[i], result);
        ret = memcmp(result, sha1_test_sum[i], 20);
        if (ret != 0)
        {
            abort();
        }
    }
    static const unsigned char sha256_test_sum[TEST_ARRAY_NUMBER][32] =
    {
        {0xe3, 0xb0, 0xc4, 0x42, 0x98, 0xfc, 0x1c, 0x14, 0x9a, 0xfb, 0xf4, 0xc8, 0x99, 0x6f, 0xb9, 0x24, 0x27, 0xae, 0x41, 0xe4, 0x64, 0x9b, 0x93, 0x4c, 0xa4, 0x95, 0x99, 0x1b, 0x78, 0x52, 0xb8, 0x55},
        {0xca, 0x97, 0x81, 0x12, 0xca, 0x1b, 0xbd, 0xca, 0xfa, 0xc2, 0x31, 0xb3, 0x9a, 0x23, 0xdc, 0x4d, 0xa7, 0x86, 0xef, 0xf8, 0x14, 0x7c, 0x4e, 0x72, 0xb9, 0x80, 0x77, 0x85, 0xaf, 0xee, 0x48, 0xbb},
        {0xba, 0x78, 0x16, 0xbf, 0x8f, 0x01, 0xcf, 0xea, 0x41, 0x41, 0x40, 0xde, 0x5d, 0xae, 0x22, 0x23, 0xb0, 0x03, 0x61, 0xa3, 0x96, 0x17, 0x7a, 0x9c, 0xb4, 0x10, 0xff, 0x61, 0xf2, 0x00, 0x15, 0xad},
        {0xf7, 0x84, 0x6f, 0x55, 0xcf, 0x23, 0xe1, 0x4e, 0xeb, 0xea, 0xb5, 0xb4, 0xe1, 0x55, 0x0c, 0xad, 0x5b, 0x50, 0x9e, 0x33, 0x48, 0xfb, 0xc4, 0xef, 0xa3, 0xa1, 0x41, 0x3d, 0x39, 0x3c, 0xb6, 0x50},
        {0x71, 0xc4, 0x80, 0xdf, 0x93, 0xd6, 0xae, 0x2f, 0x1e, 0xfa, 0xd1, 0x44, 0x7c, 0x66, 0xc9, 0x52, 0x5e, 0x31, 0x62, 0x18, 0xcf, 0x51, 0xfc, 0x8d, 0x9e, 0xd8, 0x32, 0xf2, 0xda, 0xf1, 0x8b, 0x73},
        {0x0b, 0xe6, 0x6c, 0xe7, 0x2c, 0x24, 0x67, 0xe7, 0x93, 0x20, 0x29, 0x06, 0x00, 0x06, 0x72, 0x30, 0x66, 0x61, 0x79, 0x16, 0x22, 0xe0, 0xca, 0x9a, 0xdf, 0x4a, 0x89, 0x55, 0xb2, 0xed, 0x18, 0x9c},
        {0xdb, 0x4b, 0xfc, 0xbd, 0x4d, 0xa0, 0xcd, 0x85, 0xa6, 0x0c, 0x3c, 0x37, 0xd3, 0xfb, 0xd8, 0x80, 0x5c, 0x77, 0xf1, 0x5f, 0xc6, 0xb1, 0xfd, 0xfe, 0x61, 0x4e, 0xe0, 0xa7, 0xc8, 0xfd, 0xb4, 0xc0},
        {0xf3, 0x71, 0xbc, 0x4a, 0x31, 0x1f, 0x2b, 0x00, 0x9e, 0xef, 0x95, 0x2d, 0xd8, 0x3c, 0xa8, 0x0e, 0x2b, 0x60, 0x02, 0x6c, 0x8e, 0x93, 0x55, 0x92, 0xd0, 0xf9, 0xc3, 0x08, 0x45, 0x3c, 0x81, 0x3e},
    };
    for (size_t i = 0; i < TEST_ARRAY_NUMBER; ++i)
    {
        ZCE_LIB::sha256(test_buf[i], test_buflen[i], result);
        ret = memcmp(result, sha256_test_sum[i], 32);
        if (ret != 0)
        {
            abort();
        }
    }

    //ZCE_LIB::md5_file("D:\\1.chm", result);
    //ZCE_LIB::sha1_file("D:\\1.chm", result);
    //ZCE_LIB::sha256_file("D:\\1.chm", result);
    return 0;
}


int test_bytes_encode(int /*argc*/, char * /*argv*/[])
{
    const unsigned char test_string[] = "Man is distinguished, not only by his reason,"
                                        " but by this singular passion from other animals, which is a lust of the mind,"
                                        " that by a perseverance of delight in the continued and indefatigable generation of knowledge,"
                                        " exceeds the short vehemence of any carnal pleasure.";
    unsigned char out_buf[1024 + 1];
    size_t buffer_len = 1024;
    ZCE_LIB::base64_encode(test_string, strlen((const char *)test_string), out_buf, &buffer_len);
    const unsigned char result_string[] = "TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIGJ1dCBieSB0aGlz"
                                          "IHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxzLCB3aGljaCBpcyBhIGx1c3Qgb2Yg"
                                          "dGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZXZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aGUgY29udGlu"
                                          "dWVkIGFuZCBpbmRlZmF0aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZGdlLCBleGNlZWRzIHRo"
                                          "ZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS4=";
    int ret = strcmp((const char *)out_buf, (const char *)result_string);
    if (ret != 0)
    {
        abort();
    }

    size_t in_len = strlen((const char *)result_string);
    buffer_len = 1024;
    ret = ZCE_LIB::base64_decode(result_string, in_len, out_buf, &buffer_len);
    out_buf[buffer_len] = '\0';
    ret = strcmp((const char *)out_buf, (const char *)test_string);
    if (ret != 0)
    {
        abort();
    }

    return 0;
}


int test_bytes_crc32(int /*argc*/, char * /*argv*/[])
{

    unsigned char test_string[] = "Man is distinguished, not only by his reason,"
                                  " but by this singular passion from other animals, which is a lust of the mind,"
                                  " that by a perseverance of delight in the continued and indefatigable generation of knowledge,"
                                  " exceeds the short vehemence of any carnal pleasure.";
    size_t str_len = strlen((const char *)test_string);

    const size_t TEST_SIZE = 10000000;

    unsigned char buffer[1024];
    memcpy(buffer, test_string, str_len + 1);

    size_t abc = 0;
    {
        ZCE_Auto_Progress_Timer auto_timer;

        for (size_t i = 0; i < TEST_SIZE; ++i)
        {
            buffer[0] += 1;
            abc = ZCE_LIB::bkdr_hash(buffer, str_len);
        }
    }
    printf("bkdr_hash = %zu\n" , abc);

    uint32_t uincrc = 0;
    memcpy(buffer, test_string, str_len + 1);

    {
        ZCE_Auto_Progress_Timer auto_timer;
        //uint32_t uincrc =0;
        for (size_t i = 0; i < TEST_SIZE; ++i)
        {
            buffer[0] += 1;
            uincrc += ZCE_LIB::crc32(buffer, str_len);
        }
    }
    printf("uincrc = %u\n" , uincrc);
    memcpy(buffer, test_string, str_len + 1);
    uincrc = 0;
    {
        ZCE_Auto_Progress_Timer auto_timer;
        //uint32_t uincrc =0;
        for (size_t i = 0; i < TEST_SIZE; ++i)
        {
            buffer[0] += 1;
            uincrc += ZCE_LIB::crc32(0, buffer, str_len);
        }
    }

    printf("uincrc = %u\n" , uincrc);
    memcpy(buffer, test_string, str_len + 1);
    {
        ZCE_Auto_Progress_Timer auto_timer;
        unsigned char result[32] = {0};
        for (size_t i = 0; i < TEST_SIZE; ++i)
        {
            buffer[0] += 1;
            ZCE_LIB::sha1(buffer, str_len, result);
        }
    }

    return 0;
}

template<typename crypt_class>
int test_encrypt()
{

    char source_buffer_1[] = "Man is distinguished, not only by his reason,"
                             " but by this singular passion from other animals, which is a lust of the mind,"
                             " that by a perseverance of delight in the continued and indefatigable generation of knowledge,"
                             " exceeds the short vehemence of any carnal pleasure.";
    char key_1[] = "1111222233334444";

    //为什么写的这样诡异，因为GCC的一个bug，如果source_buffer_1 写成 unsigned char *
    //会出现一个错误，char-array initialized from wide string
    unsigned char *source_buffer = (unsigned char *) source_buffer_1;
    unsigned char *key = (unsigned char *)key_1;

    size_t source_len = strlen((const char *)source_buffer) + 1;
    unsigned char cipher_buf[2048];
    size_t cipher_len = 2048;

    size_t key_len = 16;
    unsigned char de_buf[2048];
    size_t de_len = 2048;
    int ret = 0;

    crypt_class::ecb_encrypt(key, key_len, source_buffer, cipher_buf);
    crypt_class::ecb_decrypt(key, key_len, cipher_buf, de_buf);

    ret = memcmp(source_buffer, de_buf, crypt_class::CRYPT_BLOCK_SIZE);
    if (ret != 0)
    {
        abort();
    }


    ret = crypt_class::cbc_encrypt(key, key_len, source_buffer, source_len, cipher_buf, &cipher_len);
    if (ret != 0)
    {
        abort();
    }
    ret = crypt_class::cbc_decrypt(key, key_len, cipher_buf, cipher_len, de_buf, &de_len);
    if (ret != 0)
    {
        abort();
    }
    ret = memcmp(source_buffer, de_buf, source_len);
    if (ret != 0)
    {
        abort();
    }
    return 0;
}



template<typename crypt_class>
int perf_encrypt()
{

    char source_buffer_1[] = "123456789012345678901234567890123456789012345678901234567890"
                             "123456789012345678901234567890123456789012345678901234567890"
                             "123456789012345678901234567890123456789012345678901234567890"
                             "123456789012345678901234567890123456789012345678901234567890"
                             "123456789012345678901234567890123456789012345678901234567890";
    char key_1[] = "11112222333344445555666677778888";
    //为什么写的这样诡异，因为GCC的一个bug，如果source_buffer_1 写成 unsigned char *
    //会出现一个错误，char-array initialized from wide string
    unsigned char *source_buffer = (unsigned char *) source_buffer_1;
    unsigned char *key = (unsigned char *)key_1;

    size_t source_len = strlen((const char *)source_buffer) + 1;
    unsigned char cipher_buf[2048];
    size_t cipher_len = 2048;

    size_t key_len = 32;

    unsigned char de_buf[2048];
    size_t de_len = 2048;
    int ret = 0;

    ZCE_HR_Progress_Timer hr_timer;

    const size_t TEST_NUMBER = 20480;

    //
    hr_timer.restart();
    for (size_t i = 0; i < TEST_NUMBER; ++i)
    {
        source_buffer[0] += 1;
        key[0] += 1;
        cipher_len = 2048;
        ret = crypt_class::cbc_encrypt(key, key_len, source_buffer, source_len, cipher_buf, &cipher_len);
        if (ret != 0)
        {
            abort();
        }
    }
    hr_timer.end();
    double encrypt_usetime =  hr_timer.elapsed_usec();
    //
    hr_timer.restart();
    for (size_t i = 0; i < TEST_NUMBER; ++i)
    {
        source_buffer[0] += 1;
        key[0] += 1;
        cipher_len = 2048;
        ret = crypt_class::cbc_encrypt(key, key_len, source_buffer, source_len, cipher_buf, &cipher_len);
        if (ret != 0)
        {
            abort();
        }

        de_len = 2048;
        ret = crypt_class::cbc_decrypt(key, key_len, cipher_buf, cipher_len, de_buf, &de_len);
        if (ret != 0)
        {
            abort();
        }
    }
    hr_timer.end();
    double decrypt_usetime =  hr_timer.elapsed_usec() - encrypt_usetime;


    typename crypt_class::CRYPT_SUBKEY_STRUCT en_subkey, de_subkey;

    crypt_class::key_setup(key,
                           key_len,
                           &en_subkey,
                           true);

    crypt_class::key_setup(key,
                           key_len,
                           &de_subkey,
                           false);

    //
    hr_timer.restart();
    for (size_t i = 0; i < TEST_NUMBER; ++i)
    {
        source_buffer[0] += 1;
        key[0] += 1;
        cipher_len = 2048;
        ret = crypt_class::cbc_encrypt_skey(&en_subkey, source_buffer, source_len, cipher_buf, &cipher_len);
        if (ret != 0)
        {
            abort();
        }
    }
    hr_timer.end();
    double en_subkey_usetime =  hr_timer.elapsed_usec();

    //
    hr_timer.restart();
    for (size_t i = 0; i < TEST_NUMBER; ++i)
    {
        source_buffer[0] += 1;
        key[0] += 1;
        cipher_len = 2048;
        ret = crypt_class::cbc_encrypt_skey(&en_subkey, source_buffer, source_len, cipher_buf, &cipher_len);
        if (ret != 0)
        {
            abort();
        }

        de_len = 2048;
        ret = crypt_class::cbc_decrypt_skey(&de_subkey, cipher_buf, cipher_len, de_buf, &de_len);
        if (ret != 0)
        {
            abort();
        }
    }
    hr_timer.end();
    double de_subkey_usetime =  hr_timer.elapsed_usec() - en_subkey_usetime;

    printf("CBC [%-64s] usetime[%12.4f us][%12.4f us] times [%12.2f/s][%12.2f/s],speed [%10.3f M/s][%10.3f M/s],subkey[%8.2f][%8.2f]. \n",
           typeid(crypt_class).name(),
           encrypt_usetime,
           decrypt_usetime,
           (TEST_NUMBER * 1000000.0) / encrypt_usetime,
           (TEST_NUMBER * 1000000.0) / decrypt_usetime,
           (double(source_len) * TEST_NUMBER * 1000000) / (encrypt_usetime * 1024 * 1024),
           (double(source_len) * TEST_NUMBER * 1000000) / (decrypt_usetime * 1024 * 1024),
           (encrypt_usetime - en_subkey_usetime) / encrypt_usetime * 100.0,
           (decrypt_usetime - de_subkey_usetime) / decrypt_usetime * 100.0
          );
    return 0;
}


int bytes_encrypt_perf(int /*argc*/, char * /*argv*/[])
{


    perf_encrypt<ZCE_LIB::XOR_Crypt_128_128_1>();
    perf_encrypt<ZCE_LIB::DES_Crypt_64_64_16>();
    perf_encrypt<ZCE_LIB::DES3_Crypt_64_192_16>();
    perf_encrypt<ZCE_LIB::TEA_Crypt_64_128_16>();
    perf_encrypt<ZCE_LIB::TEA_Crypt_64_128_32>();
    perf_encrypt<ZCE_LIB::TEA_Crypt_64_128_64>();
    perf_encrypt<ZCE_LIB::XTEA_Crypt_64_128_16>();
    perf_encrypt<ZCE_LIB::XTEA_Crypt_64_128_32>();
    perf_encrypt<ZCE_LIB::XTEA_Crypt_64_128_64>();
    perf_encrypt<ZCE_LIB::XXTEA_Crypt_64_128_32>();
    perf_encrypt<ZCE_LIB::XXTEA_Crypt_128_128_16>();
    perf_encrypt<ZCE_LIB::GOST_Crypt_64_256_32>();
    perf_encrypt<ZCE_LIB::RC5_Crypt_64_128_12>();
    perf_encrypt<ZCE_LIB::RC5_Crypt_64_128_20>();
    perf_encrypt<ZCE_LIB::RC6_Crypt_128_128_12>();
    perf_encrypt<ZCE_LIB::RC6_Crypt_128_128_20>();
    perf_encrypt<ZCE_LIB::CAST5_Crypt_64_128_8>();
    perf_encrypt<ZCE_LIB::CAST5_Crypt_64_96_16>();
    perf_encrypt<ZCE_LIB::CAST5_Crypt_64_128_12>();
    perf_encrypt<ZCE_LIB::CAST6_Crypt_128_128_48>();
    perf_encrypt<ZCE_LIB::CAST6_Crypt_128_192_48>();
    perf_encrypt<ZCE_LIB::CAST6_Crypt_128_256_48>();
    perf_encrypt<ZCE_LIB::MARS_Crypt_128_128_1>();
    perf_encrypt<ZCE_LIB::MARS_Crypt_128_192_1>();
    perf_encrypt<ZCE_LIB::MARS_Crypt_128_256_1>();
    perf_encrypt<ZCE_LIB::AES_Crypt_128_128_10>();
    perf_encrypt<ZCE_LIB::AES_Crypt_128_192_12>();
    perf_encrypt<ZCE_LIB::AES_Crypt_128_256_14>();

    return 0;
}


int bytes_encrypt_right(int /*argc*/, char * /*argv*/[])
{

    test_encrypt<ZCE_LIB::XOR_Crypt_128_128_1>();

    test_encrypt<ZCE_LIB::DES_Crypt_64_64_16>();
    test_encrypt<ZCE_LIB::DES3_Crypt_64_192_16>();

    test_encrypt<ZCE_LIB::TEA_Crypt_64_128_16>();
    test_encrypt<ZCE_LIB::TEA_Crypt_64_128_32>();
    test_encrypt<ZCE_LIB::TEA_Crypt_64_128_64>();
    //测试一下特殊的轮数
    test_encrypt<ZCE_LIB::ZCE_Crypt< ZCE_LIB::TEA_ECB<19 > > >();

    test_encrypt<ZCE_LIB::XTEA_Crypt_64_128_16>();
    test_encrypt<ZCE_LIB::XTEA_Crypt_64_128_32>();
    test_encrypt<ZCE_LIB::XTEA_Crypt_64_128_64>();

    test_encrypt<ZCE_LIB::XXTEA_Crypt_64_128_32>();
    test_encrypt<ZCE_LIB::XXTEA_Crypt_128_128_16>();

    test_encrypt<ZCE_LIB::GOST_Crypt_64_256_32>();

    test_encrypt<ZCE_LIB::RC5_Crypt_64_128_12>();
    test_encrypt<ZCE_LIB::RC5_Crypt_64_128_20>();

    test_encrypt<ZCE_LIB::RC6_Crypt_128_128_12>();
    test_encrypt<ZCE_LIB::RC6_Crypt_128_128_20>();

    test_encrypt<ZCE_LIB::CAST5_Crypt_64_128_8>();
    test_encrypt<ZCE_LIB::CAST5_Crypt_64_96_16>();
    test_encrypt<ZCE_LIB::CAST5_Crypt_64_128_12>();

    test_encrypt<ZCE_LIB::CAST6_Crypt_128_128_12>();
    test_encrypt<ZCE_LIB::CAST6_Crypt_128_128_24>();
    test_encrypt<ZCE_LIB::CAST6_Crypt_128_128_36>();
    test_encrypt<ZCE_LIB::CAST6_Crypt_128_128_48>();
    test_encrypt<ZCE_LIB::CAST6_Crypt_128_192_48>();
    test_encrypt<ZCE_LIB::CAST6_Crypt_128_256_48>();


    test_encrypt<ZCE_LIB::MARS_Crypt_128_128_1>();

    test_encrypt<ZCE_LIB::AES_Crypt_128_128_10>();
    test_encrypt<ZCE_LIB::AES_Crypt_128_192_12>();
    test_encrypt<ZCE_LIB::AES_Crypt_128_256_14>();


    return 0;
}


//你要包装dst有足够的空间,其要求空间很可能比sz大，都是8字节补齐的。

#define ZCE_LZ_FAST_COPY(dst,src,sz)  {\
        unsigned char *_cpy_dst = dst; \
        const unsigned char *_cpy_src = src; \
        size_t _cpy_size = sz;\
        do \
        { \
            ZBYTE_TO_UINT64(_cpy_dst) = ZBYTE_TO_UINT64(_cpy_src); \
            _cpy_dst += sizeof(uint64_t); \
            _cpy_src += sizeof(uint64_t); \
        }while( _cpy_size > sizeof(uint64_t) && (_cpy_size -= sizeof(uint64_t))); \
    }

//你要包装dst有足够的空间，都是8字节补齐的。
#define ZCE_LZ_FAST_COPY_STOP(dst,src,stop)  \
    do \
    { \
        ZBYTE_TO_UINT64(dst) = ZBYTE_TO_UINT64(src); \
        dst += sizeof(uint64_t); \
        src += sizeof(uint64_t); \
    }while( (dst) < (stop));

int perf_fast_memcpy(size_t test_len)
{

    unsigned char *src_buf = new unsigned char [test_len + 8];
    unsigned char *dst_buf = new unsigned char [test_len + 8];
    memset(src_buf, 0, test_len);
    unsigned char *cpy_dst = NULL;
    const unsigned char *cpy_src = NULL;
    const unsigned char *stop_pos = NULL;

    ZCE_HR_Progress_Timer hr_timer;

    const size_t TEST_NUMBER = 10240;

    hr_timer.restart();
    for (size_t i = 0; i < TEST_NUMBER; ++i)
    {
        src_buf[0] += 1;
        memcpy(dst_buf, src_buf, test_len);
    }
    hr_timer.end();
    double libc_use =  hr_timer.elapsed_usec();

    hr_timer.restart();
    for (size_t i = 0; i < TEST_NUMBER; ++i)
    {
        src_buf[0] += 1;
        ZCE_LZ_FAST_COPY(dst_buf, src_buf, test_len);
    }
    hr_timer.end();
    double marco1_use =  hr_timer.elapsed_usec();



    stop_pos = dst_buf + test_len;
    hr_timer.restart();
    for (size_t i = 0; i < TEST_NUMBER; ++i)
    {
        src_buf[0] += 1;
        cpy_dst = dst_buf;
        cpy_src = src_buf;
        ZCE_LZ_FAST_COPY_STOP(cpy_dst, cpy_src, stop_pos);
    }
    hr_timer.end();
    double marco2_use =  hr_timer.elapsed_usec();

    //
    hr_timer.restart();
    for (size_t i = 0; i < TEST_NUMBER; ++i)
    {
        src_buf[0] += 1;
        ZCE_LIB::fast_memcpy(dst_buf, src_buf, test_len);
    }
    hr_timer.end();
    double fast_1_use =  hr_timer.elapsed_usec();

    hr_timer.restart();
    for (size_t i = 0; i < TEST_NUMBER; ++i)
    {
        src_buf[0] += 1;
        ZCE_LIB::fast_memcpy2(dst_buf, src_buf, test_len);
    }
    hr_timer.end();
    double fast_2_use =  hr_timer.elapsed_usec();



    printf("Memcpy copy mem len [%10zu] LibC [%12.2f/us] Marco1 [%12.2f/us] Marco2 [%12.2f/us] Fast 1 [%12.2f/us]  Fast2 [%12.2f/us] . \n",
           test_len,
           libc_use,
           marco1_use,
           marco2_use,
           fast_1_use,
           fast_2_use
          );
    delete []src_buf;
    delete []dst_buf;

    return 0;
}


int test_perf_memcpy(int /*argc*/, char * /*argv*/[])
{
    perf_fast_memcpy(8);
    perf_fast_memcpy(16);
    perf_fast_memcpy(32);
    perf_fast_memcpy(256);
    perf_fast_memcpy(512);
    perf_fast_memcpy(1024);
    perf_fast_memcpy(2048);
    perf_fast_memcpy(8196);
    perf_fast_memcpy(65536);
    perf_fast_memcpy(1024 * 1024);
    perf_fast_memcpy(4 * 1024 * 1024);
    return 0;
}


int perf_nonalign_memcpy(size_t test_len)
{

    unsigned char *src_buf = new unsigned char [test_len + 16];
    unsigned char *dst_buf = new unsigned char [test_len + 16];
    memset(src_buf, 0, test_len + 16);
    const unsigned char *cpy_src = NULL;
    const unsigned char *cpy_dst = NULL;
    const unsigned char *stop_pos = NULL;

    ZCE_HR_Progress_Timer hr_timer;

    const size_t TEST_NUMBER = 10240;

    hr_timer.restart();
    for (size_t i = 0; i < TEST_NUMBER; ++i)
    {
        src_buf[0] += 1;
        memcpy(dst_buf + (i + 1) % 8, src_buf + (i) % 8, test_len);
    }
    hr_timer.end();
    double libc_use =  hr_timer.elapsed_usec();

    hr_timer.restart();
    for (size_t i = 0; i < TEST_NUMBER; ++i)
    {
        src_buf[0] += 1;
        ZCE_LZ_FAST_COPY(dst_buf, src_buf, test_len);
    }
    hr_timer.end();
    double marco1_use =  hr_timer.elapsed_usec();


    stop_pos = dst_buf + test_len;
    hr_timer.restart();
    for (size_t i = 0; i < TEST_NUMBER; ++i)
    {
        src_buf[0] += 1;
        cpy_dst = dst_buf;
        cpy_src = src_buf;
        ZCE_LZ_FAST_COPY_STOP(cpy_dst, cpy_src, stop_pos);
    }
    hr_timer.end();
    double marco2_use =  hr_timer.elapsed_usec();


    //
    hr_timer.restart();
    for (size_t i = 0; i < TEST_NUMBER; ++i)
    {
        src_buf[0] += 1;
        ZCE_LIB::fast_memcpy(dst_buf, src_buf, test_len);
    }
    hr_timer.end();
    double fast_1_use =  hr_timer.elapsed_usec();

    hr_timer.restart();
    for (size_t i = 0; i < TEST_NUMBER; ++i)
    {
        src_buf[0] += 1;
        ZCE_LIB::fast_memcpy2(dst_buf, src_buf, test_len);
    }
    hr_timer.end();
    double fast_2_use =  hr_timer.elapsed_usec();



    printf("Memcpy copy mem len [%10zu] LibC [%12.2f/us] Marco1 [%12.2f/us] Marco2 [%12.2f/us] Fast 1 [%12.2f/us]  Fast2 [%12.2f/us] . \n",
           test_len,
           libc_use,
           marco1_use,
           marco2_use,
           fast_1_use,
           fast_2_use
          );
    delete []src_buf;
    delete []dst_buf;

    return 0;
}



int test_perf_nonalignmemcpy(int /*argc*/, char * /*argv*/[])
{
    perf_nonalign_memcpy(8);
    perf_nonalign_memcpy(16);
    perf_nonalign_memcpy(32);
    perf_nonalign_memcpy(256);
    perf_nonalign_memcpy(512);
    perf_nonalign_memcpy(1024);
    perf_nonalign_memcpy(2048);
    perf_nonalign_memcpy(8196);
    perf_nonalign_memcpy(65536);
    perf_nonalign_memcpy(1024 * 1024);
    perf_nonalign_memcpy(4 * 1024 * 1024);
    return 0;
}




int test_compress_fun(unsigned char *source_buf, size_t source_len)
{
    int ret = 0;

    size_t compressbuf_len = 0, decompress_len = 0;

    unsigned char compress_buf[1024];
    unsigned char decompress_buf[1024];

    compressbuf_len = 1024;
    decompress_len = 1024;

    ZCE_LIB::ZLZ_Compress zlz_fmt;
    ret = zlz_fmt.compress(source_buf, source_len, compress_buf, &compressbuf_len);
    if (ret != 0)
    {
        printf("%s\n", "ZLZ compress fail.\n");
        return ret;
    }

    ret = zlz_fmt.decompress(compress_buf, compressbuf_len, decompress_buf, &decompress_len);
    if (ret != 0)
    {
        printf("%s\n", "ZLZ decompress fail.\n");
        return ret;
    }

    printf("source len %zu,compressed len %zu.\n", source_len, compressbuf_len);
    printf("%s\n", source_buf);
    printf("%s\n", decompress_buf);
    ret = memcmp(source_buf, decompress_buf, source_len);
    if (ret != 0)
    {
        abort();
    }

    compressbuf_len = 1024;
    decompress_len = 1024;
    ZCE_LIB::LZ4_Compress lz4_fmt;
    ret = lz4_fmt.compress(source_buf, source_len, compress_buf, &compressbuf_len);
    if (ret != 0)
    {
        printf("%s\n", "LZ4 compress fail.\n");
        return ret;
    }

    ret = lz4_fmt.decompress(compress_buf, compressbuf_len, decompress_buf, &decompress_len);
    if (ret != 0)
    {
        printf("%s\n", "LZ4 decompress fail.\n");
        return ret;
    }

    printf("source len %zu,compressed len %zu.\n", source_len, compressbuf_len);
    printf("%s\n", source_buf);
    printf("%s\n", decompress_buf);
    ret = memcmp(source_buf, decompress_buf, source_len);
    if (ret != 0)
    {
        abort();
    }

    return 0;
}



int test_compress_filedata(const char *file_name)
{
    const size_t COMPRESS_TEXT_LEN = 120 * 1024 * 1024;
    auto *file_buffer = new unsigned char [COMPRESS_TEXT_LEN];
    size_t file_len;
    int ret = 0;
    ret = ZCE_LIB::read_file_data(file_name, (char *)file_buffer, COMPRESS_TEXT_LEN, &file_len);
    if (ret != 0)
    {
        delete []file_buffer;
        return -1;
    }
    size_t compressbuf_len = 0, decompress_len = 0;
    size_t source_len = file_len;


    ret = ZCE_LIB::LZ4_Compress::need_compressed_bufsize(file_len, &compressbuf_len);


    unsigned char *compress_buf = new unsigned char [COMPRESS_TEXT_LEN];
    unsigned char *decompress_buf = new unsigned char [COMPRESS_TEXT_LEN];

    decompress_len = COMPRESS_TEXT_LEN;
    printf("---------------------------------------------------------\n");
    ZCE_LIB::LZ4_Compress lz4;
    ret = lz4.compress(file_buffer, source_len, compress_buf, &compressbuf_len);
    if (ret != 0)
    {
        printf("%s\n", "ZCE Lz4 compress fail.\n");
        delete[] compress_buf;
        delete[] decompress_buf;
        delete[] file_buffer;
        return ret;
    }

    ret = lz4.decompress(compress_buf, compressbuf_len, decompress_buf, &decompress_len);
    if (ret != 0)
    {
        printf("%s\n", "ZCE Lz4 decompress fail.\n");
        delete[] compress_buf;
        delete[] decompress_buf;
        delete []file_buffer;
        return ret;
    }
    printf("ZCE ZL4 source len %zu,compressed len %zu.\n", source_len, compressbuf_len);


    //printf("%s\n",decompress_buf);
    ret = memcmp(file_buffer, decompress_buf, source_len);
    if (ret != 0)
    {
        //找出差异的字节
        for (size_t i = 0; i < source_len; ++i)
        {
            if (file_buffer[i] != decompress_buf[i])
            {
                printf("bytes [%zu] not eaqual\n.", i);
            }
        }
        printf("%s\n", "Compress data and  decompress data is not equal.\n");
        //abort();
    }


    printf("---------------------------------------------------------\n");

    ret = ZCE_LIB::ZLZ_Compress::need_compressed_bufsize(file_len, &compressbuf_len);
    decompress_len = COMPRESS_TEXT_LEN;
    ZCE_LIB::ZLZ_Compress zlz;
    ret = zlz.compress(file_buffer, source_len, compress_buf, &compressbuf_len);
    if (ret != 0)
    {
        printf("%s\n", "ZCE ZLZ compress fail.\n");
        delete[] compress_buf;
        delete[] decompress_buf;
        delete[] file_buffer;
        return ret;
    }

    ret = zlz.decompress(compress_buf, compressbuf_len, decompress_buf, &decompress_len);
    if (ret != 0)
    {
        printf("%s\n", "ZCE ZLZ decompress fail.\n");
        delete[] compress_buf;
        delete[] decompress_buf;
        delete[] file_buffer;
        return ret;
    }
    printf("ZCE ZLZ source len %zu,compressed len %zu.\n", source_len, compressbuf_len);

    printf("---------------------------------------------------------\n");

    //printf("%s\n",decompress_buf);
    ret = memcmp(file_buffer, decompress_buf, source_len);
    if (ret != 0)
    {
        //找出差异的字节
        for (size_t i = 0; i < source_len; ++i)
        {
            if (file_buffer[i] != decompress_buf[i])
            {
                printf("bytes [%zu] not eaqual\n.", i);
            }
        }
        printf("%s\n", "Compress data and  decompress data is not equal.\n");
        //abort();
    }

    //compressbuf_len = LZ4_compress((const char *)file_buffer,(char *)compress_buf,(int)source_len);
    //decompress_len = 40*1024*1024;
    //LZ4_decompress_safe((const char *)compress_buf,(char *)decompress_buf,(int)compressbuf_len,40*1024*1024);
    //printf("Raw LZ4 source len %lu,compressed len %lu.\n", source_len, compressbuf_len);
    printf("---------------------------------------------------------\n");
    delete[] compress_buf;
    delete[] decompress_buf;
    delete[] file_buffer;

    return 0;
}

int test_bytes_compress2(int /*argc*/, char * /*argv*/[])
{

    char test_filename[PATH_MAX + 1];
    for (size_t i = 0; i < 4168; ++i)
    {
        snprintf(test_filename, PATH_MAX, "D:\\TestDir\\%06d.dat", (int)i);
        test_compress_filedata(test_filename);
    }
    return 0;
}

int test_bytes_compress(int /*argc*/, char * /*argv*/[])
{
    
    uint8_t source_buf[1024];

#if 0
    strcpy((char *)source_buf, "123");
    test_compress_fun(source_buf, strlen((char *)source_buf) + 1);

    strcpy((char *)source_buf, "111111111111111111111");
    test_compress_fun(source_buf, strlen((char *)source_buf) + 1);

    strcpy((char *)source_buf,"11122221112211122222211211111111222112");
    test_compress_fun(source_buf,strlen((char *)source_buf) + 1);
    strcpy((char *)source_buf,"1111111111111111111111111111111111111111111111111111111111111111111111111111111111111");
    test_compress_fun(source_buf,strlen((char *)source_buf) + 1);
    strcpy((char *)source_buf,"111");
    test_compress_fun(source_buf,strlen((char *)source_buf) + 1);
    strcpy((char *)source_buf,"11111111111111111");
    test_compress_fun(source_buf,strlen((char *)source_buf) + 1);
#endif
    strcpy((char *)source_buf,"12345678123456781234567812345678123456781234567812345678123456781234567812345678123456781234567812345678");
    test_compress_fun(source_buf,strlen((char *)source_buf) + 1);
    strcpy((char *)source_buf,"123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789");
    test_compress_fun(source_buf,strlen((char *)source_buf) + 1);
    strcpy((char *)source_buf,"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890");
    test_compress_fun(source_buf,strlen((char *)source_buf) + 1);
    strcpy((char *)source_buf,"1212121212121212121212121212121212121212121212121212121212121212121212121212121212");
    test_compress_fun(source_buf,strlen((char *)source_buf) + 1);
    strcpy((char *)source_buf,"1111111111111111111111111111111111111111111111111111111111111111111111111111111111111");
    test_compress_fun(source_buf,strlen((char *)source_buf) + 1);
    strcpy((char *)source_buf,"123123123123123123123123123123123123123123123123123123123123123123123123123123123");
    test_compress_fun(source_buf,strlen((char *)source_buf) + 1);
    strcpy((char *)source_buf,"123412341234123412341234123412341234123412341234123412341234123412341234123412341234");
    test_compress_fun(source_buf,strlen((char *)source_buf) + 1);
    strcpy((char *)source_buf,"1234512345123451234512345123451234512345123451234512345123451234512345123451234512345");
    test_compress_fun(source_buf,strlen((char *)source_buf) + 1);
    strcpy((char *)source_buf,"123456123456123456123456123456123456123456123456123456123456123456123456123456123456123456123456");
    test_compress_fun(source_buf,strlen((char *)source_buf) + 1);
    strcpy((char *)source_buf,"12345671234567123456712345671234567123456712345671234567123456712345671234567123456712345671234567");
    test_compress_fun(source_buf,strlen((char *)source_buf) + 1);

    //test_compress_filedata("D:\\TestDir\\compress\\txt\\01.txt");
    //test_compress_filedata("D:\\TestDir\\compress\\txt\\02.txt");
    //test_compress_filedata("D:\\TestDir\\compress\\txt\\03.txt");
    //test_compress_filedata("D:\\TestDir\\compress\\txt\\04.txt");
    //test_compress_filedata("D:\\TestDir\\compress\\txt\\05.txt");
    //test_compress_filedata("D:\\TestDir\\compress\\txt\\06.txt");
    //test_compress_filedata("D:\\TestDir\\compress\\txt\\07.txt");
    //test_compress_filedata("D:\\TestDir\\compress\\txt\\08.txt");
    //test_compress_filedata("D:\\TestDir\\compress\\txt\\09.txt");
    //test_compress_filedata("D:\\TestDir\\compress\\txt\\10.txt");
    //test_compress_filedata("D:\\TestDir\\compress\\txt\\11.txt");
    //test_compress_filedata("D:\\TestDir\\compress\\txt\\12.txt");

    //test_compress_filedata("D:\\TestDir\\compress\\jpg\\01.jpg");
    //test_compress_filedata("D:\\TestDir\\compress\\jpg\\02.jpg");
    //test_compress_filedata("D:\\TestDir\\compress\\jpg\\03.jpg");
    //test_compress_filedata("D:\\TestDir\\compress\\jpg\\04.jpg");
    //test_compress_filedata("D:\\TestDir\\compress\\jpg\\05.jpg");

    //test_compress_filedata("D:\\TestDir\\compress\\rar\\01.rar");

    return 0;
}


#include "lz4.h"

#  pragma comment(lib, "lz4.lib")

int test_compress_fun3(unsigned char *source_buf, size_t source_len)
{
    int ret = 0;
    size_t compressbuf_len1 = 0, compressbuf_len2= 0,decompress_len = 0;

    unsigned char compress_buf1[1024];
    unsigned char compress_buf2[1024];
    //unsigned char decompress_buf[1024];


    compressbuf_len1 = 1024;
    compressbuf_len2 = 1024;
    decompress_len = 1024;

    ZCE_LIB::LZ4_Compress_Format zlz;
    zlz.compress_core(source_buf, source_len, compress_buf1, &compressbuf_len1);
    if (ret != 0)
    {
        printf("%s\n", "ZLZ compress fail.\n");
        return ret;
    }
    compressbuf_len2 = LZ4_compress((const char *)source_buf, (char *)compress_buf2, (int)source_len);
    return 0;
}

int test_bytes_compress3(int /*argc*/, char * /*argv*/[])
{
    uint8_t source_buf[1024];
    strcpy((char *)source_buf, "12345678123456781234567812345678123456781234567812345678123456781234567812345678123456781234567812345678");

    test_compress_fun3(source_buf, strlen((char *)source_buf) + 1);

    return 0;
}



int benchmark_compress(const char *file_name)
{

    ZCE_HR_Progress_Timer hr_timer;

    const size_t TEST_NUMBER = 32;
    const size_t COMPRESS_TEXT_LEN = 120 * 1024 * 1024;
    auto *file_buffer = new unsigned char [COMPRESS_TEXT_LEN];
    size_t file_len;
    int ret = 0;
    ret = ZCE_LIB::read_file_data(file_name, (char *)file_buffer, COMPRESS_TEXT_LEN, &file_len);
    if (ret != 0)
    {
        delete []file_buffer;
        return -1;
    }
    size_t compressbuf_len = 0, decompress_len = 0;
    size_t source_len = file_len;

    ret = ZCE_LIB::LZ4_Compress::need_compressed_bufsize(file_len, &compressbuf_len);
    ZCE_ASSERT(compressbuf_len < COMPRESS_TEXT_LEN);
    ret = ZCE_LIB::ZLZ_Compress::need_compressed_bufsize(file_len, &compressbuf_len);
    ZCE_ASSERT(compressbuf_len < COMPRESS_TEXT_LEN);

    auto *compress_buf = new unsigned char [COMPRESS_TEXT_LEN];
    auto *decompress_buf = new unsigned char [COMPRESS_TEXT_LEN];

    //ZCE_LIB::sleep(1);

    double compress_use = 0.0 , decompress_use = 0.0;
    
    decompress_len = COMPRESS_TEXT_LEN;
    compressbuf_len = COMPRESS_TEXT_LEN;

    compress_use = 0.0, decompress_use = 0.0;
    ZCE_LIB::LZ4_Compress lz4;
    ZCE_LIB::ZLZ_Compress zlz;

    ret = zlz.compress(file_buffer, source_len, compress_buf, &compressbuf_len);
    ret = zlz.decompress(compress_buf, compressbuf_len, decompress_buf, &decompress_len);
    ret = memcmp(file_buffer, decompress_buf, source_len);
    if (ret != 0)
    {
        for (size_t j = 0; j < source_len; j++)
        {
            if (file_buffer[j] != decompress_buf[j])
            {
                std::cout << "No " << j << std::endl;
                break;
            }
        }
        abort();
    }
    for (size_t i = 0; i < TEST_NUMBER; ++i)
    {
        file_buffer[0] += (unsigned char)i;
        compressbuf_len = COMPRESS_TEXT_LEN;

        hr_timer.restart();

        ret = zlz.compress(file_buffer, source_len, compress_buf, &compressbuf_len);

        hr_timer.end();
        compress_use += hr_timer.elapsed_usec();

        decompress_len = COMPRESS_TEXT_LEN;
        hr_timer.restart();
        ret = zlz.decompress(compress_buf, compressbuf_len, decompress_buf, &decompress_len);

        hr_timer.end();
        decompress_use += hr_timer.elapsed_usec();
    }

    printf("ZCE LZ4 source len[%10zu] compress len[%10zu] compress radio [%15.3f]"
           "compress use us[%15.3f]us [%15.3f]Mb/s decompress use [%15.3f]us [%15.3f]Mb/s.\n",
           source_len,
           compressbuf_len,
           double(source_len) / compressbuf_len,
           compress_use,
           (double(file_len)*TEST_NUMBER * 1000000.0) / (compress_use * 1024 * 1024),
           decompress_use,
           (double(file_len)*TEST_NUMBER * 1000000.0) / (decompress_use * 1024 * 1024));

    compressbuf_len = COMPRESS_TEXT_LEN;
    decompress_len = COMPRESS_TEXT_LEN;
    compressbuf_len = LZ4_compress((const char *)file_buffer, (char *)compress_buf, (int)source_len);
    LZ4_decompress_safe((const char *)compress_buf, (char *)decompress_buf, (int)compressbuf_len, COMPRESS_TEXT_LEN);
    ret = memcmp(file_buffer, decompress_buf, source_len);
    if (ret != 0)
    {
        abort();
    }
    compress_use = 0.0 , decompress_use = 0.0;
    for (size_t i = 0; i < TEST_NUMBER; ++i)
    {
        file_buffer[0] += (unsigned char)i;
        compressbuf_len = COMPRESS_TEXT_LEN;

        hr_timer.restart();

        compressbuf_len = LZ4_compress((const char *)file_buffer, (char *)compress_buf, (int)source_len);

        hr_timer.end();
        compress_use += hr_timer.elapsed_usec();

        decompress_len = COMPRESS_TEXT_LEN;
        hr_timer.restart();
        LZ4_decompress_safe((const char *)compress_buf, (char *)decompress_buf, (int)compressbuf_len, COMPRESS_TEXT_LEN);

        hr_timer.end();
        decompress_use += hr_timer.elapsed_usec();
    }
    printf("RAW LZ4 source len[%10zu] compress len[%10zu] compress radio [%15.3f]"
           "compress use us[%15.3f]us [%15.3f]Mb/s decompress use [%15.3f]us [%15.3f]Mb/s.\n",
           source_len,
           compressbuf_len,
           double(source_len) / compressbuf_len,
           compress_use,
           (double(file_len)*TEST_NUMBER * 1000000.0) / (compress_use * 1024 * 1024),
           decompress_use,
           (double(file_len)*TEST_NUMBER * 1000000.0) / (decompress_use * 1024 * 1024));

    compressbuf_len = COMPRESS_TEXT_LEN;
    decompress_len = COMPRESS_TEXT_LEN;
    ret = lz4.compress(file_buffer, source_len, compress_buf, &compressbuf_len);
    ret = lz4.decompress(compress_buf, compressbuf_len, decompress_buf, &decompress_len);
    ret = memcmp(file_buffer, decompress_buf, source_len);
    if (ret != 0)
    {
        abort();
    }
    compress_use = 0.0 , decompress_use = 0.0;
    for (size_t i = 0; i < TEST_NUMBER; ++i)
    {
        file_buffer[0] += (unsigned char)i;
        compressbuf_len = COMPRESS_TEXT_LEN;

        hr_timer.restart();
        ret = lz4.compress(file_buffer, source_len, compress_buf, &compressbuf_len);
        hr_timer.end();
        compress_use += hr_timer.elapsed_usec();

        decompress_len = COMPRESS_TEXT_LEN;
        hr_timer.restart();
        ret = lz4.decompress(compress_buf, compressbuf_len, decompress_buf, &decompress_len);
        hr_timer.end();
        decompress_use += hr_timer.elapsed_usec();
    }

    printf ("ZCE ZLZ source len[%10zu] compress len[%10zu] compress radio [%15.3f]"
            "compress use us[%15.3f]us [%15.3f]Mb/s decompress use [%15.3f]us [%15.3f]Mb/s.\n",
            source_len,
            compressbuf_len,
            double(source_len) / compressbuf_len,
            compress_use,
            (double(file_len)*TEST_NUMBER * 1000000.0) / (compress_use *1024 * 1024 ),
            decompress_use,
            (double (file_len)*TEST_NUMBER * 1000000.0) / (decompress_use *1024 * 1024 ));

    hr_timer.restart();
    for (size_t i = 0; i < TEST_NUMBER; ++i)
    {
        file_buffer[0] += (unsigned char)i;
        ZCE_LZ_FAST_COPY(decompress_buf, file_buffer, file_len);
    }
    hr_timer.end();
    double memcpy_use = hr_timer.elapsed_usec();

    printf ("MEMCPY [%10zu] memcpy use us[%15.3f]us [%15.3f]Mb/s .\n",
            file_len,
            memcpy_use,
            double (file_len)*TEST_NUMBER * 1000000.0 / (memcpy_use *1024 * 1024)
           );

    delete[] compress_buf;
    delete[] decompress_buf;
    delete[] file_buffer;

    return 0;
}

struct DR_DATA_1
{

    int d1_a1_ = 1;
    short d1_a2_ = 2;

    int64_t d1_a3_ = 3;
    uint64_t d1_a4_=4;
    
    float d1_b1_ = 3.0;
    double d1_b2_ = 4.001;

    std::string d1_c1_;
    std::string d1_c2_;

    static const size_t D1_C3_LEN = 6;
    static const size_t D1_C4_LEN = 8;

    char d1_c3_[D1_C3_LEN];
    int d1_c4_[D1_C4_LEN];

    
    std::vector<int> d1_d1_;

    std::list<int> d1_e1_;

    std::map<int, int> d1_f1_;

    template<typename serialize_type>
    void serialize(typename serialize_type &ss, unsigned int /*version*/ = 0)
    {
        ss & d1_a1_;
        ss & d1_a2_;
        ss & d1_a3_;
        ss & d1_a4_;

        ss & d1_b1_;
        ss & d1_b2_;

        ss & d1_c1_;
        ss & d1_c2_;
        ss & d1_c3_;
        ss & d1_c4_;

        ss & d1_d1_;
        ss & d1_e1_;
        ss & d1_f1_;
    }
};


struct DR_DATA_2
{
    static const size_t ARY_SIZE = 2048;
    int a1_ = 1;
    float b1_ = 2.0;
    double b2_ = 3.001;

    char c1_[ARY_SIZE];
    double c2_[ARY_SIZE];
    int c3_[ARY_SIZE];

    unsigned int d_num_ = 0;
    unsigned short d_ary_[ARY_SIZE];

    std::vector<int> e_vector_;
};





int test_bytes_data_represent(int /*argc*/, char * /*argv */[])
{
    const size_t SIZE_OF_BUFFER = 1024;
    char buffer_data1[SIZE_OF_BUFFER];

    DR_DATA_1 data1;
    data1.d1_a1_ = 1;
    data1.d1_a1_ = 2;

    data1.d1_b1_ = 2.02f;
    data1.d1_b2_ = 3.03;

    data1.d1_c1_ = "I love beijing tiananmen.";
    data1.d1_c2_ = "I'll stand before the lord of song With nothing on my tongue but Hallelujah";
    for (size_t i = 0; i < DR_DATA_1::D1_C3_LEN; ++i)
    {
        data1.d1_c3_[i] = static_cast<char>('A' + i);
    }

    for (size_t i = 0; i < DR_DATA_1::D1_C4_LEN; ++i)
    {
        data1.d1_c4_[i] = 100000 + (int) i;
    }

    data1.d1_d1_.push_back(888);
    data1.d1_d1_.push_back(8888);
    data1.d1_d1_.push_back(88888);

    data1.d1_e1_.push_back(66);
    data1.d1_e1_.push_back(666);
    data1.d1_e1_.push_back(6666);

    data1.d1_f1_[3] = 555;
    data1.d1_f1_[33] = 5555;
    data1.d1_f1_[333] = 55555;

    ZCE_Serialize_Write ssave(buffer_data1, SIZE_OF_BUFFER);
    data1.serialize(ssave);
    if (ssave.is_good())
    {
        std::cout << "Use len " << ssave.write_len() <<std::endl;
    }
    else
    {
        return -1;
    }

    DR_DATA_1 data2;
    ZCE_Serialize_Read sload(buffer_data1, SIZE_OF_BUFFER);
    data2.serialize(sload);

    return 0;
}
