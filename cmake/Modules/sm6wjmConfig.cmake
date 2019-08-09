INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_SM6WJM sm6wjm)

FIND_PATH(
    SM6WJM_INCLUDE_DIRS
    NAMES sm6wjm/api.h
    HINTS $ENV{SM6WJM_DIR}/include
        ${PC_SM6WJM_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    SM6WJM_LIBRARIES
    NAMES gnuradio-sm6wjm
    HINTS $ENV{SM6WJM_DIR}/lib
        ${PC_SM6WJM_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
          )

include("${CMAKE_CURRENT_LIST_DIR}/sm6wjmTarget.cmake")

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(SM6WJM DEFAULT_MSG SM6WJM_LIBRARIES SM6WJM_INCLUDE_DIRS)
MARK_AS_ADVANCED(SM6WJM_LIBRARIES SM6WJM_INCLUDE_DIRS)
