target_sources(RodEngine PRIVATE
    WindowsPlatformUtils.cpp
    WindowsProcess.cpp
    WindowsSharedLibrary.cpp
)

if(TARGET Rod::DotnetHost)
    target_sources(RodEngine PRIVATE WindowsScriptInterop.cpp)
endif()
