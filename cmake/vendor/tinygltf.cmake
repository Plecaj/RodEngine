add_library(RodTinyGltf STATIC
    "${ROD_VENDOR_DIR}/tinygltf/tiny_gltf.cpp"
)
add_library(Rod::TinyGltf ALIAS RodTinyGltf)

target_compile_features(RodTinyGltf PUBLIC cxx_std_17)
target_include_directories(RodTinyGltf SYSTEM PUBLIC
    "${ROD_VENDOR_DIR}/tinygltf"
    "${ROD_VENDOR_DIR}/json"
    "${ROD_VENDOR_DIR}/stb_image"
)
target_link_libraries(RodTinyGltf PUBLIC Rod::Json Rod::Stb)

rod_set_common_output_directories(RodTinyGltf)
