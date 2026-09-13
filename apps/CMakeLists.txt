add_executable(horcom_cli cli/main.cpp)
target_link_libraries(horcom_cli PRIVATE horcom)
target_compile_definitions(horcom_cli PRIVATE _CRT_SECURE_NO_WARNINGS)
set_target_properties(horcom_cli PROPERTIES OUTPUT_NAME horcom)

# the Qt shell builds only where Qt 6 is available
find_package(Qt6 QUIET COMPONENTS Widgets Svg PrintSupport LinguistTools)
if(Qt6_FOUND)
  set(CMAKE_AUTOMOC ON)
  qt_add_executable(horcom_gui
    gui/main.cpp
    gui/aspektarium_dialog.cpp
    gui/aspektarium_dialog.hpp
    gui/banner.cpp
    gui/banner.hpp
    gui/ingress_dialog.cpp
    gui/ingress_dialog.hpp
    gui/main_window.cpp
    gui/main_window.hpp
    gui/painter.cpp
    gui/painter.hpp
    gui/place_dialog.cpp
    gui/place_dialog.hpp
    gui/record_dialog.cpp
    gui/record_dialog.hpp
    gui/statist_dialog.cpp
    gui/statist_dialog.hpp
    gui/transit_list_dialog.cpp
    gui/transit_list_dialog.hpp
    gui/wheel_widget.cpp
    gui/wheel_widget.hpp
    gui/zone_dialog.cpp
    gui/zone_dialog.hpp
  )
  qt_add_resources(horcom_gui "identity"
    PREFIX "/"
    BASE ${CMAKE_SOURCE_DIR}/assets
    FILES ${CMAKE_SOURCE_DIR}/assets/logo.svg
  )
  target_link_libraries(horcom_gui PRIVATE horcom Qt6::Widgets Qt6::Svg Qt6::PrintSupport)
  set_target_properties(horcom_gui PROPERTIES WIN32_EXECUTABLE ON)
  # German is the source language, English ships as a translation
  if(TARGET Qt6::lrelease)
    qt_add_translations(horcom_gui
      TS_FILES gui/i18n/horcom_en.ts
      RESOURCE_PREFIX "/i18n"
    )
  endif()
else()
  message(STATUS "Qt 6 not found, the GUI target is skipped")
endif()
