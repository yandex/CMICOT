#include "bin_score_normalize.h"
#include "bin_score.h"
#include "entropy.h"

namespace NCmicot {
    TBinScoreNormalier BuildBinScoreNormalizer(EBinScoreNormalization normalizationType,
                                               TBinFeatureSet& label) {
        switch (normalizationType) {
            case EBinScoreNormalization::None:
                return [](const TBinScore& bs, const TBackground&, int) {
                    return bs.Score;
                };

            case EBinScoreNormalization::LabelEntropy: {
                const double labelEntropy = Entropy(label.AllBins());
                return [labelEntropy](const TBinScore& bs, const TBackground&, int) {
                    return bs.Score / labelEntropy;
                };
            }

            case EBinScoreNormalization::BinEntropy:
                return [](const TBinScore& bs, const TBackground& background, int binIndex) {
                    return bs.Score / Entropy(background.GetBin(binIndex));
                };

            case EBinScoreNormalization::BinAndLabelEntropy: {
                const double labelEntropy = Entropy(label.AllBins());
                return [labelEntropy](const TBinScore& bs, const TBackground& bg, int binIndex) {
                    return bs.Score / labelEntropy / Entropy(bg.GetBin(binIndex));
                };
            }
        }
    }
}
