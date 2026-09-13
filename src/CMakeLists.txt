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
  horcom/chart/transit_search.cpp
  horcom/chart/composite.cpp
  horcom/chart/directions.cpp
  horcom/chart/mundane.cpp
  horcom/chart/aspects.cpp
  horcom/render/wheel.cpp
  horcom/render/svg.cpp
  horcom/data/encoding.cpp
  horcom/data/chart_file.cpp
  horcom/data/place_file.cpp
  horcom/data/zone_names.cpp
  horcom/data/countries.cpp
  horcom/data/gfa_stream.cpp
  horcom/data/konsta.cpp
  horcom/data/aaf.cpp
)

target_include_directories(horcom PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(horcom PUBLIC cxx_std_20)
