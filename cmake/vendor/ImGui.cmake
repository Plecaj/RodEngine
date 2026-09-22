set(ROD_IMGUI_SOURCES
    "${ROD_VENDOR_DIR}/imgui/imgui.cpp"
    "${ROD_VENDOR_DIR}/imgui/imgui_draw.cpp"
    "${ROD_VENDOR_DIR}/imgui/imgui_tables.cpp"
    "${ROD_VENDOR_DIR}/imgui/imgui_widgets.cpp"
    "${ROD_VENDOR_DIR}/imgui/backends/imgui_impl_glfw.cpp"
    "${ROD_VENDOR_DIR}/imgui/backends/imgui_impl_opengl3.cpp"
)

if(ROD_IMGUI_DEMO)
    list(APPEND ROD_IMGUI_SOURCES "${ROD_VENDOR_DIR}/imgui/imgui_demo.cpp")
endif()

add_library(RodImGui STATIC ${ROD_IMGUI_SOURCES})
add_library(Rod::ImGui ALIAS RodImGui)

target_compile_features(RodImGui PUBLIC cxx_std_17)
target_compile_definitions(RodImGui PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLAD)
target_include_directories(RodImGui SYSTEM PUBLIC
    "${ROD_VENDOR_DIR}/imgui"
    "${ROD_VENDOR_DIR}/imgui/backends"
)
target_link_libraries(RodImGui PUBLIC Rod::Glad Rod::GLFW)

rod_set_common_output_directories(RodImGui)
