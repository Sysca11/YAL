#pragma once
#include <cstdio>
#include <cstdlib>

#include<string_view>
using std::string_view;
using namespace std::string_view_literals;
#define _YAL_ABORT(err) YAL::YAL_ABORT(err, __func__, __FILE__, __LINE__)
namespace YAL
{
    template <typename T>
    static inline constexpr T min(T a, T b)
    {
        if (a > b)
            return b;
        return a;
    }
    template <typename T>
    static inline constexpr T max(T a, T b)
    {
        if (a < b)
            return b;
        return a;
    }
    bool YAL_ABORT(const char *cond, const char *func, const char *file, int line)
    {
        fprintf(stderr, "abort: %s at `%s` in %s line %d\n", cond, func, file, line);
        exit(1);
        return true;
    }
}; // namespace YAL