include(RodSources)

add_library(RodEngine STATIC)
add_library(Rod::Engine ALIAS RodEngine)

rod_configure_cpp_standard(RodEngine)

rod_collect_sources(ROD_ENGINE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src")
rod_collect_headers(ROD_ENGINE_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_sources(RodEngine PRIVATE ${ROD_ENGINE_SOURCES})

target_sources(RodEngine PUBLIC
    FILE_SET HEADERS
    BASE_DIRS src
    FILES ${ROD_ENGINE_HEADERS}
)

target_include_directories(RodEngine PUBLIC src)

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
        $<$<PLATFORM_ID:Windows>:opengl32>
)

target_compile_definitions(RodEngine PRIVATE _CRT_SECURE_NO_WARNINGS)

if(ROD_ENABLE_PCH)
    target_precompile_headers(RodEngine PRIVATE src/rdpch.h)
endif()

rod_set_common_output_directories(RodEngine)
