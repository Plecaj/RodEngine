set(GLFW_SOURCES
    ${VENDOR_DIR}/GLFW/include/GLFW/glfw3.h
    ${VENDOR_DIR}/GLFW/include/GLFW/glfw3native.h
    ${VENDOR_DIR}/GLFW/src/context.c
    ${VENDOR_DIR}/GLFW/src/init.c
    ${VENDOR_DIR}/GLFW/src/input.c
    ${VENDOR_DIR}/GLFW/src/monitor.c
    ${VENDOR_DIR}/GLFW/src/null_init.c
    ${VENDOR_DIR}/GLFW/src/null_joystick.c
    ${VENDOR_DIR}/GLFW/src/null_monitor.c
    ${VENDOR_DIR}/GLFW/src/null_window.c
    ${VENDOR_DIR}/GLFW/src/platform.c
    ${VENDOR_DIR}/GLFW/src/vulkan.c
    ${VENDOR_DIR}/GLFW/src/window.c
)

if(WIN32)
    list(APPEND GLFW_SOURCES
        ${VENDOR_DIR}/GLFW/src/win32_init.c
        ${VENDOR_DIR}/GLFW/src/win32_joystick.c
        ${VENDOR_DIR}/GLFW/src/win32_module.c
        ${VENDOR_DIR}/GLFW/src/win32_monitor.c
        ${VENDOR_DIR}/GLFW/src/win32_time.c
        ${VENDOR_DIR}/GLFW/src/win32_thread.c
        ${VENDOR_DIR}/GLFW/src/win32_window.c
        ${VENDOR_DIR}/GLFW/src/wgl_context.c
        ${VENDOR_DIR}/GLFW/src/egl_context.c
        ${VENDOR_DIR}/GLFW/src/osmesa_context.c
    )
endif()

add_library(GLFW STATIC ${GLFW_SOURCES})

target_include_directories(GLFW PUBLIC ${INCLUDE_DIR_GLFW})

if(WIN32)
    target_compile_definitions(GLFW PRIVATE
        _GLFW_WIN32
        _CRT_SECURE_NO_WARNINGS
    )
endif()

if(MSVC)
    target_compile_options(GLFW PRIVATE /W0)
else()
    target_compile_options(GLFW PRIVATE -w)
endif()

string(TOLOWER "${CMAKE_SYSTEM_NAME}" SYSTEM_NAME_LOWER)
set_target_properties(GLFW PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug-${SYSTEM_NAME_LOWER}-${CMAKE_ARCHITECTURE}/GLFW"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release-${SYSTEM_NAME_LOWER}-${CMAKE_ARCHITECTURE}/GLFW"
    ARCHIVE_OUTPUT_DIRECTORY_DIST "${CMAKE_BINARY_DIR}/bin/Dist-${SYSTEM_NAME_LOWER}-${CMAKE_ARCHITECTURE}/GLFW"
)
