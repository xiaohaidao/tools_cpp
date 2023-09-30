
set(library_name "stun")
add_library(${library_name}
    stun.cpp
    StunClient.cpp
    # StunCommon.cpp
)

target_link_libraries(${library_name} PUBLIC
    proactor
    sockets
    utils
)

target_include_directories(${library_name} PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    PUBLIC $<INSTALL_INTERFACE:include/>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/>
)

target_compile_features(${library_name} PRIVATE
    cxx_std_11
)

# install and export configure
include(install_export)
