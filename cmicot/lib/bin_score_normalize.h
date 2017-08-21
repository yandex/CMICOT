#pragma once

#include "binarize.h"
#include "bin_feature_set.h"

#include <functional>

namespace NCmicot {
    struct TBinScore;
    class TBackground;
    class TBinFeatureSet;

    using TBinScoreNormalier = std::function<double(const TBinScore& bs, const TBackground& bg, int binIndex)>;

    enum class EBinScoreNormalization {
        None,
        LabelEntropy,
        BinEntropy,
        BinAndLabelEntropy,
    };

    TBinScoreNormalier BuildBinScoreNormalizer(EBinScoreNormalization normalizationType, TBinFeatureSet& label);
}
