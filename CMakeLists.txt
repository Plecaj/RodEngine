cmake_minimum_required(VERSION 4.1)

project(RodEngineWorkspace
    VERSION 0.1.0
    LANGUAGES C CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ROD_IMGUI_DEMO "Compile Dear ImGui demo window support." OFF)
option(ROD_BUILD_EDITOR "Build the Rod editor executable." ON)
option(ROD_BUILD_LAUNCHER "Build the Rod project launcher executable." ON)
option(ROD_BUILD_RUNTIME "Build the standalone Rod runtime executable." ON)
option(ROD_WARNINGS_AS_ERRORS "Treat RodEngine warnings as errors." OFF)
option(BUILD_TESTING "Build RodEngine tests." OFF)

include(CTest)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(RodBuildTypes)
include(RodCompilerOptions)
include(RodOutputDirs)
include(RodSanitizers)
include(RodDependencies)
include(RodDotnet)

add_subdirectory(RodEngine)

include(RodScripts)

if(ROD_BUILD_EDITOR)
  add_subdirectory(Rod-Editor)
endif()

if(ROD_BUILD_LAUNCHER)
  add_subdirectory(Rod-Launcher)
endif()

if(ROD_BUILD_RUNTIME)
  add_subdirectory(Rod-Runtime)
endif()

if(ROD_BUILD_EDITOR AND ROD_BUILD_RUNTIME)
  add_dependencies(Rod-Editor Rod-Runtime)
endif()

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
