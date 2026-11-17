add_executable(Rod-Editor)

rod_configure_cpp_target(Rod-Editor)

add_subdirectory(src)

target_include_directories(Rod-Editor PRIVATE src)

target_link_libraries(Rod-Editor PRIVATE
    Rod::Engine
    Rod::ImGui
    Rod::ImGuizmo
)

add_dependencies(Rod-Editor RodAssets-Copy RodSamples-Copy)
