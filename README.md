Disclaimer: source code of our algorithm will be available soon in the current repository.

## Overview

CMICOT is an efficient high-order interaction-aware feature selection based on conditional mutual information.
It was presented at [NIPS'2016](http://papers.nips.cc/paper/6584-efficient-high-order-interaction-aware-feature-selection-based-on-conditional-mutual-information), where theoretical guarantees are discussed and an experimental validation on a wide range of benchmark datasets is made. The following binary files are contributed:

* `./cmicot/cmicot` is used to select (rank) features.

* `./cmicot/cmicot_eval` is used to evaluate the utility of a single feature considering other features.

To find more usage information you can run both binaries with `--help` option.


## Quick start

Default binarization:
```bash
> ./cmicot --pool pool > feature_ranking
```

Custom binarization:
```bash
> cat pool | ./cmicot_eval --bin-feature-map > map_bin
> cat pool | ./cmicot_eval --output-pool > pool_bin
> ./cmicot --map map_bin --binary-pool pool_bin > feature_ranking
```
* `pool` is a `tsv` file with the target variable in the first column and explanatory variables in the rest columns ([example](https://yadi.sk/d/vbTVJ2NT3ExTyu)).

* `pool_bin` is a `tsv` file, where all the explanatory variables are presented as binary feature sets.

* `map_bin` is a `tsv` file with original feature indices (1st column) mapped to binary feature indices (2nd column). All indices are 0-based.

* `feature_ranking` is a ranking of the original feature indices (no importance score, just the selection order), the first feature is the strongest.


## Feature Selection

Usage: ./cmicot [OPTIONS]

#### Required parameters

*--pool VAL*
A tab-separated file with features. The first column is the target feature (label), which can be either discrete or continuous. Note that if the target column takes more than 10 unique values it is transformed to a 10-level variable using in-built binarization (so some target information is lost). The rest columns are explanatory variables (discrete or continuous), which are also transformed to discrete variables during the selection process.

*--binary-pool VAL*

A tab-separated file with features. The first column is the target feature (label), which can be binary or discrete (continuous variables must be discretized with your own means). The rest columns are binary features constructed from original ones (binary representatives, see the article).
Any non-binary feature must be preprocessed and transformed to a set of binary features. The mapping between the original feature indices and the binary representative indices is also required (even ef all the features are binary, see below).
You can use `./cmicot_eval` binarization mode to preprocess your dataset. You can also obtain `pool_bin` and `map_bin` using your methods, as long as the input format is correct. [Example](https://yadi.sk/d/4RAMii7B3ErJxS) (coil2000 dataset).

*--map VAL*

A tab-separated file with a feature-bin map. The indices of original features (column 1) are to be mapped to the indices of every binary representative it spawned (column 2). If all the original features are binary, both columns will contain a sequence from (0) to (the number of features). [Example](https://yadi.sk/d/FcDmdF403ErJxE) (coil2000 dataset). Another example: 2 representatives for original feature #0 and 4 for #1:
```bash
> 0 0
> 0 1
> 1 2
> 1 3
> 1 4
> 1 5
```

#### Optional parameters

 *-t VAL*
 
The maximal number of features whose joint interaction could be taken into account by the algorithm (see the NIPS'2016 paper for more details).

*--thread-count VAL*

The number of threads to use during maximization and minimization (default: 8).

*--select-count VAL*

The number of features to be selected (default: all input features are ranked).


## Feature Evaluation

**Experimental mode!**

Usage: ./cmicot_eval [OPTIONS] [filename]

#### Required parameters

*-k VAL*

The index of the feature to be evaluated, 0-based.

#### Optional parameters

*-i VAL*

Feature indices (comma-separated) to be used as background, 0-based. May be used multiple times. Default: all features are backgound.

*-d VAL*

Feature indices (comma-separated) to be removed from background, 0-based. May be used multiple times. Default: no features are removed.

*-t VAL*

The maximal number of features whose joint interaction could be taken into account by the algorithm (see the NIPS'2016 paper for more details). Same as above.

*--cmim*

Calculate CMIM score. Option *-k* is meaningless with *--cmim*.

*--thread-count VAL*

The number of threads to use during maximization and minimization (default: 8).

*--bin-feature-map*

(Binarization) Print the index mapping of features and binary representatives.

*--output-pool*

(Binarization) Print the preprocessed binarized dataset - all the features except the first column (the target feature) are transformed to sets of binary features (representatives).

*--binarization VAL*

(Binarization) Binarization mode. Should be one of: maxSumLog, medianInBin, minEntropy, medianPlusUniform, median (default: "medianPlusUniform").

*-x VAL*

(Binarization) The maximum number of binary representatives (default: 10).


#### Free args

*filename*

A tab-separated file with features. The first column is expected to be the target feature, the other columns are treated as explanatory features. The target feature must be binary/discrete. Default: `stdin`.
