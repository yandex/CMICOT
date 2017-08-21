#pragma once

#include "binarize.h"
#include "bin_feature_set.h"

#include <util/generic/vector.h>
#include <util/random/shuffle.h>
#include <util/generic/xrange.h>

#include <utility>

namespace NCmicot {
    template <class Rng>
    TBin RandomBin(int size, int onesPercent, Rng& rng) {
        TBin result(onesPercent * size / 100, true);
        result.resize(size, false);
        Shuffle(result.begin(), result.end(), rng);
        return result;
    }

    template <class Rng>
    TBin RandomBin(int size, Rng& rng) {
        return RandomBin(size, 50, rng);
    }

    template <class Rng>
    yvector<TBin> RandomFeature(int binCount, int binSize, int onesPercent, Rng& rng) {
        yvector<TBin> result;

        for (auto i : xrange(binCount)) {
            Y_UNUSED(i);
            result.push_back(RandomBin(binSize, onesPercent, rng));
        }

        return result;
    }

    template <class Rng>
    yvector<TBin> RandomFeature(int binCount, int binSize, Rng& rng) {
        return RandomFeature(binCount, binSize, 50, rng);
    }

    template <class Rng>
    TBin RemoveInfo(TBin bin, int percent, Rng& rng) {
        yvector<int> positions = xrange(bin.size());
        Shuffle(positions.begin(), positions.end(), rng);

        for (int i : xrange(percent * positions.ysize() / 100)) {
            bin[positions[i]] = false;
        }

        return bin;
    }

    yvector<int> InvertPermutation(const yvector<int>& permutation);

    template <class Rng>
    TBinFeatureSet MakeRandomFeatures(Rng& rng, int featureCount, int binSize,
                                      std::pair<int, int> featureSizeRange) {
        TBinFeatureSet features;
        for (int i : xrange(featureCount)) {
            Y_UNUSED(i);

            int featureSize = featureSizeRange.first != featureSizeRange.second
                                  ? rng.Uniform(featureSizeRange.first, featureSizeRange.second)
                                  : featureSizeRange.first;
            yvector<TBin> currentFeature(featureSize);
            for (TBin& bin : currentFeature) {
                bin = RandomBin(binSize, rng);
            }
            features.AddFeature(std::move(currentFeature));
        }
        return features;
    }

    template <class Rng>
    TBinFeatureSet MakeRandomLabel(Rng& rng, int binSize, std::pair<int, int> binCountRange) {
        return MakeRandomFeatures(rng, 1, binSize, binCountRange);
    }

    template <class Rng>
    TBinFeatureSet MakeRandomBinaryFeatures(Rng& rng, int featureCount, int binSize) {
        return MakeRandomFeatures(rng, featureCount, binSize, {1, 1});
    }
}
