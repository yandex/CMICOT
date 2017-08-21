#include "selection.h"
#include <cmicot/lib/binarize.h>
#include <cmicot/lib/bin_feature_set.h>
#include <cmicot/lib/test_pool_gen.h>

#include <library/unittest/registar.h>

#include <util/generic/algorithm.h>
#include <util/generic/xrange.h>
#include <util/random/fast.h>
#include <util/random/shuffle.h>

namespace NCmicot {
    SIMPLE_UNIT_TEST_SUITE(Selection) {
        SIMPLE_UNIT_TEST(Works) {
            TReallyFastRng32 rng(20160127);
            const int binSize = 7500;

            const TBin labelBin = RandomBin(binSize, 65, rng);

            yvector<TBin> meaningfulBins;
            for (int percentToRemove : xrange(55, 85, 3)) {
                meaningfulBins.push_back(RemoveInfo(labelBin, percentToRemove, rng));
            }

            yvector<int> binIds = xrange(meaningfulBins.size());
            Shuffle(binIds.begin(), binIds.end(), rng);

            TBinFeatureSet features;
            for (int binId : binIds) {
                features.AddFeature({
                    RandomBin(binSize, rng),
                    meaningfulBins[binId],
                    RandomBin(binSize, rng),
                });
            }

            const TBinFeatureSet label({labelBin});
            UNIT_ASSERT_VALUES_EQUAL(FeatureSelection(label, features, 6, 4), InvertPermutation(binIds));
        }

        SIMPLE_UNIT_TEST(Label) {
            TReallyFastRng32 rng(20160127);
            const int binSize = 1000;

            const TBin labelBin = RandomBin(binSize, 65, rng);
            const TBinFeatureSet label({labelBin});

            TBinFeatureSet features;
            for (int i = rng.Uniform(5, 15); i >= 0; --i) {
                features.AddFeature({RandomBin(binSize, rng)});
            }
            features.AddFeature({
                RandomBin(binSize, rng),
                labelBin,
                RandomBin(binSize, rng),
            });

            const auto featureIds = FeatureSelection(label, features, 6, 4);
            UNIT_ASSERT(!featureIds.empty());
            UNIT_ASSERT_VALUES_EQUAL(featureIds.front(), features.GetFeatureCount() - 1);
        }

        SIMPLE_UNIT_TEST(FastIsOK) {
            TReallyFastRng32 rng(20160203);
            const int binSize = 250;

            const TBinFeatureSet label({RandomBin(binSize, rng), RandomBin(binSize, rng),
                                        RandomBin(binSize, rng), RandomBin(binSize, rng)});

            TBinFeatureSet features;
            for (int i = 0; i < 10; ++i) {
                yvector<TBin> bins;
                for (int j = rng.Uniform(1, 10); j > 0; --j) {
                    bins.push_back(RandomBin(binSize, rng));
                }
                features.AddFeature(std::move(bins));
            }

            yvector<int> fastResult;
            FastFeatureSelection(label, features, 6, 8, features.GetFeatureCount(), [&](int feature) {
                fastResult.push_back(feature);
            });

            UNIT_ASSERT_VALUES_EQUAL(fastResult, FeatureSelection(label, features, 6, 8));
        }

        SIMPLE_UNIT_TEST(FeatureCount) {
            TReallyFastRng32 rng(20170203);
            const int binSize = 250;

            const TBinFeatureSet label({RandomBin(binSize, rng), RandomBin(binSize, rng),
                                        RandomBin(binSize, rng), RandomBin(binSize, rng)});

            TBinFeatureSet features;
            for (int i = 0; i < 10; ++i) {
                yvector<TBin> bins;
                for (int j = rng.Uniform(1, 10); j > 0; --j) {
                    bins.push_back(RandomBin(binSize, rng));
                }
                features.AddFeature(std::move(bins));
            }

            for (int featureCount : {1, 3, features.GetFeatureCount()}) {
                yvector<int> fastResult;
                FastFeatureSelection(
                    label, features, 6, 8, featureCount, [&](int feature) {
                        fastResult.push_back(feature);
                    });
                UNIT_ASSERT_VALUES_EQUAL(fastResult.size(), featureCount);
            }
        }
    }
}
