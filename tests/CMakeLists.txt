include(vendor/Catch2)

add_executable(RodEngineTests
    Math/Math.cpp
    Project/Serialization.cpp
    Renderer/Texture.cpp
    Scripting/ScriptGlue.cpp
    Scene/Scene.cpp
    Scene/SceneCamera.cpp
    Scene/Serialization.cpp
)

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
