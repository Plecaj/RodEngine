set(RODEDITOR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Rod-Editor)
set(RODENGINE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/RodEngine)

file(GLOB_RECURSE RODEDITOR_SOURCES
    ${RODEDITOR_DIR}/src/*.cpp
    ${RODEDITOR_DIR}/src/*.h
)

add_executable(Rod-Editor ${RODEDITOR_SOURCES})

target_link_libraries(Rod-Editor PRIVATE RodEngine)

if(WIN32)
    add_dependencies(Rod-Editor Shaderc-Build)
endif()

target_include_directories(Rod-Editor PRIVATE
    ${RODENGINE_DIR}/vendor/spdlog/include
    ${RODENGINE_DIR}/src
    ${INCLUDE_DIR_GLM}
    ${INCLUDE_DIR_IMGUI}
    ${INCLUDE_DIR_ENTT}
    ${INCLUDE_DIR_IMGUIZMO}
    ${INCLUDE_DIR_SHADERC}
)

if(WIN32)
    target_compile_definitions(Rod-Editor PRIVATE
        RD_PLATFORM_WINDOWS
    )
endif()

target_compile_definitions(Rod-Editor PRIVATE
    $<$<CONFIG:Debug>:RD_DEBUG>
    $<$<CONFIG:Release>:RD_RELEASE>
    $<$<CONFIG:Dist>:RD_DIST>
)

string(TOLOWER "${CMAKE_SYSTEM_NAME}" SYSTEM_NAME_LOWER)
set_target_properties(Rod-Editor PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug-${SYSTEM_NAME_LOWER}-${CMAKE_ARCHITECTURE}/Rod-Editor"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release-${SYSTEM_NAME_LOWER}-${CMAKE_ARCHITECTURE}/Rod-Editor"
    RUNTIME_OUTPUT_DIRECTORY_DIST "${CMAKE_BINARY_DIR}/bin/Dist-${SYSTEM_NAME_LOWER}-${CMAKE_ARCHITECTURE}/Rod-Editor"
)

set(ASSETS_SOURCE_DIR ${RODEDITOR_DIR}/assets)

add_custom_command(TARGET Rod-Editor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:Rod-Editor>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${ASSETS_SOURCE_DIR}"
        "$<TARGET_FILE_DIR:Rod-Editor>/assets"
    COMMENT "Copying assets to output directory"
)
