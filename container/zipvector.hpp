#pragma once
#include "../type_traits.h"
#include "../core.hpp"

#ifndef CONTAINER_ABORT
#define CONTAINER_ABORT(err) YAL::YAL_ABORT(err, __func__, __FILE__, __LINE__)
#endif
#ifndef CONTAINER_CHECK
#if 0
#define CONTAINER_CHECK(cond) ((cond) || YAL::YAL_ABORT(#cond, __func__, __FILE__, __LINE__))
#else
#define CONTAINER_CHECK(x)
#endif
#endif

namespace YAL
{
    template <typename _Elem, int STATICSZ = 8 /*, std::enable_if_t < is_bitwise_movable<_Elem>{}, bool > isgood = true*/>
    class ZipVector
    {
    private:
        int Size;
        int MyCap;
        union innerData {
            _Elem *ddat;
            _Elem d[STATICSZ];
            innerData()
            {
            }
            ~innerData()
            {
            }
        } static_dat;
        _Elem const *StoragePtr() const
        {
            return MyCap ? static_dat.ddat : static_dat.d;
        }
        _Elem *StoragePtr()
        {
            return MyCap ? static_dat.ddat : static_dat.d;
        }
        static constexpr int PR_FACTOR()
        {
            if constexpr (std::is_arithmetic_v<_Elem>)
                return 96 / sizeof(_Elem);
            if constexpr (sizeof(_Elem) <= 16)
                return 8;
            return 4;
        }
        constexpr static inline int FACTOR_MASK = ~(PR_FACTOR() - 1);
    public:
#pragma region getter
        _Elem &at_ref(int idx)
        {
            CONTAINER_CHECK(idx < Size && idx >= 0);
            return StoragePtr()[idx];
        }
        _Elem const &at_ref(int idx) const
        {
            CONTAINER_CHECK(idx < Size && idx >= 0);
            return StoragePtr()[idx];
        }
        _Elem &operator[](int idx)
        {
            return at_ref(idx);
        }
        _Elem const &operator[](int idx) const
        {
            return at_ref(idx);
        }
        _Elem at_copy(int idx) const
        {
            return at_ref(idx);
        }
        _Elem const &front() const
        {
            return at_ref(0);
        }
        _Elem &front()
        {
            return at_ref(0);
        }
        _Elem const &back() const
        {
            CONTAINER_CHECK(Size > 0);
            return at_ref(Size - 1);
        }
        _Elem &back()
        {
            CONTAINER_CHECK(Size > 0);
            return at_ref(Size - 1);
        }
        _Elem *data()
        {
            return StoragePtr();
        }
        _Elem const *data() const
        {
            return StoragePtr();
        }
#pragma endregion
#pragma region iterator
        template <typename T>
        struct iterator
        {
            using data_t = std::conditional_t<std::is_const_v<T>, _Elem const, _Elem>;
            data_t *inner;
            iterator(T *faa, int x) { inner = ((_Elem *)faa->data()) + x; }
            iterator &operator++()
            {
                inner++;
                return *this;
            }
            iterator &operator--()
            {
                inner--;
                return *this;
            }
            bool operator==(iterator y)
            {
                return inner == y.inner;
            }
            bool operator!=(iterator y)
            {
                return !operator==(y);
            }
            data_t &operator*()
            {
                return *inner;
            }
        };
        iterator<const ZipVector> end() const
        {
            return {this, Size};
        }
        iterator<const ZipVector> begin() const
        {
            return {this, 0};
        }
        iterator<ZipVector> end()
        {
            return {this, Size};
        }
        iterator<ZipVector> begin()
        {
            return {this, 0};
        }
#pragma endregion
#pragma region alloc
    private:
        static constexpr int ElemSz(int count)
        {
            return count * sizeof(_Elem);
        }
        static _Elem *MyAlloc(int count)
        {
            auto rv = (_Elem *)malloc(ElemSz(count));
            if (!rv)
            {
                CONTAINER_ABORT("malloc error");
            }
            return rv;
        }
        void MyReAlloc(int count)
        {
            auto ddat2 = (_Elem *)realloc(static_dat.ddat, ElemSz(count));
            if (!ddat2)
            {
                CONTAINER_ABORT("realloc error");
            }
            static_dat.ddat = ddat2;
        }
        template <bool overlapped = true>
        void move_data(_Elem *from, _Elem *to, int count)
        {
            if constexpr (overlapped)
            {
                memmove(to, from, ElemSz(count));
            }
            else
            {
                memcpy(to, from, ElemSz(count));
            }
        }
        void ddat_free()
        {
            MyCap = 0;
            free(static_dat.ddat);
        }
        void reserve_at_least(int count)
        {
            if (MyCap == 0)
            {
                if (count <= STATICSZ)
                    return;
                //we need to move sdat to ddat
                MyCap = max(count, STATICSZ + PR_FACTOR());
                //static_dat.ddat = MyAlloc(MyCap);
                auto dst = MyAlloc(MyCap);
                move_data<false>(static_dat.d, dst, Size);
                static_dat.ddat = dst;
            }
            else
            {
                if (MyCap < count)
                {
                    //we need to extend ddat
                    //MyCap = max(count, MyCap+PR_FACTOR());
                    MyCap = ((count + (PR_FACTOR())) & FACTOR_MASK);
                    MyReAlloc(MyCap);
                }
            }
        }

    public:
        template <bool force = false>
        void try_shrink()
        {
            if (MyCap)
            {
                if (Size <= 0.6 * STATICSZ || (force && Size <= STATICSZ))
                {
                    //shrink to sdat
                    move_data<false>(static_dat.ddat, static_dat.d, Size);
                    ddat_free(); //MyCap=0
                    return;
                }
                int delta = MyCap - Size;
                if (delta > 1.5 * PR_FACTOR() || (force && delta))
                {
                    //shrink ddat
                    MyReAlloc(Size);
                    MyCap = Size;
                }
            }
        }

    private:
        void _defunct_range(int s, int e)
        {
            _Elem *d = StoragePtr();
            for (; s < e; ++s)
                d[s].~_Elem();
        }

    public:
        void shrink_to_fit()
        {
            try_shrink<true>();
        }
        void reserve(int count)
        {
            reserve_at_least(count);
        }
#pragma endregion
#pragma region setter
        template <typename... TP>
        _Elem &emplace_at(int idx, TP &&... args)
        {
            CONTAINER_CHECK(idx < Size);
            at_ref(idx).~_Elem();
            (new (&at_ref(idx)) _Elem(std::forward<TP>(args)...));
            return at_ref(idx);
        }
        template <typename... TP>
        _Elem &emplace_back(TP &&... args)
        {
            reserve_at_least(++Size);
            (new (&at_ref(Size - 1)) _Elem(std::forward<TP>(args)...));
            return at_ref(Size - 1);
        }
        void pop_back()
        {
            CONTAINER_CHECK(Size);
            _defunct_range(Size - 1, Size);
            Size--;
            try_shrink();
        }
        void clear()
        {
            _defunct_range(0, Size);
            if (MyCap)
                ddat_free();
            Size = 0;
            MyCap = 0;
        }
#pragma endregion
#pragma region list_traits
        void remove_range(int s, int e)
        {
            CONTAINER_CHECK((s < Size && e <= Size));
            _defunct_range(s, e);
            _Elem *sp = StoragePtr();
            move_data(sp + e, sp + s, Size - e); //move back to front
            Size -= e - s;
            try_shrink();
        }
        void inserts_before(int s, _Elem const *d, int cnt)
        {
            //CONTAINER_CHECK((s < Size));
            reserve_at_least(Size + cnt);
            _Elem *sp = StoragePtr();
            //make a hole
            move_data(sp + s, sp + s + cnt, Size - s);
            for (int i = 0; i < cnt; ++i)
            {
                new (sp + s + i) _Elem(d[i]);
            }
            Size += cnt;
        }
        template <typename... Args>
        void emplace_before(int s, Args &&... a)
        {
            //CONTAINER_CHECK((s < Size));
            reserve_at_least(Size + 1);
            _Elem *sp = StoragePtr();
            //make a hole
            move_data(sp + s, sp + s + 1, Size - s);
            new (sp + s) _Elem(std::forward<Args>(a)...);
            Size += 1;
        }
        void replace_range(int s, int e, _Elem const *d, int cnt)
        {
            CONTAINER_CHECK((s < Size));
            //step 1.defunct data
            _defunct_range(s, e);
            //step 2.reserve space
            int needle = max(Size, Size - (e - s) + cnt);
            reserve_at_least(needle);
            //step 3.move out old data
            _Elem *sp = StoragePtr();
            move_data(sp + e, sp + s + cnt, Size - e);
            //step 4.copy new data
            for (int i = 0; i < cnt; ++i)
            {
                new (sp + s + i) _Elem(d[i]);
            }
            Size = Size - (e - s) + cnt;
            try_shrink();
        }
        template <typename... Args>
        _Elem &emplace_after(int s, Args &&... a)
        {
            CONTAINER_CHECK((s < Size));
            reserve_at_least(Size + 1);
            _Elem *sp = StoragePtr();
            //make a hole
            move_data(sp + s, sp + s + 1, Size - s);
            new (sp + s + 1) _Elem(std::forward<Args>(a)...);
            Size += 1;
            return sp[s + 1];
        }
        void remove_at(int s)
        {
            CONTAINER_CHECK(s < Size);
            remove_range(s, s + 1);
        }
        void remove_front()
        {
            CONTAINER_CHECK(Size != 0);
            remove_at(0);
        }
        template <typename... Args>
        _Elem &emplace_front(Args &&... a)
        {
            reserve_at_least(Size + 1);
            _Elem *sp = StoragePtr();
            move_data(sp, sp + 1, Size);
            new (sp) _Elem(std::forward<Args>(a)...);
            Size++;
            return sp[0];
        }
        void _expand_no_construct(int toSize)
        {
            CONTAINER_CHECK(toSize >= Size);
            reserve_at_least(toSize);
            Size = toSize;
        }
        void _set_size(int to) {
            Size = to;
        }
#pragma endregion
        constexpr int size() const
        {
            return Size;
        }
        ZipVector()
        {
            Size = MyCap = 0;
        }
        ZipVector(ZipVector const &r) noexcept
        {
            Size = MyCap = 0;
            reserve_at_least(r.Size);
            _Elem const *sp = r.StoragePtr();
            for (int i = 0; i < r.Size; ++i)
            {
                emplace_back(sp[i]);
            }
        }
        ZipVector &operator=(ZipVector const &r) noexcept
        {
            clear();
            reserve_at_least(r.Size);
            _Elem const *sp = r.StoragePtr();
            for (int i = 0; i < r.Size; ++i)
            {
                emplace_back(sp[i]);
            }
            return *this;
        }
        ZipVector(ZipVector &&r) noexcept
        {
            MyCap = r.MyCap;
            Size = r.Size;
            if (!MyCap)
            {
                move_data<false>(r.static_dat.d, static_dat.d, Size);
            }
            else
            {
                static_dat.ddat = r.static_dat.ddat;
            }
            r.MyCap = 0;
            r.Size = 0;
        }
        ~ZipVector() noexcept
        {
            clear();
        }
    };
} // namespace YAL