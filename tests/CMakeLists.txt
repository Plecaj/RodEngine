include(vendor/Catch2)

add_executable(RodEngineTests
    Core/InputState.cpp
    Math/Math.cpp
    Project/Serialization.cpp
    Project/Export.cpp
    Launcher/RecentProjects.cpp
    ../Rod-Launcher/src/RecentProjects.cpp
    Editor/SceneManager.cpp
    ../Rod-Editor/src/EditorSceneManager.cpp
    Editor/Commands.cpp
    ../Rod-Editor/src/EditorCommands.cpp
    Platform/SharedLibrary.cpp
    Renderer/Mesh.cpp
    Renderer/BufferLayout.cpp
    Renderer/Texture.cpp
    Renderer/OpenGLTexture.cpp
    Renderer/Framebuffer.cpp
    Renderer/OpenGLMesh.cpp
    Renderer/Shader.cpp
    Renderer/ShaderCacheDatabase.cpp
    Scripting/ScriptAssembly.cpp
    Scripting/ScriptGlue.cpp
    Scripting/ScriptRuntime.cpp
    Scene/Scene.cpp
    Scene/SceneCamera.cpp
    Scene/Serialization.cpp
    Scene/Resources.cpp
)

add_library(RodTestLibrary SHARED Platform/TestLibrary.cpp)
add_dependencies(RodEngineTests RodTestLibrary)
target_compile_definitions(RodEngineTests PRIVATE ROD_TEST_LIBRARY_PATH="$<TARGET_FILE:RodTestLibrary>")

if(DOTNET_EXE AND ROD_DOTNET_HOST_INCLUDE_DIR AND ROD_DOTNET_HOST_LIBRARY AND TARGET RodGameScripts-Build)
    add_dependencies(RodEngineTests RodGameScripts-Build)
    target_compile_definitions(RodEngineTests PRIVATE
        ROD_TEST_CORE_ASSEMBLY_PATH="${ROD_SCRIPT_CORE_OUTPUT_DIR}/Rod.ScriptCore.dll"
        ROD_TEST_GAME_ASSEMBLY_PATH="${PROJECT_SOURCE_DIR}/Samples/Assets/Scripts/bin/Debug/net9.0/RodGame.dll"
    )
endif()

if(WIN32)
    target_sources(RodEngineTests PRIVATE Platform/Process.cpp)
    add_executable(RodProcessTestHelper Platform/ProcessTestHelper.cpp)
    target_compile_features(RodProcessTestHelper PRIVATE cxx_std_17)
    target_link_libraries(RodProcessTestHelper PRIVATE Rod::ProjectOptions Rod::ProjectWarnings)
    rod_set_common_output_directories(RodProcessTestHelper)
    add_dependencies(RodEngineTests RodProcessTestHelper)
    target_compile_definitions(RodEngineTests PRIVATE
        ROD_PROCESS_TEST_HELPER_PATH="$<TARGET_FILE:RodProcessTestHelper>"
    )
endif()

rod_configure_cpp_target(RodEngineTests)

target_compile_definitions(RodEngineTests PRIVATE
    CATCH_CONFIG_NO_COUNTER
    ROD_TEST_ASSET_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/Assets"
    ROD_TEST_SHADER_DIRECTORY="${PROJECT_SOURCE_DIR}/RodAssets/Shaders"
)
target_link_libraries(RodEngineTests PRIVATE
    Rod::Engine
    Rod::Glad
    Rod::GLFW
    Rod::YamlCpp
    Catch2::Catch2WithMain
)

if(TARGET RodDotnetHost-Copy)
    add_dependencies(RodEngineTests RodDotnetHost-Copy)
endif()

catch_discover_tests(RodEngineTests ADD_TAGS_AS_LABELS)
