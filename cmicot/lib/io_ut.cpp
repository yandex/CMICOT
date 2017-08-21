#include "io.h"

#include <library/unittest/registar.h>

#include <util/generic/algorithm.h>
#include <util/generic/xrange.h>
#include <util/random/fast.h>
#include <util/random/shuffle.h>
#include <util/stream/str.h>
#include <util/string/join.h>
#include <util/thread/queue.h>

namespace NCmicot {
    SIMPLE_UNIT_TEST_SUITE(ReadPool) {
        SIMPLE_UNIT_TEST(CorrectFeatures) {
            const TString data = "1\t2\t3.4\t5.6\n"
                                "0\t7\t8\t9\n"
                                "1.2\t3.4\t5.6\t7.8";

            const yvector<yvector<double>> features = {
                {1.0, 0.0, 1.2},
                {2.0, 7.0, 3.4},
                {3.4, 8.0, 5.6},
                {5.6, 9.0, 7.8},
            };

            TStringInput si(data);
            UNIT_ASSERT_EQUAL(ReadPool(si), features);
        }

        SIMPLE_UNIT_TEST(LinesWithDifferentAmountOfNumbers) {
            const TString data = "1\t2\t3.4\t5.6\n"
                                "1.2\t3.4";
            TStringInput si(data);

            UNIT_ASSERT_EXCEPTION(ReadPool(si), yexception);
        }

        SIMPLE_UNIT_TEST(CorruptedData) {
            const TString data = Join("\n", Join("\t", 1, 2, 3, 4),
                                     Join("\t", 1, 2, "upyachka"));

            TStringInput si(data);
            UNIT_ASSERT_EXCEPTION(ReadPool(si), yexception);
        }

        SIMPLE_UNIT_TEST(HugePool) {
            TReallyFastRng32 rng(20160330);

            const int batchSize = 10;
            const int lineCount = 50 * batchSize + batchSize / 3;
            const int featureCount = 50;
            TRawPool testPool(featureCount, yvector<double>(lineCount));
            for (auto& feature : testPool) {
                for (auto& x : feature) {
                    x = rng.GenRandReal1();
                }
            }

            TStringStream ss;
            for (int line : xrange(lineCount)) {
                for (int featureIndex : xrange(featureCount)) {
                    ss << testPool[featureIndex][line];
                    if (featureIndex < featureCount - 1) {
                        ss << '\t';
                    }
                }
                ss << '\n';
            }

            auto queue = CreateMtpQueue(2);

            const yvector<yvector<double>> pool = ReadPool(ss, batchSize, *queue);
            UNIT_ASSERT_VALUES_EQUAL(pool.size(), featureCount);
            UNIT_ASSERT(AllOf(pool, [&](const yvector<double>& f) { return f.ysize() == lineCount; }));

            UNIT_ASSERT_VALUES_EQUAL(pool.size(), testPool.size());
            for (int column : xrange(pool.size())) {
                UNIT_ASSERT_VALUES_EQUAL_C(pool[column].size(), testPool[column].size(), column);

                for (int i : xrange(pool[column].size())) {
                    UNIT_ASSERT_DOUBLES_EQUAL(1e-6, pool[column][i], testPool[column][i]);
                }
            }
        }
    }

    SIMPLE_UNIT_TEST_SUITE(IO) {
        SIMPLE_UNIT_TEST(ReadCorrectBinToFeaturesMap) {
            TReallyFastRng32 rng(20160126);

            const int featureCount = rng.Uniform(1000, 2000);
            yvector<int> binCount(featureCount);
            for (auto& x : binCount) {
                x = rng.Uniform(1, 10);
            }

            yvector<std::pair<int, int>> testData;

            int binId = 0;
            for (int featureId : xrange(featureCount)) {
                for (int i : xrange(binCount[featureId])) {
                    Y_UNUSED(i);
                    testData.push_back({featureId, binId++});
                }
            }
            Shuffle(testData.begin(), testData.end(), rng);

            TStringStream testDataStream;
            for (const auto& featureBinPair : testData) {
                testDataStream << featureBinPair.first << " " << featureBinPair.second << '\n';
            }

            const auto binFeatureMap = ReadBinToFeatureMap(testDataStream);
            UNIT_ASSERT_VALUES_EQUAL(binFeatureMap.size(), Accumulate(binCount, 0));

            yvector<int> anotherBinCount(featureCount);
            for (int x : binFeatureMap) {
                UNIT_ASSERT(x >= 0);
                UNIT_ASSERT(x < featureCount);
                ++anotherBinCount[x];
            }

            UNIT_ASSERT_EQUAL(binCount, anotherBinCount);
        }

        SIMPLE_UNIT_TEST(ReadBinToFeatureMapThrowsOnIncorrectData) {
            {
                TStringStream ss("upyachka");
                UNIT_ASSERT_EXCEPTION(ReadBinToFeatureMap(ss), yexception);
            }
            {
                // The same bin maps to 2 different features
                TStringStream ss("0 0\n1 0");
                UNIT_ASSERT_EXCEPTION(ReadBinToFeatureMap(ss), yexception);
            }
            {
                TStringStream ss("1 -3");
                UNIT_ASSERT_EXCEPTION(ReadBinToFeatureMap(ss), yexception);
            }
            {
                TStringStream ss("1 1\n2 0\n2 ");
                UNIT_ASSERT_EXCEPTION(ReadBinToFeatureMap(ss), yexception);
            }
            {
                TStringStream ss("0 0\n"
                                 "0 1\n"
                                 "1 3\n"
                                 "1 4\n");
                UNIT_ASSERT_EXCEPTION(ReadBinToFeatureMap(ss), yexception);
            }
        }
    }
}
