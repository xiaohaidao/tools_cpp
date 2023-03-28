
include(CMakePackageConfigHelpers)

# install and export configure
set(library_name_target ${library_name}Targets)
install(TARGETS ${library_name}
    EXPORT ${library_name_target}
)

export(TARGETS ${library_name}
    NAMESPACE ${PROJECT_NAME}::
    FILE ${PROJECT_BINARY_DIR}/lib/cmake/${library_name_target}.cmake
)

set(cmake_files_install_dir "${CMAKE_INSTALL_LIBDIR}/cmake/${library_name}")
install(EXPORT ${library_name_target}
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION ${cmake_files_install_dir}
)

set(library_name_config ${library_name}Config)
configure_package_config_file(${PROJECT_SOURCE_DIR}/cmake/template/Config.cmake.in
    ${PROJECT_BINARY_DIR}/lib/cmake/${library_name_config}.cmake
    INSTALL_DESTINATION ${cmake_files_install_dir}
)
write_basic_package_version_file(
    ${PROJECT_BINARY_DIR}/lib/cmake/${library_name_config}Version.cmake
#     VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion # AnyNewerVersion|SameMajorVersion|SameMinorVersion|ExactVersion
)
install(FILES
    ${PROJECT_BINARY_DIR}/lib/cmake/${library_name_config}.cmake
    ${PROJECT_BINARY_DIR}/lib/cmake/${library_name_config}Version.cmake
    DESTINATION ${cmake_files_install_dir}
)
