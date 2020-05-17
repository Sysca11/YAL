#pragma once
#include "../type_alias.h"
namespace YAL::Hash
{
    constexpr u64 ap_hash(const char *x)
    {
        //ap hash
        u64 rval = 0;
        for (size_t i = 0; x[i]; ++i)
        {
            if (i & 1)
            {
                rval ^= (~((rval << 11) ^ x[i] ^ (rval >> 5)));
            }
            else
            {
                rval ^= (~((rval << 7) ^ x[i] ^ (rval >> 3)));
            }
        }
        return rval;
    }
    constexpr u64 ap_hash(const char *x, int len)
    {
        //ap hash
        u64 rval = 0;
        for (size_t i = 0; i < len; ++i)
        {
            if (i & 1)
            {
                rval ^= (~((rval << 11) ^ x[i] ^ (rval >> 5)));
            }
            else
            {
                rval ^= (~((rval << 7) ^ x[i] ^ (rval >> 3)));
            }
        }
        return rval;
    }
    template <int BKDR_MUL = 131, int BKDR_ADD = 0>
    constexpr u64 BKDRHash(const char *x, int len)
    {
        u64 rval = 0;
        for (size_t i = 0; i < len; ++i)
        {
            rval *= BKDR_MUL;
            rval += x[i];
            rval += BKDR_ADD;
        }
        return rval;
    }
}; // namespace YAL