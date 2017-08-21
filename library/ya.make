RECURSE(
    colorizer
    colorizer/ut
    containers
    cppparser
    dbg_output
    dbg_output/ut
    diff
    diff/ut
    getopt
    getopt/last_getopt_demo
    getopt/small
    getopt/ut
    grid_creator
    json
    json/writer/ut
    lcs
    lcs/ut
    lfalloc
    lfalloc/dbg
    lfalloc/dbg_info
    lfalloc/yt
    malloc
    terminate_handler
    terminate_handler/sample
    threading
    unittest
    unittest/main
    unittest/ut
)

IF (OS_LINUX)
    RECURSE(
    
)
ENDIF()

IF (OS_LINUX AND NOT OS_ANDROID)
    RECURSE(
    
)
ENDIF()

IF (OS_WINDOWS)
    RECURSE(
    
)
ELSE()
    RECURSE(
    
)
ENDIF()

CHECK_DEPENDENT_DIRS(
    ALLOW_ONLY
    library
    contrib
    util
    yandex #TO REMOVE
    yweb/config
)
