#pragma once

#include "bin_feature_set.h"

namespace NCmicot {
    struct TStepResult {
        int BinIndex;
        double Cmi;
    };

    bool operator==(const TStepResult& lhs, const TStepResult& rhs);

    struct TBinScore {
        double Score;
        yvector<TStepResult> MaximizingSteps;
        yvector<TStepResult> MinimizingSteps;

        yvector<int> GetMaximizingBinIndexes() const {
            yvector<int> result;
            for (const auto& x : MaximizingSteps) {
                result.push_back(x.BinIndex);
            }
            return result;
        }

        yvector<int> GetMinimizingBinIndexes() const {
            yvector<int> result;
            for (const auto& x : MinimizingSteps) {
                result.push_back(x.BinIndex);
            }
            return result;
        }
    };

    bool operator==(const TBinScore& lhs, const TBinScore& rhs);

    class IBinScorer {
    public:
        virtual ~IBinScorer() {
        }

        virtual TBinScore Eval(TBackground bg, int binIndex) const = 0;
    };

    template <class Func>
    class TBinScoringFunc: public IBinScorer {
    public:
        TBinScoringFunc(Func&& func)
            : Func_(std::forward<Func>(func))
        {
        }

        TBinScore Eval(TBackground bg, int binIndex) const override {
            return Func_(std::move(bg), binIndex);
        }

    private:
        Func Func_;
    };

    template <class Func>
    THolder<TBinScoringFunc<Func>> FuncToBinScorer(Func&& func) {
        return new TBinScoringFunc<Func>(std::forward<Func>(func));
    }

    THolder<IBinScorer> BuildBinScorerForEval(const TBinFeatureSet& label, int maximizationSteps,
                                              int minimizationSteps, int maxParallel);

    THolder<IBinScorer> BuildEfamBinScorer(const TBinFeatureSet& label, int stepCount, int maxParallel);
    THolder<IBinScorer> BuildBtmBinScorer(const TBinFeatureSet& label, int stepCount, int maxParallel);
};
