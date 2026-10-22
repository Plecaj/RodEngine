include(vendor/Catch2)

add_executable(RodEngineTests
    Math/MathTests.cpp
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

catch_discover_tests(RodEngineTests)
