#pragma once

#include <library/threading/algorithm/parallel_algorithm.h>

#include <functional>

namespace NCmicot {
    namespace Impl {
        template <class I, class F, class P>
        I ExtremeElementBy(I begin, I end, F func, P pred) {
            if (begin == end) {
                return end;
            }

            auto bestValue = func(*begin);
            auto bestPos = begin;

            for (auto i = ++begin; i != end; ++i) {
                auto curValue = func(*i);
                if (pred(curValue, bestValue)) {
                    bestValue = curValue;
                    bestPos = i;
                }
            }

            return bestPos;
        }

        template <class I, class F, class P>
        I ParallelExtremeElementBy(I begin, I end, F&& func, P&& pred, int threadCount) {
            using TValue = decltype(func(*begin));

            yvector<TValue> values;
            ParallelForEach(begin, end, std::forward<F>(func), values, threadCount);

            auto identity = [](const TValue& x) { return x; };
            auto iter = ExtremeElementBy(values.begin(), values.end(), identity, std::forward<P>(pred));

            return begin + (iter - values.begin());
        }
    }

    template <class I, class F>
    I ParallelMinElementBy(I begin, I end, F&& func, int threadCount) {
        using TValue = decltype(func(*begin));
        return Impl::ParallelExtremeElementBy(begin, end, std::forward<F>(func), std::less<TValue>(), threadCount);
    }

    template <class C, class F>
    auto ParallelMinElementBy(const C& c, F&& func, int threadCount = 0) -> decltype(std::begin(c)) {
        return ParallelMinElementBy(std::begin(c), std::end(c), std::forward<F>(func), threadCount);
    }

    template <class I, class F>
    I ParallelMaxElementBy(I begin, I end, F&& func, int threadCount) {
        using TValue = decltype(func(*begin));
        return Impl::ParallelExtremeElementBy(begin, end, std::forward<F>(func), std::greater<TValue>(), threadCount);
    }

    template <class C, class F>
    auto ParallelMaxElementBy(const C& c, F&& func, int threadCount = 0) -> decltype(std::begin(c)) {
        return ParallelMaxElementBy(std::begin(c), std::end(c), std::forward<F>(func), threadCount);
    }
}
