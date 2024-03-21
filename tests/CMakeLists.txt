
include(StringVersion)

set(test_name "tools_test")
add_executable(${test_name}
    stun/StunClient.cpp
    codec/test_codec.cpp
    RpcServer/test_monad.cpp
    RpcServer/test_rpc.cpp
    # main.cpp
)

target_include_directories(${test_name} PRIVATE
    ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(${test_name} PRIVATE
    GTest::gtest_main
    stun
)

add_test(NAME ${test_name}
    COMMAND $<TARGET_FILE:${test_name}>
)

if(MSVC)
    set(VLD_DIR "C:/Program Files (x86)/Visual Leak Detector")
    if (EXISTS "${VLD_DIR}/include/vld.h")
        target_compile_definitions(${test_name} PRIVATE HAS_VLD_H)
    endif()
    target_include_directories(${test_name} PRIVATE
        "${VLD_DIR}/include"
    )
    target_link_directories(${test_name} PRIVATE
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            "${VLD_DIR}/lib/Win64"
        elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
            "${VLD_DIR}/lib/Win32"
        endif()
    )
endif()
