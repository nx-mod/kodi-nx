find_program(NACPTOOL_EXECUTABLE nacptool HINTS "$ENV{DEVKITPRO}/tools/bin" REQUIRED)
find_program(ELF2NRO_EXECUTABLE elf2nro HINTS "$ENV{DEVKITPRO}/tools/bin" REQUIRED)

set(NRO_NACP ${CMAKE_BINARY_DIR}/${APP_NAME_LC}.nacp)
set(NRO_OUTPUT ${CMAKE_BINARY_DIR}/${APP_NAME_LC}.nro)

add_custom_command(TARGET ${APP_NAME_LC} POST_BUILD
  COMMAND ${NACPTOOL_EXECUTABLE} --create "${APP_NAME}" "nx-mod" "${APP_VERSION}" ${NRO_NACP}
  COMMAND ${ELF2NRO_EXECUTABLE} $<TARGET_FILE:${APP_NAME_LC}> ${NRO_OUTPUT} --nacp=${NRO_NACP} --icon=${CMAKE_SOURCE_DIR}/tools/switch/icon.jpg
  COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/switch/fix_nro_size.py ${NRO_OUTPUT}
  COMMENT "Packaging ${APP_NAME_LC}.nro"
  VERBATIM)
