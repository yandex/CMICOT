#include "test_pool_gen.h"

namespace NCmicot {
    yvector<int> InvertPermutation(const yvector<int>& permutation) {
        yvector<int> result(permutation.size());
        for (int i : xrange(permutation.size())) {
            result[permutation[i]] = i;
        }
        return result;
    }
}
