target_sources(RodEngine PRIVATE
    Project.cpp
    ProjectExporter.cpp
    ProjectInitializer.cpp
    ProjectSerializer.cpp
)

target_sources(RodEngine PUBLIC
    FILE_SET HEADERS
    FILES
        Project.h
        ProjectExporter.h
        ProjectInitializer.h
        ProjectProperties.h
        ProjectSerializer.h
)
