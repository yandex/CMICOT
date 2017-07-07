Disclaimer: source code of our algorithm will be available soon in the current repository.

## Overview

CMICOT is an efficient high-order interaction-aware feature selection based on conditional mutual information.
It was presented at [NIPS'2016](http://papers.nips.cc/paper/6584-efficient-high-order-interaction-aware-feature-selection-based-on-conditional-mutual-information), where theoretical guarantees are discussed and an experimental validation on a wide range of benchmark datasets is made. Source code for the following binary file is contributed:

* `./cmicot/cmicot` - used to select (rank) features.

To find more usage information you can run the binary with `--help` option.


## Quick start

All-in-one feature selection:
```bash
> ./cmicot --pool pool > feature_ranking
```

Separate binarization and feature selection:
```bash
> cat pool | ./cmicot --just-binarize pool_bin,map_bin --binarization minEntropy -x 20
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


#### Optional parameters

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

 *-t VAL*
 
The maximal number of features whose joint interaction could be taken into account by the algorithm (see the NIPS'2016 paper for more details).

*--thread-count VAL*

The number of threads to use during maximization and minimization (default: 8).

*--select-count VAL*

The number of features to be selected (default: all input features are ranked).

*--binarization VAL*

Binarization mode. Should be one of: maxSumLog, medianInBin, minEntropy, medianPlusUniform, median (default: "medianPlusUniform").

*-x VAL*

The maximum number of binary representatives (default: 10).

*--just-binarize POOL,MAP*

Output binarized pool and feature-bin map instead of doing feature selection. Please provide filenames where pool and map should be stored separated by a comma.

