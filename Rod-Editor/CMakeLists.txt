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

find_program(DOTNET_EXE dotnet)
if(DOTNET_EXE)
    add_custom_target(RodScriptCore-Build
        COMMAND ${DOTNET_EXE} build "${CMAKE_SOURCE_DIR}/Rod-ScriptCore/Rod.ScriptCore.csproj" --nologo
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Building Rod C# script core"
    )

    add_custom_target(RodGameScripts-Build
        COMMAND ${DOTNET_EXE} build "${CMAKE_CURRENT_SOURCE_DIR}/assets/Scripts/RodGame.csproj" --nologo /p:BuildProjectReferences=false
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Building Rod game scripts"
    )

    add_dependencies(RodGameScripts-Build RodScriptCore-Build)
    add_dependencies(Rod-Editor RodGameScripts-Build)
else()
    message(WARNING "dotnet was not found. C# scripting projects will not be built by CMake.")
endif()

add_custom_command(TARGET Rod-Editor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:Rod-Editor>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/Rod-ScriptCore/bin/Debug/net9.0"
        "$<TARGET_FILE_DIR:Rod-Editor>/assets/Scripts/Core"
    COMMENT "Copying editor assets"
)
