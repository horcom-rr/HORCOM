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
  add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

add_subdirectory(src)

enable_testing()
add_subdirectory(tests)
