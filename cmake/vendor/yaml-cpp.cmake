file(GLOB_RECURSE YAML_CPP_SOURCES
    ${VENDOR_DIR}/yaml-cpp/src/*.cpp
    ${VENDOR_DIR}/yaml-cpp/src/*.h
    ${VENDOR_DIR}/yaml-cpp/include/*.h
)

add_library(yaml-cpp STATIC ${YAML_CPP_SOURCES})

target_include_directories(yaml-cpp PUBLIC ${INCLUDE_DIR_YAML_CPP})

target_compile_definitions(yaml-cpp PUBLIC YAML_CPP_STATIC_DEFINE)

if(WIN32)
    set_property(TARGET yaml-cpp PROPERTY CXX_STANDARD 17)
endif()

string(TOLOWER "${CMAKE_SYSTEM_NAME}" SYSTEM_NAME_LOWER)
set_target_properties(yaml-cpp PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug-${SYSTEM_NAME_LOWER}-${CMAKE_ARCHITECTURE}/yaml-cpp"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release-${SYSTEM_NAME_LOWER}-${CMAKE_ARCHITECTURE}/yaml-cpp"
    ARCHIVE_OUTPUT_DIRECTORY_DIST "${CMAKE_BINARY_DIR}/bin/Dist-${SYSTEM_NAME_LOWER}-${CMAKE_ARCHITECTURE}/yaml-cpp"
)
