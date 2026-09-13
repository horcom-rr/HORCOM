add_library(horcom STATIC
  horcom/time/calendar.cpp
  horcom/time/delta_t.cpp
  horcom/time/sidereal.cpp
)

target_include_directories(horcom PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(horcom PUBLIC cxx_std_20)
