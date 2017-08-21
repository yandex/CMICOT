#pragma once

#include "bin_score.h"

namespace NCmicot {
    class TCachingBinScorer {
    public:
        TCachingBinScorer(const NCmicot::TBinFeatureSet& label, int binCount);

        TBinScore Evaluate(NCmicot::TBackground background, int evalBinIndex, int stepCount);

    private:
        const NCmicot::TBinFeatureSet& Label;
        yvector<NCmicot::TBinScore> Cache;
        const double LabelEntropy;
    };
}
