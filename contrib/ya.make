RECURSE(
    libs
    tools
)


IF (YMAKE)
    RECURSE(
    
)
ENDIF ()


CHECK_DEPENDENT_DIRS(
    ALLOW_ONLY
    contrib
    util
    library/svnversion
    library/archive
    library/charset
)
