add_library(RodProjectOptions INTERFACE)
add_library(Rod::ProjectOptions ALIAS RodProjectOptions)

add_library(RodProjectWarnings INTERFACE)
add_library(Rod::ProjectWarnings ALIAS RodProjectWarnings)

function(rod_configure_cpp_target target)
    set_target_properties(${target} PROPERTIES
        CXX_STANDARD ${ROD_CXX_STANDARD}
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        DISABLE_PRECOMPILE_HEADERS OFF
    )
    target_precompile_headers(${target} PRIVATE "${PROJECT_SOURCE_DIR}/RodEngine/src/rdpch.h")
endfunction()

target_compile_definitions(RodProjectOptions INTERFACE
    $<$<CONFIG:Debug>:RD_DEBUG>
    $<$<CONFIG:Release>:RD_RELEASE>
    $<$<CONFIG:Dist>:RD_DIST>
)

target_compile_definitions(RodProjectOptions INTERFACE GLFW_INCLUDE_NONE)

if(WIN32)
    target_compile_definitions(RodProjectOptions INTERFACE RD_PLATFORM_WINDOWS)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_definitions(RodProjectOptions INTERFACE RD_PLATFORM_LINUX)
else()
    message(FATAL_ERROR "RodEngine supports Windows and Linux.")
endif()

if(MSVC)
    target_compile_options(RodProjectWarnings INTERFACE /W4 /permissive-)
    target_compile_options(RodProjectOptions INTERFACE
        /EHsc
        /utf-8
        /Zc:__cplusplus
        $<$<CONFIG:Debug>:/Od;/Zi>
        $<$<CONFIG:Release>:/O2;/Zi>
        $<$<CONFIG:Dist>:/O2>
    )
    target_link_options(RodProjectOptions INTERFACE
        $<$<CONFIG:Release>:/DEBUG>
    )
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_compile_options(RodProjectOptions INTERFACE
            /MP
            /Zc:preprocessor
            /diagnostics:caret
            /external:anglebrackets
            /external:W0
            $<$<CONFIG:Dist>:/GL>
        )
        target_link_options(RodProjectOptions INTERFACE
            $<$<CONFIG:Dist>:/LTCG>
        )
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(RodProjectWarnings INTERFACE
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wsign-conversion
        -Wshadow
        -Wswitch-enum
        -Wnon-virtual-dtor
        -Wformat=2
    )
else()
    message(FATAL_ERROR "RodEngine requires MSVC, Clang, or GCC.")
endif()

if(ROD_WARNINGS_AS_ERRORS)
    target_compile_options(RodProjectWarnings INTERFACE
        $<$<CXX_COMPILER_ID:MSVC>:/WX>
        $<$<CXX_COMPILER_ID:Clang,GNU>:-Werror>
    )
endif()
