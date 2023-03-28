
set(library_name "your_name")
add_library(${library_name}
)

target_link_libraries(${library_name} PUBLIC
)

target_include_directories(${library_name} PRIVATE
    ${PROJECT_SOURCE_DIR}/src/
    PUBLIC $<INSTALL_INTERFACE:include/>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/>
)

# set_target_properties(${library_name}
#     PROPERTIES
#     PUBLIC_HEADER $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/>
# )

target_compile_features(${library_name} PUBLIC
#    cxx_std_11
)

# install and export configure
include(install_export)
