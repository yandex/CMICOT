#include "selection.h"
#include "algorithm.h"
#include "mutual_information_calculator.h"
#include "bin_score.h"
#include "caching_bin_scorer.h"

namespace NCmicot {
    namespace {
        int GetBinWithMaximalMutualInformationWithLabel(const TBinFeatureSet& label,
                                                        const TBinFeatureSet& features, int threadCount) {
            TMutualInformationCalculator miCalc(label.AllBins().front().size());
            for (const auto& bin : label.AllBins()) {
                miCalc.AddFirstVariableBin(bin);
            }

            auto kernel = [&](int binIndex) {
                return miCalc.GetValueWithSecondVariableBin(features.GetBin(binIndex));
            };
            return *ParallelMaxElementBy(features.AllBinIndexes(), kernel, threadCount);
        }
    }

    void FastFeatureSelection(
        const TBinFeatureSet& label,
        const TBinFeatureSet& features,
        int evalStepCount,
        int threadCount,
        int featureCount,
        std::function<void(int)> onFeatureSelected)
    {
        TBackground bg(features);

        bg.DisableAll();
        {
            int bestFeature = features.GetFeatureIndexByBinIndex(
                GetBinWithMaximalMutualInformationWithLabel(label, features, threadCount));

            bg.SetFeatureEnabled(bestFeature, true);
            onFeatureSelected(bestFeature);
        }

        TCachingBinScorer binScorer(label, features.GetBinCount());

        auto kernel = [&](int binIndex) {
            int stepCount = Min(bg.EnabledBinIndexes().ysize(), evalStepCount);
            return binScorer.Evaluate(bg, binIndex, stepCount).Score;
        };
        int featuresToSelectCount = Min(featureCount, features.GetFeatureCount()) - 1;
        for (int step = 0; step < featuresToSelectCount; ++step) {
            const auto disabledBins = bg.DisabledBinIndexes();
            const auto bestBinIter = ParallelMaxElementBy(disabledBins, kernel, threadCount);
            Y_VERIFY(bestBinIter != disabledBins.end(), "");

            int bestFeature = features.GetFeatureIndexByBinIndex(*bestBinIter);
            bg.SetFeatureEnabled(bestFeature, true);

            onFeatureSelected(bestFeature);
        }
    }

    void FeatureSelection(const TBinFeatureSet& label, const TBinFeatureSet& features,
                          int evalStepCount,
                          int threadCount, std::function<void(int)> onFeatureSelected) {
        TBackground bg(features);

        bg.DisableAll();
        {
            int bestFeature = features.GetFeatureIndexByBinIndex(
                GetBinWithMaximalMutualInformationWithLabel(label, features, threadCount));

            bg.SetFeatureEnabled(bestFeature, true);
            onFeatureSelected(bestFeature);
        }

        auto kernel = [&](int binIndex) {
            int stepCount = Min(bg.EnabledBinIndexes().ysize(), evalStepCount);
            return BuildBinScorerForEval(label, stepCount - 1, stepCount, threadCount)->Eval(bg, binIndex).Score;
        };
        for (int step = 0; step < features.GetFeatureCount() - 1; ++step) {
            int bestBin = *MaxElementBy(bg.DisabledBinIndexes(), kernel);

            int bestFeature = features.GetFeatureIndexByBinIndex(bestBin);
            bg.SetFeatureEnabled(bestFeature, true);

            onFeatureSelected(bestFeature);
        }
    }

    yvector<int> FeatureSelection(const TBinFeatureSet& label, const TBinFeatureSet& features,
                                  int evalStepCount, int threadCount) {
        yvector<int> result;
        result.reserve(features.GetFeatureCount());

        FeatureSelection(label, features, evalStepCount, threadCount, [&result](int f) {
            result.push_back(f);
        });

        return result;
    }
}
