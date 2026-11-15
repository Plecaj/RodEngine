set(ROD_SCRIPT_CORE_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/Rod-ScriptCore/bin/Debug/net9.0")

if(DOTNET_EXE)
    add_custom_target(RodScriptCore-Build
        COMMAND "${DOTNET_EXE}" build "${CMAKE_SOURCE_DIR}/Rod-ScriptCore/Rod.ScriptCore.csproj" --nologo
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        VERBATIM
    )

    add_custom_target(RodGameScripts-Build
        COMMAND "${DOTNET_EXE}" build "${CMAKE_SOURCE_DIR}/Samples/Assets/Scripts/RodGame.csproj"
            --nologo /p:BuildProjectReferences=false
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        VERBATIM
    )

    add_dependencies(RodGameScripts-Build RodScriptCore-Build)
else()
    message(WARNING "dotnet was not found. C# scripting projects will not be built by CMake.")
endif()

function(rod_copy_assets target)
    if(TARGET RodScriptCore-Build)
        add_dependencies(${target} RodScriptCore-Build)
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${ROD_SCRIPT_CORE_OUTPUT_DIR}"
                "$<TARGET_FILE_DIR:${target}>/RodAssets/Scripts/Core"
            VERBATIM
        )
    endif()

    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/RodAssets"
            "$<TARGET_FILE_DIR:${target}>/RodAssets"
        VERBATIM
    )
endfunction()

function(rod_copy_samples target)
    if(TARGET RodGameScripts-Build)
        add_dependencies(${target} RodGameScripts-Build)
    endif()

    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/Samples"
            "$<TARGET_FILE_DIR:${target}>/Samples"
        VERBATIM
    )
endfunction()
