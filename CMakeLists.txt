cmake_minimum_required(VERSION 3.25)

if(POLICY CMP0091)
  cmake_policy(SET CMP0091 NEW)
endif()

project(RodEngineWorkspace
    VERSION 0.1.0
    LANGUAGES C CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(ROD_CXX_STANDARD 23 CACHE STRING "C++ standard used by RodEngine targets.")
set_property(CACHE ROD_CXX_STANDARD PROPERTY STRINGS 20 23)

option(ROD_ENABLE_PCH "Enable RodEngine precompiled headers." ON)
option(ROD_USE_SYSTEM_SHADERC "Use shaderc from the active toolchain instead of building the vendored copy." OFF)
option(ROD_IMGUI_DEMO "Compile Dear ImGui demo window support." ON)
option(ROD_BUILD_EDITOR "Build the Rod editor executable." ON)
option(ROD_WARNINGS_AS_ERRORS "Treat RodEngine warnings as errors." OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(RodBuildTypes)
include(RodCompilerOptions)
include(RodOutputDirs)
include(RodDependencies)

add_subdirectory(RodEngine)

if(ROD_BUILD_EDITOR)
  add_subdirectory(Rod-Editor)
endif()
