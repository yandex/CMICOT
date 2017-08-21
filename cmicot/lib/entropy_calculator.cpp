#include "entropy_calculator.h"

#include <util/generic/algorithm.h>
#include <util/generic/ymath.h>
#include <util/system/yassert.h>

#include <algorithm>

namespace NCmicot {
    namespace {
        THolder<IFrequencyCounter> BuildFrequencyCounter(ui64 minValue, ui64 maxValue) {
            const auto rangeSize = maxValue - minValue + 1;
            if (rangeSize > (1 << 22)) {
                return new THashMapCounter;
            } else {
                return new TVectorCounter(minValue, maxValue);
            }
        }
    };

    /* TEntropyCalculator */

    TEntropyCalculator::TEntropyCalculator(size_t binSize)
        : BinCount(0)
        , Values(binSize, 0)
    {
    }

    void TEntropyCalculator::AddBin(const TBin& bin) {
        Y_VERIFY(bin.size() == Values.size(), "Value size = %lu, bin size = %lu", Values.size(), bin.size());
        Y_VERIFY(++BinCount <= MAX_BIN_COUNT, "You can use no more than %d bins", MAX_BIN_COUNT);

        for (size_t i = 0; i < bin.size(); ++i) {
            Values[i] <<= 1;
            if (bin[i]) {
                Values[i] |= 1;
            }
        }
    }

    double TEntropyCalculator::GetEntropy() const {
        decltype(Values.begin()) minIter, maxIter;
        std::tie(minIter, maxIter) = MinMaxElement(Values.begin(), Values.end());
        auto freqCounter = BuildFrequencyCounter(*minIter, *maxIter);

        for (auto x : Values) {
            freqCounter->Add(x);
        }

        return freqCounter->GetEntropy(Values.size());
    }

    double TEntropyCalculator::GetEntropyWithExtraBin(const TBin& bin) const {
        Y_VERIFY(bin.size() == Values.size(), "Value size = %lu, bin size = %lu", Values.size(), bin.size());

        decltype(Values.begin()) minIter, maxIter;
        std::tie(minIter, maxIter) = MinMaxElement(Values.begin(), Values.end());
        auto freqCounter = BuildFrequencyCounter(2 * *minIter, 2 * *maxIter + 1);

        for (size_t i = 0; i < bin.size(); ++i) {
            freqCounter->Add(2 * Values[i] + bin[i]);
        }

        return freqCounter->GetEntropy(Values.size());
    }

    /* TVectorCounter */

    TVectorCounter::TVectorCounter(ui64 minValue, ui64 maxValue)
        : MinValue(minValue)
        , ValueCount(maxValue - MinValue + 1, 0)
    {
    }

    void TVectorCounter::Add(ui64 value) {
        ++ValueCount[value - MinValue];
    }

    double TVectorCounter::GetEntropy(size_t totalValues) const {
        double result = 0.0;
        for (auto kv : ValueCount) {
            if (kv > 0) {
                result += kv * Log2(1.0 * kv / totalValues);
            }
        }
        return -result / totalValues;
    }

    /* THashMapCounter */

    void THashMapCounter::Add(ui64 value) {
        ++ValueCount[value];
    }

    double THashMapCounter::GetEntropy(size_t totalValues) const {
        double result = 0.0;
        for (const auto& kv : ValueCount) {
            result += kv.second * Log2(1.0 * kv.second / totalValues);
        }
        return -result / totalValues;
    }
}
