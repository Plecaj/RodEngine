add_library(RodGlad STATIC
    "${ROD_VENDOR_DIR}/Glad/src/glad.c"
)
add_library(Rod::Glad ALIAS RodGlad)

target_include_directories(RodGlad SYSTEM PUBLIC
    "${ROD_VENDOR_DIR}/Glad/include"
)

rod_set_common_output_directories(RodGlad)
