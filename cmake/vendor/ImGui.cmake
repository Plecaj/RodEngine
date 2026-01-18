set(IMGUI_SOURCES
    ${VENDOR_DIR}/imgui/imconfig.h
    ${VENDOR_DIR}/imgui/imgui.h
    ${VENDOR_DIR}/imgui/imgui.cpp
    ${VENDOR_DIR}/imgui/imgui_draw.cpp
    ${VENDOR_DIR}/imgui/imgui_internal.h
    ${VENDOR_DIR}/imgui/imgui_tables.cpp
    ${VENDOR_DIR}/imgui/imgui_widgets.cpp
    ${VENDOR_DIR}/imgui/imstb_rectpack.h
    ${VENDOR_DIR}/imgui/imstb_textedit.h
    ${VENDOR_DIR}/imgui/imstb_truetype.h
    ${VENDOR_DIR}/imgui/imgui_demo.cpp
)

add_library(ImGui STATIC ${IMGUI_SOURCES})

target_include_directories(ImGui PUBLIC ${INCLUDE_DIR_IMGUI})

if(WIN32)
    set_property(TARGET ImGui PROPERTY CXX_STANDARD 17)
endif()

string(TOLOWER "${CMAKE_SYSTEM_NAME}" SYSTEM_NAME_LOWER)
set_target_properties(ImGui PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug-${SYSTEM_NAME_LOWER}-${CMAKE_ARCHITECTURE}/ImGui"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release-${SYSTEM_NAME_LOWER}-${CMAKE_ARCHITECTURE}/ImGui"
    ARCHIVE_OUTPUT_DIRECTORY_DIST "${CMAKE_BINARY_DIR}/bin/Dist-${SYSTEM_NAME_LOWER}-${CMAKE_ARCHITECTURE}/ImGui"
)
