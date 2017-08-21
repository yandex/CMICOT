#include "caching_bin_scorer.h"
#include "test_pool_gen.h"
#include "entropy.h"

#include <library/unittest/registar.h>
#include <library/threading/future/async.h>

#include <util/random/fast.h>
#include <util/thread/queue.h>

namespace NCmicot {
    SIMPLE_UNIT_TEST_SUITE(CachingBinScorer) {
        SIMPLE_UNIT_TEST(ResultDoesntDifferFromEvalScorer) {
            TReallyFastRng32 rng(20160203);
            const int binSize = 1500;
            const int stepCount = 6;

            const TBinFeatureSet label = MakeRandomLabel(rng, binSize, {5, 10});
            TBinFeatureSet features = MakeRandomFeatures(rng, 30, binSize, {1, 4});
            int f = features.AddFeature({stepCount, RandomBin(binSize, rng)});

            TBackground bg(features);
            bg.DisableAll();
            bg.SetFeatureEnabled(f, true);
            TCachingBinScorer scorer(label, features.GetBinCount());

            auto etalonBinScorer = BuildBinScorerForEval(label, stepCount - 1, stepCount, 8);
            TMtpQueue queue;
            queue.Start(1);

            for (int i = 0; i < 4; ++i) {
                for (int binId : bg.DisabledBinIndexes()) {
                    auto future = NThreading::Async([&] { return scorer.Evaluate(bg, binId, stepCount); },
                                                    queue);

                    auto expectedBinScore = etalonBinScorer->Eval(bg, binId);
                    expectedBinScore.Score /= Entropy(label.AllBins());

                    UNIT_ASSERT_VALUES_EQUAL(future.GetValue(TDuration::Minutes(1)), expectedBinScore);
                }

                int featureToEnable = 0;
                while (bg.IsFeatureEnabled(featureToEnable)) {
                    featureToEnable = rng.Uniform(features.GetFeatureCount());
                }
                bg.SetFeatureEnabled(featureToEnable, true);
            }
        }
    }
}
