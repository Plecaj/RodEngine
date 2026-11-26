add_library(RodEngine STATIC)
add_library(Rod::Engine ALIAS RodEngine)

rod_configure_cpp_target(RodEngine)
target_compile_features(RodEngine PUBLIC cxx_std_26)

add_subdirectory(src)

find_package(Threads REQUIRED)

target_link_libraries(RodEngine
    PUBLIC
        Rod::ProjectOptions
        Rod::Glm
        Rod::Entt
        Rod::Spdlog
    PRIVATE
        Rod::Glad
        Rod::GLFW
        Rod::ImGui
        Rod::ImGuizmo
        Rod::YamlCpp
        Rod::Json
        Rod::Shaderc
        Rod::Stb
        Rod::TinyGltf
        Threads::Threads
        $<$<PLATFORM_ID:Windows>:opengl32;comdlg32;ole32;shell32>
        ${CMAKE_DL_LIBS}
)

if(TARGET Rod::DotnetHost)
    target_link_libraries(RodEngine PRIVATE Rod::DotnetHost)
endif()
