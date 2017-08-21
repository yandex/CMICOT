#include "miximizers.h"
#include "algorithm.h"

namespace NCmicot {
    TParallelMiximizer::TParallelMiximizer(const TBinFeatureSet& label, int maximizeSteps,
                                           int minimizeSteps, int maxParallel)
        : Cmi(label.GetBin(0).size())
        , MaxSteps(maximizeSteps)
        , MinSteps(minimizeSteps)
        , MaxParallel(maxParallel)
    {
        for (const TBin& bin : label.AllBins()) {
            Cmi.AddFirstVariableBin(bin);
        }
    }

    yvector<TStepResult> TParallelMiximizer::DoMaximizePhase(TBackground& bg, int evalBinIndex) {
        Y_ENSURE(bg.EnabledBins().ysize() >= MaxSteps,
                 "Not enough enabled bins, must be at least " << MaxSteps);

        auto localBg = bg;

        yvector<TStepResult> result;
        result.reserve(MaxSteps);

        TCmiCalculator maximizerCmi = Cmi;
        maximizerCmi.AddSecondVariableBin(localBg.GetBin(evalBinIndex));

        for (auto step : xrange(MaxSteps)) {
            Y_UNUSED(step);

            auto binValue = [&](int binIndex) {
                return maximizerCmi.GetValueWithConditionBin(localBg.GetBin(binIndex));
            };

            int bestBin = *ParallelMaxElementBy(localBg.EnabledBinIndexes(), binValue, MaxParallel);

            localBg.SetBinEnabled(bestBin, false);
            result.push_back({bestBin, binValue(bestBin)});
            maximizerCmi.AddConditionBin(localBg.GetBin(bestBin));
        }

        return result;
    }

    yvector<TStepResult> TParallelMiximizer::DoMinimizePhase(TBackground& bg, int evalBinIndex,
                                                             const yvector<TStepResult>& maxSteps) {
        Y_ENSURE(bg.EnabledBins().ysize() >= MinSteps,
                 "Not enough enabled bins, must be at least " << MinSteps);

        yvector<TStepResult> result;
        result.reserve(MinSteps);

        TCmiCalculator minimizerCmi = Cmi;
        minimizerCmi.AddSecondVariableBin(bg.GetBin(evalBinIndex));

        for (auto step : xrange(MinSteps)) {
            auto binValue = [&](int binIndex) {
                return minimizerCmi.GetValueWithConditionBin(bg.GetBin(binIndex));
            };

            int bestBin = *ParallelMinElementBy(bg.EnabledBinIndexes(), binValue, MaxParallel);

            result.push_back({bestBin, binValue(bestBin)});
            bg.SetBinEnabled(bestBin, false);

            minimizerCmi.AddConditionBin(bg.GetBin(bestBin));
            if (step < maxSteps.ysize()) {
                minimizerCmi.AddSecondVariableBin(bg.GetBin(maxSteps[step].BinIndex));
            }
        }

        return result;
    }
}
