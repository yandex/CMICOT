#pragma once

#include "bin_score.h"
#include "bin_score_normalize.h"

#include <util/generic/vector.h>

#include <functional>

namespace NCmicot {
    struct TFeatureScore {
        double Score;
        yvector<TBinScore> BinScore;
    };

    TFeatureScore GetFeatureScore(TBackground background, int featureIndex, const IBinScorer& binScorer,
                                  TBinScoreNormalier binScoreNormalier);

    struct TCmimScore {
        double Score;
        int MinimizingFeatureIndex;
    };

    TCmimScore GetCmimScore(const TBinFeatureSet& label, TBackground background, int featureIndex,
                            int threadCount);
}
