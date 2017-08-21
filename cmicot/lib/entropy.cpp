#include "entropy.h"

#include <util/generic/hash.h>
#include <util/generic/ymath.h>

namespace NCmicot {
    double Entropy(const yvector<ui64>& values) {
        yhash<ui64, int> count;
        for (auto x : values) {
            ++count[x];
        }

        double result = 0.0;
        for (const auto& kv : count) {
            result += kv.second * Log2(1.0 * kv.second / values.size());
        }
        return -result / values.size();
    }
}
