#include "miximizers.h"
#include "test_pool_gen.h"

#include <library/unittest/registar.h>

#include <util/random/fast.h>
#include <util/generic/algorithm.h>

namespace NCmicot {
    SIMPLE_UNIT_TEST_SUITE(ParallelMiximizer) {
        SIMPLE_UNIT_TEST(Maximization) {
            TReallyFastRng32 rng(20160404);

            const int maxSteps = 5;
            const TBinFeatureSet label = MakeRandomFeatures(rng, 1, 100, {5, 10});
            const TBinFeatureSet features = MakeRandomFeatures(rng, 30, 100, {5, 10});
            TParallelMiximizer m(label, maxSteps, maxSteps, 2);

            TBackground bg(features);
            yvector<TStepResult> result = m.DoMaximizePhase(bg, rng.Uniform(features.GetBinCount()));

            UNIT_ASSERT_VALUES_EQUAL(result.size(), maxSteps);
            UNIT_ASSERT(AllOf(result, [&](TStepResult sr) {
                return 0 <= sr.BinIndex && sr.BinIndex < features.GetBinCount();
            }));
        }

        SIMPLE_UNIT_TEST(Minimization) {
            TReallyFastRng32 rng(20160404);

            const int minSteps = 5;
            const TBinFeatureSet label = MakeRandomFeatures(rng, 1, 100, {5, 10});
            const TBinFeatureSet features = MakeRandomFeatures(rng, 30, 100, {5, 10});
            const int evalBinIndex = rng.Uniform(features.GetBinCount());
            TParallelMiximizer m(label, minSteps, minSteps, 2);

            TBackground bg(features);
            const auto maxSteps = m.DoMaximizePhase(bg, evalBinIndex);
            yvector<TStepResult> result = m.DoMinimizePhase(bg, evalBinIndex, maxSteps);

            UNIT_ASSERT_VALUES_EQUAL(result.size(), minSteps);
            UNIT_ASSERT(AllOf(result, [&](TStepResult sr) {
                return 0 <= sr.BinIndex && sr.BinIndex < features.GetBinCount();
            }));

            yhash_set<int> binIndexesInResult;
            for (const auto& sr : result) {
                binIndexesInResult.insert(sr.BinIndex);
            }
            UNIT_ASSERT_VALUES_EQUAL(binIndexesInResult.size(), result.size());
        }
    }
}
