target_sources(RodEngine PRIVATE
    ScriptAssembly.cpp
    ScriptBuilder.cpp
    ScriptClass.cpp
    ScriptEngine.cpp
    ScriptGlue.cpp
    ScriptGlue.h
    ScriptRuntime.cpp
    ScriptRuntime.h
)

target_sources(RodEngine PUBLIC
    FILE_SET HEADERS
    FILES
        ScriptAssembly.h
        ScriptBuilder.h
        ScriptClass.h
        ScriptEngine.h
        ScriptField.h
)
