target_sources(RodEngine PRIVATE
    rdpch.cpp
    rdpch.h
)

target_sources(RodEngine PUBLIC
    FILE_SET HEADERS
    BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}"
    FILES Rod.h
)

add_subdirectory(Rod/Core)
add_subdirectory(Rod/Debug)
add_subdirectory(Rod/Events)
add_subdirectory(Rod/ImGui)
add_subdirectory(Rod/Math)
add_subdirectory(Rod/Platform)
add_subdirectory(Rod/Project)
add_subdirectory(Rod/Renderer)
add_subdirectory(Rod/Scene)
add_subdirectory(Rod/Scripting)
add_subdirectory(Rod/Utils)
