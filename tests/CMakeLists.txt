add_executable(horcom_tests
  main.cpp
  test_angle.cpp
  test_calendar.cpp
  test_delta_t.cpp
  test_sidereal.cpp
  test_statist_eval.cpp
  test_statist_list.cpp
  test_statist.cpp
  test_stat_sheet.cpp
  test_vsop.cpp
  test_symbolic.cpp
  test_sunmoon.cpp
  test_kepler.cpp
  test_eph_file.cpp
  test_harmonics.cpp
  test_houses.cpp
  test_chart.cpp
  test_rectification.cpp
  test_transit_search.cpp
  test_arabic.cpp
  test_aspects.cpp
  test_histogram.cpp
  test_render.cpp
  test_data.cpp
  test_aaf_konsta.cpp
  test_rhythm.cpp
)

target_link_libraries(horcom_tests PRIVATE horcom)
target_include_directories(horcom_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/vendor)
# the vendored doctest header keeps its own warnings, newer compilers
# grow new ones and they are not ours to fix
if(MSVC)
  target_compile_options(horcom_tests PRIVATE /external:I ${CMAKE_CURRENT_SOURCE_DIR}/vendor /external:W0)
endif()
target_compile_definitions(horcom_tests PRIVATE HORCOM_TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/data")

add_test(NAME horcom_tests COMMAND horcom_tests)

# the shell under test, the MainWindow and its dialogs run headless on the
# offscreen platform, a probe struct reaches the panel state
if(TARGET horcom_gui_core)
  find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
  set(HORCOM_GUI_TEST_SOURCES
    gui/test_panel.cpp
    gui/test_korrektur.cpp
    gui/test_auswertung.cpp
    gui/test_diverses.cpp
    gui/test_print.cpp
    gui/test_hardcopy.cpp
    gui/test_a18.cpp
    gui/test_directions.cpp
    gui/test_entry.cpp
    gui/test_places.cpp
    gui/test_english.cpp
    gui/test_records.cpp
    gui/test_convert.cpp
    gui/test_wander.cpp
    gui/test_rhythm.cpp
    gui/test_coord.cpp
    gui/test_avh.cpp
    gui/test_statist.cpp
    gui/test_layout.cpp
    gui/test_painter.cpp
  )
  add_executable(horcom_gui_tests gui/gui_main.cpp ${HORCOM_GUI_TEST_SOURCES})
  # tests/gui/local holds untracked comparison drivers that replay the
  # screens of the original with charts from the local archive, their
  # cases are skipped unless asked for with --no-skip
  file(GLOB HORCOM_LOCAL_GUI_TESTS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/gui/local/*.cpp)
  target_sources(horcom_gui_tests PRIVATE ${HORCOM_LOCAL_GUI_TESTS})
  target_link_libraries(horcom_gui_tests PRIVATE horcom_gui_core)
  target_include_directories(horcom_gui_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/vendor)
  if(MSVC)
    target_compile_options(horcom_gui_tests PRIVATE /external:I ${CMAKE_CURRENT_SOURCE_DIR}/vendor /external:W0)
    target_precompile_headers(horcom_gui_tests PRIVATE ${HORCOM_GUI_PCH})
  endif()
  target_compile_definitions(horcom_gui_tests PRIVATE HORCOM_TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/data")
  # the English run loads the compiled translation of the shell
  if(TARGET horcom_gui AND TARGET Qt6::lrelease)
    add_dependencies(horcom_gui_tests horcom_gui)
    target_compile_definitions(horcom_gui_tests PRIVATE HORCOM_TEST_QM="${CMAKE_BINARY_DIR}/apps/horcom_en.qm")
  endif()
  file(GLOB HORCOM_TEST_SYMB ${CMAKE_SOURCE_DIR}/assets/symb/*.png)
  qt_add_resources(horcom_gui_tests "test_symbols"
    PREFIX "/symb"
    BASE ${CMAKE_SOURCE_DIR}/assets/symb
    FILES ${HORCOM_TEST_SYMB}
  )
  # the suite runs as shards of at most five cases of one file, ctest -j
  # runs them side by side. Each process keeps its settings and scratch
  # files apart (gui_main.cpp). The last shard of a file runs every case
  # from its first on, a case added after the configure runs there. A case
  # that hangs ends its shard through the watchdog of gui_main.cpp long
  # before the timeout
  set(HORCOM_GUI_SHARD_CASES 5)
  foreach(source IN LISTS HORCOM_GUI_TEST_SOURCES)
    get_filename_component(stem ${source} NAME_WE)
    file(STRINGS ${CMAKE_CURRENT_SOURCE_DIR}/${source} cases REGEX "^TEST_CASE[(]")
    list(LENGTH cases count)
    set(first 1)
    set(part 1)
    while(TRUE)
      math(EXPR last "${first} + ${HORCOM_GUI_SHARD_CASES} - 1")
      set(range "--first=${first}")
      if(last LESS count)
        list(APPEND range "--last=${last}")
      endif()
      add_test(NAME gui_${stem}_${part}
        COMMAND horcom_gui_tests "--source-file=*${stem}.cpp" "--test-case-exclude=layout:*" ${range})
      set_tests_properties(gui_${stem}_${part} PROPERTIES
        TIMEOUT 300
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
        ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>")
      if(NOT last LESS count)
        break()
      endif()
      math(EXPR first "${last} + 1")
      math(EXPR part "${part} + 1")
    endwhile()
  endforeach()
  # the layout checks measure real faces on a full HD screen, the boxes
  # size themselves against the screen. The offscreen platform reads its
  # screen from a file beside the test, the colon of a drive letter would
  # split the platform options so the file is named from the working
  # folder. On Windows it sets every letter as an empty box unless it is
  # pointed at the system fonts, elsewhere fontconfig finds them
  set(HORCOM_LAYOUT_ENV "QT_QPA_PLATFORM=offscreen:configfile=layout_screen.json")
  if(WIN32)
    file(TO_CMAKE_PATH "$ENV{WINDIR}/Fonts" HORCOM_SYSTEM_FONTS)
    list(APPEND HORCOM_LAYOUT_ENV "QT_QPA_FONTDIR=${HORCOM_SYSTEM_FONTS}")
  endif()
  add_test(NAME horcom_gui_layout COMMAND horcom_gui_tests "--test-case=layout:*")
  set_tests_properties(horcom_gui_layout PROPERTIES
    TIMEOUT 600
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/gui
    ENVIRONMENT "${HORCOM_LAYOUT_ENV}"
    ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>")
endif()
