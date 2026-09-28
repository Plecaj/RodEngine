if(ROD_USE_SYSTEM_SHADERC)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(SHADERC REQUIRED IMPORTED_TARGET shaderc)

  add_library(RodShaderc INTERFACE)
  target_link_libraries(RodShaderc INTERFACE PkgConfig::SHADERC)
else()
  set(SHADERC_SKIP_TESTS ON CACHE BOOL "Skip shaderc tests" FORCE)
  set(SHADERC_SKIP_EXAMPLES ON CACHE BOOL "Skip shaderc examples" FORCE)
  set(SHADERC_SKIP_EXECUTABLES ON CACHE BOOL "Skip glslc executable" FORCE)
  set(SHADERC_SKIP_COPYRIGHT_CHECK ON CACHE BOOL "Skip shaderc copyright check" FORCE)
  set(SHADERC_SKIP_INSTALL ON CACHE BOOL "Skip shaderc install" FORCE)
  set(SHADERC_ENABLE_WGSL_OUTPUT OFF CACHE BOOL "Disable WGSL/Tint output" FORCE)
  set(SHADERC_ENABLE_WERROR_COMPILE OFF CACHE BOOL "Disable -Werror for shaderc" FORCE)
  set(SPIRV_SKIP_TESTS ON CACHE BOOL "Skip SPIRV-Tools tests" FORCE)

  set(GLSLANG_ENABLE_INSTALL OFF CACHE BOOL "Enable glslang installation" FORCE)
  set(GLSLANG_TESTS OFF CACHE BOOL "Enable glslang testing" FORCE)
  set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "Builds glslang and spirv-remap" FORCE)
  set(ENABLE_SPVREMAPPER OFF CACHE BOOL "Enables building of SPVRemapper" FORCE)
  set(SKIP_SPIRV_TOOLS_INSTALL ON CACHE BOOL "Skip SPIRV-Tools installation" FORCE)
  set(SPIRV_WERROR OFF CACHE BOOL "Enable error on warning" FORCE)

  add_subdirectory(
        "${ROD_VENDOR_DIR}/shaderc"
        "${CMAKE_BINARY_DIR}/_deps/shaderc"
        EXCLUDE_FROM_ALL
    )

  add_library(RodShaderc INTERFACE)
  target_link_libraries(RodShaderc INTERFACE shaderc_combined)
  target_include_directories(RodShaderc SYSTEM INTERFACE
        "${ROD_VENDOR_DIR}/shaderc/libshaderc/include"
    )
endif()

add_library(Rod::Shaderc ALIAS RodShaderc)
