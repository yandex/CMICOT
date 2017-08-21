## Overview

CMICOT is an efficient high-order interaction-aware feature selection based on conditional mutual information.
It was presented at [NIPS'2016](http://papers.nips.cc/paper/6584-efficient-high-order-interaction-aware-feature-selection-based-on-conditional-mutual-information), where theoretical guarantees are discussed and an experimental validation on a wide range of benchmark datasets is made. Source code for the following binary file is contributed:

* `./cmicot/cmicot` - used to select (rank) features.

To find more usage information you can run the binary with `--help` option.

## Installation

The following installation methods are supported:
* Download
* Build the binary from a local copy

### Download

Download the required binary depending on your OS:
* Linux (link will be there soon)
* MacOS (link will be there soon)

### Build the binary from a local copy

To install the command-line package from a local copy of the CMICOT repository:

1. Clone the repository:

```
git clone https://github.com/cmicot/cmicot.git
```
2. Open the cmicot/cmicot catalog from the local copy of the CMICOT repository.
3. Run the following command:
```
../../ya make -r [-o <output directory>]
```
Windows installation might require [Visual C++ 2015 Build Tools](http://landinghub.visualstudio.com/visual-cpp-build-tools).

## Quick start

All-in-one feature selection:
```bash
./cmicot --pool pool > feature_ranking
```

Binarization and feature selection in two commands:
```bash
./cmicot --pool pool --just-binarize pool_bin,map_bin --binarization minEntropy -x 20
./cmicot --map map_bin --binary-pool pool_bin > feature_ranking
```
* `pool` is a `tsv` file with the target variable in the first column and explanatory variables in the rest columns ([example](https://yadi.sk/d/vbTVJ2NT3ExTyu)).

* `pool_bin` is a `tsv` file with the discretized target variable in the first column and binarized explanatory variables in the rest columns (all the explanatory variables are presented as binary feature sets).

* `map_bin` is a `tsv` file with original feature indices (1st column) mapped to binary feature indices (2nd column). All indices are 0-based.

* `feature_ranking` is a ranking of the original feature indices (0-based). There is no importance score, just the selection order, where the first feature is the strongest.


## Feature Selection

Usage: ./cmicot [OPTIONS]

#### Required parameters

*--pool VAL*
A tab-separated file with features. The first column is the target feature (label), and the rest columns are the explanatory variables. Features can be binary, discrete or continuous. Features with more than 10 (default) unique values are transformed to 10-level variables using built-in binarization (if `-x VAL ` option is used, it is `VAL` instead of 10). 

Note that some feature information is lost during this process. It might be useful to set more levels for the target, than for the explanatory variables (you need to run `--just-binarize` twice and combine the necessary pool parts, see below).


#### Optional parameters

*--binary-pool VAL*

A tab-separated file with features. The first column is the target feature (label), which can be binary or discrete (continuous variables must be discretized). The rest columns are binary features constructed from the original ones (also known as binary representatives, see the NIPS paper).

Any non-binary feature must be preprocessed and transformed to a set of binary features. The mapping between the original feature indices and the binary representative indices is also required (even if all the features are binary).

You can use the in-built binarization mode to preprocess your dataset:
```bash
./cmicot --pool pool --just-binarize pool_bin,map_bin.
```

You can also obtain `pool_bin` and `map_bin` using your methods, as long as the input format is correct. [Example](https://yadi.sk/d/4RAMii7B3ErJxS) (coil2000 dataset).

*--map VAL*

A tab-separated file with a feature-bin map. The indices of original features (column 1) are to be mapped to the indices of every binary representative it spawned (column 2). If all of the original features are binary, both columns will contain a sequence from (0) to (the number of features). [Example](https://yadi.sk/d/FcDmdF403ErJxE) (coil2000 dataset). Another example: 2 representatives for original feature #0 and 3 for #1:
```bash
> 0 0
> 0 1
> 1 2
> 1 3
> 1 4
```

 *-t VAL*
 
The maximal number of features whose joint interaction could be taken into account by the algorithm (see the NIPS'2016 paper for more details). Default: 6. The recommended values are between 3 and 8. The maximum possible value is currently 64. You can mimic CMIM feature selection method by setting the value to 1.

*--thread-count VAL*

The number of threads to use during maximization and minimization (default: 8). The option doesn't affect the quality of the feature selection process.

*--select-count VAL*

The number of features to be selected (default: all input features are ranked). The option doesn't affect the quality of the feature selection process, it only stops the program as the desired number is reached.

*--binarization VAL*

Binarization mode. Should be one of: maxSumLog, medianInBin, minEntropy, medianPlusUniform, median (default: "medianPlusUniform"). The option has minimal to no effect on the feature selection quality in most cases, but might be useful with some peculiar datasets.

*-x VAL*

The maximum number of levels (for target feature) or binary representatives (for explanatory features). Default: 10. The option can greatly affect the quality of the feature selection.
Note that `VAL` is the maximum number, i.e. depending on the binarization mode (`--binarization`) and on the distribution of the original feature values the actual number can be less (e.g., `-x 5` will not transform a binary feature to a set of five binary features).

Transform every original variable (including the target) to a binary variable:
```bash
./cmicot --pool pool --just-binarize pool_bin,map_bin -x 1
```

Transform the target to a 3-level variable, each explanatory feature to a pair of binary representatives:
```bash
./cmicot --pool pool --just-binarize pool_bin,map_bin -x 2
```

*--just-binarize POOL,MAP*

Output binarized pool and feature-bin map for the pool indicated in `--pool VAL` instead of doing feature selection. Please provide filenames where pool and map should be stored separated by a comma. This option could be combined with `--binary-pool` and `--map`, if your purpose is to make a binary dataset (possibly with continuous target) even more binary.

