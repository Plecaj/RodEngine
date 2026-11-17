set(ROD_RUNTIME_OUTPUT_DIR "${CMAKE_BINARY_DIR}/bin")
if(ROD_IS_MULTI_CONFIG)
    string(APPEND ROD_RUNTIME_OUTPUT_DIR "/$<CONFIG>")
endif()

function(rod_set_common_output_directories target)
    set_target_properties(${target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${ROD_RUNTIME_OUTPUT_DIR}"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    )
endfunction()
