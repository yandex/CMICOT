#pragma once

#include <cmicot/lib/bin.h>

#include <util/generic/vector.h>
#include <util/generic/xrange.h>

namespace NCmicot {
    class TBinFeatureSet {
    public:
        TBinFeatureSet() = default;

        explicit TBinFeatureSet(yvector<TBin> bins);

        int AddFeature(yvector<TBin> feature);

        int GetFeatureCount() const;
        int GetBinCount() const;

        const TBin& GetBin(int index) const;

        const yvector<TBin>& AllBins() const;

        yvector<TBin> GetFeature(int index) const;
        int GetFeatureIndexByBinIndex(int binIndex) const;

        decltype(xrange(0, 1)) GetFeatureBinIndexes(int index) const;

    protected:
        yvector<TBin> Bins;
        yvector<int> FeatureStart;

    public:
        auto AllBinIndexes() const -> decltype(xrange(Bins.size())) {
            return xrange(Bins.size());
        }
    };

    class TBackground {
    public:
        explicit TBackground(const TBinFeatureSet& features)
            : Features(&features)
            , IsBinEnabled(features.GetBinCount(), true)
            , LastEnabledBins(0, 0)
        {
        }

        void SetFeatureEnabled(int index, bool enabled);

        TBackground LastEnabled() const;

        void SetBinEnabled(int index, bool enabled);
        yvector<int> EnabledBinIndexes() const;
        yvector<TBin> EnabledBins() const;

        void DisableAll();
        void EnabledAll();

        yvector<int> DisabledBinIndexes() const;
        bool IsFeatureEnabled(int index) const;

        int GetBinCount() const {
            return Features->GetBinCount();
        }

        const TBin& GetBin(int index) const {
            return Features->GetBin(index);
        }

        int GetFeatureIndexByBinIndex(int binIndex) const {
            return Features->GetFeatureIndexByBinIndex(binIndex);
        }

        int GetFeatureCount() const {
            return Features->GetFeatureCount();
        }

        decltype(xrange(0, 1)) GetFeatureBinIndexes(int index) const {
            return Features->GetFeatureBinIndexes(index);
        }

        yvector<int> EnabledFeatureIndexes() const;

    private:
        TBackground(const TBinFeatureSet&&);

        const TBinFeatureSet* Features;
        yvector<bool> IsBinEnabled;
        std::pair<int, int> LastEnabledBins;
    };
};
