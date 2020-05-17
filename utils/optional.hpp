#pragma once
#include <exception>
#include <type_traits>
namespace YAL
{
    template <typename T>
    struct optional
    {
        union innerData
        {
            char _Dummy;
            T D;
            innerData() {}
            ~innerData() {}
        } dat;
        bool set;
        T &rget()
        {
            return dat.D;
        }
        T const &rget() const
        {
            return dat.D;
        }
        static void _Throw()
        {
            throw std::exception("bad optional access");
        }
        T &get() noexcept(false)
        {
            if (!set)
            {
                _Throw();
            }
            return rget();
        }
        T const &get() const noexcept(false)
        {
            if (!set)
            {
                _Throw();
            }
            return rget();
        }
        bool Set() const
        {
            return set;
        }
        void _clear()
        {
            if (set)
            {
                dat.D.~T();
                set = false;
            }
        }
        ~optional() noexcept
        {
            _clear();
        }
        optional()
        {
            set = false;
        }
        optional(optional const &r) noexcept
        {
            set = r.set;
            if (set)
            {
                new (&dat.D) T(r.rget());
            }
        }
        optional(optional &&r) noexcept
        {
            set = r.set;
            if (set)
            {
                new (&dat.D) T(std::move(r.rget()));
            }
            r._clear();
        }
        optional &operator=(optional const &r) noexcept
        {
            _clear();
            set = r.set;
            if (set)
            {
                new (&dat.D) T(r.rget());
            }
            return *this;
        }
        void operator=(T const &r) noexcept
        {
            _clear();
            set = true;
            new (&dat.D) T(r);
        }
        template <typename... Args, std::enable_if_t<std::is_constructible_v<T, Args...>, bool> good = true>
        optional(Args &&... r) noexcept
        {
            set = true;
            new (&dat.D) T(std::forward<Args>(r)...);
        }
        template <typename... Args>
        void emplace(Args &&... r) noexcept
        {
            _clear();
            set = true;
            new (&dat.D) T(std::forward<Args>(r)...);
        }
        bool operator==(T const &r) const
        {
            if (set && rget() == r)
                return true;
            return false;
        }
        bool operator==(optional const &r) const
        {
            if (set && r.set && rget() == r.rget())
                return true;
            return false;
        }
    };
}; // namespace YAL