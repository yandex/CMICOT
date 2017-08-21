#include "entropy.h"
#include "binarize.h"

#include <library/unittest/registar.h>

#include <util/generic/algorithm.h>
#include <util/generic/xrange.h>
#include <util/random/fast.h>

#include <initializer_list>
#include <numeric>

namespace NCmicot {
    SIMPLE_UNIT_TEST_SUITE(TestEntropy) {
        SIMPLE_UNIT_TEST(Flatten) {
            const TBin bin1 = {0, 0, 0, 0, 1, 1, 1, 1};
            const TBin bin2 = {0, 0, 1, 1, 0, 0, 1, 1};
            const TBin bin3 = {0, 1, 0, 1, 0, 1, 0, 1};
            const yvector<ui64> expected123 = {0, 1, 2, 3, 4, 5, 6, 7};
            const yvector<ui64> expected321 = {0, 4, 2, 6, 1, 5, 3, 7};

            using TBins = std::initializer_list<TBin>;

            UNIT_ASSERT_EQUAL(FlattenBins(TBins{bin1, bin2}, TBins{bin3}), expected123);
            UNIT_ASSERT_EQUAL(FlattenBins(TBins{bin1}, TBins{bin2, bin3}), expected123);
            UNIT_ASSERT_EQUAL(FlattenBins(TBins{bin1}, TBins{bin2}, TBins{bin3}), expected123);

            UNIT_ASSERT_EQUAL(FlattenBins(TBins{bin3, bin2}, TBins{bin1}), expected321);
        }

        SIMPLE_UNIT_TEST(Constant) {
            const yvector<ui64> constant(10, 5);
            UNIT_ASSERT_VALUES_EQUAL(Entropy(constant), 0.0);
        }

        namespace {
            yvector<yvector<bool>> Binarize(const yvector<ui64>& values) {
                yvector<yvector<bool>> result;

                for (auto bit : xrange(64)) {
                    yvector<bool> bin(values.size());
                    for (auto i : xrange(values.size())) {
                        bin[i] = ((values[i] >> bit) & 1);
                    }
                    result.push_back(bin);
                }

                while (result.size() > 1 && AllOf(result.back(), [](bool v) { return !v; })) {
                    result.pop_back();
                }
                Reverse(result.begin(), result.end());

                return result;
            }
        }

        SIMPLE_UNIT_TEST(DependantValues) {
            auto valuesOne = Binarize({1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
            auto valuesTwo = Binarize({2, 4, 6, 8, 10, 12, 14, 16, 18, 20});

            UNIT_ASSERT_VALUES_EQUAL(Entropy(valuesTwo, valuesOne), Entropy(valuesOne));
        }

        SIMPLE_UNIT_TEST(RandomDoesntDecreaseEntropy) {
            auto valuesOne = Binarize({1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
            auto valuesTwo = Binarize({1, 9, 4, 5, 9, 23, 6, 0, 9, 12});

            UNIT_ASSERT(Entropy(valuesOne, valuesTwo) >= Entropy(valuesOne));
        }

        SIMPLE_UNIT_TEST(TheMoreValuesTheMoreEntropy) {
            auto valuesOne = Binarize({1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
            auto valuesTwo = Binarize({1, 2, 3, 4, 5});

            UNIT_ASSERT(Entropy(valuesOne) > Entropy(valuesTwo));
        }

        SIMPLE_UNIT_TEST(TestMutualInformation) {
            auto valuesOne = Binarize({1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
            auto valuesTwo = Binarize({2, 4, 6, 8, 10, 13, 14, 16, 18, 21});

            auto mi = MutualInformation(valuesOne, valuesTwo);
            UNIT_ASSERT(mi > 0.0);
            UNIT_ASSERT(mi <= Entropy(valuesOne));
            UNIT_ASSERT(mi <= Entropy(valuesTwo));
            UNIT_ASSERT_VALUES_EQUAL(MutualInformation(valuesOne, valuesTwo), MutualInformation(valuesTwo, valuesOne));
        }

        SIMPLE_UNIT_TEST(MutualInformationForIndependantValues) {
            const int size = 50000;
            TReallyFastRng32 rand(20150916);

            yvector<ui64> one, two;
            yvector<ui64>* vectors[] = {&one, &two};

            for (auto v : vectors) {
                for (int i : xrange(size)) {
                    Y_UNUSED(i);
                    v->push_back(rand.Uniform(100));
                }
            }

            UNIT_ASSERT(MutualInformation(Binarize(one), Binarize(two)) < 0.2);
        }

        SIMPLE_UNIT_TEST(MutualInformationForFunction) {
            auto valuesOne = Binarize({1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
            auto valuesTwo = Binarize({2, 4, 6, 8, 10, 12, 14, 16, 18, 20});

            UNIT_ASSERT_VALUES_EQUAL(MutualInformation(valuesOne, valuesTwo), Entropy(valuesOne));
        }

        SIMPLE_UNIT_TEST(ConditionalMutualInformationForCopy) {
            TReallyFastRng32 rng(20151112);

            for (auto step : xrange(30)) {
                const auto size = rng.Uniform(50, 300);

                yvector<ui64> numbers(size);

                std::generate(numbers.begin(), numbers.end(), [&] { return rng.Uniform(1024); });
                auto first = Binarize(numbers);

                std::generate(numbers.begin(), numbers.end(), [&] { return rng.Uniform(1024); });
                auto second = Binarize(numbers);

                const auto cmi = ConditionalMutualInformation(first, second, second);
                //        Cerr << cmi << Endl;
                UNIT_ASSERT_DOUBLES_EQUAL_C(cmi, 0, 1e-6, step);
            }
        }

        SIMPLE_UNIT_TEST(Commutativity) {
            const int vectorCount = 3;

            TReallyFastRng32 rng(20151224);

            yvector<yvector<ui64>> data(vectorCount);
            for (auto& currentVector : data) {
                currentVector.resize(20);
                std::generate(currentVector.begin(), currentVector.end(), [&] { return rng.Uniform(1024); });
            }

            yvector<double> entropy;
            yvector<int> permutation(vectorCount);
            std::iota(permutation.begin(), permutation.end(), 0);
            do {
                entropy.push_back(Entropy(Binarize(data[permutation[0]]),
                                          Binarize(data[permutation[1]]),
                                          Binarize(data[permutation[2]])));
            } while (std::next_permutation(permutation.begin(), permutation.end()));

            for (auto entropyValue : entropy) {
                UNIT_ASSERT_DOUBLES_EQUAL(entropyValue, entropy.front(), 1e-7);
            }
        }

        SIMPLE_UNIT_TEST(BinEntropy) {
            UNIT_ASSERT_DOUBLES_EQUAL(Entropy(yvector<bool>(100, false)), 0.0, 1e-7);
            UNIT_ASSERT_DOUBLES_EQUAL(Entropy(yvector<bool>(100, true)), 0.0, 1e-7);

            double e1 = Entropy(yvector<bool>{false, true, false, true, false, true, false, false});
            double e2 = Entropy(yvector<bool>{false, true, false, true, false, true, false, true});
            UNIT_ASSERT(e2 > e1 + 1e-7);
        }
    }
}
