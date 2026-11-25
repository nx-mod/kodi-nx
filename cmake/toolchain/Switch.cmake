include("$ENV{DEVKITPRO}/cmake/Switch.cmake")

if(DEFINED ENV{LIBNX} AND EXISTS "$ENV{LIBNX}/switch.specs")
  set(NX_ROOT "$ENV{LIBNX}")
endif()

set(CMAKE_PROJECT_INCLUDE "${CMAKE_CURRENT_LIST_DIR}/SwitchProject.cmake")

if(DEFINED ENV{LIBNX} AND EXISTS "$ENV{LIBNX}/include/switch.h")
  set(NX_ROOT "$ENV{LIBNX}")
  list(PREPEND CMAKE_FIND_ROOT_PATH "${NX_ROOT}")
  list(PREPEND CMAKE_PREFIX_PATH "${NX_ROOT}")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I${NX_ROOT}/include")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${NX_ROOT}/include")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${NX_ROOT}/lib")
endif()
