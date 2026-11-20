target_sources(RodEngine PRIVATE
    FileUtils.cpp
    SharedLibrary.cpp
)

target_sources(RodEngine PUBLIC
    FILE_SET HEADERS
    FILES
        FileUtils.h
        FunctionPointerCast.h
        PlatformUtils.h
        Process.h
        SharedLibrary.h
)
