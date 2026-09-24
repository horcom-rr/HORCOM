cmake_minimum_required(VERSION 3.25)

project(horcom
  VERSION 0.1.0
  DESCRIPTION "C++ rewrite of HORCOM by Robert Rettig"
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
  add_compile_options(/W4 /WX /utf-8)
else()
  # partial aggregate initialisers lean on default member initialisers
  # on purpose (render primitives), MSVC /W4 has no such lint either.
  # No FMA contraction, the original computation order is binding and
  # a packager's -march must not change the numbers
  add_compile_options(-Wall -Wextra -Wpedantic -Werror
                      -Wno-missing-field-initializers -ffp-contract=off)
  # GCC 12, the AppImage compiler, reports false -Wrestrict hits inside
  # the libstdc++ string code (GCC bug 105651, fixed in 13)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 13)
    add_compile_options(-Wno-restrict)
  endif()
endif()

add_subdirectory(src)
add_subdirectory(apps)

enable_testing()
add_subdirectory(tests)
