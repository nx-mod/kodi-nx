set(SOURCES DlStubs.cpp
            PlatformSwitch.cpp
            PosixCompat.cpp
            SystemInfoSwitch.cpp)

set(HEADERS Shutdown.h)

core_add_library(platform_switch)
