set(APP_BINARY ${APP_NAME_LC}${APP_BINARY_SUFFIX})

target_link_libraries(${APP_NAME_LC}
  -ldrm_nouveau -lEGL -lglapi -luuid -lsqlite3 -lnx
  -lEGL -lglapi -ldrm_nouveau)
target_link_options(${APP_NAME_LC} PRIVATE
  -Wl,-z,notext -Wl,-z,nopack-relative-relocs
  -Wl,--wrap=diagAbortWithResult
  )

include(${CMAKE_SOURCE_DIR}/cmake/scripts/switch/Package.cmake OPTIONAL)
