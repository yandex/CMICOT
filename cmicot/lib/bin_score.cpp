#include "bin_score.h"
#include "algorithm.h"
#include "cmi_calculator.h"
#include "miximizers.h"

#include <util/generic/ymath.h>

namespace NCmicot {
    namespace {
        TBinScore GetScoreWithEvalFeatureAwareMaximization(TBackground bg, int binIndex, IMiximizer& m) {
            Y_ENSURE(0 <= binIndex && binIndex < bg.GetBinCount(),
                     "Eval index " << binIndex << " is out of range [0; " << bg.GetBinCount() - 1
                                   << "]. There are only " << bg.GetBinCount() << " bins");

            bg.SetFeatureEnabled(bg.GetFeatureIndexByBinIndex(binIndex), true);
            yvector<TStepResult> maxSteps = m.DoMaximizePhase(bg, binIndex);

            bg.SetFeatureEnabled(bg.GetFeatureIndexByBinIndex(binIndex), false);
            yvector<TStepResult> minSteps = m.DoMinimizePhase(bg, binIndex, maxSteps);

            const double binScore = minSteps.back().Cmi;
            return {binScore, std::move(maxSteps), std::move(minSteps)};
        }

        TBinScore GetScoreWithBtm(TBackground bg, int binIndex, IMiximizer& m) {
            auto maximizationBg = bg;
            maximizationBg.EnabledAll();
            yvector<TStepResult> maxSteps = m.DoMaximizePhase(maximizationBg, binIndex);

            bg.SetBinEnabled(binIndex, false);
            yvector<TStepResult> minSteps = m.DoMinimizePhase(bg, binIndex, maxSteps);

            const double binScore = minSteps.back().Cmi;
            return {binScore, std::move(maxSteps), std::move(minSteps)};
        }

        TBinScore GetBinScore(TBackground background, int evalBinIndex, IMiximizer& m) {
            Y_ENSURE(0 <= evalBinIndex && evalBinIndex < background.GetBinCount(),
                     "Eval index " << evalBinIndex << " is out of range [0; " << background.GetBinCount() - 1
                                   << "]. There are only " << background.GetBinCount() << " bins");

            background.SetBinEnabled(evalBinIndex, true);
            yvector<TStepResult> maxSteps = m.DoMaximizePhase(background, evalBinIndex);

            background.SetFeatureEnabled(background.GetFeatureIndexByBinIndex(evalBinIndex), false);
            yvector<TStepResult> minSteps = m.DoMinimizePhase(background, evalBinIndex, maxSteps);

            const double binScore = minSteps.back().Cmi;
            return {binScore, std::move(maxSteps), std::move(minSteps)};
        }
    }

    bool operator==(const TStepResult& lhs, const TStepResult& rhs) {
        return Abs(lhs.Cmi - rhs.Cmi) < 1e-8 && lhs.BinIndex == rhs.BinIndex;
    }

    bool operator==(const TBinScore& lhs, const TBinScore& rhs) {
        return Abs(lhs.Score - rhs.Score) < 1e-8 && lhs.MaximizingSteps == rhs.MaximizingSteps && lhs.MinimizingSteps == rhs.MinimizingSteps;
    }

    THolder<IBinScorer> BuildBinScorerForEval(const TBinFeatureSet& label, int maximizationSteps,
                                              int minimizationSteps, int maxParallel) {
        auto miximizer = MakeAtomicShared<TParallelMiximizer>(label, maximizationSteps,
                                                              minimizationSteps, maxParallel);
        return FuncToBinScorer([miximizer](TBackground bg, int binIndex) {
            return GetBinScore(std::move(bg), binIndex, *miximizer);
        });
    }

    THolder<IBinScorer> BuildEfamBinScorer(const TBinFeatureSet& label, int stepCount, int maxParallel) {
        auto miximizer = MakeAtomicShared<TParallelMiximizer>(label, stepCount - 1, stepCount,
                                                              maxParallel);
        return FuncToBinScorer([miximizer](TBackground bg, int binIndex) {
            return GetScoreWithEvalFeatureAwareMaximization(std::move(bg), binIndex, *miximizer);
        });
    }

    THolder<IBinScorer> BuildBtmBinScorer(const TBinFeatureSet& label, int stepCount, int maxParallel) {
        auto miximizer = MakeAtomicShared<TParallelMiximizer>(label, stepCount - 1, stepCount,
                                                              maxParallel);
        return FuncToBinScorer([miximizer](TBackground bg, int binIndex) {
            return GetScoreWithBtm(std::move(bg), binIndex, *miximizer);
        });
    }
}
