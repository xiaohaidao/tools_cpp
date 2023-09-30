
cmake_minimum_required(VERSION 3.14)

project("tools_cpp"
    VERSION 0.0.1
)

# import common configure
include(cmake/common_cmake.cmake)

# import configure
include(clang_format)
include(clang_tidy)
include(CPackSourceConfig)
include(doxygen)

#import dependencies
include(dependencies_${PROJECT_NAME})

add_subdirectory(src)

option(BUILD_TESTING "Build test" OFF)
include(CTest)
if(BUILD_TESTING AND PROJECT_IS_TOP_LEVEL)
    include(dependencies_dev_${PROJECT_NAME})

    ADD_SUBDIRECTORY(tests)
endif()

