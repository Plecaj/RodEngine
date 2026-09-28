add_library(RodImGuizmo STATIC
    "${ROD_VENDOR_DIR}/ImGuizmo/ImGuizmo.cpp"
)
add_library(Rod::ImGuizmo ALIAS RodImGuizmo)

target_compile_features(RodImGuizmo PUBLIC cxx_std_17)
target_include_directories(RodImGuizmo SYSTEM PUBLIC
    "${ROD_VENDOR_DIR}/ImGuizmo"
)
target_link_libraries(RodImGuizmo PUBLIC Rod::ImGui Rod::Glm)

rod_set_common_output_directories(RodImGuizmo)
