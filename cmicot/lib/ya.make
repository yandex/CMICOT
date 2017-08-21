LIBRARY()



PEERDIR(
    library/getopt/small
    library/grid_creator
    library/threading/algorithm
    library/threading/future
)

SRCS(
    algorithm.cpp
    binarize.cpp
    bin_feature_set.cpp
    bin_score_normalize.cpp
    caching_bin_scorer.cpp
    cmi_calculator.cpp
    entropy.cpp
    entropy_calculator.cpp
    feature_score.cpp
    io.cpp
    miximizers.cpp
    mutual_information_calculator.cpp
    options.cpp
    bin_score.cpp
    selection.cpp
)

GENERATE_ENUM_SERIALIZATION(bin_score_normalize.h)

END()
