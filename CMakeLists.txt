cmake_minimum_required(VERSION 3.0)
project(net-access C)

set(netAccess_SOURCES "src/net-access.c")

set(NA_INTERFACE_MAX CACHE STRING
  "Maximum size of interface names (default: 255)")
option(NA_ALLOW_INTERFACE "Allow switching of network interface from wlan0" ON)
option(NA_USE_LINENOISE "Use `linenoise` for input" OFF)

add_subdirectory(deps)

add_executable(net-access ${netAccess_SOURCES})

#ensure NA_INTERFACE_MAX is a valid number
if (NA_INTERFACE_MAX GREATER 0)
  target_compile_definitions(net-access
      PRIVATE "NA_INTERFACE_MAX=${NA_INTERFACE_MAX}")
endif (NA_INTERFACE_MAX GREATER 0)

#enable interface switching if the user so desires
if (NA_ALLOW_INTERFACE)
  target_compile_definitions(net-access
    PRIVATE "NA_ALLOW_INTERFACE")
endif(NA_ALLOW_INTERFACE)

if (NA_USE_LINENOISE)
  target_compile_definitions(net-access
    PRIVATE "NA_USE_LINENOISE")
  target_link_libraries(net-access linenoise)
endif(NA_USE_LINENOISE)
