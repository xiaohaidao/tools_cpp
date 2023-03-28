file(GLOB_RECURSE  cxx_format_files
    "include/*.h"
    "include/*.hpp"
    "example/*.h"
    "example/*.hpp"
    "example/*.cpp"
    "example/*.c"
    "src/*.h"
    "src/*.hpp"
    "src/*.cpp"
    "src/*.c"
    "tests/*.h"
    "tests/*.cpp"
)

if (NOT TARGET clang_format)
add_custom_target(clang_format
    COMMAND clang-format -style=file -i ${cxx_format_files}
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
    COMMENT "Format code with clang-format"
)
endif()
