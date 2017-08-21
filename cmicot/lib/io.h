#pragma once

#include <util/generic/vector.h>

#include <functional>

class IInputStream;
class IOutputStream;
class IMtpQueue;

namespace NCmicot {
    using TRawPool = yvector<yvector<double>>;

    THolder<IInputStream> OpenInput(const TString& filename);

    TRawPool ReadPool(IInputStream& in, int linesInBatch, IMtpQueue& queue);
    TRawPool ReadPool(IInputStream& in);

    yvector<int> ReadBinToFeatureMap(IInputStream& in);

    enum class EOutputFormat {
        FullResult,
        UsedBins,
        Score,
        Pool,
        FeatureSizes,
        BinFeatureMap,
    };

    struct TBinScore;
    struct TFeatureScore;
    struct TCmimScore;

    void OutputUsedBins(const TBinScore& result, IOutputStream& out);
    void OutputUsedBins(const TFeatureScore& result, IOutputStream& out);
    void OutputScore(double score, IOutputStream& out);

    class TBinFeatureSet;

    void OutputPool(const TBinFeatureSet& label, const TBinFeatureSet& features, IOutputStream& out);
    void OutputFeatureSizes(const TBinFeatureSet& features, IOutputStream& out);
    void OutputBinFeatureMap(const TBinFeatureSet& features, IOutputStream& out);

    using TOutputter = std::function<void(IOutputStream&)>;

    TOutputter BuildOutputChain(yvector<EOutputFormat> flags, TBinFeatureSet& label,
                                TBinFeatureSet& features, TFeatureScore& scoringResult);

    TOutputter BuildOutputChain(yvector<EOutputFormat> flags, TBinFeatureSet& label,
                                TBinFeatureSet& features, TCmimScore& scoringResult);
}
