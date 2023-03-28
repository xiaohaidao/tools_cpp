
cmake_minimum_required(VERSION 3.14)

project("tools"
    VERSION 0.0.1
)

# import common configure
include(cmake/common.cmake)

# import configure
include(clang_format)
include(clang_tidy)
include(CPackSourceConfig)

add_subdirectory(src)
# add_subdirectory(example)

option(BUILD_TESTING "Build test" OFF)
include(CTest)
if(BUILD_TESTING)
    include(googletest)

    ADD_SUBDIRECTORY(tests)
endif()

