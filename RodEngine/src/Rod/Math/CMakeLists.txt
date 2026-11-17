target_sources(RodEngine PRIVATE Math.cpp)

target_sources(RodEngine PUBLIC
    FILE_SET HEADERS
    FILES Math.h
)
