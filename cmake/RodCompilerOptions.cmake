if(NOT CMAKE_C_COMPILER_ID STREQUAL "Clang" OR
   NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR
   CMAKE_C_COMPILER_VERSION VERSION_LESS "23.1" OR
   CMAKE_CXX_COMPILER_VERSION VERSION_LESS "23.1")
    message(FATAL_ERROR "RodEngine requires Clang 23.1 or newer for C and C++. Use the Clang presets with LLVM on PATH.")
endif()

if(WIN32 AND NOT MINGW)
    message(FATAL_ERROR "On Windows, use the LLVM-MinGW UCRT toolchain with its bin directory first on PATH.")
endif()

add_library(RodProjectOptions INTERFACE)
add_library(Rod::ProjectOptions ALIAS RodProjectOptions)

add_library(RodProjectWarnings INTERFACE)
add_library(Rod::ProjectWarnings ALIAS RodProjectWarnings)

function(rod_configure_cpp_target target)
    set_target_properties(${target} PROPERTIES CXX_EXTENSIONS OFF)
    target_precompile_headers(${target} PRIVATE "${PROJECT_SOURCE_DIR}/RodEngine/src/rdpch.h")
    target_link_libraries(${target} PRIVATE Rod::ProjectWarnings)
    rod_set_common_output_directories(${target})

    get_target_property(target_type ${target} TYPE)
    if(target_type STREQUAL "EXECUTABLE")
        target_compile_definitions(${target} PRIVATE ROD_EXECUTABLE_SUFFIX="${CMAKE_EXECUTABLE_SUFFIX}")
    endif()
endfunction()

target_compile_definitions(RodProjectOptions INTERFACE
    GLFW_INCLUDE_NONE
    $<$<CONFIG:Debug>:RD_DEBUG>
    $<$<CONFIG:Release>:RD_RELEASE>
)

if(WIN32)
    target_compile_definitions(RodProjectOptions INTERFACE RD_PLATFORM_WINDOWS)
    target_link_options(RodProjectOptions INTERFACE
        "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:-static>"
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_definitions(RodProjectOptions INTERFACE RD_PLATFORM_LINUX)
else()
    message(FATAL_ERROR "RodEngine supports Windows and Linux.")
endif()

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

if(ROD_WARNINGS_AS_ERRORS)
    target_compile_options(RodProjectWarnings INTERFACE -Werror)
endif()
