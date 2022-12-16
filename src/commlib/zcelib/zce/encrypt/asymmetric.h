#pragma once

#include "zce/math/big_uint.h"

namespace zce
{
//L 长度单位，bit，512,1024,2048,推荐 1024,2048
template <std::size_t L>
class rsa
{
public:
    rsa() = default;
    ~rsa() = default;

    typedef zce::big_uint<L> bignumber;

public:
    //私钥加密
    void private_encrypt(const bignumber& in, bignumber& out);
    //公钥解密
    void public_decrypt(const bignumber& in, bignumber& out);

    //公钥加密
    void public_encrypt(const bignumber& in, bignumber& out);
    //私钥解密
    void private_decrypt(const bignumber& in, bignumber& out);

private:

    //friend ostream& operator <<(ostream& out, const Rsa& rsa)//输出
    //{
    //    out << "N:" << rsa.N << "\n";
    //    out << "p:" << rsa._p << "\n";
    //    out << "q:" << rsa._q << "\n";
    //    out << "e:" << rsa.e << "\n";
    //    out << "d:" << rsa._d;
    //    return out;
    //}

public:

    //算法一次处理一个BLOCK的长度
    const static size_t BLOCK_SIZE = L / 8;
    //算法KEY的长度
    const static size_t KEY_SIZE = L / 8;

public:
    bignumber e, N;//公钥
private:
    bignumber _d;//私钥
    bignumber _p, _q;//
    bignumber _ol;//欧拉数
};
}