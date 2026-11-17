add_executable(Rod-Launcher)

add_subdirectory(src)

rod_configure_cpp_target(Rod-Launcher)

target_link_libraries(Rod-Launcher PRIVATE
    Rod::Engine
    Rod::ImGui
    Rod::YamlCpp
)

if(TARGET Rod-Editor)
    add_dependencies(Rod-Launcher Rod-Editor)
endif()
add_dependencies(Rod-Launcher RodAssets-Copy)
