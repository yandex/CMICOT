UNITTEST_FOR(cmicot/lib)



SRCS(
    algorithm_ut.cpp
    bin_feature_set_ut.cpp
    bin_score_ut.cpp
    binarize_ut.cpp
    caching_bin_scorer_ut.cpp
    entropy_ut.cpp
    entropy_calculator_ut.cpp
    feature_score_ut.cpp
    io_ut.cpp
    miximizers_ut.cpp
    selection_ut.cpp

    test_pool_gen.cpp
)

FORK_SUBTESTS()

END()
