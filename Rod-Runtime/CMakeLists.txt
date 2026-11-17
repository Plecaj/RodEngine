add_executable(Rod-Runtime)

rod_configure_cpp_target(Rod-Runtime)

add_subdirectory(src)

target_include_directories(Rod-Runtime PRIVATE src)

target_link_libraries(Rod-Runtime PRIVATE
    Rod::Engine
    Rod::ProjectOptions
    Rod::ProjectWarnings
)

rod_set_common_output_directories(Rod-Runtime)
rod_copy_dotnet_host(Rod-Runtime)

rod_copy_assets(Rod-Runtime)
rod_copy_samples(Rod-Runtime)

set(ROD_GAME_EXPORT_DIR "${CMAKE_BINARY_DIR}/export/RodGame")

add_executable(Rod-Package tools/PackageMain.cpp)
rod_configure_cpp_target(Rod-Package)
target_link_libraries(Rod-Package PRIVATE Rod::Engine Rod::ProjectOptions Rod::ProjectWarnings)
target_compile_definitions(Rod-Package PRIVATE ROD_EXECUTABLE_SUFFIX="${CMAKE_EXECUTABLE_SUFFIX}")
rod_set_common_output_directories(Rod-Package)
add_dependencies(Rod-Package Rod-Runtime)

add_custom_target(RodGame-Export
    COMMAND "$<TARGET_FILE:Rod-Package>"
        "${CMAKE_SOURCE_DIR}/Samples/Samples.rodproj"
        "${ROD_GAME_EXPORT_DIR}"
    DEPENDS Rod-Package
    VERBATIM
)

