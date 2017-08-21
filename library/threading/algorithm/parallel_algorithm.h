#pragma once

#include <library/threading/future/legacy_future.h>

#include <util/draft/holder_vector.h>
#include <util/generic/algorithm.h>
#include <util/thread/queue.h>

#include <iterator>

namespace NDetail {
    template <class TIter, class TFunc, class TFutureType>
    void RunParallelForEach(TIter begin, TIter end, TFunc f, THolderVector<TFutureType>& v, size_t maxParallel = 0) {
        IThreadPool* pool;

        THolder<TMtpQueue> mtpQueue;

        if (maxParallel != 0) {
            mtpQueue.Reset(new TMtpQueue());
            mtpQueue->Start(maxParallel);
            pool = mtpQueue.Get();
        } else {
            pool = SystemThreadPool();
        }

        for (; begin != end; ++begin) {
            v.PushBack(new TFutureType(std::bind(f, *begin), pool));
        }
    }

    template <class TF, class TRes>
    TRes CallNoArgs(TF f) {
        return f();
    }

    template <class TF>
    void CallNoArgsNoRes(TF f) {
        f();
    }
}

template <class TIter, class TFunc, class TRes>
void ParallelForEach(TIter begin, TIter end, TFunc f, yvector<TRes>& res, size_t maxParallel = 0) {
    res.clear();

    using TFutureType = NThreading::TLegacyFuture<TRes, false>;
    THolderVector<TFutureType> v;

    ::NDetail::RunParallelForEach(begin, end, f, v, maxParallel);

    for (typename THolderVector<TFutureType>::const_iterator it = v.begin(); it != v.end(); ++it) {
        res.push_back((*it)->Get());
    }
}

template <class TIter, class TFunc>
inline void ParallelForEach(TIter begin, TIter end, TFunc f, size_t maxParallel = 0) {
    using TFutureType = NThreading::TLegacyFuture<void, false>;
    THolderVector<TFutureType> v;

    ::NDetail::RunParallelForEach(begin, end, f, v, maxParallel);
}

// Call f with args [0, n) in parallel
template <class TFunc, class TRes>
inline void ParallelForEach(size_t n, TFunc f, yvector<TRes>& res, size_t maxParallel = 0) {
    yvector<size_t> nums(n);
    Iota(nums.begin(), nums.end(), 0);
    ParallelForEach(nums.begin(), nums.end(), f, res, maxParallel);
}

// Call f with args [0, n) in parallel
template <class TFunc>
inline void ParallelForEach(size_t n, TFunc f, size_t maxParallel = 0) {
    yvector<size_t> nums(n);
    Iota(nums.begin(), nums.end(), 0);
    ParallelForEach(nums.begin(), nums.end(), f, maxParallel);
}

template <class TIter, class TRes>
void ParallelCall(TIter begin, TIter end, yvector<TRes>& res, size_t maxParallel = 0) {
    ParallelForEach(begin, end, ::NDetail::CallNoArgs<typename std::iterator_traits<TIter>::value_type, TRes>, res, maxParallel);
}

template <class TIter>
void ParallelCall(TIter begin, TIter end, size_t maxParallel = 0) {
    ParallelForEach(begin, end, ::NDetail::CallNoArgsNoRes<typename std::iterator_traits<TIter>::value_type>, maxParallel);
}
