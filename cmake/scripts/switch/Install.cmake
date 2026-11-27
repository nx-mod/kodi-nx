set(APP_BINARY ${APP_NAME_LC}${APP_BINARY_SUFFIX})

target_link_libraries(${APP_NAME_LC}
  ${CMAKE_SOURCE_DIR}/xbmc/platform/switch/binary-addons/libvfs_tmdbm_nx.a
  -ldrm_nouveau -lEGL -lglapi -luuid -lsqlite3 -lnx
  -lEGL -lglapi -ldrm_nouveau)
target_link_options(${APP_NAME_LC} PRIVATE
  -Wl,-z,notext -Wl,-z,nopack-relative-relocs
  -Wl,--wrap=exit -Wl,--wrap=svcExitProcess -Wl,--wrap=abort -Wl,--wrap=_exit
  -Wl,--wrap=__cxa_throw
  -Wl,--wrap=_Py_FatalErrorFunc -Wl,--wrap=_Py_write_noraise
  )

include(${CMAKE_SOURCE_DIR}/cmake/scripts/switch/Package.cmake OPTIONAL)
