#pragma once

#include "zce/math/big_uint.h"
#include "zce/bytes/hash_value.h"

namespace zce
{
enum class RSA_PADDING
{
    //! NOPADDING，理论上只能对文本加密，因为最后一个BLOCK padding的是0，
    //! 如果您的明文中也有0x0，又正好在最后一个分组，会导致识别错误。
    NOPADDING,
    //! RFC 2313 https://www.rfc-editor.org/rfc/rfc2313
    PKCS1PADDING,
    //! 最优非对称加密填充（英语：Optimal Asymmetric Encryption Padding，缩写：OAEP）
    //! RFC 8017 https://www.rfc-editor.org/rfc/rfc8017
    // https://blog.csdn.net/guyongqiangx/article/details/121055655
    OAEPPADDING
};

class rsa_base
{
protected:
    //!
    const static size_t OAEP_HLEN = 20;
    //!
    static unsigned char EMPTY_L_SHA1[OAEP_HLEN];
};

//L 长度单位，bit，512,1024,2048,4096推荐 1024,2048
template <std::size_t L>
class rsa : public rsa_base
{
protected:

    typedef zce::big_uint<L> bignumber;

    enum class KEY_TYPE
    {
        KEY_PRIVATE,
        KEY_PUBLIC,
    };

public:
    rsa() = default;
    ~rsa() = default;

public:

    template <class RANDOM_ENGINE>
    void init(RANDOM_ENGINE & engine)
    {
        //产生大素数p、q
        p_.create_prime(engine,
                        PRIME_SIZE,
                        10);
        engine.discard();
        engine.discard();
        q_.create_prime(engine,
                        PRIME_SIZE,
                        10);
        //计算N
        n_ = p_ * q_;
        //计算出欧拉数
        ol_ = (p_ - 1) * (q_ - 1);

        e_ = 65537;
        //扩展欧几里得算法,计算秘钥d_
        bignumber x, y;
        bignumber::ext_euc(e_, ol_, x, y);
        d_ = x;
    }

    //!根据明文长度，得到密文BUFFER的长度
    size_t cipher_buflen(RSA_PADDING padding, size_t plain_len)
    {
        if (padding == RSA_PADDING::NOPADDING)
        {
            return (plain_len / (BLOCK_SIZE)+1) * BLOCK_SIZE;
        }
        else if (padding == RSA_PADDING::PKCS1PADDING)
        {
            return (plain_len / (BLOCK_SIZE - 11) + 1) * BLOCK_SIZE;
        }
        else if (padding == RSA_PADDING::OAEPPADDING)
        {
            return (plain_len / (BLOCK_SIZE - 11) + 1) * BLOCK_SIZE;
        }
        else
        {
        }
    }
    //!根据密文长度，得到明文BUFFER的长度
    size_t plain_buflen(RSA_PADDING padding, size_t cipher_len)
    {
        if (padding == RSA_PADDING::NOPADDING)
        {
            return cipher_len;
        }
        else if (padding == RSA_PADDING::PKCS1PADDING)
        {
            return (cipher_len / BLOCK_SIZE) * (BLOCK_SIZE - 11);
        }
        else if (padding == RSA_PADDING::OAEPPADDING)
        {
            return (cipher_len / BLOCK_SIZE) * (BLOCK_SIZE - 11);
        }
        else
        {
        }
    }

    int private_encrypt(RSA_PADDING padding,
                        const unsigned char* plain_buf,
                        size_t plain_len,
                        unsigned char* cipher_buf,
                        size_t* cipher_len)
    {
        return encrypt_i(padding,
                         KEY_TYPE::KEY_PRIVATE,
                         plain_buf,
                         plain_len,
                         cipher_buf,
                         cipher_len);
    }

    int public_encrypt(RSA_PADDING padding,
                       const unsigned char* plain_buf,
                       size_t plain_len,
                       unsigned char* cipher_buf,
                       size_t* cipher_len)
    {
        return encrypt_i(padding,
                         KEY_TYPE::KEY_PUBLIC,
                         plain_buf,
                         plain_len,
                         cipher_buf,
                         cipher_len);
    }

    int private_decrypt(RSA_PADDING padding,
                        const unsigned char* cipher_buf,
                        size_t cipher_len,
                        unsigned char* plain_buf,
                        size_t* plain_len)
    {
        return decrypt_i(padding,
                         KEY_TYPE::KEY_PRIVATE,
                         plain_buf,
                         plain_len,
                         cipher_buf,
                         cipher_len);
    }
    int public_decrypt(RSA_PADDING padding,
                       const unsigned char* cipher_buf,
                       size_t cipher_len,
                       unsigned char* plain_buf,
                       size_t* plain_len)
    {
        return decrypt_i(padding,
                         KEY_TYPE::KEY_PUBLIC,
                         plain_buf,
                         plain_len,
                         cipher_buf,
                         cipher_len);
    }

protected:
    //! 加密内部实现
    int encrypt_i(RSA_PADDING padding,
                  KEY_TYPE kt,
                  const unsigned char* plain_buf,
                  size_t plain_len,
                  unsigned char* cipher_buf,
                  size_t* cipher_len)
    {
        ssize_t remain_len = plain_len;
        const unsigned char* read_ptr = plain_buf;
        unsigned char* write_ptr = cipher_buf;
        cipher_len = 0;
        size_t read_block_size = 0;
        while (remain_len > 0)
        {
            if (padding == RSA_PADDING::NOPADDING)
            {
                encrypt_ecb_nopadding(kt, read_ptr, remain_len, write_ptr);
                read_block_size = BLOCK_SIZE;
            }
            else if (padding == RSA_PADDING::PKCS1PADDING)
            {
                encrypt_ecb_pkcs1padding(kt, read_ptr, remain_len, write_ptr);
                read_block_size = PKCS1_VALID_SIZE;
            }
            else if (padding == RSA_PADDING::OAEPPADDING)
            {
                encrypt_ecb_oaeppadding(kt, read_ptr, remain_len, write_ptr);
                read_block_size = OAEP_VALID_SIZE;
            }
            else
            {
            }
            remain_len -= read_block_size;
            read_ptr += read_block_size;
            write_ptr += BLOCK_SIZE;
        }
        cipher_len = write_ptr - cipher_buf;
        return 0;
    }
    //!
    void encrypt_ecb_nopadding(KEY_TYPE kt,
                               const unsigned char* read_ptr,
                               ssize_t remain_len,
                               unsigned char* write_ptr)
    {
        const bignumber *key = nullptr;
        bignumber bn_in, bn_out;
        if (kt == KEY_TYPE::KEY_PRIVATE)
        {
            key = &d_;
        }
        else
        {
            key = &e_;
        }
        if (remain_len >= KEY_TYPE::BLOCK_SIZE)
        {
            bn_in.putin(write_ptr);
        }
        else
        {
            char block_in[BLOCK_SIZE] = { 0 };
            ::memset(block_in, 0, BLOCK_SIZE - remain_len);
            ::memcpy(block_in + BLOCK_SIZE - remain_len, read_ptr, remain_len);
            bn_in.putin(block_in);
        }
        bn_out = bignumber::mod_exp(bn_in, *key, n_);
        bn_out.takeout(write_ptr);
    }
    //RFC 2313 https://www.rfc-editor.org/rfc/rfc2313
    void encrypt_ecb_pkcs1padding(KEY_TYPE kt,
                                  const unsigned char* read_ptr,
                                  ssize_t remain_len,
                                  unsigned char* write_ptr)
    {
        char block_in[BLOCK_SIZE] = { 0 };
        size_t padding_len = 0;
        bignumber bn_in, bn_out;
        const bignumber *key = nullptr;
        block_in[0] = 0x0;
        if (kt == KEY_TYPE::KEY_PRIVATE)
        {
            key = &d_;
            block_in[1] = 0x1;
        }
        else
        {
            key = &e_;
            block_in[1] = 0x2;
        }
        if (remain_len >= PKCS1_VALID_SIZE)
        {
            padding_len = 8;
        }
        else
        {
            padding_len = BLOCK_SIZE - 3 - remain_len;
        }
        if (kt == KEY_TYPE::KEY_PRIVATE)
        {
            ::memset(block_in + 2, 0xFF, padding_len);
        }
        else
        {
            for (size_t i = 0; i < padding_len; ++i)
            {
                unsigned char rd = 0;
                do
                {
                    rd = rand() & 0xFF;
                    block_in[2 + i] = rd;
                } while (rd == 0);
            }
        }
        block_in[2 + padding_len] = 0x0;

        size_t data_len = BLOCK_SIZE - (padding_len + 3);
        ::memcpy(block_in + padding_len + 3, read_ptr, data_len);
        bn_in.putin(block_in);
        bn_out = bignumber::mod_exp(bn_in, *key, n_);
        bn_out.takeout(write_ptr);
    }

    //RFC 8017 https://www.rfc-editor.org/rfc/rfc8017
    //内部Hash 函数采用了SHA1，所以lLen=20，同时L选择了为空
    void encrypt_ecb_oaeppadding(KEY_TYPE kt,
                                 const unsigned char* read_ptr,
                                 ssize_t remain_len,
                                 unsigned char* write_ptr)
    {
        char block_in[BLOCK_SIZE] = { 0 };
        char mgf_buf[BLOCK_SIZE] = { 0 };
        size_t padding_len = 0;
        bignumber bn_in, bn_out;
        const bignumber *key = nullptr;
        block_in[0] = 0x0;
        if (kt == KEY_TYPE::KEY_PRIVATE)
        {
            key = &d_;
        }
        else
        {
            key = &e_;
        }
        unsigned char* pseed = block_in + 1;
        for (size_t i = 0; i < OAEP_HLEN; ++i)
        {
            unsigned char rd = 0;
            do
            {
                rd = rand() & 0xFF;
                pseed[i] = rd;
            } while (rd == 0);
        }
        unsigned char* pdb = block_in + 1 + OAEP_HLEN;
        unsigned char* pdb_wpos = pdb;
        //L为空，选择了一个固定的Hash值
        memcpy(pdb_wpos, EMPTY_L_SHA1, OAEP_HLEN);
        pdb_wpos += OAEP_HLEN;
        if (remain_len >= OAEP_VALID_SIZE)
        {
            padding_len = 0;
        }
        else
        {
            padding_len = OAEP_VALID_SIZE - remain_len;
            memset(pdb_wpos, 0, padding_len);
            pdb_wpos += padding_len;
        }
        *(pdb + padding_len) = 0x1;
        pdb_wpos += 1;
        memcpy(pdb_wpos, read_ptr, OAEP_VALID_SIZE - padding_len);

        mgf1(pseed, OAEP_HLEN, mgf_buf + 1 + OAEP_HLEN, BLOCK_SIZE - OAEP_HLEN - 1);
        zce::bytes_xor(block_in + 1 + OAEP_HLEN, mgf_buf + 1 + OAEP_HLEN, BLOCK_SIZE - 1 - OAEP_HLEN);

        mgf1(pdb, BLOCK_SIZE - OAEP_HLEN - 1, mgf_buf + 1, OAEP_HLEN);
        zce::bytes_xor(block_in + 1, mgf_buf + 1, OAEP_HLEN);

        bn_in.putin(block_in);
        bn_out = bignumber::mod_exp(bn_in, *key, n_);
        bn_out.takeout(write_ptr);
    }

    //! 解密
    int decrypt_i(RSA_PADDING padding,
                  KEY_TYPE kt,
                  const unsigned char* cipher_buf,
                  size_t cipher_len,
                  unsigned char* plain_buf,
                  size_t* plain_len)
    {
        int ret = 0;
        ssize_t remain_len = cipher_len;
        char block_in[BLOCK_SIZE] = { 0 };
        const unsigned char* read_ptr = (cipher_buf);
        unsigned char* write_ptr = plain_buf;
        size_t write_len = 0;
        while (remain_len > 0)
        {
            if (padding == RSA_PADDING::NOPADDING)
            {
                ret = decrypt_ecb_nopadding(kt, read_ptr, remain_len, write_ptr, write_len);
            }
            else if (padding == RSA_PADDING::PKCS1PADDING)
            {
                ret = decrypt_ecb_pkcs1padding(kt, read_ptr, remain_len, write_ptr, write_len);
            }
            else if (padding == RSA_PADDING::OAEPPADDING)
            {
                ret = decrypt_ecb_oaeppadding(kt, read_ptr, remain_len, write_ptr, write_len);
            }
            else
            {
                assert(false);
                return -1;
            }
            if (ret != 0)
            {
                return -1;
            }
            remain_len -= BLOCK_SIZE;
            read_ptr += BLOCK_SIZE;
            write_ptr += write_len;
        }
        return 0;
    }
    //! nopadding编码，对一个数据快解密
    int decrypt_ecb_nopadding(KEY_TYPE kt,
                              const unsigned char* read_ptr,
                              ssize_t remain_len,
                              unsigned char* write_ptr,
                              ssize_t &write_len)
    {
        const bignumber *key = nullptr;
        bignumber bn_in, bn_out;
        if (kt == KEY_TYPE::KEY_PRIVATE)
        {
            key = &d_;
        }
        else
        {
            key = &e_;
        }
        bn_in.putin(read_ptr);
        bn_out = bignumber::mod_exp(bn_in, *key, n_);

        if (remain_len > BLOCK_SIZE)
        {
            bn_out.takeout(write_ptr);
            write_len = BLOCK_SIZE;
        }
        else
        {
            char block_out[BLOCK_SIZE] = { 0 };
            bn_in.putin(block_out);
            size_t i = 0;
            for (; i < BLOCK_SIZE; ++i)
            {
                if (block_out[i] != 0x0)
                {
                    break;
                }
            }
            write_len = BLOCK_SIZE - i;
            ::memcpy(write_ptr, block_out + i, write_len);
        }
        return 0;
    }
    //! pkcs1padding编码，对一个数据快解密
    int decrypt_ecb_pkcs1padding(KEY_TYPE kt,
                                 const unsigned char* read_ptr,
                                 ssize_t remain_len,
                                 unsigned char* write_ptr,
                                 ssize_t &write_len)
    {
        const bignumber *key = nullptr;
        bignumber bn_in, bn_out;
        char block_out[BLOCK_SIZE] = { 0 };
        if (kt == KEY_TYPE::KEY_PRIVATE)
        {
            key = &d_;
        }
        else
        {
            key = &e_;
        }
        bn_in.putin(read_ptr);
        bn_out = bignumber::mod_exp(bn_in, *key, n_);
        bn_out.takeout(block_out);
        if (block_out[0] != 0x0)
        {
            return -1;
        }

        if (remain_len > BLOCK_SIZE)
        {
            ::memcpy(write_ptr,
                     block_out + PKCS1_HEAD_SIZE,
                     PKCS1_VALID_SIZE);
            write_len = PKCS1_VALID_SIZE;
        }
        else
        {
            size_t i = 2;
            for (; i < BLOCK_SIZE; ++i)
            {
                if (block_out[i] == 0x0)
                {
                    break;
                }
            }
            if (i == BLOCK_SIZE || i < 11)
            {
                return -1;
            }
            write_len = BLOCK_SIZE - i - 1;
            ::memcpy(write_ptr, block_out + i + 1, write_len);
        }
        return 0;
    }
    //! oaeppadding编码，对一个数据快解密
    int decrypt_ecb_oaeppadding(KEY_TYPE kt,
                                const unsigned char* read_ptr,
                                ssize_t remain_len,
                                unsigned char* write_ptr,
                                ssize_t &write_len)
    {
        const bignumber *key = nullptr;
        bignumber bn_in, bn_out;
        char block_out[BLOCK_SIZE] = { 0 };
        char mgf_buf[BLOCK_SIZE] = { 0 };
        if (kt == KEY_TYPE::KEY_PRIVATE)
        {
            key = &d_;
        }
        else
        {
            key = &e_;
        }
        bn_in.putin(read_ptr);
        bn_out = bignumber::mod_exp(bn_in, *key, n_);
        bn_out.takeout(block_out);
        if (block_out[0] != 0x0)
        {
            return -1;
        }

        mgf1(block_out + 1 + OAEP_HLEN, BLOCK_SIZE - OAEP_HLEN - 1, mgf_buf + 1, OAEP_HLEN);
        zce::bytes_xor(block_out + 1, mgf_buf + 1, OAEP_HLEN);
        mgf1(block_out + 1, OAEP_HLEN, mgf_buf + 1 + OAEP_HLEN, BLOCK_SIZE - OAEP_HLEN - 1);
        zce::bytes_xor(block_out + 1 + OAEP_HLEN, mgf_buf + 1 + OAEP_HLEN, BLOCK_SIZE - 1 - OAEP_HLEN);
        //
        if (0 != ::memcmp(block_out + 1 + OAEP_HLEN, EMPTY_L_SHA1, OAEP_HLEN))
        {
            return -1;
        }

        if (remain_len > BLOCK_SIZE)
        {
            ::memcpy(write_ptr,
                     block_out + OAEP_HEAD_SIZE,
                     OAEP_VALID_SIZE);
            write_len = OAEP_VALID_SIZE;
        }
        else
        {
            size_t i = 1 + OAEP_HLEN + OAEP_HLEN;
            for (; i < BLOCK_SIZE; ++i)
            {
                if (block_out[i] == 0x0)
                {
                    continue;
                }
                else
                {
                    if (block_out[i] != 0x1)
                    {
                        return -1;
                    }
                    break;
                }
            }
            if (i == BLOCK_SIZE)
            {
                return -1;
            }
            write_len = BLOCK_SIZE - i - 1;
            ::memcpy(write_ptr, block_out + i + 1, write_len);
        }
        return 0;
    }

    //MGF1 is a mask generation function based on a hash function.
    int mgf1(const unsigned char *mgf_seed,
             size_t mgf_seed_len,
             unsigned char *mask,
             size_t mask_len)
    {
        unsigned char buf[BLOCK_SIZE], *p;

        size_t counter, rest_len;

        if (mgf_seed_len > BLOCK_SIZE - 4)
        {
            printf("MGF1 buffer is not long enough!\n");
            return -1;
        }

        // copy mgf_seed to buffer
        memcpy(buf, mgf_seed, mgf_seed_len);

        // clear rest buffer to 0
        p = buf + mgf_seed_len;
        memset(p, 0, BLOCK_SIZE - mgf_seed_len);

        counter = 0;
        rest_len = mask_len;

        while (rest_len > 0)
        {
            p[0] = (counter >> 24) & 0xff;
            p[1] = (counter >> 16) & 0xff;
            p[2] = (counter >> 8) & 0xff;
            p[3] = counter & 0xff;

            if (rest_len >= OAEP_HLEN)
            {
                zce::sha1(buf, mgf_seed_len + 4, (unsigned char *)mask);
                rest_len -= OAEP_HLEN;
                mask += OAEP_HLEN;

                counter++;
            }
            // 剩余的不足单次哈希长度的部分
            else
            {
                unsigned char digest[64]; /* 最长支持 SHA-512 */
                zce::sha1(buf, mgf_seed_len + 4, (unsigned char *)digest);
                memcpy(mask, digest, rest_len);
                rest_len = 0;
            }
        }

        return 0;
    }

public:

    //! 算法一次处理一个BLOCK的长度
    const static size_t BLOCK_SIZE = L / 8;

    //! PKCS1的头部是11个字节
    const static size_t PKCS1_HEAD_SIZE = 11;
    //! PKCS1的有效长度
    const static size_t PKCS1_VALID_SIZE = BLOCK_SIZE - PKCS1_HEAD_SIZE;

    //! OAEP头部的长度，sHA1用20个字节，20*2+2，不是有点浪费呀。
    const static size_t OAEP_HEAD_SIZE = 42;
    //! OAEP最大负载长度
    const static size_t OAEP_VALID_SIZE = BLOCK_SIZE - OAEP_HEAD_SIZE;

    //! 算法质数的长度
    const static size_t KEY_SIZE = L / 8;
    //! 算法KEY的长度
    const static size_t KEY_BITS = L / 8;

    //! 算法生成2个质数的长度
    const static size_t PRIME_SIZE = L / 8 / 2;

protected:
    //公钥
    bignumber e_, n_;
    //私钥
    bignumber d_;
    //两个素数
    bignumber p_, q_;
    //欧拉数
    bignumber ol_;
};

typedef rsa<512>  rsa512;
typedef rsa<1024> rsa1024;
typedef rsa<2048> rsa2048;
}