# the Windows identity of the executables, icon plus version block,
# CI passes the release number as HORCOM_VERSION
set(HORCOM_VERSION "0.0" CACHE STRING "version stamped into the executables")
if(HORCOM_VERSION MATCHES "^([0-9]+)\\.([0-9]+)")
  set(HORCOM_RC_VERSION "${CMAKE_MATCH_1},${CMAKE_MATCH_2},0,0")
else()
  set(HORCOM_RC_VERSION "0,0,0,0")
endif()
set(HORCOM_ICO "${CMAKE_SOURCE_DIR}/assets/logo.ico")

function(horcom_stamp target description internal filename)
  if(NOT MSVC)
    return()
  endif()
  set(HORCOM_RC_DESCRIPTION "${description}")
  set(HORCOM_RC_INTERNAL "${internal}")
  set(HORCOM_RC_FILENAME "${filename}")
  configure_file(version.rc.in "${CMAKE_CURRENT_BINARY_DIR}/${internal}.rc" @ONLY)
  target_sources(${target} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/${internal}.rc")
endfunction()

add_executable(horcom_cli cli/main.cpp)
target_link_libraries(horcom_cli PRIVATE horcom)
target_compile_definitions(horcom_cli PRIVATE _CRT_SECURE_NO_WARNINGS)
set_target_properties(horcom_cli PROPERTIES OUTPUT_NAME horcom)
horcom_stamp(horcom_cli "HORCOM command line tool" horcom_cli horcom.exe)

# the Qt shell builds only where Qt 6 is available
find_package(Qt6 QUIET COMPONENTS Widgets Svg PrintSupport LinguistTools)
if(Qt6_FOUND)
  set(CMAKE_AUTOMOC ON)
  qt_add_executable(horcom_gui
    gui/main.cpp
    gui/kommen_dialog.cpp
    gui/kommen_dialog.hpp
    gui/aspektarium_dialog.cpp
    gui/aspektarium_dialog.hpp
    gui/banner.cpp
    gui/banner.hpp
    gui/direction_list_dialog.cpp
    gui/direction_list_dialog.hpp
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
    FILES ${CMAKE_SOURCE_DIR}/assets/logo.svg ${CMAKE_SOURCE_DIR}/assets/arrow-up.svg ${CMAKE_SOURCE_DIR}/assets/arrow-down.svg
  )
  target_link_libraries(horcom_gui PRIVATE horcom Qt6::Widgets Qt6::Svg Qt6::PrintSupport)
  set_target_properties(horcom_gui PROPERTIES WIN32_EXECUTABLE ON)
  horcom_stamp(horcom_gui "HORCOM desktop shell" horcom_gui horcom_gui.exe)
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
