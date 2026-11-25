set(APP_BINARY ${APP_NAME_LC}${APP_BINARY_SUFFIX})

target_link_libraries(${APP_NAME_LC} PRIVATE
  -ldrm_nouveau -lglapi -lEGL -luuid -lsqlite3 -lnx)
target_link_options(${APP_NAME_LC} PRIVATE -Wl,-z,notext)

include(${CMAKE_SOURCE_DIR}/cmake/scripts/switch/Package.cmake OPTIONAL)
