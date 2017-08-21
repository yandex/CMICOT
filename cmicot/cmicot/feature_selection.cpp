#include <cmicot/lib/io.h>
#include <cmicot/lib/bin_feature_set.h>
#include <cmicot/lib/binarize.h>
#include <cmicot/lib/options.h>
#include <cmicot/lib/selection.h>

#include <library/grid_creator/binarization.h>

#include <library/terminate_handler/terminate_handler.h>

#include <util/generic/string.h>
#include <util/generic/vector.h>
#include <util/stream/file.h>

using NCmicot::TBinFeatureSet;

std::pair<TBinFeatureSet, TBinFeatureSet> ReadBinarizedPool(
    const TString& poolFile,
    const TString& mapFile,
    NCmicot::TBorderBuilder borderBuilder
) {
    const yvector<yvector<double>> pool = NCmicot::ReadPool(*NCmicot::OpenInput(poolFile));
    const yvector<int> binToFeatureMap = NCmicot::ReadBinToFeatureMap(*NCmicot::OpenInput(mapFile));

    return {
        TBinFeatureSet(NCmicot::BinarizeFeature(pool.front(), borderBuilder)),
        NCmicot::BinarizeWithMap(pool.begin() + 1, pool.end(), binToFeatureMap)
    };
}

void PrintFeature(int featureId) {
    Cout << featureId << Endl;
}

int main(int argc, char* argv[]) {
    SetFancyTerminateHandler();

    NCmicot::TOptions options;
    auto clOptions = NCmicot::CreateCommandLineOptions(options);
    NLastGetopt::TOptsParseResult opts(&clOptions, argc, argv);

    auto borderBuilder = [&options](yvector<float>& values) {
        return options.Binarizer->BestSplit(values, options.BorderCount);
    };

    TBinFeatureSet label, features;
    if (options.RawPoolFilename) {
        if (options.BinaryPoolFilename || options.FeatureBinMapFilename) {
            Cerr << "Provide either only --pool option or both --binary-pool and --map" << Endl;
            return 1;
        }

        std::tie(label, features) = NCmicot::BinarizeRawPool(
            NCmicot::ReadPool(*NCmicot::OpenInput(*options.RawPoolFilename)),
            borderBuilder,
            options.ThreadCount
        );
    } else if (options.BinaryPoolFilename && options.FeatureBinMapFilename) {
        std::tie(label, features) = ReadBinarizedPool(
            *options.BinaryPoolFilename,
            *options.FeatureBinMapFilename,
            borderBuilder
        );
    } else {
        Cerr << "Provide either only --pool option or both --binary-pool and --map" << Endl;
        return 2;
    }

    if (options.BinaryPoolOutputFile && options.FeatureBinMapOutputFile) {
        TOFStream poolOutput(*options.BinaryPoolOutputFile);
        NCmicot::OutputPool(label, features, poolOutput);

        TOFStream mapOutput(*options.FeatureBinMapOutputFile);
        NCmicot::OutputBinFeatureMap(features, mapOutput);
    } else {
        NCmicot::FastFeatureSelection(
            label,
            features,
            options.EvalStepCount,
            options.ThreadCount,
            options.FeatureCountToSelect.GetOrElse(features.GetFeatureCount()),
            PrintFeature
        );
    }

    return 0;
}
