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
rod_copy_dotnet_host(Rod-Runtime)

rod_copy_assets(Rod-Runtime)

set(ROD_GAME_EXPORT_DIR "${CMAKE_BINARY_DIR}/export/RodGame")

add_custom_target(RodGame-Export
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "${ROD_GAME_EXPORT_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy
        "$<TARGET_FILE:Rod-Runtime>"
        "${ROD_GAME_EXPORT_DIR}/RodGame${CMAKE_EXECUTABLE_SUFFIX}"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "$<TARGET_FILE_DIR:Rod-Runtime>/assets"
        "${ROD_GAME_EXPORT_DIR}/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/Rod-Editor/assets/scenes/Example3D.rod"
        "${ROD_GAME_EXPORT_DIR}/assets/scenes/Startup.rod"
    DEPENDS Rod-Runtime
    VERBATIM
)

