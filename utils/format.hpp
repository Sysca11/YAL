#pragma once
#include"../core.hpp"
#include"../container/smallstr.hpp"
namespace YAL
{
    template<typename WRITEABLE>
    inline void __FORMAT(WRITEABLE &s, string_view format)
    {
        s.append(format);
    }

    template <typename WRITEABLE,typename T, typename... Targs>
    inline void __FORMAT(WRITEABLE &out, string_view format, T &&value, Targs &&... Fargs)
    {
        for (int i = 0; i < format.size(); i++)
        {
            if (format[i] == '%')
            {
                if (format[i + 1] == '%')
                {
                    i++;
                    out.push_back('%');
                    continue;
                }
                out.append(S(std::forward<T>(value)));
                __FORMAT(out, format.substr(i + 1), std::forward<Targs>(Fargs)...);
                return;
            }
            else
            {
                out.push_back(format[i]);
            }
        }
    }
    template<typename INNER=smallStr<160>>
    struct FORMAT
    {
        INNER inner;
        template <typename... T>
        FORMAT(string_view f, T &&... a)
        {
            __FORMAT(inner, f, std::forward<T>(a)...);
        }
        operator INNER &&()
        {
            return std::move(inner);
        }
    };
} // namespace YAL
