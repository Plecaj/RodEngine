find_program(DOTNET_EXE dotnet)

if(DOTNET_EXE)
  file(REAL_PATH "${DOTNET_EXE}" ROD_DOTNET_EXECUTABLE)
  get_filename_component(ROD_DOTNET_ROOT "${ROD_DOTNET_EXECUTABLE}" DIRECTORY)

  if(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
      if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(ARM64|arm64|aarch64)$" OR CMAKE_VS_PLATFORM_NAME MATCHES "ARM64")
        set(ROD_DOTNET_RUNTIME_ID win-arm64)
      else()
        set(ROD_DOTNET_RUNTIME_ID win-x64)
      endif()
    else()
      set(ROD_DOTNET_RUNTIME_ID win-x86)
    endif()
  elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(ARM64|arm64|aarch64)$")
      set(ROD_DOTNET_RUNTIME_ID linux-arm64)
    else()
      set(ROD_DOTNET_RUNTIME_ID linux-x64)
    endif()
  endif()

  file(GLOB ROD_DOTNET_NATIVE_DIRS
        "${ROD_DOTNET_ROOT}/packs/Microsoft.NETCore.App.Host.${ROD_DOTNET_RUNTIME_ID}/[0-9]*/runtimes/${ROD_DOTNET_RUNTIME_ID}/native"
    )
  if(NOT ROD_DOTNET_NATIVE_DIRS AND CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT CMAKE_CROSSCOMPILING)
    execute_process(
            COMMAND "${CMAKE_COMMAND}" -E env DOTNET_CLI_UI_LANGUAGE=en-US "${DOTNET_EXE}" --info
            OUTPUT_VARIABLE ROD_DOTNET_INFO
            RESULT_VARIABLE ROD_DOTNET_INFO_RESULT
            ERROR_QUIET
            TIMEOUT 10
        )
    if(ROD_DOTNET_INFO_RESULT EQUAL 0 AND ROD_DOTNET_INFO MATCHES "RID:[ \t]+([^ \t\r\n]+)")
      set(ROD_DOTNET_SDK_RUNTIME_ID "${CMAKE_MATCH_1}")
      string(REGEX REPLACE "^linux-" "" ROD_DOTNET_ARCH "${ROD_DOTNET_RUNTIME_ID}")
      if(ROD_DOTNET_SDK_RUNTIME_ID MATCHES "-${ROD_DOTNET_ARCH}$")
        file(GLOB ROD_DOTNET_NATIVE_DIRS
                    "${ROD_DOTNET_ROOT}/packs/Microsoft.NETCore.App.Host.${ROD_DOTNET_SDK_RUNTIME_ID}/[0-9]*/runtimes/${ROD_DOTNET_SDK_RUNTIME_ID}/native"
                )
      endif()
    endif()
  endif()
  list(SORT ROD_DOTNET_NATIVE_DIRS COMPARE NATURAL ORDER DESCENDING)

  unset(ROD_DOTNET_HOST_INCLUDE_DIR CACHE)
  unset(ROD_DOTNET_HOST_LIBRARY CACHE)
  unset(ROD_DOTNET_HOST_RUNTIME_LIBRARY CACHE)
  find_path(ROD_DOTNET_HOST_INCLUDE_DIR nethost.h PATHS ${ROD_DOTNET_NATIVE_DIRS} NO_DEFAULT_PATH)
  if(WIN32)
    find_file(ROD_DOTNET_HOST_LIBRARY NAMES nethost.lib
            PATHS "${ROD_DOTNET_HOST_INCLUDE_DIR}" NO_DEFAULT_PATH
        )
    find_file(ROD_DOTNET_HOST_RUNTIME_LIBRARY NAMES nethost.dll
            PATHS "${ROD_DOTNET_HOST_INCLUDE_DIR}" NO_DEFAULT_PATH
        )
  else()
    find_file(ROD_DOTNET_HOST_LIBRARY NAMES libnethost.a
            PATHS "${ROD_DOTNET_HOST_INCLUDE_DIR}" NO_DEFAULT_PATH
        )
  endif()
endif()

if(DOTNET_EXE AND ROD_DOTNET_HOST_INCLUDE_DIR AND ROD_DOTNET_HOST_LIBRARY)
  find_package(Threads REQUIRED)
  target_include_directories(RodEngine SYSTEM PRIVATE "${ROD_DOTNET_HOST_INCLUDE_DIR}")
  target_link_libraries(RodEngine PRIVATE "${ROD_DOTNET_HOST_LIBRARY}" Threads::Threads)
  target_compile_definitions(RodEngine PRIVATE
        ROD_HAS_DOTNET_HOST
        $<$<NOT:$<PLATFORM_ID:Windows>>:NETHOST_USE_AS_STATIC>
    )
else()
  message(WARNING ".NET hosting files were not found. Install the .NET 9 SDK to enable C# scripting.")
endif()

function(rod_copy_dotnet_host target)
  if(WIN32 AND ROD_DOTNET_HOST_RUNTIME_LIBRARY)
    add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${ROD_DOTNET_HOST_RUNTIME_LIBRARY}"
                "$<TARGET_FILE_DIR:${target}>"
            VERBATIM
        )
  endif()
endfunction()
