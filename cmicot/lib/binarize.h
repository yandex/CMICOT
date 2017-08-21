#pragma once

#include "bin.h"
#include "bin_feature_set.h"

#include <util/generic/vector.h>
#include <util/generic/hash_set.h>
#include <util/generic/xrange.h>
#include <util/generic/algorithm.h>

#include <functional>

namespace NSplitSelection {
    class IBinarizer;
}

namespace NCmicot {
    using TBorderBuilder = std::function<yhash_set<float>(yvector<float>& values)>;

    yvector<TBin> BinarizeFeature(const yvector<double>& feature, TBorderBuilder borderBuilder);

    yvector<ui64> UniteLabelBins(const yvector<TBin>& binarizedLabel);

    std::pair<TBinFeatureSet, TBinFeatureSet> BinarizeRawPool(
        const yvector<yvector<double>>& inputData,
        TBorderBuilder borderBuilder, int maxParallel
    );

    template <class Iter>
    TBinFeatureSet BinarizeWithMap(Iter begin, Iter end, const yvector<int>& binToFeatureMap) {
        yvector<TBin> poolBins;
        for (auto floatFeature : xrange(begin, end)) {
            poolBins.push_back(TBin());
            for (auto value : *floatFeature) {
                Y_ENSURE(static_cast<ui32>(value) <= 1, value << " is not a binary value");
                poolBins.back().push_back(static_cast<ui32>(value));
            }
        }

        Y_ENSURE(poolBins.size() == binToFeatureMap.size(),
            "Pool has " << poolBins.size() << " bins, but map has only " << binToFeatureMap.size()
                        << ". These numbers should be the same");

        const int featureCount = *MaxElement(binToFeatureMap.begin(), binToFeatureMap.end());
        yvector<yvector<TBin>> features(featureCount + 1);
        for (int i : xrange(poolBins.size())) {
            features[binToFeatureMap[i]].push_back(std::move(poolBins[i]));
        }

        TBinFeatureSet result;
        for (auto& bins : features) {
            result.AddFeature(std::move(bins));
        }
        return result;
    }
}
