add_executable(Rod-Launcher
    src/RodLauncherApp.cpp
    src/ProjectSetupPanel.cpp
    src/ProjectSetupPanel.h
)

rod_configure_cpp_target(Rod-Launcher)

target_link_libraries(Rod-Launcher PRIVATE
    Rod::Engine
    Rod::ProjectOptions
    Rod::ProjectWarnings
)

rod_set_common_output_directories(Rod-Launcher)
target_compile_definitions(Rod-Launcher PRIVATE ROD_EXECUTABLE_SUFFIX="${CMAKE_EXECUTABLE_SUFFIX}")
if(TARGET Rod-Editor)
    add_dependencies(Rod-Launcher Rod-Editor)
endif()
rod_copy_assets(Rod-Launcher)
