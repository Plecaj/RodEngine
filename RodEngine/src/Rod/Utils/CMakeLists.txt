target_sources(RodEngine PRIVATE
    FileUtils.cpp
    SharedLibrary.cpp
)

target_sources(RodEngine PUBLIC
    FILE_SET HEADERS
    FILES
        FileUtils.h
        PlatformUtils.h
        Process.h
        SharedLibrary.h
)
