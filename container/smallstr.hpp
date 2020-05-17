#pragma once
#include "zipvector.hpp"
namespace YAL
{
    struct smallStr_base{};
    template <int BUFSZ = 160>
    class smallStr : ZipVector<char, BUFSZ>, public smallStr_base
    {
    public:
        using ZipVector<char, BUFSZ>::operator[];
        using ZipVector<char, BUFSZ>::data, ZipVector<char, BUFSZ>::front, ZipVector<char, BUFSZ>::shrink_to_fit, ZipVector<char, BUFSZ>::begin;
        using ZipVector<char, BUFSZ>::emplace_front;
        using ZipVector<char, BUFSZ>::remove_range;
        const char *c_str() const
        {
            return data();
        }
        smallStr &operator=(smallStr const &r) noexcept
        {
            ZipVector<char, BUFSZ>::operator=(r);
            return *this;
        }
        int size() const
        {
            return ZipVector<char, BUFSZ>::size() - 1;
        }
        operator string_view() const
        {
            return {ZipVector<char, BUFSZ>::data(), (size_t)smallStr::size()};
        }
        string_view view() const
        {
            return (string_view) * this;
        }
        bool operator==(const char *r) const
        {
            int len = int(strlen(r));
            if (len == size())
            {
                if (memcmp(r, data(), len) == 0)
                    return true;
            }
            return false;
        }
        bool operator==(string_view r) const
        {
            if (r.size() == size())
            {
                if (memcmp(r.data(), data(), r.size()) == 0)
                    return true;
            }
            return false;
        }
        void clear()
        {
            ZipVector<char, BUFSZ>::clear();
            ZipVector<char, BUFSZ>::emplace_back(0);
        }
        void push_back(char r) noexcept
        {
            ZipVector<char, BUFSZ>::back() = r;
            ZipVector<char, BUFSZ>::emplace_back(0);
        }
        void pop_back() noexcept
        {
            ZipVector<char, BUFSZ>::pop_back();
            ZipVector<char, BUFSZ>::back() = 0;
        }
        void remove_front() noexcept
        {
            ZipVector<char, BUFSZ>::remove_front();
        }
        void reserve(int sz)
        {
            ZipVector<char, BUFSZ>::reserve(sz + 1);
        }
        void resize(int newlen) {
            // !!! we wont fill char(0)
            if (newlen > smallStr::size()) {
                ZipVector<char, BUFSZ>::_expand_no_construct(newlen + 1);
                ZipVector<char, BUFSZ>::back() = 0;
            }
            else {
                //shrink
                ZipVector<char, BUFSZ>::_set_size(newlen+1);
                ZipVector<char, BUFSZ>::back() = 0;
                ZipVector<char, BUFSZ>::try_shrink();
            }
        }
        auto end()
        {
            return --ZipVector<char, BUFSZ>::end();
        }
        auto end() const
        {
            return --ZipVector<char, BUFSZ>::end();
        }
        char& back() {
            return *(data() + smallStr::size() - 1);
        }
        char back() const {
            return *(data() + smallStr::size() - 1);
        }
        smallStr& append(const char* v, int l) noexcept {
            auto oldsize = ZipVector<char, BUFSZ>::size();
            ZipVector<char, BUFSZ>::_expand_no_construct(oldsize + l);
            auto ptr = ZipVector<char, BUFSZ>::data();
            memcpy(ptr + oldsize - 1, v, l);
            ZipVector<char, BUFSZ>::back() = 0;
            return *this;
        }
        smallStr& append(string_view r) noexcept
        {
            auto oldsize = ZipVector<char, BUFSZ>::size();
            ZipVector<char, BUFSZ>::_expand_no_construct(oldsize + r.size());
            auto ptr = ZipVector<char, BUFSZ>::data();
            memcpy(ptr + oldsize - 1, r.data(), r.size());
            ZipVector<char, BUFSZ>::back() = 0;
            return *this;
        }
        smallStr &append(char r) noexcept
        {
            push_back(r);
            return *this;
        }
        template <int rvsz>
        smallStr &append(smallStr<rvsz> const &r) noexcept
        {
            return append(r.operator std::string_view());
        }

        template <typename T>
        auto operator+=(T &&r)
        {
            return append(std::forward<T>(r));
        }
        smallStr &operator=(string_view y) noexcept
        {
            ZipVector<char, BUFSZ>::_expand_no_construct(y.size() + 1);
            memcpy(data(), y.data(), y.size());
            ZipVector<char, BUFSZ>::back() = 0;
            ZipVector<char, BUFSZ>::try_shrink();
            return *this;
        }
        smallStr() noexcept
        {
            ZipVector<char, BUFSZ>::emplace_back(0);
        }
        smallStr(string_view y) noexcept
        {
            ZipVector<char, BUFSZ>::_expand_no_construct(int(y.size() + 1));
            memcpy(data(), y.data(), y.size());
            ZipVector<char, BUFSZ>::back() = 0;
        }
        smallStr(smallStr const &r) noexcept : ZipVector<char, BUFSZ>(r) {}
        smallStr(smallStr &&r) noexcept : ZipVector<char, BUFSZ>(std::forward<ZipVector<char, BUFSZ>>(r)) {}
    };
    template<int CAP,typename T>
    std::enable_if_t<std::is_floating_point_v<T>> concat_tostr(smallStr<CAP>& s, T v) {
        int len=_scprintf("%f", double(v));
        int sz = s.size();
        s.resize(sz + len);
        snprintf(s.data() + sz,len+1, "%f", double(v));
    }
    template<int CAP, typename T>
    std::enable_if_t<std::is_integral_v<T>> concat_tostr(smallStr<CAP>& s, T v) {
        char lbuf[21];
        bool fg = false;
        char* ptr = lbuf+21;
        if constexpr (std::is_signed_v<T>) {
            if (v < 0) {
                fg = true;
                v = -v;
            }
        }
        while (v) {
            *(--ptr) = '0' + v % 10;
            v /= 10;
        }
        char* oldback = s.data()+s.size();
        s.resize(int(s.size() + int(fg) + (lbuf + 21 - ptr)));
        if (fg) *(oldback++) = '-';
        memcpy(oldback, ptr, (lbuf + 21 - ptr));
    }
}; // namespace YAL