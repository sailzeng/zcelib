#include "zce/predefine.h"
#include "zce/logger/logging.h"
#include "zce/os_adapt/string.h"
#include "zce/bytes/hash_value.h"

namespace zce
{
//================================================================================================
//MD4的算法

//初始化MD4的context，内容
void hash_md4::initialize(context* ctx)
{
    ctx->length_ = 0;
    ctx->unprocessed_ = 0;

    /* initialize state */
    ctx->hash_[0] = 0x67452301;
    ctx->hash_[1] = 0xefcdab89;
    ctx->hash_[2] = 0x98badcfe;
    ctx->hash_[3] = 0x10325476;
}

#define MD4_F(x, y, z) ((((y) ^ (z)) & (x)) ^ (z))
#define MD4_G(x, y, z) (((x) & (y)) | ((x) & (z)) | ((y) & (z)))
#define MD4_H(x, y, z) ((x) ^ (y) ^ (z))

/* transformations for rounds 1, 2, and 3. */
#define MD4_ROUND1(a, b, c, d, x, s) { \
	(a) += MD4_F((b), (c), (d)) + (x); \
	(a) = ZCE_ROTL32((a), (s)); \
}
#define MD4_ROUND2(a, b, c, d, x, s) { \
	(a) += MD4_G((b), (c), (d)) + (x) + 0x5a827999; \
	(a) = ZCE_ROTL32((a), (s)); \
}
#define MD4_ROUND3(a, b, c, d, x, s) { \
	(a) += MD4_H((b), (c), (d)) + (x) + 0x6ed9eba1; \
	(a) = ZCE_ROTL32((a), (s)); \
}

void hash_md4::process_block(uint32_t state[HASH_RESULT_SIZE / 4],
                             const uint32_t block[PROCESS_BLOCK_SIZE / 4])
{
    uint32_t a, b, c, d;
    const uint32_t* x = nullptr;
    a = state[0];
    b = state[1];
    c = state[2];
    d = state[3];

    //MD5的代码用的会比较多，追求极致速度了，
#if ZCE_ENDIAN_ORDER == ZCE_ENDIAN_LITTLE
    //小头序下,假设是x86,x64，不考虑对齐的麻烦，直接上，
    //当然如果不是对其环境,理论上转换一次对齐也许更快
    x = block;
#else
    //注意下面的转换同时也避免了对齐偏移的拷贝问题
    uint32_t  wblock[PROCESS_BLOCK_SIZE / 4];
    endian_copy(wblock, block, PROCESS_BLOCK_SIZE);
    x = wblock;
#endif

    MD4_ROUND1(a, b, c, d, x[0], 3);
    MD4_ROUND1(d, a, b, c, x[1], 7);
    MD4_ROUND1(c, d, a, b, x[2], 11);
    MD4_ROUND1(b, c, d, a, x[3], 19);
    MD4_ROUND1(a, b, c, d, x[4], 3);
    MD4_ROUND1(d, a, b, c, x[5], 7);
    MD4_ROUND1(c, d, a, b, x[6], 11);
    MD4_ROUND1(b, c, d, a, x[7], 19);
    MD4_ROUND1(a, b, c, d, x[8], 3);
    MD4_ROUND1(d, a, b, c, x[9], 7);
    MD4_ROUND1(c, d, a, b, x[10], 11);
    MD4_ROUND1(b, c, d, a, x[11], 19);
    MD4_ROUND1(a, b, c, d, x[12], 3);
    MD4_ROUND1(d, a, b, c, x[13], 7);
    MD4_ROUND1(c, d, a, b, x[14], 11);
    MD4_ROUND1(b, c, d, a, x[15], 19);

    MD4_ROUND2(a, b, c, d, x[0], 3);
    MD4_ROUND2(d, a, b, c, x[4], 5);
    MD4_ROUND2(c, d, a, b, x[8], 9);
    MD4_ROUND2(b, c, d, a, x[12], 13);
    MD4_ROUND2(a, b, c, d, x[1], 3);
    MD4_ROUND2(d, a, b, c, x[5], 5);
    MD4_ROUND2(c, d, a, b, x[9], 9);
    MD4_ROUND2(b, c, d, a, x[13], 13);
    MD4_ROUND2(a, b, c, d, x[2], 3);
    MD4_ROUND2(d, a, b, c, x[6], 5);
    MD4_ROUND2(c, d, a, b, x[10], 9);
    MD4_ROUND2(b, c, d, a, x[14], 13);
    MD4_ROUND2(a, b, c, d, x[3], 3);
    MD4_ROUND2(d, a, b, c, x[7], 5);
    MD4_ROUND2(c, d, a, b, x[11], 9);
    MD4_ROUND2(b, c, d, a, x[15], 13);

    MD4_ROUND3(a, b, c, d, x[0], 3);
    MD4_ROUND3(d, a, b, c, x[8], 9);
    MD4_ROUND3(c, d, a, b, x[4], 11);
    MD4_ROUND3(b, c, d, a, x[12], 15);
    MD4_ROUND3(a, b, c, d, x[2], 3);
    MD4_ROUND3(d, a, b, c, x[10], 9);
    MD4_ROUND3(c, d, a, b, x[6], 11);
    MD4_ROUND3(b, c, d, a, x[14], 15);
    MD4_ROUND3(a, b, c, d, x[1], 3);
    MD4_ROUND3(d, a, b, c, x[9], 9);
    MD4_ROUND3(c, d, a, b, x[5], 11);
    MD4_ROUND3(b, c, d, a, x[13], 15);
    MD4_ROUND3(a, b, c, d, x[3], 3);
    MD4_ROUND3(d, a, b, c, x[11], 9);
    MD4_ROUND3(c, d, a, b, x[7], 11);
    MD4_ROUND3(b, c, d, a, x[15], 15);

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;

    return;
}

//================================================================================================
//MD5的算法

//初始化MD5的context，内容
void hash_md5::initialize(context* ctx)
{
    ctx->length_ = 0;
    ctx->unprocessed_ = 0;

    /* initialize state */
    ctx->hash_[0] = 0x67452301;
    ctx->hash_[1] = 0xefcdab89;
    ctx->hash_[2] = 0x98badcfe;
    ctx->hash_[3] = 0x10325476;
}

/* First, define four auxiliary functions that each take as input
 * three 32-bit words and returns a 32-bit word.*/

 /* F(x,y,z) = ((y XOR z) AND x) XOR z - is faster then original version */
#define MD5_F(x, y, z) ((((y) ^ (z)) & (x)) ^ (z))
#define MD5_G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define MD5_H(x, y, z) ((x) ^ (y) ^ (z))
#define MD5_I(x, y, z) ((y) ^ ((x) | (~z)))

/* transformations for rounds 1, 2, 3, and 4. */
#define MD5_ROUND1(a, b, c, d, x, s, ac) { \
        (a) += MD5_F((b), (c), (d)) + (x) + (ac); \
        (a) = ZCE_ROTL32((a), (s)); \
        (a) += (b); \
    }
#define MD5_ROUND2(a, b, c, d, x, s, ac) { \
        (a) += MD5_G((b), (c), (d)) + (x) + (ac); \
        (a) = ZCE_ROTL32((a), (s)); \
        (a) += (b); \
    }
#define MD5_ROUND3(a, b, c, d, x, s, ac) { \
        (a) += MD5_H((b), (c), (d)) + (x) + (ac); \
        (a) = ZCE_ROTL32((a), (s)); \
        (a) += (b); \
    }
#define MD5_ROUND4(a, b, c, d, x, s, ac) { \
        (a) += MD5_I((b), (c), (d)) + (x) + (ac); \
        (a) = ZCE_ROTL32((a), (s)); \
        (a) += (b); \
    }

//将64个字节，16个uint32_t的数组进行摘要（杂凑）处理，处理的数据自己序是小头数据
void hash_md5::process_block(uint32_t state[HASH_RESULT_SIZE / 4],
                             const uint32_t block[PROCESS_BLOCK_SIZE / 4])
{
    uint32_t a, b, c, d;
    const uint32_t* x = nullptr;
    a = state[0];
    b = state[1];
    c = state[2];
    d = state[3];

    //MD5的代码用的会比较多，追求极致速度了，
#if ZCE_ENDIAN_ORDER == ZCE_ENDIAN_LITTLE
    //小头序下,假设是x86,x64，不考虑对齐的麻烦，直接上，
    //当然如果不是对其环境,理论上转换一次对齐也许更快
    x = block;
#else
    //注意下面的转换同时也避免了对齐偏移的拷贝问题
    uint32_t  wblock[PROCESS_BLOCK_SIZE / 4];
    endian_copy(wblock, block, PROCESS_BLOCK_SIZE);
    x = wblock;
#endif

    MD5_ROUND1(a, b, c, d, x[0], 7, 0xd76aa478);
    MD5_ROUND1(d, a, b, c, x[1], 12, 0xe8c7b756);
    MD5_ROUND1(c, d, a, b, x[2], 17, 0x242070db);
    MD5_ROUND1(b, c, d, a, x[3], 22, 0xc1bdceee);
    MD5_ROUND1(a, b, c, d, x[4], 7, 0xf57c0faf);
    MD5_ROUND1(d, a, b, c, x[5], 12, 0x4787c62a);
    MD5_ROUND1(c, d, a, b, x[6], 17, 0xa8304613);
    MD5_ROUND1(b, c, d, a, x[7], 22, 0xfd469501);
    MD5_ROUND1(a, b, c, d, x[8], 7, 0x698098d8);
    MD5_ROUND1(d, a, b, c, x[9], 12, 0x8b44f7af);
    MD5_ROUND1(c, d, a, b, x[10], 17, 0xffff5bb1);
    MD5_ROUND1(b, c, d, a, x[11], 22, 0x895cd7be);
    MD5_ROUND1(a, b, c, d, x[12], 7, 0x6b901122);
    MD5_ROUND1(d, a, b, c, x[13], 12, 0xfd987193);
    MD5_ROUND1(c, d, a, b, x[14], 17, 0xa679438e);
    MD5_ROUND1(b, c, d, a, x[15], 22, 0x49b40821);

    MD5_ROUND2(a, b, c, d, x[1], 5, 0xf61e2562);
    MD5_ROUND2(d, a, b, c, x[6], 9, 0xc040b340);
    MD5_ROUND2(c, d, a, b, x[11], 14, 0x265e5a51);
    MD5_ROUND2(b, c, d, a, x[0], 20, 0xe9b6c7aa);
    MD5_ROUND2(a, b, c, d, x[5], 5, 0xd62f105d);
    MD5_ROUND2(d, a, b, c, x[10], 9, 0x2441453);
    MD5_ROUND2(c, d, a, b, x[15], 14, 0xd8a1e681);
    MD5_ROUND2(b, c, d, a, x[4], 20, 0xe7d3fbc8);
    MD5_ROUND2(a, b, c, d, x[9], 5, 0x21e1cde6);
    MD5_ROUND2(d, a, b, c, x[14], 9, 0xc33707d6);
    MD5_ROUND2(c, d, a, b, x[3], 14, 0xf4d50d87);
    MD5_ROUND2(b, c, d, a, x[8], 20, 0x455a14ed);
    MD5_ROUND2(a, b, c, d, x[13], 5, 0xa9e3e905);
    MD5_ROUND2(d, a, b, c, x[2], 9, 0xfcefa3f8);
    MD5_ROUND2(c, d, a, b, x[7], 14, 0x676f02d9);
    MD5_ROUND2(b, c, d, a, x[12], 20, 0x8d2a4c8a);

    MD5_ROUND3(a, b, c, d, x[5], 4, 0xfffa3942);
    MD5_ROUND3(d, a, b, c, x[8], 11, 0x8771f681);
    MD5_ROUND3(c, d, a, b, x[11], 16, 0x6d9d6122);
    MD5_ROUND3(b, c, d, a, x[14], 23, 0xfde5380c);
    MD5_ROUND3(a, b, c, d, x[1], 4, 0xa4beea44);
    MD5_ROUND3(d, a, b, c, x[4], 11, 0x4bdecfa9);
    MD5_ROUND3(c, d, a, b, x[7], 16, 0xf6bb4b60);
    MD5_ROUND3(b, c, d, a, x[10], 23, 0xbebfbc70);
    MD5_ROUND3(a, b, c, d, x[13], 4, 0x289b7ec6);
    MD5_ROUND3(d, a, b, c, x[0], 11, 0xeaa127fa);
    MD5_ROUND3(c, d, a, b, x[3], 16, 0xd4ef3085);
    MD5_ROUND3(b, c, d, a, x[6], 23, 0x4881d05);
    MD5_ROUND3(a, b, c, d, x[9], 4, 0xd9d4d039);
    MD5_ROUND3(d, a, b, c, x[12], 11, 0xe6db99e5);
    MD5_ROUND3(c, d, a, b, x[15], 16, 0x1fa27cf8);
    MD5_ROUND3(b, c, d, a, x[2], 23, 0xc4ac5665);

    MD5_ROUND4(a, b, c, d, x[0], 6, 0xf4292244);
    MD5_ROUND4(d, a, b, c, x[7], 10, 0x432aff97);
    MD5_ROUND4(c, d, a, b, x[14], 15, 0xab9423a7);
    MD5_ROUND4(b, c, d, a, x[5], 21, 0xfc93a039);
    MD5_ROUND4(a, b, c, d, x[12], 6, 0x655b59c3);
    MD5_ROUND4(d, a, b, c, x[3], 10, 0x8f0ccc92);
    MD5_ROUND4(c, d, a, b, x[10], 15, 0xffeff47d);
    MD5_ROUND4(b, c, d, a, x[1], 21, 0x85845dd1);
    MD5_ROUND4(a, b, c, d, x[8], 6, 0x6fa87e4f);
    MD5_ROUND4(d, a, b, c, x[15], 10, 0xfe2ce6e0);
    MD5_ROUND4(c, d, a, b, x[6], 15, 0xa3014314);
    MD5_ROUND4(b, c, d, a, x[13], 21, 0x4e0811a1);
    MD5_ROUND4(a, b, c, d, x[4], 6, 0xf7537e82);
    MD5_ROUND4(d, a, b, c, x[11], 10, 0xbd3af235);
    MD5_ROUND4(c, d, a, b, x[2], 15, 0x2ad7d2bb);
    MD5_ROUND4(b, c, d, a, x[9], 21, 0xeb86d391);

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
}

//================================================================================================
//SHA1的算法

//SHA1算法的上下文的初始化
void hash_sha1::initialize(context* ctx)
{
    ctx->length_ = 0;
    ctx->unprocessed_ = 0;
    // 初始化算法的几个常量，魔术数
    ctx->hash_[0] = 0x67452301;
    ctx->hash_[1] = 0xefcdab89;
    ctx->hash_[2] = 0x98badcfe;
    ctx->hash_[3] = 0x10325476;
    ctx->hash_[4] = 0xc3d2e1f0;
}

//内部函数，对一个64bit内存块进行杂凑处理，
void hash_sha1::process_block(uint32_t hash[HASH_RESULT_SIZE / 4],
                              const uint32_t block[PROCESS_BLOCK_SIZE / 4])
{
    size_t        t;
    uint32_t      wblock[80];
    uint32_t      a, b, c, d, e, temp;

    endian_copy(wblock, block, PROCESS_BLOCK_SIZE);

    //处理
    for (t = 16; t < 80; t++)
    {
        wblock[t] = ZCE_ROTL32(wblock[t - 3] ^ wblock[t - 8] ^ wblock[t - 14] ^ wblock[t - 16], 1);
    }

    a = hash[0];
    b = hash[1];
    c = hash[2];
    d = hash[3];
    e = hash[4];

    for (t = 0; t < 20; t++)
    {
        /* the following is faster than ((b & c) | ((~b) & d)) */
        temp = ZCE_ROTL32(a, 5) + (((c ^ d) & b) ^ d)
            + e + wblock[t] + 0x5A827999;
        e = d;
        d = c;
        c = ZCE_ROTL32(b, 30);
        b = a;
        a = temp;
    }

    for (t = 20; t < 40; t++)
    {
        temp = ZCE_ROTL32(a, 5) + (b ^ c ^ d) + e + wblock[t] + 0x6ED9EBA1;
        e = d;
        d = c;
        c = ZCE_ROTL32(b, 30);
        b = a;
        a = temp;
    }

    for (t = 40; t < 60; t++)
    {
        temp = ZCE_ROTL32(a, 5) + ((b & c) | (b & d) | (c & d))
            + e + wblock[t] + 0x8F1BBCDC;
        e = d;
        d = c;
        c = ZCE_ROTL32(b, 30);
        b = a;
        a = temp;
    }

    for (t = 60; t < 80; t++)
    {
        temp = ZCE_ROTL32(a, 5) + (b ^ c ^ d) + e + wblock[t] + 0xCA62C1D6;
        e = d;
        d = c;
        c = ZCE_ROTL32(b, 30);
        b = a;
        a = temp;
    }

    hash[0] += a;
    hash[1] += b;
    hash[2] += c;
    hash[3] += d;
    hash[4] += e;
}

//================================================================================================
//SHA256的算法

#define SHR(x,n) ((x & 0xFFFFFFFF) >> n)

#define SHA2_S0(x) (ZCE_ROTR32(x, 7) ^ ZCE_ROTR32(x,18) ^  SHR(x, 3))
#define SHA2_S1(x) (ZCE_ROTR32(x,17) ^ ZCE_ROTR32(x,19) ^  SHR(x,10))

#define SHA2_S2(x) (ZCE_ROTR32(x, 2) ^ ZCE_ROTR32(x,13) ^ ZCE_ROTR32(x,22))
#define SHA2_S3(x) (ZCE_ROTR32(x, 6) ^ ZCE_ROTR32(x,11) ^ ZCE_ROTR32(x,25))

#define F0(x,y,z) ((x & y) | (z & (x | y)))
#define CAST_F1(x,y,z) (z ^ (x & (y ^ z)))

#define SHA2_R(t)                                                            \
                  (                                                          \
                      wblock[t] = SHA2_S1(wblock[t -  2]) + wblock[t -  7] + \
                      SHA2_S0(wblock[t - 15]) + wblock[t - 16]               \
                  )

#define SHA2_P(a,b,c,d,e,f,g,h,x,K)                       \
    {                                                     \
        temp1 = h + SHA2_S3(e) + CAST_F1(e,f,g) + K + x;  \
        temp2 = SHA2_S2(a) + F0(a,b,c);                   \
        d += temp1; h = temp1 + temp2;                    \
    }

//SHA256算法的上下文的初始化
void hash_sha256::initialize(context* ctx)
{
    ctx->length_ = 0;
    ctx->unprocessed_ = 0;
    // 初始化算法的几个常量，魔术数
    ctx->hash_[0] = 0x6a09e667;
    ctx->hash_[1] = 0xbb67ae85;
    ctx->hash_[2] = 0x3c6ef372;
    ctx->hash_[3] = 0xa54ff53a;
    ctx->hash_[4] = 0x510e527f;
    ctx->hash_[5] = 0x9b05688c;
    ctx->hash_[6] = 0x1f83d9ab;
    ctx->hash_[7] = 0x5be0cd19;
}

//对一个64bit内存块进行杂凑处理，
void hash_sha256::process_block(uint32_t hash[HASH_RESULT_SIZE / 4],
                                const uint32_t block[PROCESS_BLOCK_SIZE / 4])
{
    uint32_t  wblock[64];
    uint32_t a, b, c, d, e, f, g, h, temp1, temp2;

    endian_copy(wblock, block, PROCESS_BLOCK_SIZE);

    a = hash[0];
    b = hash[1];
    c = hash[2];
    d = hash[3];
    e = hash[4];
    f = hash[5];
    g = hash[6];
    h = hash[7];

    SHA2_P(a, b, c, d, e, f, g, h, wblock[0], 0x428A2F98);
    SHA2_P(h, a, b, c, d, e, f, g, wblock[1], 0x71374491);
    SHA2_P(g, h, a, b, c, d, e, f, wblock[2], 0xB5C0FBCF);
    SHA2_P(f, g, h, a, b, c, d, e, wblock[3], 0xE9B5DBA5);
    SHA2_P(e, f, g, h, a, b, c, d, wblock[4], 0x3956C25B);
    SHA2_P(d, e, f, g, h, a, b, c, wblock[5], 0x59F111F1);
    SHA2_P(c, d, e, f, g, h, a, b, wblock[6], 0x923F82A4);
    SHA2_P(b, c, d, e, f, g, h, a, wblock[7], 0xAB1C5ED5);
    SHA2_P(a, b, c, d, e, f, g, h, wblock[8], 0xD807AA98);
    SHA2_P(h, a, b, c, d, e, f, g, wblock[9], 0x12835B01);
    SHA2_P(g, h, a, b, c, d, e, f, wblock[10], 0x243185BE);
    SHA2_P(f, g, h, a, b, c, d, e, wblock[11], 0x550C7DC3);
    SHA2_P(e, f, g, h, a, b, c, d, wblock[12], 0x72BE5D74);
    SHA2_P(d, e, f, g, h, a, b, c, wblock[13], 0x80DEB1FE);
    SHA2_P(c, d, e, f, g, h, a, b, wblock[14], 0x9BDC06A7);
    SHA2_P(b, c, d, e, f, g, h, a, wblock[15], 0xC19BF174);
    SHA2_P(a, b, c, d, e, f, g, h, SHA2_R(16), 0xE49B69C1);
    SHA2_P(h, a, b, c, d, e, f, g, SHA2_R(17), 0xEFBE4786);
    SHA2_P(g, h, a, b, c, d, e, f, SHA2_R(18), 0x0FC19DC6);
    SHA2_P(f, g, h, a, b, c, d, e, SHA2_R(19), 0x240CA1CC);
    SHA2_P(e, f, g, h, a, b, c, d, SHA2_R(20), 0x2DE92C6F);
    SHA2_P(d, e, f, g, h, a, b, c, SHA2_R(21), 0x4A7484AA);
    SHA2_P(c, d, e, f, g, h, a, b, SHA2_R(22), 0x5CB0A9DC);
    SHA2_P(b, c, d, e, f, g, h, a, SHA2_R(23), 0x76F988DA);
    SHA2_P(a, b, c, d, e, f, g, h, SHA2_R(24), 0x983E5152);
    SHA2_P(h, a, b, c, d, e, f, g, SHA2_R(25), 0xA831C66D);
    SHA2_P(g, h, a, b, c, d, e, f, SHA2_R(26), 0xB00327C8);
    SHA2_P(f, g, h, a, b, c, d, e, SHA2_R(27), 0xBF597FC7);
    SHA2_P(e, f, g, h, a, b, c, d, SHA2_R(28), 0xC6E00BF3);
    SHA2_P(d, e, f, g, h, a, b, c, SHA2_R(29), 0xD5A79147);
    SHA2_P(c, d, e, f, g, h, a, b, SHA2_R(30), 0x06CA6351);
    SHA2_P(b, c, d, e, f, g, h, a, SHA2_R(31), 0x14292967);
    SHA2_P(a, b, c, d, e, f, g, h, SHA2_R(32), 0x27B70A85);
    SHA2_P(h, a, b, c, d, e, f, g, SHA2_R(33), 0x2E1B2138);
    SHA2_P(g, h, a, b, c, d, e, f, SHA2_R(34), 0x4D2C6DFC);
    SHA2_P(f, g, h, a, b, c, d, e, SHA2_R(35), 0x53380D13);
    SHA2_P(e, f, g, h, a, b, c, d, SHA2_R(36), 0x650A7354);
    SHA2_P(d, e, f, g, h, a, b, c, SHA2_R(37), 0x766A0ABB);
    SHA2_P(c, d, e, f, g, h, a, b, SHA2_R(38), 0x81C2C92E);
    SHA2_P(b, c, d, e, f, g, h, a, SHA2_R(39), 0x92722C85);
    SHA2_P(a, b, c, d, e, f, g, h, SHA2_R(40), 0xA2BFE8A1);
    SHA2_P(h, a, b, c, d, e, f, g, SHA2_R(41), 0xA81A664B);
    SHA2_P(g, h, a, b, c, d, e, f, SHA2_R(42), 0xC24B8B70);
    SHA2_P(f, g, h, a, b, c, d, e, SHA2_R(43), 0xC76C51A3);
    SHA2_P(e, f, g, h, a, b, c, d, SHA2_R(44), 0xD192E819);
    SHA2_P(d, e, f, g, h, a, b, c, SHA2_R(45), 0xD6990624);
    SHA2_P(c, d, e, f, g, h, a, b, SHA2_R(46), 0xF40E3585);
    SHA2_P(b, c, d, e, f, g, h, a, SHA2_R(47), 0x106AA070);
    SHA2_P(a, b, c, d, e, f, g, h, SHA2_R(48), 0x19A4C116);
    SHA2_P(h, a, b, c, d, e, f, g, SHA2_R(49), 0x1E376C08);
    SHA2_P(g, h, a, b, c, d, e, f, SHA2_R(50), 0x2748774C);
    SHA2_P(f, g, h, a, b, c, d, e, SHA2_R(51), 0x34B0BCB5);
    SHA2_P(e, f, g, h, a, b, c, d, SHA2_R(52), 0x391C0CB3);
    SHA2_P(d, e, f, g, h, a, b, c, SHA2_R(53), 0x4ED8AA4A);
    SHA2_P(c, d, e, f, g, h, a, b, SHA2_R(54), 0x5B9CCA4F);
    SHA2_P(b, c, d, e, f, g, h, a, SHA2_R(55), 0x682E6FF3);
    SHA2_P(a, b, c, d, e, f, g, h, SHA2_R(56), 0x748F82EE);
    SHA2_P(h, a, b, c, d, e, f, g, SHA2_R(57), 0x78A5636F);
    SHA2_P(g, h, a, b, c, d, e, f, SHA2_R(58), 0x84C87814);
    SHA2_P(f, g, h, a, b, c, d, e, SHA2_R(59), 0x8CC70208);
    SHA2_P(e, f, g, h, a, b, c, d, SHA2_R(60), 0x90BEFFFA);
    SHA2_P(d, e, f, g, h, a, b, c, SHA2_R(61), 0xA4506CEB);
    SHA2_P(c, d, e, f, g, h, a, b, SHA2_R(62), 0xBEF9A3F7);
    SHA2_P(b, c, d, e, f, g, h, a, SHA2_R(63), 0xC67178F2);

    hash[0] += a;
    hash[1] += b;
    hash[2] += c;
    hash[3] += d;
    hash[4] += e;
    hash[5] += f;
    hash[6] += g;
    hash[7] += h;
}

//================================================================================================
//CRC32算法的函数和策略类

static const uint32_t ZCE_CRC32_TABLE[256] =
{
    0x00000000,0x77073096,0xee0e612c,0x990951ba,0x076dc419,0x706af48f,0xe963a535,0x9e6495a3,
    0x0edb8832,0x79dcb8a4,0xe0d5e91e,0x97d2d988,0x09b64c2b,0x7eb17cbd,0xe7b82d07,0x90bf1d91,
    0x1db71064,0x6ab020f2,0xf3b97148,0x84be41de,0x1adad47d,0x6ddde4eb,0xf4d4b551,0x83d385c7,
    0x136c9856,0x646ba8c0,0xfd62f97a,0x8a65c9ec,0x14015c4f,0x63066cd9,0xfa0f3d63,0x8d080df5,
    0x3b6e20c8,0x4c69105e,0xd56041e4,0xa2677172,0x3c03e4d1,0x4b04d447,0xd20d85fd,0xa50ab56b,
    0x35b5a8fa,0x42b2986c,0xdbbbc9d6,0xacbcf940,0x32d86ce3,0x45df5c75,0xdcd60dcf,0xabd13d59,
    0x26d930ac,0x51de003a,0xc8d75180,0xbfd06116,0x21b4f4b5,0x56b3c423,0xcfba9599,0xb8bda50f,
    0x2802b89e,0x5f058808,0xc60cd9b2,0xb10be924,0x2f6f7c87,0x58684c11,0xc1611dab,0xb6662d3d,
    0x76dc4190,0x01db7106,0x98d220bc,0xefd5102a,0x71b18589,0x06b6b51f,0x9fbfe4a5,0xe8b8d433,
    0x7807c9a2,0x0f00f934,0x9609a88e,0xe10e9818,0x7f6a0dbb,0x086d3d2d,0x91646c97,0xe6635c01,
    0x6b6b51f4,0x1c6c6162,0x856530d8,0xf262004e,0x6c0695ed,0x1b01a57b,0x8208f4c1,0xf50fc457,
    0x65b0d9c6,0x12b7e950,0x8bbeb8ea,0xfcb9887c,0x62dd1ddf,0x15da2d49,0x8cd37cf3,0xfbd44c65,
    0x4db26158,0x3ab551ce,0xa3bc0074,0xd4bb30e2,0x4adfa541,0x3dd895d7,0xa4d1c46d,0xd3d6f4fb,
    0x4369e96a,0x346ed9fc,0xad678846,0xda60b8d0,0x44042d73,0x33031de5,0xaa0a4c5f,0xdd0d7cc9,
    0x5005713c,0x270241aa,0xbe0b1010,0xc90c2086,0x5768b525,0x206f85b3,0xb966d409,0xce61e49f,
    0x5edef90e,0x29d9c998,0xb0d09822,0xc7d7a8b4,0x59b33d17,0x2eb40d81,0xb7bd5c3b,0xc0ba6cad,
    0xedb88320,0x9abfb3b6,0x03b6e20c,0x74b1d29a,0xead54739,0x9dd277af,0x04db2615,0x73dc1683,
    0xe3630b12,0x94643b84,0x0d6d6a3e,0x7a6a5aa8,0xe40ecf0b,0x9309ff9d,0x0a00ae27,0x7d079eb1,
    0xf00f9344,0x8708a3d2,0x1e01f268,0x6906c2fe,0xf762575d,0x806567cb,0x196c3671,0x6e6b06e7,
    0xfed41b76,0x89d32be0,0x10da7a5a,0x67dd4acc,0xf9b9df6f,0x8ebeeff9,0x17b7be43,0x60b08ed5,
    0xd6d6a3e8,0xa1d1937e,0x38d8c2c4,0x4fdff252,0xd1bb67f1,0xa6bc5767,0x3fb506dd,0x48b2364b,
    0xd80d2bda,0xaf0a1b4c,0x36034af6,0x41047a60,0xdf60efc3,0xa867df55,0x316e8eef,0x4669be79,
    0xcb61b38c,0xbc66831a,0x256fd2a0,0x5268e236,0xcc0c7795,0xbb0b4703,0x220216b9,0x5505262f,
    0xc5ba3bbe,0xb2bd0b28,0x2bb45a92,0x5cb36a04,0xc2d7ffa7,0xb5d0cf31,0x2cd99e8b,0x5bdeae1d,
    0x9b64c2b0,0xec63f226,0x756aa39c,0x026d930a,0x9c0906a9,0xeb0e363f,0x72076785,0x05005713,
    0x95bf4a82,0xe2b87a14,0x7bb12bae,0x0cb61b38,0x92d28e9b,0xe5d5be0d,0x7cdcefb7,0x0bdbdf21,
    0x86d3d2d4,0xf1d4e242,0x68ddb3f8,0x1fda836e,0x81be16cd,0xf6b9265b,0x6fb077e1,0x18b74777,
    0x88085ae6,0xff0f6a70,0x66063bca,0x11010b5c,0x8f659eff,0xf862ae69,0x616bffd3,0x166ccf45,
    0xa00ae278,0xd70dd2ee,0x4e048354,0x3903b3c2,0xa7672661,0xd06016f7,0x4969474d,0x3e6e77db,
    0xaed16a4a,0xd9d65adc,0x40df0b66,0x37d83bf0,0xa9bcae53,0xdebb9ec5,0x47b2cf7f,0x30b5ffe9,
    0xbdbdf21c,0xcabac28a,0x53b39330,0x24b4a3a6,0xbad03605,0xcdd70693,0x54de5729,0x23d967bf,
    0xb3667a2e,0xc4614ab8,0x5d681b02,0x2a6f2b94,0xb40bbe37,0xc30c8ea1,0x5a05df1b,0x2d02ef8d,
};

void hash_crc32::initialize(context* ctx)
{
    *ctx = 0;
}

void hash_crc32::process(context* ctx,
                         const char* buf,
                         size_t buf_size)
{
    uint32_t crc = *ctx;
    while (buf_size--)
    {
        crc = ZCE_CRC32_TABLE[(crc ^ *buf++) & 0xFF] ^ (crc >> 8);
    }
    *ctx = crc;
}

void hash_crc32::finalize(context* ctx,
                          const char* buf,
                          size_t buf_size,
                          char result[HASH_RESULT_SIZE])
{
    uint32_t crc = *ctx;
    while (buf_size--)
    {
        crc = ZCE_CRC32_TABLE[(crc ^ *buf++) & 0xFF] ^ (crc >> 8);
    }
    *ctx = crc;
    *((uint32_t*)result) = *(ctx);
}
}

//CRC32的函数，

//求一个buffer的CRC32值，可以用于一些要求速度的简单校验,

uint32_t zce::crc32(uint32_t crcinit, const char* buf, size_t buf_size)
{
    const char* p = buf;
    uint32_t crc = crcinit;

    //默认采用一种快速的读取方法，一次取16个字节读取的版本，
    //在我的简单数据测试中，快速版本比慢速版本快15%左右，快速Hash算法来自rhashlib
#if 1

    //我理解快速CRC32计算为什么快，但不理解两者为啥含义相同。天生不具备数据计算的脑子呀。

    //处理不对齐的内存地址部分，
    for (; (3 & (p - (char*)0)) && buf_size > 0; p++, buf_size--)
    {
        crc = ZCE_CRC32_TABLE[(crc ^ *p) & 0xFF] ^ (crc >> 8);
    }

    // 一次处理16个字节的数据，当作4个DWORD计算，
    for (const char* e = p + (buf_size & ~15); p < e; p += 16)
    {
        crc ^= ZINDEX_TO_LEUINT32(p, 0);
        crc = ZCE_CRC32_TABLE[crc & 0xFF] ^ (crc >> 8);
        crc = ZCE_CRC32_TABLE[crc & 0xFF] ^ (crc >> 8);
        crc = ZCE_CRC32_TABLE[crc & 0xFF] ^ (crc >> 8);
        crc = ZCE_CRC32_TABLE[crc & 0xFF] ^ (crc >> 8);

        crc ^= ZINDEX_TO_LEUINT32(p, 1);
        crc = ZCE_CRC32_TABLE[crc & 0xFF] ^ (crc >> 8);
        crc = ZCE_CRC32_TABLE[crc & 0xFF] ^ (crc >> 8);
        crc = ZCE_CRC32_TABLE[crc & 0xFF] ^ (crc >> 8);
        crc = ZCE_CRC32_TABLE[crc & 0xFF] ^ (crc >> 8);

        crc ^= ZINDEX_TO_LEUINT32(p, 2);
        crc = ZCE_CRC32_TABLE[crc & 0xFF] ^ (crc >> 8);
        crc = ZCE_CRC32_TABLE[crc & 0xFF] ^ (crc >> 8);
        crc = ZCE_CRC32_TABLE[crc & 0xFF] ^ (crc >> 8);
        crc = ZCE_CRC32_TABLE[crc & 0xFF] ^ (crc >> 8);

        crc ^= ZINDEX_TO_LEUINT32(p, 3);
        crc = ZCE_CRC32_TABLE[crc & 0xFF] ^ (crc >> 8);
        crc = ZCE_CRC32_TABLE[crc & 0xFF] ^ (crc >> 8);
        crc = ZCE_CRC32_TABLE[crc & 0xFF] ^ (crc >> 8);
        crc = ZCE_CRC32_TABLE[crc & 0xFF] ^ (crc >> 8);
    }

    // 处理非对齐的尾部信息。
    for (const char* e = p + (buf_size & 15); p < e; p++)
    {
        crc = ZCE_CRC32_TABLE[(crc ^ *p) & 0xFF] ^ (crc >> 8);
    }
#else
    while (buf_size--)
    {
        crc = ZCE_CRC32_TABLE[(crc ^ *p++) & 0xFF] ^ (crc >> 8);
    }
#endif
    return crc;
}

//================================================================================================
//CRC16算法的函数和策略类

static const uint16_t ZCE_CRC16_TABLE[256] =
{
    0x0000,0xC0C1,0xC181,0x0140,0xC301,0x03C0,0x0280,0xC241,
    0xC601,0x06C0,0x0780,0xC741,0x0500,0xC5C1,0xC481,0x0440,
    0xCC01,0x0CC0,0x0D80,0xCD41,0x0F00,0xCFC1,0xCE81,0x0E40,
    0x0A00,0xCAC1,0xCB81,0x0B40,0xC901,0x09C0,0x0880,0xC841,
    0xD801,0x18C0,0x1980,0xD941,0x1B00,0xDBC1,0xDA81,0x1A40,
    0x1E00,0xDEC1,0xDF81,0x1F40,0xDD01,0x1DC0,0x1C80,0xDC41,
    0x1400,0xD4C1,0xD581,0x1540,0xD701,0x17C0,0x1680,0xD641,
    0xD201,0x12C0,0x1380,0xD341,0x1100,0xD1C1,0xD081,0x1040,
    0xF001,0x30C0,0x3180,0xF141,0x3300,0xF3C1,0xF281,0x3240,
    0x3600,0xF6C1,0xF781,0x3740,0xF501,0x35C0,0x3480,0xF441,
    0x3C00,0xFCC1,0xFD81,0x3D40,0xFF01,0x3FC0,0x3E80,0xFE41,
    0xFA01,0x3AC0,0x3B80,0xFB41,0x3900,0xF9C1,0xF881,0x3840,
    0x2800,0xE8C1,0xE981,0x2940,0xEB01,0x2BC0,0x2A80,0xEA41,
    0xEE01,0x2EC0,0x2F80,0xEF41,0x2D00,0xEDC1,0xEC81,0x2C40,
    0xE401,0x24C0,0x2580,0xE541,0x2700,0xE7C1,0xE681,0x2640,
    0x2200,0xE2C1,0xE381,0x2340,0xE101,0x21C0,0x2080,0xE041,
    0xA001,0x60C0,0x6180,0xA141,0x6300,0xA3C1,0xA281,0x6240,
    0x6600,0xA6C1,0xA781,0x6740,0xA501,0x65C0,0x6480,0xA441,
    0x6C00,0xACC1,0xAD81,0x6D40,0xAF01,0x6FC0,0x6E80,0xAE41,
    0xAA01,0x6AC0,0x6B80,0xAB41,0x6900,0xA9C1,0xA881,0x6840,
    0x7800,0xB8C1,0xB981,0x7940,0xBB01,0x7BC0,0x7A80,0xBA41,
    0xBE01,0x7EC0,0x7F80,0xBF41,0x7D00,0xBDC1,0xBC81,0x7C40,
    0xB401,0x74C0,0x7580,0xB541,0x7700,0xB7C1,0xB681,0x7640,
    0x7200,0xB2C1,0xB381,0x7340,0xB101,0x71C0,0x7080,0xB041,
    0x5000,0x90C1,0x9181,0x5140,0x9301,0x53C0,0x5280,0x9241,
    0x9601,0x56C0,0x5780,0x9741,0x5500,0x95C1,0x9481,0x5440,
    0x9C01,0x5CC0,0x5D80,0x9D41,0x5F00,0x9FC1,0x9E81,0x5E40,
    0x5A00,0x9AC1,0x9B81,0x5B40,0x9901,0x59C0,0x5880,0x9841,
    0x8801,0x48C0,0x4980,0x8941,0x4B00,0x8BC1,0x8A81,0x4A40,
    0x4E00,0x8EC1,0x8F81,0x4F40,0x8D01,0x4DC0,0x4C80,0x8C41,
    0x4400,0x84C1,0x8581,0x4540,0x8701,0x47C0,0x4680,0x8641,
    0x8201,0x42C0,0x4380,0x8341,0x4100,0x81C1,0x8081,0x4040
};

//计算CRC16的值。
uint16_t zce::crc16(uint16_t crcinit,
                    const char* buf,
                    size_t buf_size)
{
    uint16_t crc = crcinit;
    const char* p = buf;
    while (buf_size--)
    {
        crc = ZCE_CRC16_TABLE[(crc ^ *p++) & 0xFF] ^ (crc >> 8);
    }

    return crc;
}

//================================================================================================

// BKDR Hash Function
size_t zce::hash_bkdr(const char* str, size_t str_len)
{
    // 31 131 1313 13131 131313 etc..
    static const size_t seed = 131313;
    size_t hash = 0;
    for (size_t i = 0; i < str_len; ++i)
    {
        hash = hash * seed + (size_t)(*(str + i));
    }
    return hash;
}

// AP Hash Function
size_t zce::hash_ap(const char* str, size_t str_len)
{
    size_t hash = 0;
    for (size_t i = 0; i < str_len; ++i)
    {
        if ((i & 1) == 0)
        {
            hash ^= ((hash << 7) ^ (size_t)(*(str + i)) ^ (hash >> 3));
        }
        else
        {
            hash ^= (~((hash << 11) ^ (size_t)(*(str + i)) ^ (hash >> 5)));
        }
    }
    return hash;
}

// JS Hash Function
size_t zce::hash_js(const char* str, size_t str_len)
{
    size_t hash = 1315423911;
    for (size_t i = 0; i < str_len; ++i)
    {
        hash ^= ((hash << 5) + (size_t)(*(str + i)) + (hash >> 2));
    }

    return hash;
}

// DJB Hash Function
size_t zce::hash_djb(const char* str, size_t str_len)
{
    size_t hash = 5381;
    for (size_t i = 0; i < str_len; ++i)
    {
        hash += (hash << 5) + (size_t)(*(str + i));
    }

    return hash;
}