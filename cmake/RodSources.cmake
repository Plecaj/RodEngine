# Collect implementation files under a directory tree.
# CONFIGURE_DEPENDS makes CMake re-run when matching files are added or removed.
function(rod_collect_sources out_var base_dir)
    file(GLOB_RECURSE _rod_sources CONFIGURE_DEPENDS
        LIST_DIRECTORIES false
        "${base_dir}/*.cpp"
    )
    set(${out_var} ${_rod_sources} PARENT_SCOPE)
endfunction()

# Collect public headers under a directory tree.
function(rod_collect_headers out_var base_dir)
    file(GLOB_RECURSE _rod_headers CONFIGURE_DEPENDS
        LIST_DIRECTORIES false
        "${base_dir}/*.h"
    )
    set(${out_var} ${_rod_headers} PARENT_SCOPE)
endfunction()
