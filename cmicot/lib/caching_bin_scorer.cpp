#include "caching_bin_scorer.h"
#include "cmi_calculator.h"
#include "entropy.h"

#include <util/generic/algorithm.h>
#include <util/generic/xrange.h>

namespace NCmicot {
    TCachingBinScorer::TCachingBinScorer(const TBinFeatureSet& label, int binCount)
        : Label(label)
        , Cache(binCount)
        , LabelEntropy(Entropy(Label.AllBins()))
    {
    }

    TBinScore TCachingBinScorer::Evaluate(TBackground background, int evalBinIndex, int stepCount) {
        //    Cerr << __func__ << " " << evalBinIndex << Endl;
        auto binsToProcess = background.LastEnabled();

        background.SetBinEnabled(evalBinIndex, true);

        if (background.EnabledBins().ysize() < stepCount + 1) {
            ythrow yexception() << "Not enough enabled bins, must be at least " << stepCount + 1;
        }
        if (evalBinIndex < 0 || evalBinIndex >= background.GetBinCount()) {
            ythrow yexception() << "Eval index " << evalBinIndex << " is out of range [0; "
                                << background.GetBinCount() - 1 << "]. There are only "
                                << background.GetBinCount() << " bins";
        }

        auto& maxStepsCached = Cache[evalBinIndex].MaximizingSteps;
        auto& minStepsCached = Cache[evalBinIndex].MinimizingSteps;
        maxStepsCached.resize(stepCount - 1, {-1, -1.0});
        minStepsCached.resize(stepCount, {-1, 1000000.0});
        {
            TCmiCalculator maximizerCmi(Label.AllBins().front().size());
            for (const auto& bin : Label.AllBins()) {
                maximizerCmi.AddFirstVariableBin(bin);
            }
            maximizerCmi.AddSecondVariableBin(background.GetBin(evalBinIndex));

            auto binValue = [&](int binIndex) {
                return maximizerCmi.GetValueWithConditionBin(background.GetBin(binIndex));
            };

            for (auto step : xrange(stepCount - 1)) {
                auto enabledBins = binsToProcess.EnabledBinIndexes();
                /* Следующий цикл - это грязный хак. Нам надо, чтобы один и тот же
               бин не мог дважды использоваться как помогающий. Поэтому мы явно на каждом шаге
               удаляем текущих помощников из enabledBins. Мы это делаем вместо изменения
               binsToProcess, потому что binsToProcess может поменяться в if'е в строке 71
            */
                for (int i : xrange(step)) {
                    enabledBins.erase(
                        std::remove(enabledBins.begin(), enabledBins.end(), maxStepsCached[i].BinIndex),
                        enabledBins.end());
                }

                //            Cerr << "max: ";
                //            for (int x : enabledBins) {
                //                Cerr << x << " ";
                //            }
                //            Cerr << Endl;

                auto bestBinIter = MaxElementBy(enabledBins, binValue);
                //            auto bestBinIter = ParallelMaxElementBy(enabledBins, binValue, ThreadCount);
                Y_VERIFY(bestBinIter != enabledBins.end(), "");

                const double binCmi = binValue(*bestBinIter);

                if (binCmi > maxStepsCached[step].Cmi) {
                    maxStepsCached[step] = {*bestBinIter, binCmi};
                    binsToProcess = background;
                    Fill(maxStepsCached.begin() + step + 1, maxStepsCached.end(),
                         TStepResult{-1, -1.0});
                    Fill(minStepsCached.begin() + step + 1, minStepsCached.end(),
                         TStepResult{-1, 100000.0});
                }
                maximizerCmi.AddConditionBin(background.GetBin(maxStepsCached[step].BinIndex));
            }
        }
        //    Cerr << "Maximizers:" << Endl;
        //    for (const auto& sr : maxStepsCached) {
        //        Cerr << sr.BinIndex << '\t' << sr.Cmi << Endl;
        //    }

        binsToProcess.SetBinEnabled(evalBinIndex, false);
        background.SetBinEnabled(evalBinIndex, false);

        TCmiCalculator minimizerCmi(Label.AllBins().front().size());
        for (const auto& bin : Label.AllBins()) {
            minimizerCmi.AddFirstVariableBin(bin);
        }
        minimizerCmi.AddSecondVariableBin(background.GetBin(evalBinIndex));

        auto binValue = [&](int binIndex) {
            return minimizerCmi.GetValueWithConditionBin(background.GetBin(binIndex));
        };

        for (auto step : xrange(stepCount)) {
            const auto enabledBins = binsToProcess.EnabledBinIndexes();

            //        Cerr << "min: ";
            //        for (int x : enabledBins) {
            //            Cerr << x << " ";
            //        }
            //        Cerr << Endl;
            auto bestBinIter = MinElementBy(enabledBins, binValue);
            //        auto bestBinIter = ParallelMinElementBy(enabledBins, binValue, ThreadCount);
            Y_VERIFY(bestBinIter != enabledBins.end(), "");

            const double binCmi = binValue(*bestBinIter);

            if (binCmi < minStepsCached[step].Cmi) {
                minStepsCached[step] = {*bestBinIter, binCmi};
                Fill(minStepsCached.begin() + step + 1, minStepsCached.end(),
                     TStepResult{-1, 100000.0});
                binsToProcess = background;
            }

            binsToProcess.SetBinEnabled(minStepsCached[step].BinIndex, false);
            background.SetBinEnabled(minStepsCached[step].BinIndex, false);

            minimizerCmi.AddConditionBin(background.GetBin(minStepsCached[step].BinIndex));
            if (step < maxStepsCached.ysize()) {
                minimizerCmi.AddSecondVariableBin(background.GetBin(maxStepsCached[step].BinIndex));
            }
        }

        Cache[evalBinIndex].Score = minimizerCmi.GetValue() / LabelEntropy;
        //    Cerr << "Minimizers:" << Endl;
        //    for (const auto& sr : minStepsCached) {
        //        Cerr << sr.BinIndex << '\t' << sr.Cmi << Endl;
        //    }

        //    Cerr << Endl;
        return Cache[evalBinIndex];
    }
}
