target_sources(RodEngine PRIVATE
    LinuxPlatformUtils.cpp
    LinuxProcess.cpp
    LinuxSharedLibrary.cpp
)

if(TARGET Rod::DotnetHost)
    target_sources(RodEngine PRIVATE LinuxScriptInterop.cpp)
endif()
