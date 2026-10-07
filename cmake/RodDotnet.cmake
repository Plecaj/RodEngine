find_program(DOTNET_EXE dotnet)

if(DOTNET_EXE)
    file(REAL_PATH "${DOTNET_EXE}" ROD_DOTNET_EXECUTABLE)
    get_filename_component(ROD_DOTNET_ROOT "${ROD_DOTNET_EXECUTABLE}" DIRECTORY)
    file(GLOB ROD_DOTNET_NATIVE_DIRS
        "${ROD_DOTNET_ROOT}/packs/Microsoft.NETCore.App.Host.*/[0-9]*/runtimes/*/native"
    )
    list(SORT ROD_DOTNET_NATIVE_DIRS COMPARE NATURAL ORDER DESCENDING)
    find_path(ROD_DOTNET_HOST_INCLUDE_DIR nethost.h PATHS ${ROD_DOTNET_NATIVE_DIRS} NO_DEFAULT_PATH)
    find_file(ROD_DOTNET_HOST_LIBRARY NAMES libnethost.a libnethost.lib
        PATHS "${ROD_DOTNET_HOST_INCLUDE_DIR}" NO_DEFAULT_PATH
    )
endif()

if(DOTNET_EXE AND ROD_DOTNET_HOST_INCLUDE_DIR AND ROD_DOTNET_HOST_LIBRARY)
    find_package(Threads REQUIRED)
    target_include_directories(RodEngine SYSTEM PRIVATE "${ROD_DOTNET_HOST_INCLUDE_DIR}")
    target_link_libraries(RodEngine PRIVATE "${ROD_DOTNET_HOST_LIBRARY}" Threads::Threads)
    target_compile_definitions(RodEngine PRIVATE ROD_HAS_DOTNET_HOST NETHOST_USE_AS_STATIC)
else()
    message(WARNING ".NET hosting files were not found. Install the .NET 9 SDK to enable C# scripting.")
endif()
