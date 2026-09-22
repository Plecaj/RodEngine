add_library(RodProjectOptions INTERFACE)
add_library(Rod::ProjectOptions ALIAS RodProjectOptions)

add_library(RodProjectWarnings INTERFACE)
add_library(Rod::ProjectWarnings ALIAS RodProjectWarnings)

function(rod_configure_cpp_standard target)
    set_target_properties(${target} PROPERTIES
        CXX_STANDARD ${ROD_CXX_STANDARD}
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )
endfunction()

target_compile_definitions(RodProjectOptions INTERFACE
    $<$<CONFIG:Debug>:RD_DEBUG>
    $<$<CONFIG:Release>:RD_RELEASE>
    $<$<CONFIG:Dist>:RD_DIST>
)

if(WIN32)
    target_compile_definitions(RodProjectOptions INTERFACE
        RD_PLATFORM_WINDOWS
        GLFW_INCLUDE_NONE
    )
endif()

if(MSVC)
    target_compile_options(RodProjectWarnings INTERFACE /W4 /permissive-)
    target_compile_options(RodProjectOptions INTERFACE
        /MP
        /EHsc
        /utf-8
        /Zc:__cplusplus
        /Zc:preprocessor
        /diagnostics:caret
        /external:anglebrackets
        /external:W0
        $<$<CONFIG:Debug>:/Od;/Zi>
        $<$<CONFIG:Release>:/O2;/Zi>
        $<$<CONFIG:Dist>:/O2;/GL>
    )
    target_link_options(RodProjectOptions INTERFACE
        $<$<CONFIG:Release>:/DEBUG>
        $<$<CONFIG:Dist>:/LTCG>
    )
else()
    message(FATAL_ERROR "RodEngine CMake setup is MSVC-first. Configure with Visual Studio/MSVC.")
endif()

if(ROD_WARNINGS_AS_ERRORS)
    target_compile_options(RodProjectWarnings INTERFACE /WX)
endif()
