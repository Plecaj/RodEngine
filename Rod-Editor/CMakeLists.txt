add_executable(Rod-Editor)

rod_configure_cpp_target(Rod-Editor)

add_subdirectory(src)

target_include_directories(Rod-Editor PRIVATE src)

target_link_libraries(Rod-Editor PRIVATE
    Rod::Engine
    Rod::ImGui
    Rod::ImGuizmo
    Rod::ProjectOptions
    Rod::ProjectWarnings
)

rod_set_common_output_directories(Rod-Editor)
rod_copy_dotnet_host(Rod-Editor)

target_compile_definitions(Rod-Editor PRIVATE ROD_EXECUTABLE_SUFFIX="${CMAKE_EXECUTABLE_SUFFIX}")

rod_copy_assets(Rod-Editor)
rod_copy_samples(Rod-Editor)
