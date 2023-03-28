
set(example_name "your_name")
add_executable(${example_name}
)

target_include_directories(${example_name} PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    #${CMAKE_CURRENT_SOURCE_DIR}
    #${CMAKE_SOURCE_DIR}/include/
)

target_link_libraries(${example_name} PRIVATE
)

install(TARGETS ${example_name}
)

