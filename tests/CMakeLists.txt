include(vendor/Catch2)

add_executable(RodEngineTests
    Math/Math.cpp
    Scene/Scene.cpp
)

rod_configure_cpp_standard(RodEngineTests)

target_link_libraries(RodEngineTests PRIVATE
    Rod::Engine
    Rod::ProjectOptions
    Rod::ProjectWarnings
    Catch2::Catch2WithMain
)

rod_set_common_output_directories(RodEngineTests)
rod_copy_dotnet_host(RodEngineTests)

target_precompile_headers(RodEngineTests
  PUBLIC ../RodEngine/src/rdpch.h
)

catch_discover_tests(RodEngineTests)
