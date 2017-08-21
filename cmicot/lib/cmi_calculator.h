#pragma once

#include "binarize.h"
#include "entropy_calculator.h"

namespace NCmicot {
    class TCmiCalculator {
    public:
        TCmiCalculator(size_t binSize);

        void AddFirstVariableBin(const TBin& bin);
        void AddSecondVariableBin(const TBin& bin);
        void AddConditionBin(const TBin& bin);

        double GetValueWithConditionBin(const TBin& bin) const;
        double GetValue() const;

    private:
        TEntropyCalculator FirstCondition;
        TEntropyCalculator Condition;
        TEntropyCalculator FirstSecondCondition;
        TEntropyCalculator SecondCondition;
    };
}
