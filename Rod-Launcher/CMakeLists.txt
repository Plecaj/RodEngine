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
rod_copy_assets(Rod-Launcher)
