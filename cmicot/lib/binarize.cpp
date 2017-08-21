#include "binarize.h"

#include <library/getopt/small/last_getopt.h>
#include <library/threading/algorithm/parallel_algorithm.h>

#include <util/generic/algorithm.h>
#include <util/generic/xrange.h>

namespace NCmicot {
    yvector<TBin> BinarizeFeature(const yvector<double>& feature, TBorderBuilder borderBuilder) {
        yvector<float> values(feature.begin(), feature.end());
        const yhash_set<float> borders = borderBuilder(values);

        if (borders.empty()) {
            return {TBin(feature.size(), 0)};
        }

        yvector<TBin> result;
        result.reserve(borders.size());

        for (float border : borders) {
            result.push_back(TBin());
            auto& currentBin = result.back();

            for (auto x : feature) {
                currentBin.push_back(x > border);
            }
        }

        return result;
    }

    yvector<ui64> UniteLabelBins(const yvector<TBin>& binarizedLabel) {
        yvector<ui64> result(binarizedLabel.front().size());

        for (int binIndex : xrange(binarizedLabel.size())) {
            for (int valueIndex : xrange(result.size())) {
                result[valueIndex] |= (binarizedLabel[binIndex][valueIndex] << binIndex);
            }
        }

        return result;
    }

    std::pair<TBinFeatureSet, TBinFeatureSet> BinarizeRawPool(
        const yvector<yvector<double>>& inputData,
        TBorderBuilder borderBuilder,
        int maxParallel
    ) {
        yvector<yvector<TBin>> binarizedFeatures;
        binarizedFeatures.reserve(inputData.size());

        auto kernel = [borderBuilder](const yvector<double>& values) {
            return BinarizeFeature(values, borderBuilder);
        };
        ParallelForEach(inputData.begin(), inputData.end(), kernel, binarizedFeatures, maxParallel);

        TBinFeatureSet label(std::move(binarizedFeatures[0]));

        TBinFeatureSet features;
        for (auto iter = binarizedFeatures.begin() + 1; iter != binarizedFeatures.end(); ++iter) {
            features.AddFeature(std::move(*iter));
        }
        return {label, features};
    }
}
