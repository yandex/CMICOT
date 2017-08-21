#pragma once

#include <library/getopt/last_getopt.h>

#include <util/generic/ptr.h>
#include <util/generic/string.h>
#include <util/generic/maybe.h>

namespace NSplitSelection {
    class IBinarizer;
}

namespace NCmicot {
    struct TOptions {
        int ThreadCount;
        int EvalStepCount;
        TMaybe<TString> RawPoolFilename;
        TMaybe<TString> BinaryPoolFilename;
        TMaybe<TString> FeatureBinMapFilename;
        TMaybe<TString> BinaryPoolOutputFile;
        TMaybe<TString> FeatureBinMapOutputFile;
        TMaybe<int> FeatureCountToSelect;
        THolder<NSplitSelection::IBinarizer> Binarizer;
        int BorderCount;
    };

    NLastGetopt::TOpts CreateCommandLineOptions(TOptions& opts);
}
