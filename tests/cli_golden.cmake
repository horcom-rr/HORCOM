# SPDX-License-Identifier: GPL-3.0-or-later
# horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
# Copyright (c) 2026 Dominik Schwimmbeck
#
# Runs the command line tool on every chart of golden/manifest.json and
# compares its output with the recorded new side, byte for byte up to the
# line endings. Charts beyond the polar circle were recorded in house mode
# 8, AC and MC only like the original, see golden/README.md. A chart that
# differs leaves its output in OUT for a diff
#
#   cmake -DCLI=<horcom> -DDATA=<data> -DGOLDEN=<tests/golden> -DOUT=<dir> -P cli_golden.cmake

file(READ "${GOLDEN}/manifest.json" manifest)
string(JSON count LENGTH "${manifest}")
math(EXPR last "${count} - 1")
set(failed "")
foreach(i RANGE ${last})
  string(JSON id GET "${manifest}" ${i} id)
  string(JSON date GET "${manifest}" ${i} date_ut)
  string(JSON time GET "${manifest}" ${i} time_ut)
  string(JSON lon GET "${manifest}" ${i} lon_deg_east)
  string(JSON lat GET "${manifest}" ${i} lat_deg_north)
  string(REGEX REPLACE "^([0-9]+)-([0-9]+)-([0-9]+)$" "\\3.\\2.\\1" day "${date}")
  set(houses 1)
  if(lat GREATER 66.5 OR lat LESS -66.5)
    set(houses 8)
  endif()
  execute_process(
    COMMAND "${CLI}" --data "${DATA}" --date "${day}" --time "${time}" --lon "${lon}" --lat "${lat}"
            --parallax --true-node --houses ${houses}
    OUTPUT_VARIABLE got ERROR_VARIABLE err RESULT_VARIABLE rc)
  file(READ "${GOLDEN}/chart${id}_new_koordinaten.txt" want)
  string(REPLACE "\r\n" "\n" got "${got}")
  string(REPLACE "\r\n" "\n" want "${want}")
  if(NOT rc EQUAL 0 OR NOT got STREQUAL want)
    list(APPEND failed "${id}")
    file(WRITE "${OUT}/cli_golden_chart${id}.txt" "${got}${err}")
  endif()
endforeach()
if(failed)
  string(REPLACE ";" ", " failed "${failed}")
  message(FATAL_ERROR "charts ${failed} differ from tests/golden, their output is in ${OUT}")
endif()
message(STATUS "${count} golden charts reproduced")
