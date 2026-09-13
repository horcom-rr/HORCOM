add_executable(horcom_cli cli/main.cpp)
target_link_libraries(horcom_cli PRIVATE horcom)
target_compile_definitions(horcom_cli PRIVATE _CRT_SECURE_NO_WARNINGS)
set_target_properties(horcom_cli PROPERTIES OUTPUT_NAME horcom)

# the Qt shell builds only where Qt 6 is available
find_package(Qt6 QUIET COMPONENTS Widgets Svg)
if(Qt6_FOUND)
  set(CMAKE_AUTOMOC ON)
  qt_add_executable(horcom_gui
    gui/main.cpp
    gui/main_window.cpp
    gui/main_window.hpp
    gui/wheel_widget.cpp
    gui/wheel_widget.hpp
  )
  qt_add_resources(horcom_gui "identity"
    PREFIX "/"
    BASE ${CMAKE_SOURCE_DIR}/assets
    FILES ${CMAKE_SOURCE_DIR}/assets/logo.svg
  )
  target_link_libraries(horcom_gui PRIVATE horcom Qt6::Widgets Qt6::Svg)
  set_target_properties(horcom_gui PROPERTIES WIN32_EXECUTABLE ON)
else()
  message(STATUS "Qt 6 not found, the GUI target is skipped")
endif()
