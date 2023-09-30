
add_subdirectory(stun)

set(library_name stun)

if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/main.cpp)
    add_executable(${PROJECT_NAME} main.cpp)

    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${library_name}
    )
endif()
if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/bin)
    file(GLOB_RECURSE bin_files "bin/*.cpp")
    foreach(f ${bin_files})
        get_filename_component(name ${f} NAME_WE)
        add_executable(${name} ${f})

        target_link_libraries(${name} PRIVATE
            ${library_name}
        )
    endforeach()
endif()