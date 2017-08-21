#pragma once

#include "binarize.h"
#include "entropy_calculator.h"

namespace NCmicot {
    class TMutualInformationCalculator {
    public:
        TMutualInformationCalculator(size_t binSize);

        void AddFirstVariableBin(const TBin& bin);
        double GetValueWithSecondVariableBin(const TBin& bin) const;

    private:
        TEntropyCalculator First;
    };
}
