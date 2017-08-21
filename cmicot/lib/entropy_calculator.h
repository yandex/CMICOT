#pragma once

#include "binarize.h"

#include <util/generic/vector.h>
#include <util/system/types.h>

namespace NCmicot {
    class TEntropyCalculator {
    public:
        TEntropyCalculator(size_t binSize);

        void AddBin(const TBin& bin);

        double GetEntropy() const;
        double GetEntropyWithExtraBin(const TBin& bin) const;

        static constexpr int MAX_BIN_COUNT = 64;

    private:
        int BinCount;
        yvector<ui64> Values;
    };

    struct IFrequencyCounter {
        virtual ~IFrequencyCounter() {
        }

        virtual void Add(ui64 value) = 0;
        virtual double GetEntropy(size_t totalValues) const = 0;
    };

    class TVectorCounter: public IFrequencyCounter {
    public:
        TVectorCounter(ui64 minValue, ui64 maxValue);
        void Add(ui64 value) override;
        double GetEntropy(size_t totalValues) const override;

    private:
        ui64 MinValue;
        yvector<size_t> ValueCount;
    };

    class THashMapCounter: public IFrequencyCounter {
    public:
        void Add(ui64 value) override;
        double GetEntropy(size_t totalValues) const override;

    private:
        yhash<ui64, size_t> ValueCount;
    };
}
