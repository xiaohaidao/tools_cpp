
include(FetchContent)
FetchContent_Declare(
    common_cpp
    GIT_REPOSITORY https://github.com/xiaohaidao/common_cpp.git
    GIT_TAG dev
)
FetchContent_MakeAvailable(common_cpp)
