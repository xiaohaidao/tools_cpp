
include(StringVersion)

set(test_name "tools_test")
add_executable(${test_name}
    stun/stun.cpp
    main.cpp
)

target_include_directories(${test_name} PRIVATE
    ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(${test_name} PRIVATE
    gtest_main
    stun
)

add_test(NAME ${test_name}
    COMMAND $<TARGET_FILE:${test_name}>
)
