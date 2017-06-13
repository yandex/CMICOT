Disclaimer: source code of our algorithm will be available soon in the current repository.

## Overview

CMICOT is an efficient high-order interaction-aware feature selection based on conditional mutual information.
It was presented at [NIPS'2016.](http://papers.nips.cc/paper/6584-efficient-high-order-interaction-aware-feature-selection-based-on-conditional-mutual-information)

* `./cmicot/cmicot` is used to select the desired number of features from the dataset.

* `./cmicot/cmicot_eval` is used to evaluate the utility of a single feature considering other features of the dataset.

To find more usage information you can run both binaries with `--help` option.


## Quick start

```bash
> cat pool | ./cmicot_eval --bin-feature-map > map_bin
> cat pool | ./cmicot_eval --output-pool > pool_bin
> ./cmicot --map map_bin --pool pool_bin > feature_ranking
```
Pool is a `tsv` file with label in the first column and original features in the rest columns ([example](https://yadi.sk/d/vbTVJ2NT3ExTyu)).
Feature indices will be ranked in the output file (no importance score, just order of selection), the first is the strongest.

**Note!** Label must be binary or discrete (float labels can be discretized using the appropriate mode of `./cmicot_eval`).

## Feature Selection

Usage: ./cmicot [OPTIONS]

#### Required parameters

*--pool VAL*

Tab-separated file with features. The first column is the target feature (label), can be binary or discrete (not float/string/..). The rest columns are binary representatives, not original features. That is why we need the mapping between the original feature indices and the binary representative indices. So you have to preprocess your original pool for it to be compatible with the sbfs binary. Another part of our software does that for now (produces both map and pool files), see below. Or you can use any implementation you have (or implement it yourself). [Example](https://yadi.sk/d/4RAMii7B3ErJxS) (coil2000 dataset).

*--map VAL*

File with feature-bin map. Tab-separated file with indices of original features (column 1) mapped to indices of every binary representative it spawned (column 2). [Example](https://yadi.sk/d/FcDmdF403ErJxE) (coil2000 dataset). Another example: 2 representatives for original feature #0 and 4 for #1:
```bash
> 0 0
> 0 1
> 1 2
> 1 3
> 1 4
> 1 5
```

#### Optional parameters
 *-k VAL*
 
Eval algorithm step count (default: 6). This is the *t* value presented in the paper.

*--thread-count VAL*

Threads to use during maximization and minimization (default: 8)

*--select-count VAL*

How many features should be selected (default: all features in pool are ranked).


## Feature Evaluation

**Experimental mode!**

Usage: ./cmicot_eval [OPTIONS] [filename]

#### Required parameters

*-t VAL*

Feature index to be evaluated, 0-based.

#### Optional parameters

*-i VAL*

Feature indexes (comma-separated) to be used as background, 0-based. May be used multiple times. Default: all features are backgound.

*-d VAL*

Feature indexes (comma-separated) to be removed from background, 0-based. May be used multiple times. Default: no features are removed.

*-k VAL*

Eval algorithm step count (default: 6). Same as above.

*--steps max,min*        

Maximization and minimization steps count separated by comma.

*--bin-score-norm VAL*

Binary representative score normalization mode. Possible values are 'None', 'LabelEntropy', 'BinEntropy', 'BinAndLabelEntropy' (default: 'LabelEntropy').

*--cmim*

Calculate CMIM score. Options *--output-bins*, *--bin-score-norm* and *-k* are meaningless with this option.

*--thread-count VAL*

Threads to use during maximization and minimization (default: 8).

*--output-bins*

(Debugging) Print tab-separated binary representative indices used by the scoring algorithm.

*--full-result*

(Debugging) For each algorithm step print binary representative index and its CMI (conditional mutual information).

*--bin-feature-map*

(Binarization) Print index mapping of features and binary representatives.

*--output-pool*

(Binarization) Print binarized pool - pool where all the features except the first column (target feature) are transformed to sets of binary representatives.

*--binarization VAL*

(Binarization) Binarization mode. Should be one of: maxSumLog, medianInBin, minEntropy, medianPlusUniform, median (default: "medianPlusUniform").

*-x VAL*

(Binarization) The maximum number of binary representatives (default: 10).

*--output-feature-sizes*

(Binarization) (Debugging) Print the number of binary representatives for each feature after binarization.

#### Free args

*filename*

File with pool. First column is expected to be label, other columns are treated as features. Label must be binary/discrete. Default: `stdin`.

![Teaser Image: Our poster at NIPS](poster.png)
