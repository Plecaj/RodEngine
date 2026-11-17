add_library(RodEngine STATIC)
add_library(Rod::Engine ALIAS RodEngine)

rod_configure_cpp_target(RodEngine)

add_subdirectory(src)

target_include_directories(RodEngine PUBLIC src)

find_package(Threads REQUIRED)

target_link_libraries(RodEngine
    PUBLIC
        Rod::ProjectOptions
        Rod::ProjectWarnings
        Rod::Glad
        Rod::GLFW
        Rod::Glm
        Rod::ImGui
        Rod::ImGuizmo
        Rod::Entt
        Rod::YamlCpp
        Rod::Json
        Rod::Spdlog
        Rod::Shaderc
        Rod::Stb
        Rod::TinyGltf
    PRIVATE
        Threads::Threads
        $<$<PLATFORM_ID:Windows>:opengl32;comdlg32;ole32;shell32>
        ${CMAKE_DL_LIBS}
)

target_compile_definitions(RodEngine PRIVATE
    $<$<PLATFORM_ID:Windows>:_CRT_SECURE_NO_WARNINGS>
)

include(RodDotnet)

rod_set_common_output_directories(RodEngine)
