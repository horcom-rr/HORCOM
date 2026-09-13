add_executable(horcom_tests
  main.cpp
  test_angle.cpp
  test_calendar.cpp
  test_delta_t.cpp
  test_sidereal.cpp
)

target_link_libraries(horcom_tests PRIVATE horcom)
target_include_directories(horcom_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/vendor)

add_test(NAME horcom_tests COMMAND horcom_tests)
