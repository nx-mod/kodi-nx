set(APP_BINARY ${APP_NAME_LC}${APP_BINARY_SUFFIX})

target_link_options(${APP_NAME_LC} PRIVATE
  -specs=${CMAKE_SOURCE_DIR}/../libnx/nx/switch.specs
  -ldrm_nouveau -lglapi -lEGL -luuid -lsqlite3 -lnx -Wl,-z,notext)

include(${CMAKE_SOURCE_DIR}/cmake/scripts/switch/Package.cmake OPTIONAL)
