#include "bin_feature_set.h"

#include <library/unittest/registar.h>

#include <util/generic/algorithm.h>
#include <util/generic/xrange.h>

#include <initializer_list>

namespace NCmicot {
    SIMPLE_UNIT_TEST_SUITE(BinFeatureSet) {
        SIMPLE_UNIT_TEST(Construction) {
            const int size = 10;
            const yvector<TBin> feature = {TBin(size, true), TBin(size, false)};

            TBinFeatureSet fs(feature);

            UNIT_ASSERT_VALUES_EQUAL(fs.GetFeatureCount(), 1);
            UNIT_ASSERT_EQUAL(feature, fs.GetFeature(0));
        }

        SIMPLE_UNIT_TEST(AddFeature) {
            const int size = 10;
            const yvector<TBin> feature1 = {TBin(size, true), TBin(size, false)};
            const yvector<TBin> feature2 = {TBin(size, true), TBin(size, false), TBin(size, true)};

            TBinFeatureSet fs;
            fs.AddFeature(feature1);
            fs.AddFeature(feature2);

            UNIT_ASSERT_VALUES_EQUAL(fs.GetFeatureCount(), 2);
            UNIT_ASSERT_EQUAL(feature1, fs.GetFeature(0));
            UNIT_ASSERT_EQUAL(feature2, fs.GetFeature(1));
        }

        SIMPLE_UNIT_TEST(AddFeatureReturnValue) {
            const yvector<TBin> feature;

            TBinFeatureSet fs;
            UNIT_ASSERT_VALUES_EQUAL(fs.AddFeature(feature), 0);
            UNIT_ASSERT_VALUES_EQUAL(fs.AddFeature(feature), 1);

            for (auto i : xrange(10)) {
                Y_UNUSED(i);
                UNIT_ASSERT_VALUES_EQUAL(fs.AddFeature(feature), fs.GetFeatureCount() - 1);
            }
        }

        SIMPLE_UNIT_TEST(GetBin) {
            const int size = 10;
            const yvector<TBin> feature1 = {TBin(size, true), TBin(size, false)};
            const yvector<TBin> feature2 = {TBin(size, true), TBin(size, false), TBin(size, true)};

            TBinFeatureSet fs;
            fs.AddFeature(feature1);
            fs.AddFeature(feature2);

            UNIT_ASSERT_EQUAL(fs.GetBin(0), feature1[0]);
            UNIT_ASSERT_EQUAL(fs.GetBin(1), feature1[1]);
            UNIT_ASSERT_EQUAL(fs.GetBin(2), feature2[0]);
            UNIT_ASSERT_EQUAL(fs.GetBin(3), feature2[1]);
            UNIT_ASSERT_EQUAL(fs.GetBin(4), feature2[2]);
        }

        SIMPLE_UNIT_TEST(GetFeatureByBin) {
            const int binSize = 10;
            const int featureBinCount[] = {5, 1, 10, 22, 8, 6, 4, 3, 2};

            TBinFeatureSet fs;
            for (int binCount : featureBinCount) {
                fs.AddFeature(yvector<TBin>(binCount, TBin(binSize, false)));
            }

            int binIndex = 0;
            for (int featureId : xrange(Y_ARRAY_SIZE(featureBinCount))) {
                for (int i = 0; i < featureBinCount[featureId]; ++i) {
                    UNIT_ASSERT_VALUES_EQUAL(fs.GetFeatureIndexByBinIndex(binIndex++), featureId);
                }
            }
        }

        SIMPLE_UNIT_TEST(InvalidIndexes) {
            TBinFeatureSet fs;

            UNIT_ASSERT_EXCEPTION(fs.GetBin(-1), TWithBackTrace<yexception>);
            UNIT_ASSERT_EXCEPTION(fs.GetBin(10), TWithBackTrace<yexception>);
            UNIT_ASSERT_EXCEPTION(fs.GetFeature(-1), TWithBackTrace<yexception>);
            UNIT_ASSERT_EXCEPTION(fs.GetFeature(10), TWithBackTrace<yexception>);
            UNIT_ASSERT_EXCEPTION(fs.GetFeatureIndexByBinIndex(-1), TWithBackTrace<yexception>);
            UNIT_ASSERT_EXCEPTION(fs.GetFeatureIndexByBinIndex(10), TWithBackTrace<yexception>);
            UNIT_ASSERT_EXCEPTION(fs.GetFeatureBinIndexes(-1), TWithBackTrace<yexception>);
            UNIT_ASSERT_EXCEPTION(fs.GetFeatureBinIndexes(10), TWithBackTrace<yexception>);
        }
    }

    SIMPLE_UNIT_TEST_SUITE(Background) {
        SIMPLE_UNIT_TEST(Construction) {
            const int size = 10;
            TBinFeatureSet fs;
            fs.AddFeature({TBin(size, true), TBin(size, false)});
            fs.AddFeature({TBin(size, true), TBin(size, false), TBin(size, false)});
            fs.AddFeature({TBin(size, true), TBin(size, false), TBin(size, false), TBin(size, false)});
            fs.AddFeature({TBin(size, true)});

            TBackground bg(fs);

            UNIT_ASSERT(AllOf(xrange(fs.GetFeatureCount()), [&](int f) { return bg.IsFeatureEnabled(f); }));
            UNIT_ASSERT_VALUES_EQUAL(bg.EnabledBinIndexes().ysize(), fs.GetBinCount());
        }

        SIMPLE_UNIT_TEST(EnableFeature) {
            const int size = 10;
            const yvector<TBin> feature1 = {TBin(size, true), TBin(size, false)};
            const yvector<TBin> feature2 = {TBin(size, true), TBin(size, false), TBin(size, true)};

            TBinFeatureSet fs;
            fs.AddFeature(feature1);
            fs.AddFeature(feature2);

            TBackground bg(fs);
            bg.SetFeatureEnabled(0, false);
            UNIT_ASSERT_VALUES_EQUAL(fs.GetFeatureCount(), 2);
            UNIT_ASSERT_VALUES_EQUAL(bg.EnabledBins().size(), 3);

            bg.SetFeatureEnabled(1, false);
            UNIT_ASSERT_VALUES_EQUAL(bg.EnabledBins().size(), 0);

            bg.SetFeatureEnabled(0, true);
            UNIT_ASSERT_VALUES_EQUAL(bg.EnabledBins().size(), 2);
        }

        SIMPLE_UNIT_TEST(EnableBin) {
            const int size = 10;
            const yvector<TBin> feature1 = {TBin(size, true), TBin(size, false)};
            const yvector<TBin> feature2 = {TBin(size, true), TBin(size, false), TBin(size, true)};

            TBinFeatureSet fs;
            fs.AddFeature(feature1);
            fs.AddFeature(feature2);

            TBackground bg(fs);

            bg.SetBinEnabled(0, false);
            UNIT_ASSERT_VALUES_EQUAL(bg.EnabledBins().size(), 4);

            bg.SetBinEnabled(2, false);
            UNIT_ASSERT_VALUES_EQUAL(bg.EnabledBins().size(), 3);

            const yvector<TBin> enabledBins = {feature1[1], feature2[1], feature2[2]};
            UNIT_ASSERT_EQUAL(bg.EnabledBins(), enabledBins);

            for (int i : xrange(feature1.size() + feature2.size())) {
                bg.SetBinEnabled(i, true);
            }
            UNIT_ASSERT_VALUES_EQUAL(bg.EnabledBins().size(), 5);
        }

        SIMPLE_UNIT_TEST(EnabledBinIndexes) {
            const int size = 10;
            const yvector<TBin> feature1 = {TBin(size, true), TBin(size, false)};
            const yvector<TBin> feature2 = {TBin(size, true), TBin(size, false), TBin(size, true)};

            TBinFeatureSet fs;
            fs.AddFeature(feature1);
            fs.AddFeature(feature2);

            TBackground bg(fs);

            UNIT_ASSERT_EQUAL(bg.EnabledBinIndexes(), (yvector<int>{0, 1, 2, 3, 4}));

            bg.SetBinEnabled(0, false);
            bg.SetBinEnabled(2, false);
            UNIT_ASSERT_EQUAL(bg.EnabledBinIndexes(), (yvector<int>{1, 3, 4}));

            for (int i : xrange(feature1.size() + feature2.size())) {
                bg.SetBinEnabled(i, true);
            }
            UNIT_ASSERT_EQUAL(bg.EnabledBinIndexes(), (yvector<int>{0, 1, 2, 3, 4}));
        }

        SIMPLE_UNIT_TEST(DisableAll) {
            const int binSize = 10;

            TBinFeatureSet fs;
            fs.AddFeature({TBin(binSize, true), TBin(binSize, false), TBin(binSize, true)});
            fs.AddFeature({TBin(binSize, true)});
            fs.AddFeature({TBin(binSize, false), TBin(binSize, true)});

            TBackground bg(fs);

            bg.DisableAll();
            UNIT_ASSERT(bg.EnabledBins().empty());
            UNIT_ASSERT(bg.EnabledBinIndexes().empty());
        }

        SIMPLE_UNIT_TEST(IsFeatureEnabled) {
            const int binSize = 10;

            TBinFeatureSet fs;
            fs.AddFeature({TBin(binSize, true), TBin(binSize, false), TBin(binSize, true)});
            fs.AddFeature({TBin(binSize, true)});
            fs.AddFeature({TBin(binSize, false), TBin(binSize, true)});

            TBackground bg(fs);

            for (int i : xrange(fs.GetFeatureCount())) {
                UNIT_ASSERT(bg.IsFeatureEnabled(i));
            }

            bg.SetBinEnabled(0, false);
            UNIT_ASSERT(!bg.IsFeatureEnabled(0));
            for (int i : xrange(1, fs.GetFeatureCount())) {
                UNIT_ASSERT(bg.IsFeatureEnabled(i));
            }

            bg.SetFeatureEnabled(0, true);
            UNIT_ASSERT(bg.IsFeatureEnabled(0));

            bg.DisableAll();
            for (int i : xrange(fs.GetFeatureCount())) {
                UNIT_ASSERT(!bg.IsFeatureEnabled(i));
            }
        }

        SIMPLE_UNIT_TEST(LastEnabledBins) {
            const TBin bin(1000, true);

            TBinFeatureSet fs;
            fs.AddFeature({bin, bin, bin});
            fs.AddFeature({bin});
            fs.AddFeature({bin, bin});

            TBackground bg(fs);

            UNIT_ASSERT(bg.LastEnabled().EnabledBins().empty());

            bg.SetFeatureEnabled(0, true);
            UNIT_ASSERT_VALUES_EQUAL(bg.LastEnabled().EnabledBinIndexes(), (yvector<int>{0, 1, 2}));

            bg.SetFeatureEnabled(0, false);
            UNIT_ASSERT_VALUES_EQUAL(bg.LastEnabled().EnabledBinIndexes(), (yvector<int>{0, 1, 2}));

            bg.SetBinEnabled(1, true);
            UNIT_ASSERT_VALUES_EQUAL(bg.LastEnabled().EnabledBinIndexes(), (yvector<int>{1}));

            bg.SetFeatureEnabled(1, true);
            UNIT_ASSERT_VALUES_EQUAL(bg.LastEnabled().EnabledBinIndexes(), (yvector<int>{3}));

            bg.SetFeatureEnabled(bg.GetFeatureCount() - 1, true);
            UNIT_ASSERT_VALUES_EQUAL(bg.LastEnabled().EnabledBinIndexes(), (yvector<int>{fs.GetBinCount() - 2, fs.GetBinCount() - 1}));
        }

        SIMPLE_UNIT_TEST(InvalidIndexes) {
            TBinFeatureSet fs;
            TBackground bg(fs);

            UNIT_ASSERT_EXCEPTION(bg.SetFeatureEnabled(-1, true), TWithBackTrace<yexception>);
            UNIT_ASSERT_EXCEPTION(bg.SetFeatureEnabled(10, true), TWithBackTrace<yexception>);
            UNIT_ASSERT_EXCEPTION(bg.SetBinEnabled(-1, true), TWithBackTrace<yexception>);
            UNIT_ASSERT_EXCEPTION(bg.SetBinEnabled(10, true), TWithBackTrace<yexception>);
            UNIT_ASSERT_EXCEPTION(bg.IsFeatureEnabled(-1), TWithBackTrace<yexception>);
            UNIT_ASSERT_EXCEPTION(bg.IsFeatureEnabled(10), TWithBackTrace<yexception>);
        }

        SIMPLE_UNIT_TEST(EnabledFeatureIndexes) {
            const TBin bin(1000, true);

            TBinFeatureSet fs;
            fs.AddFeature({bin, bin, bin});
            fs.AddFeature({bin});
            fs.AddFeature({bin, bin});

            TBackground bg(fs);

            const yvector<int> expected = xrange(fs.GetFeatureCount());
            UNIT_ASSERT_VALUES_EQUAL(bg.EnabledFeatureIndexes(), expected);

            for (int i : xrange(bg.GetFeatureCount())) {
                bg.SetFeatureEnabled(i, false);
            }
            UNIT_ASSERT(bg.EnabledFeatureIndexes().empty());

            bg.SetFeatureEnabled(0, true);
            UNIT_ASSERT_VALUES_EQUAL(bg.EnabledFeatureIndexes(), yvector<int>{0});

            // A feature is enabled only if all its bins are enabled
            bg.SetBinEnabled(1, false);
            UNIT_ASSERT(bg.EnabledFeatureIndexes().empty());
        }
    }
}
