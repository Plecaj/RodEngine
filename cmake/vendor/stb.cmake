add_library(RodStb STATIC
    "${ROD_VENDOR_DIR}/stb_image/stb_image.cpp"
)
add_library(Rod::Stb ALIAS RodStb)

target_include_directories(RodStb SYSTEM PUBLIC
    "${ROD_VENDOR_DIR}/stb_image"
)

rod_set_common_output_directories(RodStb)
