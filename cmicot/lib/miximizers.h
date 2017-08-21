#pragma once

#include "bin_score.h"
#include "cmi_calculator.h"

namespace NCmicot {
    /// Does maximization and minimization steps. "Miximizer" is a weird union of words "minimizer"
    /// and "maximizer".
    struct IMiximizer {
        virtual ~IMiximizer() {
        }

        virtual yvector<TStepResult> DoMaximizePhase(TBackground& bg, int evalBinIndex) = 0;
        virtual yvector<TStepResult> DoMinimizePhase(TBackground& bg, int evalBinIndex,
                                                     const yvector<TStepResult>& maxSteps) = 0;
    };

    class TParallelMiximizer: public IMiximizer {
    public:
        TParallelMiximizer(const TBinFeatureSet& label, int maximizeSteps, int minimizeSteps,
                           int maxParallel);

        yvector<TStepResult> DoMaximizePhase(TBackground& bg, int evalBinIndex) override;
        yvector<TStepResult> DoMinimizePhase(TBackground& bg, int evalBinIndex,
                                             const yvector<TStepResult>& maxSteps) override;

    private:
        TCmiCalculator Cmi;
        int MaxSteps, MinSteps;
        int MaxParallel;
    };
}
