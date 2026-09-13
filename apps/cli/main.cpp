// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

// The command line front end. Computes one chart and prints the
// coordinate table in the spirit of the original PLANETEN-KOORDINATEN
// screen, optionally writing the wheel as SVG.

#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

#include "horcom/chart/aspects.hpp"
#include "horcom/chart/chart.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/render/svg.hpp"
#include "horcom/render/wheel.hpp"

namespace {

using namespace horcom;

// the sign tags of the original zei$ table
constexpr const char* kSignTag[12] = {"AR", "TA", "GM", "CN", "LE", "VI", "LI", "SC", "SG", "CP", "AQ", "PS"};

std::string format_zodiac(double rad) {
  double deg = norm_deg(rad * kRadToDeg);
  const int sign = static_cast<int>(deg / kDegPerSign);
  const double in_sign = deg - sign * kDegPerSign;
  int total_sec = static_cast<int>(in_sign * 3600.0 + 0.5);
  const int d = total_sec / 3600;
  const int m = (total_sec / 60) % 60;
  const int sec = total_sec % 60;
  char buf[32];
  std::snprintf(buf, sizeof(buf), "%2d %s %02d'%02d\"", d, kSignTag[sign], m, sec);
  return buf;
}

std::string format_deg(double rad) {
  const double deg = rad * kRadToDeg;
  char buf[32];
  std::snprintf(buf, sizeof(buf), "%+9.4f", deg);
  return buf;
}

void usage() {
  std::cout << "horcom_cli --date DD.MM.YYYY --time HH:MM [options]\n"
               "  --lon DEG        geographic longitude, east positive\n"
               "  --lat DEG        geographic latitude, north positive\n"
               "  --data DIR       data directory, default ./data\n"
               "  --houses N       1 Placidus .. 7 Vehlow, 8 angles only, 9 none\n"
               "  --parallax       topocentric positions, his par = 1\n"
               "  --true-node      osculating lunar node\n"
               "  --true-apogee    osculating Black Moon\n"
               "  --extras         Chiron, asteroids, Quaoar and friends\n"
               "  --julian         force the Julian calendar\n"
               "  --svg FILE       write the chart wheel as SVG\n";
}

}  // namespace

int main(int argc, char** argv) {
  ChartInput in;
  ChartSettings s;
  std::string data_dir = "data";
  std::string svg_path;
  bool have_date = false;

  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    const auto next = [&]() -> const char* { return (i + 1 < argc) ? argv[++i] : ""; };
    if (arg == "--date") {
      int d = 0;
      int mo = 0;
      int y = 0;
      if (std::sscanf(next(), "%d.%d.%d", &d, &mo, &y) == 3) {
        in.date_ut.day = d;
        in.date_ut.month = mo;
        in.date_ut.year = y;
        have_date = true;
      }
    } else if (arg == "--time") {
      int h = 0;
      int m = 0;
      int sec = 0;
      const char* v = next();
      if (std::sscanf(v, "%d:%d:%d", &h, &m, &sec) >= 2) {
        in.date_ut.hour = h;
        in.date_ut.minute = m + sec / 60.0;
      }
    } else if (arg == "--lon") {
      in.lon_deg_east = std::atof(next());
    } else if (arg == "--lat") {
      in.lat_deg = std::atof(next());
    } else if (arg == "--data") {
      data_dir = next();
    } else if (arg == "--houses") {
      s.houses = static_cast<HouseSystem>(std::atoi(next()));
    } else if (arg == "--parallax") {
      s.topocentric_parallax = true;
    } else if (arg == "--true-node") {
      s.true_node = true;
    } else if (arg == "--true-apogee") {
      s.true_apogee = true;
    } else if (arg == "--extras") {
      s.enable_standard_extras();
    } else if (arg == "--julian") {
      s.calendar = Calendar::kJulian;
    } else if (arg == "--svg") {
      svg_path = next();
    } else {
      usage();
      return arg == "--help" ? 0 : 1;
    }
  }
  if (!have_date) {
    usage();
    return 1;
  }

  VsopTables vsop;
  try {
    vsop = VsopTables::load(data_dir + "/planets.ndx", data_dir + "/planets.dat");
  } catch (const std::exception& e) {
    std::cerr << "cannot load the planetary tables from " << data_dir << ", " << e.what() << "\n";
    return 1;
  }
  const Ephemerides eph{data_dir + "/eph"};
  const Chart chart = compute_chart(in, s, vsop, eph);
  if (!chart.ok) {
    std::cerr << "Geog. Breite zu gross fuer dieses Haeusersystem\n";
    return 1;
  }
  const AspectResult aspects = scan_aspects(chart, s, {});

  std::printf("horcom  |  %02d.%02d.%d  %02d:%05.2f UT  |  Lon %+.4f  Lat %+.4f\n",
              in.date_ut.day, in.date_ut.month, in.date_ut.year,
              static_cast<int>(in.date_ut.hour), in.date_ut.minute, in.lon_deg_east, in.lat_deg);
  std::printf("JD(UT) %.5f  Delta T %.2f min  ARMC %.4f  %s%s\n\n",
              chart.jd_ut, chart.delt_minutes, chart.armc_deg,
              std::string(chart.houses.name).c_str(),
              s.topocentric_parallax ? "  MitParall." : "");
  std::printf("%-4s %-14s %-10s %-10s %-10s\n", "", "LAENGE", "BREITE", "DEKLIN.", "GESCHW.");
  for (int slot = 0; slot < body::kSlotCount; ++slot) {
    const BodyState& b = chart.b[static_cast<std::size_t>(slot)];
    if (!b.present) {
      continue;
    }
    const std::string tag(body::kTag[static_cast<std::size_t>(slot)]);
    if (!b.valid) {
      std::printf("%-4s ausserhalb der Ephemeride\n", tag.c_str());
      continue;
    }
    const bool angle_slot = slot == body::kAscendant || slot == body::kMc;
    std::printf("%-4s %-14s %-10s %-10s %-10s%s\n", tag.c_str(), format_zodiac(b.el).c_str(),
                angle_slot ? "" : format_deg(b.eb).c_str(),
                angle_slot ? "" : format_deg(b.de).c_str(),
                angle_slot ? "" : format_deg(b.tb).c_str(),
                (!angle_slot && b.tb < 0.0) ? "  R" : "");
  }
  if (!(s.houses == HouseSystem::kAcMcOnly || s.houses == HouseSystem::kNone)) {
    std::printf("\nHAEUSER (%s)\n", std::string(chart.houses.name).c_str());
    for (int i = 1; i <= 12; ++i) {
      std::printf("%2d   %s\n", i, format_zodiac(chart.houses.cusp[static_cast<std::size_t>(i)]).c_str());
    }
  }
  std::printf("\nASPEKTE  konj %d  opp %d  trigon %d  quadrat %d  sextil %d\n",
              aspects.zh[1], aspects.zh[2], aspects.zh[3], aspects.zh[4], aspects.zh[6]);

  if (!svg_path.empty()) {
    const DisplayList dl = build_wheel(chart, s, aspects);
    std::ofstream f(svg_path, std::ios::binary);
    f << to_svg(dl);
    std::printf("Wheel written to %s\n", svg_path.c_str());
  }
  return 0;
}
