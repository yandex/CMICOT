#include "io.h"
#include "feature_score.h"

#include <library/threading/future/async.h>
#include <library/threading/future/future.h>

#include <util/generic/algorithm.h>
#include <util/generic/string.h>
#include <util/generic/yexception.h>
#include <util/generic/hash.h>
#include <util/stream/input.h>
#include <util/stream/file.h>
#include <util/stream/output.h>
#include <util/string/cast.h>
#include <util/string/iterator.h>
#include <util/string/join.h>
#include <util/stream/format.h>
#include <util/stream/labeled.h>
#include <util/generic/xrange.h>
#include <util/thread/queue.h>

namespace NCmicot {
    namespace {
        auto FormatScore(double score) -> decltype(Prec(0.0, PREC_POINT_DIGITS, 1)) {
            return Prec(score + 1e-9, PREC_POINT_DIGITS, 8);
        }

        struct TBatchParser {
            yvector<TString> Lines;
            int FirstLineNo;

            TRawPool operator()() const {
                TRawPool result;
                yvector<double> numbers;

                for (int i = 0, curLine = FirstLineNo; i < Lines.ysize(); ++i, ++curLine) {
                    numbers.clear();
                    for (const auto& iter : StringSplitter(Lines[i]).Split('\t')) {
                        double currentNumber;
                        if (TryFromString(iter.Token(), currentNumber)) {
                            numbers.push_back(currentNumber);
                        } else {
                            ythrow yexception() << "Failed to parse double from \"" << iter.Token()
                                                << "\" in line " << curLine;
                        }
                    }

                    if (result.empty()) {
                        result.resize(numbers.size());
                        for (auto& column : result) {
                            column.reserve(Lines.size());
                        }
                    } else {
                        Y_ENSURE(result.size() == numbers.size(),
                                 "In line " << curLine << " there are " << numbers.size() << " columns while "
                                                                                             "in line "
                                            << FirstLineNo << " there are " << result.size() << " columns");
                    }

                    for (int column : xrange(numbers.size())) {
                        result[column].push_back(numbers[column]);
                    }
                }

                return result;
            }
        };

        class TCinWrapper : public IInputStream {
        protected:
            size_t DoRead(void* buf, size_t len) override {
                return Cin.Read(buf, len);
            }
        };
    }

    TRawPool ReadPool(IInputStream& in, int linesInBatch, IMtpQueue& queue) {
        yvector<NThreading::TFuture<TRawPool>> futures;

        for (int lineNo = 1;;) {
            yvector<TString> lines;
            lines.reserve(linesInBatch);

            TString currentLine;
            for (int i = 0; i < linesInBatch && in.ReadLine(currentLine); ++i) {
                lines.push_back(currentLine);
            }

            if (lines.empty()) {
                break;
            }

            int firstLineNo = lineNo;
            lineNo += lines.size();
            futures.push_back(NThreading::Async(TBatchParser{std::move(lines), firstLineNo}, queue));
        }

        TRawPool result;
        for (const NThreading::TFuture<TRawPool>& parseResult : futures) {
            parseResult.Wait();
            const auto& data = parseResult.GetValue();

            if (result.empty()) {
                result.resize(data.size());
            }
            Y_ENSURE(result.size() == data.size(), LabeledOutput(result.size(), data.size()));
            for (int column : xrange(result.size())) {
                result[column].insert(result[column].end(), data[column].begin(), data[column].end());
            }
        }

        return result;
    }

    TRawPool ReadPool(IInputStream& in) {
        TMtpQueue queue;
        queue.Start(16);
        return ReadPool(in, 20000, queue);
    }

    yvector<int> ReadBinToFeatureMap(IInputStream& in) {
        constexpr int uninitialized = -1;

        yvector<int> result;

        for (TString line; in.ReadLine(line);) {
            TStringInput si(line);

            ui64 featureIndex, binIndex;
            si >> featureIndex >> binIndex;

            if (binIndex >= result.size()) {
                result.resize(binIndex + 1, uninitialized);
            }
            Y_ENSURE(result[binIndex] == uninitialized, "Bin " << binIndex << " shows up twice in the map");
            result[binIndex] = featureIndex;
        }

        const auto iter = Find(result.begin(), result.end(), uninitialized);
        if (iter != result.end()) {
            ythrow yexception() << "Bin " << (iter - result.begin()) << " isn't present in the map";
        }

        return result;
    }

    void OutputScore(double score, IOutputStream& out) {
        out << FormatScore(score) << Endl;
    }

    void OutputPool(const TBinFeatureSet& label, const TBinFeatureSet& features, IOutputStream& out) {
        const yvector<ui64> labelValues = UniteLabelBins(label.AllBins());
        const int lineCount = labelValues.ysize();

        for (int line : xrange(lineCount)) {
            out << labelValues[line];
            for (const TBin& bin : features.AllBins()) {
                out << '\t' << bin[line];
            }
            out << '\n';
        }
    }

    void OutputFeatureSizes(const TBinFeatureSet& features, IOutputStream& out) {
        for (int featureIndex : xrange(features.GetFeatureCount())) {
            out << featureIndex << '\t' << features.GetFeature(featureIndex).size() << '\n';
        }
    }

    void OutputBinFeatureMap(const TBinFeatureSet& features, IOutputStream& out) {
        int globalBinIndex = 0;
        for (int featureIndex : xrange(features.GetFeatureCount())) {
            for (int binIndex : xrange(features.GetFeature(featureIndex).size())) {
                Y_UNUSED(binIndex);
                out << featureIndex << '\t' << globalBinIndex++ << '\n';
            }
        }
    }

    TOutputter BuildOutputChain(yvector<EOutputFormat> flags, TBinFeatureSet& label,
                                TBinFeatureSet& features, TFeatureScore& scoringResult) {
        if (flags.empty()) {
            flags.push_back(EOutputFormat::Score);
        }

        const yhash<EOutputFormat, TOutputter> outputterByFlag = {
            {EOutputFormat::FullResult, [&](IOutputStream& out) { out << scoringResult << Endl; }},
            {EOutputFormat::UsedBins, [&](IOutputStream& out) { OutputUsedBins(scoringResult, out); }},
            {EOutputFormat::Score, [&](IOutputStream& out) { OutputScore(scoringResult.Score, out); }},
            {EOutputFormat::Pool, [&](IOutputStream& out) { OutputPool(label, features, out); }},
            {EOutputFormat::FeatureSizes, [&](IOutputStream& out) { OutputFeatureSizes(features, out); }},
            {EOutputFormat::BinFeatureMap, [&](IOutputStream& out) { OutputBinFeatureMap(features, out); }},
        };

        yvector<TOutputter> outputters;
        for (EOutputFormat format : flags) {
            outputters.push_back(outputterByFlag.at(format));
        }

        return [outputters](IOutputStream& out) {
            for (const auto& outputter : outputters) {
                outputter(out);
            }
        };
    };

    void OutputUsedBins(const TBinScore& result, IOutputStream& out) {
        const TString separator = "\t";
        out << Join(separator, JoinSeq(separator, result.GetMaximizingBinIndexes()),
                    JoinSeq(separator, result.GetMinimizingBinIndexes()),
                    FormatScore(result.Score))
            << Endl;
    }

    void OutputUsedBins(const TFeatureScore& result, IOutputStream& out) {
        for (const auto& binScore : result.BinScore) {
            OutputUsedBins(binScore, out);
        }
    }

    TOutputter BuildOutputChain(yvector<EOutputFormat> flags, TBinFeatureSet& label,
                                TBinFeatureSet& features, TCmimScore& scoringResult) {
        if (flags.empty()) {
            flags.push_back(EOutputFormat::Score);
        }

        const yhash<EOutputFormat, TOutputter> outputterByFlag = {
            {EOutputFormat::FullResult, [&](IOutputStream& out) { out << scoringResult << Endl; }},
            {EOutputFormat::Score, [&](IOutputStream& out) { OutputScore(scoringResult.Score, out); }},
            {EOutputFormat::Pool, [&](IOutputStream& out) { OutputPool(label, features, out); }},
            {EOutputFormat::FeatureSizes, [&](IOutputStream& out) { OutputFeatureSizes(features, out); }},
            {EOutputFormat::BinFeatureMap, [&](IOutputStream& out) { OutputBinFeatureMap(features, out); }},
        };

        yvector<TOutputter> outputters;
        for (EOutputFormat format : flags) {
            outputters.push_back(outputterByFlag.at(format));
        }

        return [outputters](IOutputStream& out) {
            for (const auto& outputter : outputters) {
                outputter(out);
            }
        };
    }

    THolder<IInputStream> OpenInput(const TString& filename) {
        if (filename == STRINGBUF("-")) {
            return new TCinWrapper;
        }
        return new TIFStream(filename);
    }
}

template <>
void Out<NCmicot::TBinScore>(IOutputStream& os, const NCmicot::TBinScore& sr) {
    auto toString = [](double value) {
        return NCmicot::FormatScore(value);
    };

    for (int i : xrange(sr.MaximizingSteps.ysize())) {
        os << "max step " << i + 1 << ", bin = " << sr.MaximizingSteps[i].BinIndex
           << ", CMI = " << toString(sr.MaximizingSteps[i].Cmi)
           << '\n';
    }
    for (int i : xrange(sr.MinimizingSteps.ysize())) {
        os << "min step " << i + 1 << ", bin = " << sr.MinimizingSteps[i].BinIndex
           << ", CMI = " << toString(sr.MinimizingSteps[i].Cmi)
           << '\n';
    }
    os << "score: " << toString(sr.Score);
}

template <>
void Out<NCmicot::TFeatureScore>(IOutputStream& os, const NCmicot::TFeatureScore& fs) {
    for (int i : xrange(fs.BinScore.size())) {
        os << "Bin " << i << '\n';
        os << fs.BinScore[i] << '\n';
    }
    os << "Feature score: " << NCmicot::FormatScore(fs.Score);
}

template <>
void Out<NCmicot::TCmimScore>(IOutputStream& os, const NCmicot::TCmimScore& fs) {
    os << "Minimizing feature index: " << fs.MinimizingFeatureIndex << '\n'
       << "Feature score: " << NCmicot::FormatScore(fs.Score);
}
