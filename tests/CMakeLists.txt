add_executable(horcom_tests
  main.cpp
  test_angle.cpp
  test_calendar.cpp
  test_delta_t.cpp
  test_sidereal.cpp
  test_vsop.cpp
  test_sunmoon.cpp
  test_kepler.cpp
  test_eph_file.cpp
  test_houses.cpp
  test_chart.cpp
  test_transit_search.cpp
  test_aspects.cpp
  test_render.cpp
  test_data.cpp
  test_aaf_konsta.cpp
)

target_link_libraries(horcom_tests PRIVATE horcom)
target_include_directories(horcom_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/vendor)
target_compile_definitions(horcom_tests PRIVATE HORCOM_TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/data")

add_test(NAME horcom_tests COMMAND horcom_tests)
