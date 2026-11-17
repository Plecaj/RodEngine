add_executable(Rod-Runtime)

rod_configure_cpp_target(Rod-Runtime)

add_subdirectory(src)

target_include_directories(Rod-Runtime PRIVATE src)

target_link_libraries(Rod-Runtime PRIVATE Rod::Engine)

add_dependencies(Rod-Runtime RodAssets-Copy RodSamples-Copy)

set(ROD_GAME_EXPORT_DIR "${CMAKE_BINARY_DIR}/export/RodGame")

add_executable(Rod-Package tools/PackageMain.cpp)
rod_configure_cpp_target(Rod-Package)
target_link_libraries(Rod-Package PRIVATE Rod::Engine)
add_dependencies(Rod-Package Rod-Runtime)

add_custom_target(RodGame-Export
    COMMAND "$<TARGET_FILE:Rod-Package>"
        "${CMAKE_SOURCE_DIR}/Samples/Samples.rodproj"
        "${ROD_GAME_EXPORT_DIR}"
    DEPENDS Rod-Package
    VERBATIM
)

