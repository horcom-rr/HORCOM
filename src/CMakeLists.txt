add_library(horcom STATIC
  horcom/time/calendar.cpp
  horcom/time/delta_t.cpp
  horcom/time/sidereal.cpp
  horcom/ephem/vsop.cpp
  horcom/ephem/sunmoon.cpp
  horcom/ephem/moon.cpp
  horcom/ephem/kepler.cpp
  horcom/ephem/elements.cpp
  horcom/ephem/precession.cpp
  horcom/ephem/eph_file.cpp
  horcom/ephem/pluto_chapront.cpp
  horcom/chart/houses.cpp
  horcom/chart/corrections.cpp
  horcom/chart/geo.cpp
  horcom/chart/chart.cpp
)

target_include_directories(horcom PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(horcom PUBLIC cxx_std_20)
