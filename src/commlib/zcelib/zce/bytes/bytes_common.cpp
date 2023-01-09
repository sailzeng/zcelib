#include "zce/predefine.h"
#include "zce/bytes/bytes_common.h"

namespace zce
{
// a = b ^ c
void bytes_xor(char* a,
               const char* b,
               const char* c,
               size_t len)
{
    for (size_t i = 0; i < len; ++i)
    {
        a[i] = b[i] ^ c[i];
    }
}

// a ^= b
void bytes_xor(char* a,
               const char* b,
               size_t len)
{
    for (size_t i = 0; i < len; ++i)
    {
        a[i] ^= b[i];
    }
}
}