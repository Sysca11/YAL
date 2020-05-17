#pragma once
#include <random>
#include <ctime>
namespace YAL::Fuzz
{
    struct RngState
    {
        unsigned long long now;
        auto operator()()
        {
            now = now * 114514 + 19260817;
            return now;
        }
        RngState(unsigned int seed)
        {
            now = seed;
        }
    };

    template <typename T1, typename T2>
    struct default_comp
    {
        bool operator()(T1 const &x, T2 const &y) const
        {
            return x == y;
        }
    };

    clock_t F1_time, F2_time;
    template <typename F1, typename F2, typename Comp = default_comp<F1, F2>>
    class Fuzzer
    {
        F1 x;
        F2 y;

    public:
        bool do_fuzz(const char *desc, void (*xrun)(RngState, F1 &), void (*yrun)(RngState, F2 &))
        {
            Comp comp;
            RngState sta((std::random_device()()));
            clock_t t1 = clock();
            xrun(sta, x);
            clock_t t2 = clock();
            yrun(sta, y);
            clock_t t3 = clock();
            F1_time += t2 - t1;
            F2_time += t3 - t2;
            if (!comp(x, y))
            {
                YAL_ABORT(desc,"fuzzing","",-1);
                return false;
            }
            return true;
        }
        bool operator()(const char *desc, void (*xrun)(RngState, F1 &), void (*yrun)(RngState, F2 &))
        {
            bool sta = do_fuzz(desc, xrun, yrun);
            return sta;
        }
        bool newState(const char *desc, void (*xrun)(RngState, F1 &), void (*yrun)(RngState, F2 &))
        {
            x = {};
            y = {};
            bool sta = do_fuzz(desc, xrun, yrun);
            return sta;
        }
    };
}; // namespace YAL::Fuzz