#include "feature_score.h"
#include "test_pool_gen.h"

#include <library/unittest/registar.h>

#include <util/random/fast.h>

namespace NCmicot {
    SIMPLE_UNIT_TEST_SUITE(FeatureScoring) {
        const int FEATURE_COUNT = 18;
        const int BIN_SIZE = 1000;

        double TestNormalizer(const TBinScore& bs, const TBackground&, int) {
            return bs.Score;
        }

        TBinScore TestBinScorer(TBackground /*bg*/, int /*binIndex*/, int stepCount) {
            return {
                0.0,
                yvector<TStepResult>(stepCount),
                yvector<TStepResult>(stepCount),
            };
        }

        SIMPLE_UNIT_TEST(FeatureScoreIsWellFormed) {
            TReallyFastRng32 rng(20160328);

            TBinFeatureSet label(RandomFeature(10, BIN_SIZE, rng));
            TBinFeatureSet features;
            for (int i : xrange(FEATURE_COUNT)) {
                Y_UNUSED(i);
                features.AddFeature(RandomFeature(rng.Uniform(3, 8), BIN_SIZE, rng));
            }

            const int stepCount = 5;
            auto binScorer = FuncToBinScorer([stepCount](TBackground bg, int binIndex) {
                return TestBinScorer(bg, binIndex, stepCount);
            });

            for (int i : xrange(3)) {
                Y_UNUSED(i);

                const int featureIndexToEval = rng.Uniform(FEATURE_COUNT);

                const TFeatureScore fs = GetFeatureScore(TBackground(features), featureIndexToEval,
                                                         *binScorer, TestNormalizer);
                UNIT_ASSERT_VALUES_EQUAL(fs.BinScore.size(),
                                         features.GetFeature(featureIndexToEval).size());

                for (int i : xrange(fs.BinScore.size())) {
                    UNIT_ASSERT_VALUES_EQUAL(fs.BinScore[i].MaximizingSteps.size(), stepCount);
                    UNIT_ASSERT_VALUES_EQUAL(fs.BinScore[i].MinimizingSteps.size(), stepCount);
                }
            }
        }
    }
}
