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
  # the shell apart from its entry point, shared by the program and the
  # GUI tests so both run the very same objects
  add_library(horcom_gui_core OBJECT
    gui/kommen_dialog.cpp
    gui/kommen_dialog.hpp
    gui/aspektarium_dialog.cpp
    gui/aspektarium_dialog.hpp
    gui/aaf_mask_dialog.cpp
    gui/aaf_mask_dialog.hpp
    gui/auto_advance.cpp
    gui/auto_advance.hpp
    gui/banner.cpp
    gui/banner.hpp
    gui/choice_dialog.cpp
    gui/choice_dialog.hpp
    gui/record_list_dialog.cpp
    gui/record_list_dialog.hpp
    gui/record_mask_dialog.cpp
    gui/record_mask_dialog.hpp
    gui/robert_input.cpp
    gui/robert_text.cpp
    gui/robert_input.hpp
    gui/direction_list_dialog.cpp
    gui/direction_list_dialog.hpp
    gui/main_window.cpp
    gui/main_window.hpp
    gui/main_window_auswertung.cpp
    gui/main_window_korrektur.cpp
    gui/main_window_a18.cpp
    gui/main_window_convert.cpp
    gui/main_window_help.cpp
    gui/main_window_wander.cpp
    gui/main_window_statist.cpp
    gui/main_window_statist.hpp
    gui/main_window_statist_cond.cpp
    gui/main_window_statist_list.cpp
    gui/main_window_uhr.cpp
    gui/main_window_dynamo.cpp
    gui/main_window_rhythm.cpp
    gui/main_window_coord.cpp
    gui/main_window_arabic.cpp
    gui/main_window_ingress.cpp
    gui/main_window_avh.cpp
    gui/main_window_pair.cpp
    gui/main_window_multi.cpp
    gui/main_window_print.cpp
    gui/print_pages.cpp
    gui/print_pages.hpp
    gui/zodiac_cells.cpp
    gui/painter.cpp
    gui/painter.hpp
    gui/place_dialog.cpp
    gui/place_dialog.hpp
    gui/place_hub.cpp
    gui/place_hub.hpp
    gui/transit_list_dialog.cpp
    gui/transit_list_dialog.hpp
    gui/wheel_widget.cpp
    gui/wheel_widget.hpp
    gui/zeitzon_dialog.cpp
    gui/zeitzon_dialog.hpp
    gui/zone_dialog.cpp
    gui/zone_dialog.hpp
  )
  target_link_libraries(horcom_gui_core PUBLIC horcom Qt6::Widgets Qt6::Svg Qt6::PrintSupport)
  target_include_directories(horcom_gui_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/gui)
  target_compile_definitions(horcom_gui_core PUBLIC HORCOM_VERSION_STRING="${HORCOM_VERSION}")
  qt_add_executable(horcom_gui
    gui/main.cpp
  )
  qt_add_resources(horcom_gui "identity"
    PREFIX "/"
    BASE ${CMAKE_SOURCE_DIR}/assets
    FILES ${CMAKE_SOURCE_DIR}/assets/logo.svg ${CMAKE_SOURCE_DIR}/assets/arrow-up.svg ${CMAKE_SOURCE_DIR}/assets/arrow-down.svg ${CMAKE_SOURCE_DIR}/assets/arrow-up-dim.svg ${CMAKE_SOURCE_DIR}/assets/arrow-down-dim.svg ${CMAKE_SOURCE_DIR}/assets/arrow-left.svg ${CMAKE_SOURCE_DIR}/assets/arrow-right.svg ${CMAKE_SOURCE_DIR}/assets/arrow-up-ink.svg ${CMAKE_SOURCE_DIR}/assets/arrow-down-ink.svg ${CMAKE_SOURCE_DIR}/assets/arrow-up-faint.svg ${CMAKE_SOURCE_DIR}/assets/arrow-down-faint.svg ${CMAKE_SOURCE_DIR}/assets/arrow-left-ink.svg ${CMAKE_SOURCE_DIR}/assets/arrow-right-ink.svg
  )
  # Robert Rettig's own symbol drawings from symbbmp, the large variant
  file(GLOB HORCOM_SYMB ${CMAKE_SOURCE_DIR}/assets/symb/*.png)
  qt_add_resources(horcom_gui "symbols"
    PREFIX "/symb"
    BASE ${CMAKE_SOURCE_DIR}/assets/symb
    FILES ${HORCOM_SYMB}
  )
  target_link_libraries(horcom_gui PRIVATE horcom_gui_core)
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

# the FHS layout of Linux packages and the AppImage, the binaries in
# bin, the shipped data under share/horcom where both front ends look
# for it. Windows keeps its flat zip beside the executables
if(UNIX AND NOT APPLE)
  include(GNUInstallDirs)
  set(HORCOM_APP_ID "io.github.horcom_rr.horcom")
  set(HORCOM_LINUX_DIR "${CMAKE_SOURCE_DIR}/packaging/linux")
  target_compile_definitions(horcom_cli PRIVATE
    HORCOM_INSTALL_DATADIR="${CMAKE_INSTALL_FULL_DATADIR}/horcom/data")
  install(TARGETS horcom_cli RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
  install(DIRECTORY ${CMAKE_SOURCE_DIR}/data/ DESTINATION ${CMAKE_INSTALL_DATADIR}/horcom/data)
  install(FILES ${CMAKE_SOURCE_DIR}/LICENSE ${CMAKE_SOURCE_DIR}/README.md
          DESTINATION ${CMAKE_INSTALL_DOCDIR})
  if(TARGET horcom_gui)
    install(TARGETS horcom_gui RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(FILES ${HORCOM_LINUX_DIR}/${HORCOM_APP_ID}.desktop
            DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
    install(FILES ${HORCOM_LINUX_DIR}/${HORCOM_APP_ID}.metainfo.xml
            DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
    install(FILES ${CMAKE_SOURCE_DIR}/assets/logo.svg
            DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
            RENAME ${HORCOM_APP_ID}.svg)
    install(FILES ${CMAKE_SOURCE_DIR}/assets/logo.png
            DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/512x512/apps
            RENAME ${HORCOM_APP_ID}.png)
  endif()
endif()
