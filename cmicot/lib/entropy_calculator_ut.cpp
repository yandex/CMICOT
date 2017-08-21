#include "entropy_calculator.cpp"
#include "test_pool_gen.h"

#include <library/unittest/registar.h>

#include <util/generic/xrange.h>
#include <util/random/fast.h>

namespace NCmicot {
    SIMPLE_UNIT_TEST_SUITE(EntropyCalculator) {
        SIMPLE_UNIT_TEST(DefaultConstrcutor) {
            TEntropyCalculator ec(5);
            UNIT_ASSERT_DOUBLES_EQUAL(ec.GetEntropy(), 0.0, 1e-8);
        }

        SIMPLE_UNIT_TEST(Constant) {
            const int binSize = 100;
            const TBin contantBin = TBin(binSize, 1);

            TEntropyCalculator ec(binSize);
            for (int i : xrange(10)) {
                Y_UNUSED(i);
                ec.AddBin(contantBin);
                UNIT_ASSERT_DOUBLES_EQUAL(ec.GetEntropy(), 0.0, 1e-8);
            }
        }

        SIMPLE_UNIT_TEST(EntropyWithExtraBinEqualsAddBinAndGetEntropy) {
            const int binSize = 100;

            TReallyFastRng32 rng(20151225);

            TEntropyCalculator ec(binSize);
            for (int i : xrange(22)) {
                //    for (int i : xrange(TEntropyCalculator::MAX_BIN_COUNT)) {
                Y_UNUSED(i);

                const TBin bin = RandomBin(binSize, rng);

                double entropyOne = ec.GetEntropyWithExtraBin(bin);

                ec.AddBin(bin);
                UNIT_ASSERT_DOUBLES_EQUAL(entropyOne, ec.GetEntropy(), 1e-8);
            }
        }

        SIMPLE_UNIT_TEST(DependantValues) {
            const int binSize = 1000;
            TReallyFastRng32 rng(20151228);

            const auto freeBin = RandomBin(binSize, rng);
            TBin dependantBin;
            for (bool bit : freeBin) {
                dependantBin.push_back(!bit);
            }

            TEntropyCalculator ec(binSize);
            ec.AddBin(freeBin);

            UNIT_ASSERT_DOUBLES_EQUAL(ec.GetEntropy(), ec.GetEntropyWithExtraBin(freeBin), 1e-8);
        }

        SIMPLE_UNIT_TEST(TheMoreValuesTheMoreEntropy) {
            const int binSize = 1000;
            TReallyFastRng32 rng(20151228);

            TEntropyCalculator ec(binSize);
            ec.AddBin(RandomBin(binSize, rng));
            UNIT_ASSERT(ec.GetEntropy() < ec.GetEntropyWithExtraBin(RandomBin(binSize, rng)) - 1e-8);
        }

        SIMPLE_UNIT_TEST(WideRangeOfValues) {
            const int binSize = TEntropyCalculator::MAX_BIN_COUNT;
            TEntropyCalculator ec(binSize);

            // The range of values is [1..2^(MAX_BIN_COUNT - 1)]
            for (int i : xrange(TEntropyCalculator::MAX_BIN_COUNT)) {
                TBin bin(binSize, 0);
                bin[i] = true;
                ec.AddBin(bin);
            }

            UNIT_ASSERT_NO_EXCEPTION(ec.GetEntropy());
        }

        SIMPLE_UNIT_TEST(FrequencyCountersGiveSameResults) {
            const ui64 minValue = 13574;
            const ui64 maxValue = minValue + 1000000;

            TVectorCounter vc(minValue, maxValue);
            THashMapCounter hm;
            IFrequencyCounter* counters[] = {&vc, &hm};

            TReallyFastRng32 rng(20160330);

            const int totalValues = 5000002;
            for (IFrequencyCounter* fc : counters) {
                fc->Add(minValue);
                fc->Add(maxValue);
            }

            for (int i : xrange(totalValues - 2)) {
                Y_UNUSED(i);

                auto value = rng.Uniform(minValue, maxValue);

                for (IFrequencyCounter* fc : counters) {
                    fc->Add(value);
                }
            }

            UNIT_ASSERT_DOUBLES_EQUAL(1e-8, vc.GetEntropy(totalValues), hm.GetEntropy(totalValues));
        }

        SIMPLE_UNIT_TEST(VectorCounterIsOkWithSmallRangeWithHugeBoundaries) {
            const ui64 minValue = 1ULL << 53;
            const ui64 rangeSize = 10;
            TVectorCounter vc(minValue, minValue + rangeSize);
            for (ui64 i : xrange(rangeSize)) {
                UNIT_ASSERT_NO_EXCEPTION(vc.Add(minValue + i));
            }
            UNIT_ASSERT(vc.GetEntropy(rangeSize) > 0.0);
        }
    }
}
