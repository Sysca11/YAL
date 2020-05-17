#pragma once
#include"type_traits.h"
#include<functional>
namespace YAL
{
    template <typename _FNTP, typename EXTRA_DATA, bool nothrow = true>
    struct lightFunc : bitwise_movable_t
    {
        union
        {
            _FNTP *cptr;
            std::function<_FNTP> *uniqptr;
        } inner;
        int innertype;
        EXTRA_DATA extra;
        lightFunc() { innertype = 0; }
        bool IsbadFunc() const
        {
            return innertype == 0;
        }
        bool IsFunptr() const
        {
            return innertype == 1;
        }
        ~lightFunc() noexcept
        {
            if (innertype == 2)
                delete inner.uniqptr;
        }
        lightFunc(lightFunc const &r) noexcept
        {
            innertype = r.innertype;
            extra = r.extra;
            if (innertype == 2)
            {
                inner.uniqptr = new std::function<_FNTP>(*r.inner.uniqptr);
            }
            else
            {
                inner.cptr = r.inner.cptr;
            }
        }
        lightFunc(lightFunc &&r) noexcept
        {
            innertype = r.innertype;
            extra = r.extra;
            if (innertype == 2)
            {
                inner.uniqptr = r.inner.uniqptr;
            }
            else
            {
                inner.cptr = r.inner.cptr;
            }
            r.innertype = 0;
        }
        template <typename Callable, std::enable_if_t<std::is_same_v<std::function<_FNTP>, decltype(std::function(std::declval<Callable>()))>, bool> _goodfn = true>
        lightFunc(Callable &&c) noexcept
        {
            if constexpr (std::is_convertible_v<Callable, _FNTP *>)
            {
                inner.cptr = (_FNTP *)c;
                innertype = 1;
            }
            else
            {
                auto funptr = new std::function<_FNTP>(std::forward<Callable>(c));
                inner.uniqptr = funptr;
                innertype = 2;
            }
        }
        template <typename... Args>
        std::invoke_result_t<_FNTP, Args...> operator()(Args &&... a) const noexcept(nothrow)
        {
            if (innertype == 1)
            {
                return inner.cptr(std::forward<Args>(a)...);
            }
            else
            {
                return inner.uniqptr->operator()(std::forward<Args>(a)...);
            }
        }
    };
}; // namespace YAL