target_sources(RodEngine PRIVATE
    ScriptAssembly.cpp
    ScriptBuilder.cpp
    ScriptClass.cpp
)

if(TARGET Rod::DotnetHost)
    target_sources(RodEngine PRIVATE
        ScriptEngine.cpp
        ScriptGlue.cpp
        ScriptGlue.h
        ScriptInterop.h
        ScriptRuntime.cpp
        ScriptRuntime.h
    )
else()
    target_sources(RodEngine PRIVATE ScriptEngineDisabled.cpp)
endif()

target_sources(RodEngine PUBLIC
    FILE_SET HEADERS
    FILES
        ScriptAssembly.h
        ScriptBuilder.h
        ScriptClass.h
        ScriptEngine.h
        ScriptField.h
)
