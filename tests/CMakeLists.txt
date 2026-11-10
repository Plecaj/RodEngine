include(vendor/Catch2)

add_executable(RodEngineTests
    Math/Math.cpp
    Project/Serialization.cpp
    Platform/SharedLibrary.cpp
    Renderer/Texture.cpp
    Scripting/ScriptAssembly.cpp
    Scripting/ScriptGlue.cpp
    Scripting/ScriptRuntime.cpp
    Scene/Scene.cpp
    Scene/SceneCamera.cpp
    Scene/Serialization.cpp
)

add_library(RodTestLibrary SHARED Platform/TestLibrary.cpp)
add_dependencies(RodEngineTests RodTestLibrary)
target_compile_definitions(RodEngineTests PRIVATE ROD_TEST_LIBRARY_PATH="$<TARGET_FILE:RodTestLibrary>")

if(DOTNET_EXE AND ROD_DOTNET_HOST_INCLUDE_DIR AND ROD_DOTNET_HOST_LIBRARY AND TARGET RodGameScripts-Build)
    add_dependencies(RodEngineTests RodGameScripts-Build)
    target_compile_definitions(RodEngineTests PRIVATE
        ROD_TEST_CORE_ASSEMBLY_PATH="${ROD_SCRIPT_CORE_OUTPUT_DIR}/Rod.ScriptCore.dll"
        ROD_TEST_GAME_ASSEMBLY_PATH="${PROJECT_SOURCE_DIR}/Rod-Editor/assets/Scripts/bin/Debug/net9.0/RodGame.dll"
    )
endif()

if(WIN32)
    target_sources(RodEngineTests PRIVATE Platform/Process.cpp)
endif()

rod_configure_cpp_target(RodEngineTests)

# Use Catch2's standard line-number fallback instead of the __COUNTER__ extension.
target_compile_definitions(RodEngineTests PRIVATE CATCH_CONFIG_NO_COUNTER)

target_link_libraries(RodEngineTests PRIVATE
    Rod::Engine
    Rod::ProjectOptions
    Rod::ProjectWarnings
    Catch2::Catch2WithMain
)

rod_set_common_output_directories(RodEngineTests)
rod_copy_dotnet_host(RodEngineTests)

catch_discover_tests(RodEngineTests)
