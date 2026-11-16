function(rod_collect_sources out_var base_dir)
  file(GLOB_RECURSE _rod_sources CONFIGURE_DEPENDS
        LIST_DIRECTORIES false
        "${base_dir}/*.cpp"
    )
  set(${out_var} ${_rod_sources} PARENT_SCOPE)
endfunction()

function(rod_collect_headers out_var base_dir)
  file(GLOB_RECURSE _rod_headers CONFIGURE_DEPENDS
        LIST_DIRECTORIES false
        "${base_dir}/*.h"
    )
  set(${out_var} ${_rod_headers} PARENT_SCOPE)
endfunction()
