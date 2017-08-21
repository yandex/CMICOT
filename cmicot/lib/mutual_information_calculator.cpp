#include "mutual_information_calculator.h"
#include "entropy.h"

namespace NCmicot {
    void TMutualInformationCalculator::AddFirstVariableBin(const TBin& bin) {
        First.AddBin(bin);
    }

    TMutualInformationCalculator::TMutualInformationCalculator(size_t binSize)
        : First(binSize)
    {
    }

    double TMutualInformationCalculator::GetValueWithSecondVariableBin(const TBin& bin) const {
        return First.GetEntropy() + Entropy(bin) - First.GetEntropyWithExtraBin(bin);
    }
}
