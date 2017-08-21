#include "bin_feature_set.h"

#include <utility>
#include <util/generic/algorithm.h>
#include <util/generic/xrange.h>
#include <util/generic/bt_exception.h>

namespace NCmicot {
    using TRangeEx = TWithBackTrace<yexception>;

    /* TBackground */

    void TBackground::SetFeatureEnabled(int index, bool enabled) {
        Y_ENSURE_EX(0 <= index && index < GetFeatureCount(), TRangeEx() << index << " " << GetFeatureCount());

        const auto indexRange = Features->GetFeatureBinIndexes(index);
        const auto begin = IsBinEnabled.begin() + *indexRange.begin();
        const auto end = IsBinEnabled.begin() + *indexRange.end();
        std::fill(begin, end, enabled);

        if (enabled) {
            LastEnabledBins = {*indexRange.begin(), *indexRange.end()};
        }
    }

    void TBackground::SetBinEnabled(int index, bool enabled) {
        Y_ENSURE_EX(0 <= index && index < GetBinCount(), TRangeEx() << index << " " << GetBinCount());

        IsBinEnabled[index] = enabled;
        if (enabled) {
            LastEnabledBins = {index, index + 1};
        }
    }

    yvector<int> TBackground::EnabledBinIndexes() const {
        yvector<int> result;

        for (auto i : xrange(IsBinEnabled.size())) {
            if (IsBinEnabled[i]) {
                result.push_back(i);
            }
        }

        return result;
    }

    yvector<TBin> TBackground::EnabledBins() const {
        yvector<TBin> result;
        for (auto i : xrange(IsBinEnabled.size())) {
            if (IsBinEnabled[i]) {
                result.push_back(GetBin(i));
            }
        }
        return result;
    }

    void TBackground::DisableAll() {
        Fill(IsBinEnabled.begin(), IsBinEnabled.end(), false);
    }

    yvector<int> TBackground::DisabledBinIndexes() const {
        yvector<int> result;

        for (auto i : xrange(IsBinEnabled.size())) {
            if (!IsBinEnabled[i]) {
                result.push_back(i);
            }
        }

        return result;
    }

    bool TBackground::IsFeatureEnabled(int index) const {
        Y_ENSURE_EX(0 <= index && index < GetFeatureCount(), TRangeEx() << index << " " << GetFeatureCount());

        const auto indexRange = Features->GetFeatureBinIndexes(index);

        const auto begin = IsBinEnabled.begin() + *indexRange.begin();
        const auto end = IsBinEnabled.begin() + *indexRange.end();

        return AllOf(begin, end, [](bool enabled) { return enabled; });
    }

    /* TBinFeatureSet */

    TBinFeatureSet::TBinFeatureSet(yvector<TBin> bins)
        : Bins(std::move(bins))
        , FeatureStart(1, 0)
    {
    }

    int TBinFeatureSet::AddFeature(yvector<TBin> feature) {
        FeatureStart.push_back(Bins.size());

        for (auto& bin : feature) {
            Bins.push_back(TBin());
            Bins.back() = std::move(bin);
        }

        return GetFeatureCount() - 1;
    }

    int TBinFeatureSet::GetFeatureCount() const {
        return FeatureStart.ysize();
    }

    const TBin& TBinFeatureSet::GetBin(int index) const {
        Y_ENSURE_EX(0 <= index && index < GetBinCount(), TRangeEx() << index << " " << GetBinCount());
        return Bins[index];
    }

    yvector<TBin> TBinFeatureSet::GetFeature(int index) const {
        Y_ENSURE_EX(0 <= index && index < GetFeatureCount(), TRangeEx() << index << " " << GetFeatureCount());

        auto begin = Bins.begin() + FeatureStart[index];
        auto end = index < FeatureStart.ysize() - 1 ? Bins.begin() + FeatureStart[index + 1]
                                                    : Bins.end();
        return {begin, end};
    }

    int TBinFeatureSet::GetFeatureIndexByBinIndex(int binIndex) const {
        Y_ENSURE_EX(0 <= binIndex && binIndex < GetBinCount(), TRangeEx() << binIndex << " " << GetBinCount());
        return LowerBound(FeatureStart.begin(), FeatureStart.end(), binIndex + 1) -
               FeatureStart.begin() - 1;
    }

    int TBinFeatureSet::GetBinCount() const {
        return Bins.ysize();
    }

    const yvector<TBin>& TBinFeatureSet::AllBins() const {
        return Bins;
    }

    decltype(xrange(0, 1)) TBinFeatureSet::GetFeatureBinIndexes(int index) const {
        Y_ENSURE_EX(0 <= index && index < GetFeatureCount(), TRangeEx() << index << " " << GetFeatureCount());

        const int begin = FeatureStart[index];
        const int end = index < FeatureStart.ysize() - 1 ? FeatureStart[index + 1]
                                                         : GetBinCount();
        return xrange(begin, end);
    }

    TBackground TBackground::LastEnabled() const {
        TBackground result(*Features);
        result.DisableAll();
        std::fill(result.IsBinEnabled.begin() + LastEnabledBins.first,
                  result.IsBinEnabled.begin() + LastEnabledBins.second, true);
        return result;
    }

    yvector<int> TBackground::EnabledFeatureIndexes() const {
        yvector<int> result;
        for (int i : xrange(GetFeatureCount())) {
            if (IsFeatureEnabled(i)) {
                result.push_back(i);
            }
        }
        return result;
    }

    void TBackground::EnabledAll() {
        Fill(IsBinEnabled.begin(), IsBinEnabled.end(), true);
    }
}
