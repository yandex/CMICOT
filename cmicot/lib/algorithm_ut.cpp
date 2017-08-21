#include "algorithm.h"

#include <library/unittest/registar.h>

#include <util/generic/vector.h>
#include <util/generic/xrange.h>

namespace NCmicot {
    SIMPLE_UNIT_TEST_SUITE(ParallelAlgorithms) {
        SIMPLE_UNIT_TEST(ThreadCountDoesntAffectResult) {
            const int testData[] = {10, 6, 8, 7, 1, 2, 5, 4, 3, 9};
            auto identity = [](int x) { return x; };

            yvector<int> result;
            for (int threadCount : xrange(10)) {
                result.push_back(*ParallelMinElementBy(testData, identity, threadCount));
            }

            UNIT_ASSERT(AllOf(result, [](int value) { return value == 1; }));
        }

        SIMPLE_UNIT_TEST(TestParallelMinElementBy) {
            const int array[] = {1, 2, 5, 3, 4, 5};
            UNIT_ASSERT_VALUES_EQUAL(*ParallelMinElementBy(array, [](int x) {
                return x * x;
            }),
                                     1);

            const yvector<int> vec(std::begin(array), std::end(array));
            UNIT_ASSERT_VALUES_EQUAL(*ParallelMinElementBy(vec, [](int x) {
                return -1.0 * x;
            }),
                                     5);
        }

        SIMPLE_UNIT_TEST(TestParallelMaxElementBy) {
            const int array[] = {1, 2, 5, 3, 4, 5};
            UNIT_ASSERT_VALUES_EQUAL(*ParallelMaxElementBy(array, [](int x) {
                return x * x;
            }),
                                     5);

            const yvector<int> vec(std::begin(array), std::end(array));
            UNIT_ASSERT_VALUES_EQUAL(*ParallelMaxElementBy(vec, [](int x) {
                return -1.0 * x;
            }),
                                     1);
        }

        SIMPLE_UNIT_TEST(EmptyRange) {
            const yvector<int> empty;
            UNIT_ASSERT_EQUAL(ParallelMinElementBy(empty, [](int) { return 0; }), empty.end());
            UNIT_ASSERT_EQUAL(ParallelMaxElementBy(empty, [](int) { return 0; }), empty.end());
        }
    }
}
