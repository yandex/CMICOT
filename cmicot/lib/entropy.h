#pragma once

#include <util/generic/vector.h>
#include <util/generic/xrange.h>
#include <util/system/yassert.h>

namespace NCmicot {
    namespace Impl {
        template <typename C>
        void Flatten(yvector<ui64>& result, const C& bin) {
            if (result.empty()) {
                result.resize(bin.size(), 0);
            }

            auto resIter = result.begin();
            auto binIter = bin.begin();
            for (; resIter != result.end() && binIter != bin.end(); ++resIter, ++binIter) {
                *resIter <<= 1;
                if (*binIter) {
                    *resIter |= 1;
                }
            }
        }

        inline void FlattenBins(yvector<ui64>&) {
        }

        template <typename C, typename... Args>
        void FlattenBins(yvector<ui64>& result, const C& binContainer, const Args&... args) {
            for (const auto& bin : binContainer) {
                Impl::Flatten(result, bin);
            }
            FlattenBins(result, args...);
        }

        template <typename... Args>
        void FlattenBins(yvector<ui64>& result, const yvector<bool>& bin, const Args&... args) {
            Impl::Flatten(result, bin);
            FlattenBins(result, args...);
        }
    }

    template <typename... Args>
    yvector<ui64> FlattenBins(const Args&... args) {
        yvector<ui64> result;
        Impl::FlattenBins(result, args...);
        return result;
    }

    double Entropy(const yvector<ui64>& values);

    template <typename... Args>
    double Entropy(const Args&... args) {
        return Entropy(FlattenBins(args...));
    }

    template <class C1, class C2>
    double MutualInformation(const C1& first, const C2& second) {
        return Entropy(first) + Entropy(second) - Entropy(first, second);
    }

    template <class C1, class C2, class C3>
    double ConditionalMutualInformation(const C1& first, const C2& second, const C3& condition) {
        yvector<ui64> flattenedBins;

        Impl::FlattenBins(flattenedBins, condition);
        const auto conditionEntropy = Entropy(flattenedBins);

        auto flattenedBinsCopy = flattenedBins;

        Impl::FlattenBins(flattenedBins, first);
        const auto firstConditionEntropy = Entropy(flattenedBins);

        Impl::FlattenBins(flattenedBins, second);
        const auto firstSecondConditionEntropy = Entropy(flattenedBins);

        Impl::FlattenBins(flattenedBinsCopy, second);
        const auto secondConditionEntropy = Entropy(flattenedBinsCopy);

        //    return Entropy(condition, first) - Entropy(condition) - Entropy(condition, first, second) + Entropy(condition, second);
        return firstConditionEntropy - conditionEntropy - firstSecondConditionEntropy + secondConditionEntropy;
    }
}
