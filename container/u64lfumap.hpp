#pragma once
#include "lfucache.hpp"
#include "clist.hpp"
#include "../type_alias.h"
namespace YAL
{
    template <typename T, int CAP = 1000, int BUCKETS = 67>
    class U64LfuMap
    {
        struct MyPair {
            u64 key;
            T val;
        };
        LfuCache<clistNode<MyPair, false>, CAP> cache;
        clistNode<MyPair, true> buckets[BUCKETS];

    public:
        U64LfuMap()
        {
        }
        T *lookup(u64 key) noexcept
        {
            clistNode<MyPair, true> &list = buckets[key % BUCKETS];
            auto ptr = list.myPtr();
            auto now = ptr->next;
            while (now != ptr)
            {
                if (now->inner.key == key) {
                    cache._moveAhead(cache._dataIDX(now));
                    return &now->inner.val;
                }
                now = now->next;
            }
            return nullptr;
        }
        T *insert(u64 key, T &&x) noexcept
        {
            clistNode<MyPair, false> &nd = *cache.insert_getplace();
            nd.detach();
            nd.insert_after(buckets[key % BUCKETS].myPtr());
            nd.inner.val = std::forward<T>(x);
            nd.inner.key = key;
            return &nd.inner.val;
        }
        void purge() noexcept
        {
            cache.purge();
            for (auto& i : buckets) {
                i = {};
            }
        }
    };
}; // namespace YAL