#include "cmi_calculator.h"

namespace NCmicot {
    TCmiCalculator::TCmiCalculator(size_t binSize)
        : FirstCondition(binSize)
        , Condition(binSize)
        , FirstSecondCondition(binSize)
        , SecondCondition(binSize)
    {
    }

    void TCmiCalculator::AddFirstVariableBin(const TBin& bin) {
        FirstCondition.AddBin(bin);
        FirstSecondCondition.AddBin(bin);
    }

    void TCmiCalculator::AddSecondVariableBin(const TBin& bin) {
        SecondCondition.AddBin(bin);
        FirstSecondCondition.AddBin(bin);
    }

    void TCmiCalculator::AddConditionBin(const TBin& bin) {
        FirstCondition.AddBin(bin);
        Condition.AddBin(bin);
        FirstSecondCondition.AddBin(bin);
        SecondCondition.AddBin(bin);
    }

    double TCmiCalculator::GetValueWithConditionBin(const TBin& bin) const {
        return FirstCondition.GetEntropyWithExtraBin(bin) - Condition.GetEntropyWithExtraBin(bin) - FirstSecondCondition.GetEntropyWithExtraBin(bin) + SecondCondition.GetEntropyWithExtraBin(bin);
    }

    double TCmiCalculator::GetValue() const {
        return FirstCondition.GetEntropy() - Condition.GetEntropy() - FirstSecondCondition.GetEntropy() + SecondCondition.GetEntropy();
    }
}
