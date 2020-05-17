#pragma once
#include "../core.hpp"
namespace YAL::Serializer
{
    static inline unsigned long long ZigZag(long long x)
    {
        return (x << 1) ^ (x >> 63);
    }
    static inline unsigned int ZigZag(int x)
    {
        return (x << 1) ^ (x >> 63);
    }
    template <typename T>
    struct VarInt
    {
        T v;
        operator T()
        {
            return v;
        }
        VarInts() {}
        explict VarInts(T x) { v = x; }
        template <typename _TP>
        void pack(WBStreamImpl<_TP> &ws) const
        {
            unsigned char buf[16];
            int ptr = 0;
            T enc = v;
            do
            {
                buf[ptr++] = enc & 0x7f;
                enc >>= 7;
            } while (enc);
            for (int i = 0; i < ptr - 1; ++i)
            {
                buf[i] |= 0x80;
            }
            ws.write(buf, ptr);
        }
        void unpack(RBStream &rs)
        {
            unsigned char buf[16];
            v = 0;
            int ptr = 0;
            for (; ptr < 16; ++ptr)
            {
                rs.apply(buf[ptr]);
                if (!(buf[ptr] & 0x80))
                {
                    ++ptr;
                    break;
                }
                buf[ptr] &= 0x7f;
            }
            for (ptr = ptr - 1; ptr >= 0; --ptr)
            {
                v <<= 7;
                v |= buf[ptr];
            }
        }
    };
    using VarUInt = VarInt<unsigned int>;
    using VarULong = VarInt<unsigned long long>;
    struct MCString
    {
        string_view view;
        MCString() {}
        MCString(string_view sv) : view(sv) {}
        template <typename T>
        void pack(T &ws) const
        {
            ws.apply(VarUInt((unsigned int)view.size()));
            ws.write(view.data(), view.size());
        }
        void unpack(RBStream &rs)
        {
            VarUInt sz;
            rs.apply(sz);
            view = string_view((const char *)rs.data, sz.v);
            rs.data += sz.v;
        }
    };
    struct BinVariant
    {
        /*long long or string*/
        union VType {
            long long x;
            std::string y;
            VType() {}
            ~VType() {}
        } v;
        unsigned char type;
        BinVariant(long long x)
        {
            type = 1;
            v.x = x;
        }
        BinVariant(std::string &&x)
        {
            type = 2;
            new (&v.y) std::string(std::move(x));
        }
        BinVariant(std::string const &x)
        {
            type = 2;
            new (&v.y) std::string(x);
        }
        BinVariant()
        {
            type = 0;
        }
        ~BinVariant()
        {
            if (type == 2)
            {
                v.y.~string();
            }
        }
        void unpack(RBStream &rs)
        {
            rs.apply(type);
            switch (type)
            {
            case 1:
            {
                rs.apply(v.x);
            }
            break;
            case 2:
            {
                new (&v.y) std::string();
                rs.apply(v.y);
            }
            }
        }
        void pack(WBStream &ws) const
        {
            ws.apply(type);
            switch (type)
            {
            case 1:
            {
                ws.apply(v.x);
            }
            break;
            case 2:
            {
                ws.apply(v.y);
            }
            }
        }
    };
}; // namespace YAL::Serializer