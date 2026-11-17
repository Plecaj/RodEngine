target_sources(RodEngine PRIVATE StbImage.h)

add_subdirectory(GLFW)
add_subdirectory(OpenGL)

if(WIN32)
    add_subdirectory(Windows)
else()
    add_subdirectory(Linux)
endif()
