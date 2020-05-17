#pragma once
#include <cstring>
#include "../type_traits.h"
namespace YAL
{
    template <typename INNER, int CAP>
    class LfuCache
    {
    public:
        INNER dat[CAP];
        INNER* datIDX[CAP];
        int head, tail;
        struct iterator {
            int step = 0;
            int head;
            LfuCache* fa;
            void operator++() {
                step++;
                head++;
                if (head == CAP) head = 0;
            }
            bool operator==(iterator const&) {
                return step == CAP;
            }
            bool end() {
                return step == CAP;
            }
            iterator(LfuCache* f,int h):head(h),fa(f){}
            INNER& operator*() {
                return *fa->datIDX[head];
            }
        };
        void _moveAhead(int idx) {
            if (idx != head) {
                int to = idx - 1;
                if (to < 0) to = CAP - 1;
                std::swap(datIDX[to], datIDX[idx]);
            }
        }
        int _dataIDX(INNER* in) {
            return int(((uintptr_t)in - (uintptr_t)dat) / sizeof(INNER));
        }
        void purge()
        {
            for (int i = 0; i < CAP; ++i)
            {
                datIDX[i] = dat + i;
            }
            for (int i = 0; i < CAP; ++i)
            {
                dat[i] = {};
            }
        }
        LfuCache()
        {
            head = tail = 0;
            for (int i = 0; i < CAP; ++i)
            {
                datIDX[i] = dat + i;
            }
        }
        INNER* insert_getplace()
        {
            head--;
            tail--;
            if (tail < 0)
                tail = CAP - 1;
            if (head < 0)
                head = CAP - 1;
            return datIDX[head];
        }
        iterator begin() {
            return iterator(this,head);
        }
        iterator end() {
            return iterator(this, 0);
        }

    };
}; // namespace YAL