function(core_link_library lib wraplib)
  if(TARGET ${lib})
    set_property(GLOBAL APPEND PROPERTY SWITCH_STATIC_WRAPLIBS ${lib})
  endif()
endfunction()

function(find_soname lib)
endfunction()
