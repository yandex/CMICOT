#include "bin_score.h"
#include "test_pool_gen.h"
#include "entropy.h"

#include <library/unittest/registar.h>

#include <util/generic/algorithm.h>

namespace NCmicot {
    /// Test for the bin scorer returned by BuildBinScorerForEval
    SIMPLE_UNIT_TEST_SUITE(BinScorerForEval) {
        const int FEATURE_COUNT = 18;
        const int STEP_COUNT = 6;
        const int BIN_SIZE = 1000;
        const int RUN_TEST_COUNT = 20;
        const int THREAD_COUNT = 4;

        template <class Rng>
        std::pair<TBinFeatureSet, TBinFeatureSet> BuildTestSet(int featureCount, int binSize, Rng& rng) {
            yvector<TBin> bins(featureCount);
            for (auto& bin : bins) {
                bin = RandomBin(binSize, rng);
            }
            TBin labelBin(binSize);
            for (auto i : xrange(binSize)) {
                for (const auto& bin : bins) {
                    labelBin[i] = (labelBin[i] != bin[i]);
                }
            }

            const TBinFeatureSet label({1, labelBin});
            TBinFeatureSet features;
            for (const auto& bin : bins) {
                features.AddFeature({1, bin});
            }

            return {label, features};
        }

        THolder<IBinScorer> BuildBinScorer(const TBinFeatureSet& label) {
            return BuildBinScorerForEval(label, STEP_COUNT - 1, STEP_COUNT, THREAD_COUNT);
        }

        template <class Rng>
        void TestRandom(Rng & rng) {
            TBinFeatureSet label, features;
            std::tie(label, features) = BuildTestSet(FEATURE_COUNT, BIN_SIZE, rng);

            int evalIndex = features.AddFeature({1, RandomBin(BIN_SIZE, rng)});

            auto result = BuildBinScorer(label)->Eval(TBackground(features), evalIndex);

            UNIT_ASSERT_C(result.Score < 0.2, result);
        }

        SIMPLE_UNIT_TEST(Random) {
            TReallyFastRng32 rng(20151117);
            for (int i : xrange(RUN_TEST_COUNT)) {
                Y_UNUSED(i);
                TestRandom(rng);
            }
        }

        template <class Rng>
        void TestLabel(Rng & rng) {
            TBinFeatureSet label, features;
            std::tie(label, features) = BuildTestSet(FEATURE_COUNT, BIN_SIZE, rng);

            int evalIndex = features.AddFeature(label.GetFeature(0));

            auto result = BuildBinScorer(label)->Eval(TBackground(features), evalIndex);
            UNIT_ASSERT_C(result.Score > 0.8, result);
        }

        SIMPLE_UNIT_TEST(Label) {
            TReallyFastRng32 rng(20151118);

            for (int i : xrange(RUN_TEST_COUNT)) {
                Y_UNUSED(i);
                TestLabel(rng);
            }
        }

        template <class Rng>
        void TestFeatureCopy(Rng & rng) {
            TBinFeatureSet label, features;
            std::tie(label, features) = BuildTestSet(FEATURE_COUNT, BIN_SIZE, rng);

            const int copiedFeature = 0;
            int evalIndex = features.AddFeature(features.GetFeature(copiedFeature));

            auto result = BuildBinScorer(label)->Eval(TBackground(features), evalIndex);
            UNIT_ASSERT_C(result.Score < 1e-5, result);
            UNIT_ASSERT_VALUES_EQUAL_C(result.MinimizingSteps.front().BinIndex, copiedFeature, result);
        }

        SIMPLE_UNIT_TEST(FeatureCopy) {
            TReallyFastRng32 rng(20151119);

            for (auto i : xrange(RUN_TEST_COUNT)) {
                Y_UNUSED(i);
                TestFeatureCopy(rng);
            }
        }

        template <class Rng>
        void TestBurningFeatureMoreThanRandom(Rng & rng) {
            const auto labelBin = RandomBin(BIN_SIZE, rng);

            auto burningFeatureBin = labelBin;
            for (auto& x : burningFeatureBin) {
                if (x && rng.Uniform(100) < 30) {
                    x = false;
                }
            }

            TBinFeatureSet label({1, labelBin});
            TBinFeatureSet features = MakeRandomBinaryFeatures(rng, 2 * STEP_COUNT, BIN_SIZE);

            int randomIndex = features.AddFeature({1, RandomBin(BIN_SIZE, rng)});
            int burningIndex = features.AddFeature({1, burningFeatureBin});

            auto binScorer = BuildBinScorer(label);
            auto randomResult = binScorer->Eval(TBackground(features), randomIndex);
            auto featureResult = binScorer->Eval(TBackground(features), burningIndex);

            UNIT_ASSERT_C(featureResult.Score > randomResult.Score, '\n' << randomResult << '\n'
                                                                         << featureResult);
        }

        SIMPLE_UNIT_TEST(BurningFeatureMoreThanRandom) {
            TReallyFastRng32 rng(20151120);

            for (auto i : xrange(RUN_TEST_COUNT)) {
                Y_UNUSED(i);
                TestBurningFeatureMoreThanRandom(rng);
            }
        }

        template <class Rng>
        void TestRandomizedLabel(Rng & rng) {
            TBin labelBin = RandomBin(BIN_SIZE, rng);
            TBin randomBin = RandomBin(BIN_SIZE, rng);

            TBin randomizedLabel(BIN_SIZE);
            for (auto i : xrange(BIN_SIZE)) {
                randomizedLabel[i] = (labelBin[i] != randomBin[i]);
            }

            TBinFeatureSet label({1, labelBin});
            TBinFeatureSet features = MakeRandomBinaryFeatures(rng, STEP_COUNT * 2, BIN_SIZE);

            features.AddFeature({1, randomizedLabel});
            int evalIndex = features.AddFeature({1, randomBin});

            auto result = BuildBinScorer(label)->Eval(TBackground(features), evalIndex);
            UNIT_ASSERT_C(result.Score > 0.8, result);
        }

        SIMPLE_UNIT_TEST(RandomizedLabel) {
            TReallyFastRng32 rng(20151121);

            for (auto i : xrange(RUN_TEST_COUNT)) {
                Y_UNUSED(i);
                TestRandomizedLabel(rng);
            }
        }

        SIMPLE_UNIT_TEST(InvalidEvalIndex) {
            TReallyFastRng32 rng(20151122);
            TBinFeatureSet label, features;
            std::tie(label, features) = BuildTestSet(FEATURE_COUNT, BIN_SIZE, rng);
            TBackground bg(features);

            UNIT_ASSERT_EXCEPTION(BuildBinScorer(label)->Eval(bg, features.GetFeatureCount()), yexception);
            UNIT_ASSERT_EXCEPTION(BuildBinScorer(label)->Eval(bg, features.GetFeatureCount() + 1), yexception);
            UNIT_ASSERT_EXCEPTION(BuildBinScorer(label)->Eval(bg, -1), yexception);
        }

        template <class Rng>
        void TestCmiDoesntGrowDuringMinimization(Rng & rng) {
            TBinFeatureSet label({RandomBin(BIN_SIZE, rng)});
            TBinFeatureSet features = label;
            for (auto i : xrange(FEATURE_COUNT)) {
                Y_UNUSED(i);
                features.AddFeature({RandomBin(BIN_SIZE, rng)});
            }

            auto result = BuildBinScorer(label)->Eval(TBackground(features), rng.Uniform(features.GetFeatureCount()));
            for (int i : xrange(result.MinimizingSteps.ysize() - 1)) {
                auto cur = result.MinimizingSteps[i].Cmi;
                auto next = result.MinimizingSteps[i + 1].Cmi;
                UNIT_ASSERT_C(cur > next - 1e-6, result);
            }
        }

        SIMPLE_UNIT_TEST(CmiDoesntGrowDuringMinimization) {
            TReallyFastRng32 rng(20151130);

            for (auto i : xrange(RUN_TEST_COUNT)) {
                Y_UNUSED(i);
                TestCmiDoesntGrowDuringMinimization(rng);
            }
        }

        SIMPLE_UNIT_TEST(ResultFormat) {
            TReallyFastRng32 rng(20160404);
            const int binSize = 1000;
            const std::pair<int, int> binCountRange(1, 4);

            const TBinFeatureSet label = MakeRandomLabel(rng, binSize, binCountRange);
            const TBinFeatureSet features = MakeRandomFeatures(rng, 30, binSize, binCountRange);
            const TBackground bg(features);

            const std::pair<int, int> stepPairs[] = {{5, 3}, {5, 8}, {7, 7}};
            for (auto maxMinSteps : stepPairs) {
                const int maxSteps = maxMinSteps.first;
                const int minSteps = maxMinSteps.second;

                auto binScorer = BuildBinScorerForEval(label, maxSteps, minSteps, 8);
                TBinScore result = binScorer->Eval(bg, rng.Uniform(features.GetBinCount()));

                UNIT_ASSERT_VALUES_EQUAL(result.MaximizingSteps.size(), maxSteps);
                UNIT_ASSERT_VALUES_EQUAL(result.MinimizingSteps.size(), minSteps);
            }
        }
    }
}
