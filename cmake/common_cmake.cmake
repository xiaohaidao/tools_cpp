
if(CMAKE_VERSION VERSION_LESS 3.21)
    get_directory_property(hasParent PARENT_DIRECTORY)
    if(NOT hasParent)
        set(PROJECT_IS_TOP_LEVEL true)
    else()
        set(PROJECT_IS_TOP_LEVEL)
    endif()
endif()
if(PROJECT_IS_TOP_LEVEL)
    option(ENABLE_URL_PREFIX "enable url prefix" OFF)
    if(ENABLE_URL_PREFIX)
        if(NOT GITHUB_URL_PREFIX)
            set(GITHUB_URL_PREFIX https://gh.api.99988866.xyz/) # https://gh.api.99988866.xyz  https://ghproxy.com/ https://toolwa.com/github/
        endif()
    else()
        set(GITHUB_URL_PREFIX "")
    endif()
    message("ENABLE_URL_PREFIX: ${ENABLE_URL_PREFIX}, GITHUB_URL_PREFIX: ${GITHUB_URL_PREFIX}")

    # set(FETCHCONTENT_BASE_DIR ${CMAKE_SOURCE_DIR}/.third_party/)
    # set(FETCHCONTENT_SOURCE_DIR_COMMON_CMAKE ${CMAKE_SOURCE_DIR}/.third_party/common_cmake-src)
    set(FETCHCONTENT_QUIET OFF)
endif()

if (NOT GIT_COMMON_CMAKE_REPOSITORY)
    set(GIT_COMMON_CMAKE_REPOSITORY ${GITHUB_URL_PREFIX}https://github.com/xiaohaidao/common_cmake.git)
endif()
if (NOT COMMON_CMAKE_VERSION)
    set(COMMON_CMAKE_VERSION dev)
endif()

include(FetchContent)
FetchContent_Declare(
    common_cmake
    GIT_REPOSITORY ${GIT_COMMON_CMAKE_REPOSITORY}
    GIT_TAG ${COMMON_CMAKE_VERSION}
    GIT_SHALLOW ON
    GIT_PROGRESS ON
)
FetchContent_MakeAvailable(common_cmake)

configure_file(${CMAKE_CURRENT_LIST_DIR}/dependencies.cmake cmake/dependencies_${PROJECT_NAME}.cmake)
configure_file(${CMAKE_CURRENT_LIST_DIR}/dependencies_dev.cmake cmake/dependencies_dev_${PROJECT_NAME}.cmake)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_BINARY_DIR}/cmake/")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}")
list(APPEND CMAKE_MODULE_PATH "${common_cmake_SOURCE_DIR}/cmake")
# list(APPEND CMAKE_MODULE_PATH "${FETCHCONTENT_BASE_DIR}/common_cmake-src/cmake")

#import tool
include(common)
include(msvc_static_runtime)
