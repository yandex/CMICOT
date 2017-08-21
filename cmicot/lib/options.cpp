#include "options.h"

#include <library/grid_creator/binarization.h>

#include <util/stream/input.h>
#include <util/stream/file.h>
#include <util/generic/hash.h>
#include <util/string/join.h>
#include <util/string/split.h>

extern const TString& NCmicotEBinScoreNormalizationAllNames();

namespace NCmicot {
    template <class Map>
    yvector<typename Map::key_type> Keys(const Map& map) {
        yvector<typename Map::key_type> result;
        for (const auto& keyValue : map) {
            result.push_back(keyValue.first);
        }
        return result;
    }

    void EnsurePositive(int value, int& dest) {
        Y_ENSURE(value > 0);
        dest = value;
    }

    NLastGetopt::TOpts CreateCommandLineOptions(TOptions& opts) {
        auto result = NLastGetopt::TOpts::Default();

        result.AddCharOption('t', "Eval algorithm step count")
              .DefaultValue("6")
              .Handler1T<int>([&](int value) { EnsurePositive(value, opts.EvalStepCount); });
        result.AddLongOption("thread-count", "Threads to use during maximization and minimization")
              .RequiredArgument()
              .Handler1T<int>([&](int value) { EnsurePositive(value, opts.ThreadCount); })
              .DefaultValue("8");
        result.AddLongOption("binary-pool")
              .RequiredArgument()
              .StoreResultT<TString>(&opts.BinaryPoolFilename)
              .Help("File with binarized pool (1st column - nonbinarized label, other columns - binarized features). This option should be used with --map");
        result.AddLongOption("map", "File with feature-bin map")
              .RequiredArgument()
              .StoreResultT<TString>(&opts.FeatureBinMapFilename);
        result.AddLongOption("pool", "File with raw pool. This is option can't be used with --binary-pool and --map")
              .RequiredArgument()
              .StoreResultT<TString>(&opts.RawPoolFilename);
        result.AddLongOption("select-count", "How many features should be selected")
              .RequiredArgument("FEATURE COUNT")
              .Handler1T<int>([&](int featureCount) {
                  Y_ENSURE(featureCount > 0);
                  opts.FeatureCountToSelect = featureCount;
              });

        using TBinBuilder = std::function<NSplitSelection::IBinarizer*()>;
        static const yhash<TString, TBinBuilder> builderByName = {
            {"medianPlusUniform", [] { return new NSplitSelection::TMedianPlusUniformBinarizer; }},
            {"median", [] { return new NSplitSelection::TMedianBinarizer; }},
            {"minEntropy", [] { return new NSplitSelection::TMinEntropyBinarizer; }},
            {"medianInBin", [] { return new NSplitSelection::TMedianInBinBinarizer; }},
            {"maxSumLog", [] { return new NSplitSelection::TMaxSumLogBinarizer; }},
        };

        const TString help = "Binarization mode. Should be one of: " + JoinSeq(", ", Keys(builderByName));
        result.AddLongOption("binarization", help)
              .RequiredArgument()
              .Handler1T<TString>([&opts](const TString& param) {
                  opts.Binarizer = builderByName.at(param)();
              })
              .DefaultValue("medianPlusUniform");

        result.AddCharOption('x', "Discretization level count")
              .RequiredArgument()
              .Handler1T<int>([&](int value) { EnsurePositive(value, opts.BorderCount); })
              .DefaultValue("10");

        result.AddLongOption("just-binarize", "Output binarized pool and feature-bin map instead of doing feature selection. Please provide filenames where pool and map should be stored separated by a comma")
              .RequiredArgument("PoolFile,MapFile")
              .Handler1T<TString>([&opts](const TString& param) {
                  TString poolFile, mapFile;
                  Split(param, ',', poolFile, mapFile);
                  opts.BinaryPoolOutputFile = poolFile;
                  opts.FeatureBinMapOutputFile = mapFile;
              });

        result.SetFreeArgsMax(0);

        return result;
    }
}
