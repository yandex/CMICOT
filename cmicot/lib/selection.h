#pragma once

#include "bin_feature_set.h"

#include <util/generic/vector.h>

#include <functional>

namespace NCmicot {
    void FeatureSelection(const TBinFeatureSet& label, const TBinFeatureSet& features,
                          int evalStepCount, int threadCount, std::function<void(int)> onFeatureSelected);

    void FastFeatureSelection(
        const TBinFeatureSet& label,
        const TBinFeatureSet& features,
        int evalStepCount,
        int threadCount,
        int featureCount,
        std::function<void(int)> onFeatureSelected);

    yvector<int> FeatureSelection(const TBinFeatureSet& label, const TBinFeatureSet& features,
                                  int evalStepCount, int threadCount);
}
