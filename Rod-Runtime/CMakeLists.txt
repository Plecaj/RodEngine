include(RodSources)

add_executable(Rod-Runtime)

rod_configure_cpp_standard(Rod-Runtime)

rod_collect_sources(ROD_RUNTIME_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src")
rod_collect_headers(ROD_RUNTIME_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_sources(Rod-Runtime PRIVATE ${ROD_RUNTIME_SOURCES})

target_sources(Rod-Runtime PRIVATE
    FILE_SET HEADERS
    BASE_DIRS src
    FILES ${ROD_RUNTIME_HEADERS}
)

target_include_directories(Rod-Runtime PRIVATE src)

target_link_libraries(Rod-Runtime PRIVATE
    Rod::Engine
    Rod::ProjectOptions
    Rod::ProjectWarnings
)

rod_set_common_output_directories(Rod-Runtime)

if(TARGET RodGameScripts-Build)
    add_dependencies(Rod-Runtime RodGameScripts-Build)
endif()

add_custom_command(TARGET Rod-Runtime POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/Rod-Editor/assets"
        "$<TARGET_FILE_DIR:Rod-Runtime>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/Rod-ScriptCore/bin/Debug/net9.0"
        "$<TARGET_FILE_DIR:Rod-Runtime>/assets/Scripts/Core"
    COMMENT "Copying runtime assets"
)

