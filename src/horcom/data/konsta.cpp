// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/data/konsta.hpp"

#include <cstdlib>
#include <fstream>

#include "horcom/core/constants.hpp"
#include "horcom/data/encoding.hpp"
#include "horcom/data/gfa_stream.hpp"

namespace horcom {

ChartSettings Konsta::chart_settings() const {
  ChartSettings s;
  s.houses = static_cast<HouseSystem>(haw);
  s.apparent = static_cast<ApparentMode>(appa);
  s.topocentric_parallax = par == 1.0;
  s.true_apogee = apogw;
  s.true_node = moknw;
  s.apparent_sidereal = stzw == 1;
  s.nk = nk;
  int active = 0;
  for (int i = 1; i <= 22; ++i) {
    if (nk[static_cast<std::size_t>(i)] > 0) {
      ++active;
    }
  }
  s.extra_bodies = active > 0;
  return s;
}

AspectSettings Konsta::aspect_settings() const {
  AspectSettings a;
  a.orb = orb;
  a.equal_probability = orbe_on;
  int n = nasp;
  if (orbe_on && n == 16) {
    n = 12;
  }
  a.divisors = n;
  for (int i = 0; i <= 40; ++i) {
    a.weight[static_cast<std::size_t>(i)] = or_weight[static_cast<std::size_t>(i)];
  }
  if (orbe_on) {
    for (int i = 0; i <= 14; ++i) {
      a.orbe[static_cast<std::size_t>(i)] = kDegToRad * std::abs(std::strtod(orb_text[static_cast<std::size_t>(i)].c_str(), nullptr));
    }
  }
  return a;
}

Konsta robert_profile() {
  Konsta k;
  k.haw = 1;
  k.haus = "Placidus";
  k.appa = 1;
  k.appa_name = "App.1";
  k.gen = 2;
  k.gena = " Ephem ::App.1,MitParall.";
  k.apogw = true;
  k.moknw = true;
  k.orb = 1.0;
  k.voll = true;
  k.nasp = 12;
  k.bdsp = true;
  k.par = 1.0;
  k.fza = 3.808214611307;
  k.begz = 1;
  k.hard = 1;
  k.bres = 1;
  k.brep = 1;
  k.zwhd = true;
  k.horm = 1;
  k.orbe_on = true;
  k.slist = true;
  k.plusl = true;
  k.col_dial = 16777088;
  k.col_backg = 8421440;
  k.plinv = 3;
  k.prenbl = 1;
  k.halbs = 1;
  k.comp_hand = true;
  k.selbst_cl_st = true;
  k.eigfarb = true;
  k.elem = 2;
  k.gebherr_dop = true;
  k.haus1_dop = true;
  k.entf = 1;
  k.stzw = 1;
  k.erase_ = 1;
  k.fixpunkt = 2;
  k.or_weight = {100, 150, 150, 1,   100, 100, 100, 100, 100, 100, 100, 100, 100, 150,
                 150, 100, 0,   0,   0,   100, 100, 100, 100, 100, 100, 100, 100, 100,
                 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100};
  k.orb_text = {"",      " 5.40", " 6.80", " 3.10", " 3.00", " 1.60", " 2.60", " 1.00",
                " 1.50", " 1.00", " 1.50", " 0.60", " 1.50", " 2.00", " 1.00"};
  // nk stays all zero, he kept no extras preconfigured
  k.aspli_flag = {0, 0, 2, 3, 4, 5, 6, 7, 8, 0, 0, 0, 0, 13, 14, 15, 0, 0, 0, 0};
  k.aspli_col = {0, 0,        255, 65280,    255,      16711680, 65280, 16711680, 255, 8388736,
                 0, 0,        65280, 16711680, 16711680, 255,     8388736, 0,      0,   65280};
  k.aspst = {0, 0, 0, 0, 0, 0, 0, 3, 2, 2, 0, 0, 2, 0, 3, 2, 2, 0, 0, 2};
  k.cols = {0, 255, 65280, 16776960, 16711680};
  k.pn = {0, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 1};
  return k;
}

Konsta parse_konsta(std::string_view text) {
  GfaReader r(text);
  Konsta k;
  k.haw = r.next_int();
  k.haus = cp1252_to_utf8(r.next_string());
  k.appa = r.next_int();
  k.appa_name = cp1252_to_utf8(r.next_string());
  k.gen = r.next_int();
  k.gena = cp1252_to_utf8(r.next_string());
  k.apogw = r.next_bool();
  k.moknw = r.next_bool();
  k.orb = r.next_number();
  k.klsy = r.next_bool();
  k.klsyt = r.next_bool();
  k.sext = r.next_number();
  k.pziff = r.next_number();
  k.voll = r.next_bool();
  k.nasp = r.next_int();
  k.bdsp = r.next_bool();
  k.ryt = r.next_bool();
  k.par = r.next_number();
  k.fza = r.next_number();
  k.begz = r.next_int();
  k.begz_name = cp1252_to_utf8(r.next_string());
  k.hard = r.next_int();
  k.lfm = r.next_int();
  k.bres = r.next_int();
  k.brep = r.next_int();
  k.zwhd = r.next_bool();
  k.kard = r.next_bool();
  k.horm = r.next_int();
  k.orbe_on = r.next_bool();
  k.stats = r.next_bool();
  k.slist = r.next_bool();
  k.plusl = r.next_bool();
  k.col_dial = r.next_int();
  k.col_backg = r.next_int();
  k.farbs = r.next_bool();
  k.linie = r.next_bool();
  k.plinv = r.next_int();
  k.tabstop = r.next_int();
  k.prenbl = r.next_int();
  k.halbs = r.next_int();
  k.zeichen = r.next_bool();
  k.comp_mstz = r.next_bool();
  k.comp_hand = r.next_bool();
  k.gitter = r.next_bool();
  k.selbst_cl_st = r.next_bool();
  k.eigfarb = r.next_bool();
  k.farbp = r.next_bool();
  k.weiss = r.next_bool();
  k.elem = r.next_int();
  k.gebherr_dop = r.next_bool();
  k.haus1_dop = r.next_bool();
  k.jdgross = r.next_number();
  k.zal_grossj = r.next_int();
  k.entf = r.next_int();
  k.stzw = r.next_int();
  k.erase_ = r.next_int();
  k.fixpunkt = r.next_int();
  k.fixpunkt_name = cp1252_to_utf8(r.next_string());
  k.fixpunkt_rh = cp1252_to_utf8(r.next_string());
  k.lpktg = r.next_bool();
  k.anzweg = r.next_int();
  k.halbs_dir = r.next_int();
  k.nursymb = r.next_int();
  k.lin_inv = r.next_bool();
  for (int i = 0; i <= 40; ++i) {
    k.or_weight[static_cast<std::size_t>(i)] = r.next_int();
  }
  if (k.orbe_on) {
    for (int i = 0; i <= 14; ++i) {
      k.orb_text[static_cast<std::size_t>(i)] = r.next_string();
    }
  }
  for (int i = 1; i <= 22; ++i) {
    k.nk[static_cast<std::size_t>(i)] = r.next_int();
  }
  for (int i = 1; i <= 19; ++i) {
    k.aspli_flag[static_cast<std::size_t>(i)] = r.next_int();
  }
  for (int i = 1; i <= 19; ++i) {
    k.aspli_col[static_cast<std::size_t>(i)] = r.next_int();
  }
  for (int i = 1; i <= 19; ++i) {
    k.aspst[static_cast<std::size_t>(i)] = r.next_int();
  }
  for (int i = 1; i <= 4; ++i) {
    k.cols[static_cast<std::size_t>(i)] = r.next_int();
  }
  for (int i = 1; i <= 15; ++i) {
    k.pn[static_cast<std::size_t>(i)] = r.next_int();
  }
  return k;
}

std::optional<Konsta> load_konsta(const std::filesystem::path& path) {
  std::ifstream f(path, std::ios::binary);
  if (!f) {
    //RR FORMAT-ÄNDERUNG ! ALLE VORGABEN in HORCOM neu FESTLEGEN !
    return std::nullopt;
  }
  std::string text((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
  return parse_konsta(text);
}

std::string format_konsta(const Konsta& k) {
  const auto q = [](const std::string& utf8) { return gfa_quoted(utf8_to_cp1252(utf8)); };
  const auto n = [](double v) { return gfa_number(v); };
  const auto b = [](bool v) { return gfa_bool(v); };
  std::string out;
  const auto line = [&](const std::vector<std::string>& fields) {
    out += gfa_write_line(fields);
    out += "\r\n";
  };
  // the line grouping of kon_dsp, kept for byte faithful output
  line({n(k.haw), q(k.haus), n(k.appa), q(k.appa_name), n(k.gen), q(k.gena), b(k.apogw), b(k.moknw), n(k.orb), b(k.klsy)});
  line({b(k.klsyt), n(k.sext), n(k.pziff), b(k.voll), n(k.nasp), b(k.bdsp)});
  line({b(k.ryt), n(k.par), n(k.fza), n(k.begz), q(k.begz_name), n(k.hard), n(k.lfm), n(k.bres), n(k.brep), b(k.zwhd), b(k.kard), n(k.horm)});
  line({b(k.orbe_on), b(k.stats), b(k.slist), b(k.plusl), n(k.col_dial), n(k.col_backg), b(k.farbs), b(k.linie)});
  line({n(k.plinv), n(k.tabstop), n(k.prenbl), n(k.halbs), b(k.zeichen), b(k.comp_mstz)});
  line({b(k.comp_hand), b(k.gitter), b(k.selbst_cl_st), b(k.eigfarb), b(k.farbp), b(k.weiss)});
  line({n(k.elem), b(k.gebherr_dop), b(k.haus1_dop), n(k.jdgross), n(k.zal_grossj), n(k.entf), n(k.stzw)});
  line({n(k.erase_), n(k.fixpunkt), q(k.fixpunkt_name), q(k.fixpunkt_rh), b(k.lpktg), n(k.anzweg), n(k.halbs_dir), n(k.nursymb), b(k.lin_inv)});
  for (int i = 0; i <= 40; ++i) {
    line({n(k.or_weight[static_cast<std::size_t>(i)])});
  }
  if (k.orbe_on) {
    for (int i = 0; i <= 14; ++i) {
      line({q(k.orb_text[static_cast<std::size_t>(i)])});
    }
  }
  for (int i = 1; i <= 22; ++i) {
    line({n(k.nk[static_cast<std::size_t>(i)])});
  }
  for (int i = 1; i <= 19; ++i) {
    line({n(k.aspli_flag[static_cast<std::size_t>(i)])});
  }
  for (int i = 1; i <= 19; ++i) {
    line({n(k.aspli_col[static_cast<std::size_t>(i)])});
  }
  for (int i = 1; i <= 19; ++i) {
    line({n(k.aspst[static_cast<std::size_t>(i)])});
  }
  for (int i = 1; i <= 4; ++i) {
    line({n(k.cols[static_cast<std::size_t>(i)])});
  }
  for (int i = 1; i <= 15; ++i) {
    line({n(k.pn[static_cast<std::size_t>(i)])});
  }
  return out;
}

bool save_konsta(const std::filesystem::path& path, const Konsta& k) {
  std::ofstream f(path, std::ios::binary | std::ios::trunc);
  if (!f) {
    return false;
  }
  const std::string text = format_konsta(k);
  f.write(text.data(), static_cast<std::streamsize>(text.size()));
  return static_cast<bool>(f);
}

}  // namespace horcom
