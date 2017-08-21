#include "feature_score.h"
#include "cmi_calculator.h"
#include "algorithm.h"

#include <util/generic/algorithm.h>

namespace NCmicot {
    TCmimScore GetCmimScore(const TBinFeatureSet& label, TBackground background, int featureIndex,
                            int threadCount) {
        TCmiCalculator cmiCalc(label.GetBin(0).size());
        for (const TBin& labelBin : label.AllBins()) {
            cmiCalc.AddFirstVariableBin(labelBin);
        }

        for (int binIndex : background.GetFeatureBinIndexes(featureIndex)) {
            cmiCalc.AddSecondVariableBin(background.GetBin(binIndex));
        }

        auto kernel = [&cmiCalc, &background](int index) {
            TCmiCalculator localCmi = cmiCalc;
            for (int binIndex : background.GetFeatureBinIndexes(index)) {
                localCmi.AddConditionBin(background.GetBin(binIndex));
            }

            return localCmi.GetValue();
        };

        background.SetFeatureEnabled(featureIndex, false);

        const auto searchRange = background.EnabledFeatureIndexes();
        auto bestFeatureIter = ParallelMinElementBy(searchRange, kernel, threadCount);

        if (bestFeatureIter != searchRange.end()) {
            int bestFeatureIndex = bestFeatureIter - searchRange.begin();
            return {kernel(bestFeatureIndex), bestFeatureIndex};
        } else {
            return {};
        }
    }

    TFeatureScore GetFeatureScore(TBackground background, int featureIndex, const IBinScorer& binScorer,
                                  TBinScoreNormalier binScoreNormalier) {
        const auto binIndexRange = background.GetFeatureBinIndexes(featureIndex);

        yvector<TBinScore> scores;
        scores.reserve(binIndexRange.size());

        for (int binIndex : binIndexRange) {
            TBinScore currentBinScore = binScorer.Eval(background, binIndex);
            currentBinScore.Score = binScoreNormalier(currentBinScore, background, binIndex);
            scores.push_back(std::move(currentBinScore));
        }

        const double featureScore = MaxElementBy(scores, [](const TBinScore& s) { return s.Score; })->Score;

        return {
            featureScore,
            std::move(scores),
        };
    }
}
