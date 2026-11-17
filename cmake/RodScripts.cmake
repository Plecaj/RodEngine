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

add_custom_target(RodAssets-Copy
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
        "${CMAKE_SOURCE_DIR}/RodAssets"
        "${ROD_RUNTIME_OUTPUT_DIR}/RodAssets"
    COMMENT "Copying engine assets"
    VERBATIM
)

if(TARGET RodScriptCore-Build)
    add_dependencies(RodAssets-Copy RodScriptCore-Build)
    add_custom_command(TARGET RodAssets-Copy POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_directory
            "${ROD_SCRIPT_CORE_OUTPUT_DIR}"
            "${ROD_RUNTIME_OUTPUT_DIR}/RodAssets/Scripts/Core"
        VERBATIM
    )
endif()

if(TARGET RodDotnetHost-Copy)
    add_dependencies(RodAssets-Copy RodDotnetHost-Copy)
endif()

add_custom_target(RodSamples-Copy
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
        "${CMAKE_SOURCE_DIR}/Samples"
        "${ROD_RUNTIME_OUTPUT_DIR}/Samples"
    COMMENT "Copying samples"
    VERBATIM
)

if(TARGET RodGameScripts-Build)
    add_dependencies(RodSamples-Copy RodGameScripts-Build)
endif()
