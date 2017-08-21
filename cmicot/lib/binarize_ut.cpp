#include "binarize.h"

#include <library/unittest/registar.h>

#include <library/grid_creator/binarization.h>

#include <util/generic/adaptor.h>
#include <util/generic/algorithm.h>
#include <util/generic/vector.h>
#include <util/generic/xrange.h>
#include <util/string/join.h>

namespace NCmicot {
    SIMPLE_UNIT_TEST_SUITE(Binarize) {
        SIMPLE_UNIT_TEST(SimpleBinarize) {
            const yvector<double> feature = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

            const yhash_set<float> borders = {2.5, 5.5, 7.5};
            const yvector<TBin> bins = BinarizeFeature(feature, [&](yvector<float>&) { return borders; });

            UNIT_ASSERT_VALUES_EQUAL(bins.size(), borders.size());
        }

        SIMPLE_UNIT_TEST(BinarizaionSavesRelativeOrder) {
            const yvector<double> feature = {8, 1, 3, 4, 2, 9, 6, 5, 4, 10, 10, 11, 0};

            auto borderBuilder = [&](yvector<float>& values) {
                return NSplitSelection::TMedianBinarizer().BestSplit(values, feature.size(), false);
            };
            const yvector<ui64> afterBinarization = UniteLabelBins(BinarizeFeature(feature, borderBuilder));

            UNIT_ASSERT_VALUES_EQUAL(feature.size(), afterBinarization.size());
            for (int i : xrange(feature.ysize() - 1)) {
                if (feature[i + 1] > feature[i]) {
                    UNIT_ASSERT_C(afterBinarization[i + 1] > afterBinarization[i], JoinSeq(" ", afterBinarization));
                } else if (feature[i + 1] < feature[i]) {
                    UNIT_ASSERT_C(afterBinarization[i + 1] < afterBinarization[i], JoinSeq(" ", afterBinarization));
                } else {
                    UNIT_ASSERT_VALUES_EQUAL(afterBinarization[i + 1], afterBinarization[i]);
                }
            }
        }

        SIMPLE_UNIT_TEST(ConstantFeature) {
            const int valueCount = 10000;
            const yvector<double> constantFeature(valueCount, 0.0);
            auto borderBuilder = [&](yvector<float>& values) {
                return NSplitSelection::TMedianBinarizer().BestSplit(values, 10, false);
            };
            yvector<TBin> bins = BinarizeFeature(constantFeature, borderBuilder);
            UNIT_ASSERT_VALUES_EQUAL(bins.size(), 1);
            UNIT_ASSERT_VALUES_EQUAL(bins.front(), TBin(valueCount, false));
        }
    }
}
