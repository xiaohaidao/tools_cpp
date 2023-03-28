
if (NOT TARGET clang_tidy)
set(CLANG_TIDY_TOOL clang-tidy)
add_custom_target(clang_tidy
    COMMAND
        # CXX=clang++ CC=clang
        ${CMAKE_COMMAND}
        -GNinja
        -DCMAKE_BUILD_TYPE=Debug
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
        -DCMAKE_C_CLANG_TIDY=${CLANG_TIDY_TOOL}
        -DCMAKE_CXX_CLANG_TIDY=${CLANG_TIDY_TOOL}
        -S${PROJECT_SOURCE_DIR} -B${PROJECT_BINARY_DIR}/build_clang_tidy
    COMMAND ${CMAKE_COMMAND} --build ${PROJECT_BINARY_DIR}/build_clang_tidy -j
    COMMENT "Check code with Clang-Tidy"
)
endif()
