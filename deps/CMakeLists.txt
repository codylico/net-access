cmake_minimum_required(VERSION 3.0)

#linenoise library
if (NA_USE_LINENOISE)
  option(LINENOISE_BUILD_TESTS "Build test program for linenoise library")
  option(LINENOISE_USE_UTF8 "Build support for UTF-8 into linenoise")
  add_library(linenoise STATIC
    "linenoise/linenoise.c" "linenoise/linenoise.h"
    "linenoise/stringbuf.h" "linenoise/stringbuf.c"
    "linenoise/utf8.h" "linenoise/utf8.c"
    )
  if (LINENOISE_USE_UTF8)
    target_compile_definitions(linenoise PRIVATE "USE_UTF8")
  endif (LINENOISE_USE_UTF8)
  if (LINENOISE_BUILD_TESTS AND NOT CMAKE_DISABLE_TESTING)
    add_executable("linenoise_example" "linenoise/example.c")
    target_link_libraries("linenoise_example" linenoise)
  endif (LINENOISE_BUILD_TESTS AND NOT CMAKE_DISABLE_TESTING)
endif(NA_USE_LINENOISE)
