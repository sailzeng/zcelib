#pragma once

#include "zce/math/big_uint.h"

namespace zce
{
enum class RSA_PADDING
{
    //! NOPADDING，理论上只能对文本加密，因为最后一个BLOCK padding的是0，
    //! 如果您的明文中也有0x0，又正好在最后一个分组，会导致识别错误。
    NOPADDING,
    PKCS1PADDING,
    OAEPPADDING
};

//L 长度单位，bit，512,1024,2048,4096推荐 1024,2048
template <std::size_t L>
class rsa
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
    }
    int public_decrypt(RSA_PADDING padding,
                       const unsigned char* cipher_buf,
                       size_t cipher_len,
                       unsigned char* plain_buf,
                       size_t* plain_len)
    {
    }

protected:
    //!
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
                read_block_size = PKCS1_BLOCK_SIZE;
            }
            else if (padding == RSA_PADDING::OAEPPADDING)
            {
                encrypt_ecb_oaeppadding(kt, read_ptr, remain_len, write_ptr);
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
        if (remain_len >= PKCS1_BLOCK_SIZE)
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
    //RFC 8017 https://www.rfc-editor.org/rfc/rfc8017.txt
    //https://blog.csdn.net/guyongqiangx/article/details/121055655
    void encrypt_ecb_oaeppadding(KEY_TYPE kt,
                                 const unsigned char* read_ptr,
                                 ssize_t remain_len,
                                 unsigned char* write_ptr)
    {
    }

    //!
    int decrypt_i(RSA_PADDING padding,
                  KEY_TYPE kt,
                  const unsigned char* cipher_buf,
                  size_t cipher_len,
                  unsigned char* plain_buf,
                  size_t* plain_len)
    {
        ssize_t remain_len = cipher_len;
        char block_in[BLOCK_SIZE] = { 0 };
        const unsigned char* read_ptr = (cipher_buf);
        unsigned char* write_ptr = plain_buf;
        size_t write_len = 0;
        while (remain_len > 0)
        {
            if (padding == RSA_PADDING::NOPADDING)
            {
                decrypt_ecb_nopadding(kt, read_ptr, remain_len, write_ptr, write_len);
            }
            else if (padding == RSA_PADDING::PKCS1PADDING)
            {
                decrypt_ecb_pkcs1padding(kt, read_ptr, remain_len, write_ptr, write_len);
            }
            else if (padding == RSA_PADDING::OAEPPADDING)
            {
                decrypt_ecb_oaeppadding(kt, read_ptr, remain_len, write_ptr, write_len);
            }
            else
            {
                assert(false);
                return -1;
            }
            remain_len -= BLOCK_SIZE;
            read_ptr += BLOCK_SIZE;
            write_ptr += write_len;
        }
        return 0;
    }
    //!
    void decrypt_ecb_nopadding(KEY_TYPE kt,
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

        if (remain_len >= BLOCK_SIZE)
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
    }
    //!
    void decrypt_ecb_pkcs1padding(KEY_TYPE kt,
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
        if (remain_len >= BLOCK_SIZE)
        {
            ::memcpy(write_ptr,
                     block_out + PKCS1_HEAD_SIZE,
                     PKCS1_BLOCK_SIZE);
            write_len = PKCS1_BLOCK_SIZE;
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
    }
    void decrypt_ecb_oaeppadding(KEY_TYPE kt,
                                 const unsigned char* read_ptr,
                                 ssize_t remain_len,
                                 unsigned char* write_ptr,
                                 ssize_t &write_len)
    {
    }
public:

    //算法一次处理一个BLOCK的长度
    const static size_t BLOCK_SIZE = L / 8;
    //
    const static size_t PKCS1_HEAD_SIZE = 11;
    //
    const static size_t PKCS1_BLOCK_SIZE = BLOCK_SIZE - PKCS1_HEAD_SIZE;
    //算法质数的长度
    const static size_t KEY_SIZE = L / 8;
    //算法KEY的长度
    const static size_t KEY_BITS = L / 8;

    //算法生成2个质数的长度
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

typedef rsa<512> rsa512;
typedef rsa<1024> rsa1024;
typedef rsa<2048> rsa2048;
}