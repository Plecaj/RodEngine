include(RodSources)

add_executable(Rod-Editor)

rod_configure_cpp_standard(Rod-Editor)

rod_collect_sources(ROD_EDITOR_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src")
rod_collect_headers(ROD_EDITOR_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_sources(Rod-Editor PRIVATE ${ROD_EDITOR_SOURCES})

target_sources(Rod-Editor PRIVATE
    FILE_SET HEADERS
    BASE_DIRS src
    FILES ${ROD_EDITOR_HEADERS}
)

target_include_directories(Rod-Editor PRIVATE src)

target_link_libraries(Rod-Editor PRIVATE
    Rod::Engine
    Rod::ProjectOptions
    Rod::ProjectWarnings
)

rod_set_common_output_directories(Rod-Editor)

add_custom_command(TARGET Rod-Editor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:Rod-Editor>/assets"
    COMMENT "Copying editor assets"
)
