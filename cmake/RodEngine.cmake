set(RODENGINE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/RodEngine)

file(GLOB_RECURSE RODENGINE_CPP_SOURCES
    ${RODENGINE_DIR}/src/*.cpp
    ${RODENGINE_DIR}/src/*.h
)

list(APPEND RODENGINE_CPP_SOURCES
    ${RODENGINE_DIR}/vendor/stb_image/stb_image.h    
    ${RODENGINE_DIR}/vendor/stb_image/stb_image_write.h
    ${RODENGINE_DIR}/vendor/stb_image/stb_image.cpp
    ${RODENGINE_DIR}/vendor/entt/entt.hpp
    ${RODENGINE_DIR}/vendor/json/json.hpp
    ${RODENGINE_DIR}/vendor/tinygltf/tiny_gltf.h
)

file(GLOB_RECURSE GLM_SOURCES
    ${RODENGINE_DIR}/vendor/glm/glm/*.hpp
    ${RODENGINE_DIR}/vendor/glm/glm/*.inl
)
list(APPEND RODENGINE_CPP_SOURCES ${GLM_SOURCES})

list(APPEND RODENGINE_CPP_SOURCES
    ${RODENGINE_DIR}/vendor/ImGuizmo/ImGuizmo.h
    ${RODENGINE_DIR}/vendor/ImGuizmo/ImGuizmo.cpp
)

add_library(RodEngine STATIC ${RODENGINE_CPP_SOURCES})

target_precompile_headers(RodEngine PRIVATE ${RODENGINE_DIR}/src/rdpch.h)

set_source_files_properties(
    ${RODENGINE_DIR}/vendor/ImGuizmo/ImGuizmo.cpp
    PROPERTIES SKIP_PRECOMPILE_HEADERS ON
)

target_include_directories(RodEngine PUBLIC
    ${RODENGINE_DIR}/src
    ${RODENGINE_DIR}/vendor/spdlog/include
    ${INCLUDE_DIR_GLFW}
    ${INCLUDE_DIR_GLAD}
    ${INCLUDE_DIR_IMGUI}
    ${INCLUDE_DIR_GLM}
    ${INCLUDE_DIR_STB_IMAGE}
    ${INCLUDE_DIR_ENTT}
    ${INCLUDE_DIR_YAML_CPP}
    ${INCLUDE_DIR_IMGUIZMO}
    ${INCLUDE_DIR_SHADERC}
    ${INCLUDE_DIR_JSON}
    ${INCLUDE_DIR_TINYGLTF}
)

if(WIN32)
    target_link_directories(RodEngine PUBLIC
        ${RODENGINE_DIR}/vendor/shaderc/build/libshaderc_util/$<CONFIG>
        ${RODENGINE_DIR}/vendor/shaderc/build/libshaderc/$<CONFIG>
        ${RODENGINE_DIR}/vendor/shaderc/build/third_party/glslang/glslang/$<CONFIG>
        ${RODENGINE_DIR}/vendor/shaderc/build/third_party/spirv-tools/source/$<CONFIG>
        ${RODENGINE_DIR}/vendor/shaderc/build/third_party/spirv-tools/source/opt/$<CONFIG>
    )
endif()

target_link_libraries(RodEngine PUBLIC
    GLFW
    Glad
    ImGui
    yaml-cpp
)

if(WIN32)
    target_link_libraries(RodEngine PUBLIC opengl32)
endif()

if(WIN32)
    add_dependencies(RodEngine Shaderc-Build)
    target_link_libraries(RodEngine PRIVATE
        shaderc_combined
        shaderc_util
        glslangd
        SPIRV-Tools-opt
        SPIRV-Tools
    )
endif()

target_compile_definitions(RodEngine PRIVATE
    _CRT_SECURE_NO_WARNINGS
)

if(WIN32)
    target_compile_definitions(RodEngine PUBLIC
        RD_PLATFORM_WINDOWS
        GLFW_INCLUDE_NONE
        YAML_CPP_STATIC_DEFINE
    )
endif()

target_compile_definitions(RodEngine PRIVATE
    $<$<CONFIG:Debug>:RD_DEBUG>
    $<$<CONFIG:Release>:RD_RELEASE>
    $<$<CONFIG:Dist>:RD_DIST>
)

string(TOLOWER "${CMAKE_SYSTEM_NAME}" SYSTEM_NAME_LOWER)
set_target_properties(RodEngine PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug-${SYSTEM_NAME_LOWER}-${CMAKE_ARCHITECTURE}/RodEngine"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release-${SYSTEM_NAME_LOWER}-${CMAKE_ARCHITECTURE}/RodEngine"
    ARCHIVE_OUTPUT_DIRECTORY_DIST "${CMAKE_BINARY_DIR}/bin/Dist-${SYSTEM_NAME_LOWER}-${CMAKE_ARCHITECTURE}/RodEngine"
)
