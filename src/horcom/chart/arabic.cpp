// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/chart/arabic.hpp"

#include <cstdio>
#include <fstream>

#include "horcom/chart/bodies.hpp"
#include "horcom/chart/signs.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/data/encoding.hpp"
#include "horcom/data/statist_eval.hpp"

namespace horcom {

namespace {

// the c4 codes of his own point files
constexpr int kTermBody = 0;
constexpr int kTermCusp = 12;
constexpr int kTermRuler = 13;
constexpr int kTermDegree = 14;
constexpr std::size_t kNameBytes = 39;
constexpr std::size_t kTermBytes = 16;
// the whole degrees of a circle and of a sign for his degree terms
constexpr auto kWholeCircle = static_cast<int>(kDegPerCircle);
constexpr auto kWholeSign = static_cast<int>(kDegPerSign);

std::string tag_of(int slot) {
  return std::string(body::kName[static_cast<std::size_t>(slot)]);
}

// resolves one term against the chart, label as his columns spelled it
double term_value(const Chart& c, const OwnArabicTerm& t, bool classic, std::string& label) {
  switch (t.kind) {
    case kTermCusp:
      label = "H" + std::to_string(t.value);
      return c.houses.cusp[static_cast<std::size_t>(t.value)];
    case kTermRuler: {
      label = "Hv" + std::to_string(t.value);
      const int kp = sign_ruler(c.houses.cusp[static_cast<std::size_t>(t.value)], classic);
      return kp > 0 ? c.b[static_cast<std::size_t>(kp)].el : 0.0;
    }
    case kTermDegree: {
      // his gz4$, "%2d°" + the sign tag, 105 reads 15°CN
      const int deg = ((t.value % kWholeCircle) + kWholeCircle) % kWholeCircle;
      char buf[16];
      std::snprintf(buf, sizeof(buf), "%2d\xC2\xB0%s", deg % kWholeSign, kSignTag[deg / kWholeSign]);
      label = buf;
      return deg * kDegToRad;
    }
    default:
      label = tag_of(t.value);
      return c.b[static_cast<std::size_t>(t.value)].el;
  }
}

// the lower case name the port writes, his upper case one when it exists
std::filesystem::path own_file(const std::filesystem::path& dir, const char* lower, const char* upper) {
  std::error_code ec;
  if (std::filesystem::exists(dir / upper, ec)) {
    return dir / upper;
  }
  return dir / lower;
}

// ported from HORCOM art12, the one formula every part follows
ArabicPart make_part(const Chart& c, ArabicFormula af, const std::string& name, const std::string& remark, const std::string& base_label, double base, const std::string& a_label, double a, const std::string& b_label, double b, bool plus_form = false) {
  int n = ta_na(c.b[body::kAscendant].el, c.b[body::kSun].el);
  if (af == ArabicFormula::kAlwaysDay) {
    n = 1;
  } else if (af == ArabicFormula::kAlwaysNight) {
    n = 2;
  }
  const char* joiner = plus_form ? " " : "-";
  double di = 0.0;
  std::string d;
  if (n == 1) {
    //RR Tag
    di = norm_rad(a - b);
    d = " + " + a_label + joiner + b_label;
  } else {
    di = norm_rad(b - a);
    d = " + " + b_label + joiner + a_label;
    if (plus_form) {
      di = norm_rad(a + b);
    }
  }
  ArabicPart p;
  p.name = name;
  p.remark = remark;
  p.formula = base_label + d;
  p.la = norm_rad(base + di);
  return p;
}

// the four character ASCII number fields of the ARABTEI records
int field_int(const std::string& s, std::size_t pos) {
  if (pos + 4 > s.size()) {
    return 0;
  }
  return std::atoi(s.substr(pos, 4).c_str());
}

}  // namespace

// ported from HORCOM arabt with the term reader of arabl0
std::vector<ArabicPart> arabic_parts(const Chart& chart, ArabicFormula af, const std::filesystem::path& own_dir, bool classic_rulers) {
  std::vector<ArabicPart> out;
  const double ac = chart.houses.cusp[1];
  const double dc = chart.houses.cusp[7];
  const double mc = chart.houses.cusp[10];
  const auto el = [&chart](int slot) { return chart.b[static_cast<std::size_t>(slot)].el; };
  const auto ruler = [&chart, classic_rulers](int house, std::string& label) {
    label = "Hv" + std::to_string(house);
    const int kp = sign_ruler(chart.houses.cusp[static_cast<std::size_t>(house)], classic_rulers);
    return kp > 0 ? chart.b[static_cast<std::size_t>(kp)].el : 0.0;
  };
  const auto body_part = [&](const std::string& name, const std::string& remark, const std::string& base_label, double base, int j, int k) {
    out.push_back(make_part(chart, af, name, remark, base_label, base, tag_of(j), el(j), tag_of(k), el(k)));
  };
  const char* mahl = "Von B.MAHL";
  // the table of arabt, base, first and second Glied
  body_part("Intuition", mahl, "AC", ac, 8, 10);
  body_part("Medialität", mahl, "AC", ac, 2, 8);
  body_part("Occultismus", "", "AC", ac, 9, 8);
  body_part("Jenseits-Einfluß", mahl, "AC", ac, 10, 9);
  body_part("Ehefrau Mann", mahl, "DC", dc, 4, 2);
  body_part("Ehemann Frau", mahl, "DC", dc, 4, 5);
  body_part("Glück", "", "AC", ac, 2, 1);
  body_part("Liebe und Ehe", "", "AC", ac, 4, 1);
  body_part("Mutter", "", "AC", ac, 2, 4);
  body_part("Vater", "", "AC", ac, 7, 1);
  body_part("Brüder u.Schwestern", "", "AC", ac, 6, 7);
  body_part("Kinder", "", "AC", ac, 2, 6);
  body_part("Krankheit", "", "AC", ac, 5, 7);
  body_part("Esprit", "", "AC", ac, 5, 3);
  body_part("Wissen,Lernen", "UNGENANNT", "AC", ac, 3, body::kChiron);
  body_part("Kunst,Poesie", "", "AC", ac, 3, 4);
  {
    std::string l9;
    std::string l12;
    const double a = ruler(9, l9);
    const double b = ruler(12, l12);
    out.push_back(make_part(chart, af, "Astrologie", "", "AC", ac, l9, a, l12, b));
  }
  body_part("Todesfall", mahl, "AC", ac, 5, 10);
  body_part("Berühmtheit", "", "AC", ac, 6, 1);
  body_part("Erhöhung", "", "AC", ac, 2, 5);
  body_part("Prozesse", "", "AC", ac, 5, 6);
  body_part("Sexual-Punkt", mahl, "AC", ac, 5, body::kApogee);
  body_part("Beruflicher Erfolg", "", "MC", mc, 2, 1);
  body_part("Erbschaft,Geschenke", "", "AC", ac, 2, 7);
  {
    // the wealth point adds instead of subtracting on a night birth,
    // his starred exception
    std::string l2;
    const double a = ruler(2, l2);
    out.push_back(make_part(chart, af, "Vermögen,Geld     *", "", "AC", ac, l2, a, "  ", 0.0, true));
  }
  body_part("Handels-Erfolg", mahl, "AC", ac, 6, 3);
  out.push_back(make_part(chart, af, "Immobilien", "Von L.RATHKE", "AC", ac, "15\xC2\xB0""CN", 105.0 * kDegToRad, "SA", el(7)));
  out.push_back(make_part(chart, af, "Musik", "UNGENANNT", "AC", ac, "VE", el(4), "0\xC2\xB0PS", 330.0 * kDegToRad));
  body_part("Reisen", "", "AC", ac, 3, 2);
  out.push_back(make_part(chart, af, "Reisen im Flugzeug", mahl, "AC", ac, "UR", el(8), "15\xC2\xB0""AQ", 315.0 * kDegToRad));
  {
    std::string l9;
    const double b = ruler(9, l9);
    out.push_back(make_part(chart, af, "Reisen zu Lande", "", "AC", ac, "H9", chart.houses.cusp[9], l9, b));
  }
  body_part("'Sokrates-Konstell.'", "UNGENANNT", "AC", ac, 1, body::kChiron);
  {
    std::string l8;
    const double b = ruler(8, l8);
    out.push_back(make_part(chart, af, "Gefahren", "", "AC", ac, "ME", el(3), l8, b));
  }
  {
    std::string l12;
    const double b = ruler(12, l12);
    out.push_back(make_part(chart, af, "Feindschaften", "", "AC", ac, "H12", chart.houses.cusp[12], l12, b));
  }
  // the point of captivity rides on the already built point of luck
  out.push_back(make_part(chart, af, "Gefangenschaft,Exil", "GL = Glücksp.", "AC", ac, "GL", out[6].la, "SA", el(7)));
  out.push_back(make_part(chart, af, "Erlösung", "", "SO", el(1), "MO", el(2), "AC", ac));
  out.push_back(make_part(chart, af, "Tod", "", "SA", el(7), "H8", chart.houses.cusp[8], "MO", el(2)));

  //RR EIGENPUNKTE REKONSTR.
  // his arabte, the own points take the rows from the top
  if (!own_dir.empty()) {
    const std::vector<OwnArabicPoint> own = read_own_arabic(own_dir);
    for (std::size_t i = 0; i < own.size() && i < out.size(); ++i) {
      const OwnArabicPoint& o = own[i];
      bool valid = true;
      for (const OwnArabicTerm& t : o.terms) {
        valid = valid && own_term_valid(t);
      }
      if (!valid) {
        ArabicPart bad;
        bad.name = o.name;
        bad.remark = o.remark;
        bad.own = true;
        bad.valid = false;
        out[i] = bad;
        continue;
      }
      std::string bl;
      std::string al;
      std::string blb;
      const double base = term_value(chart, o.terms[0], classic_rulers, bl);
      const double a = term_value(chart, o.terms[1], classic_rulers, al);
      const double b = term_value(chart, o.terms[2], classic_rulers, blb);
      ArabicPart p = make_part(chart, af, o.name, o.remark, bl, base, al, a, blb, b);
      p.own = true;
      out[i] = std::move(p);
    }
  }
  return out;
}

bool own_term_valid(const OwnArabicTerm& t) {
  switch (t.kind) {
    case kTermBody:
      // his ausw_pl_hs offered the Fixpunkt, SO to MC, the Black Moon and Chiron
      return (t.value >= body::kFixpunkt && t.value <= body::kMc) || t.value == body::kApogee || t.value == body::kChiron;
    case kTermCusp:
    case kTermRuler:
      return t.value >= 1 && t.value <= 12;
    case kTermDegree:
      return t.value >= 0 && t.value <= kWholeCircle;
    default:
      return false;
  }
}

// ported from arab_eig, both files or none
std::vector<OwnArabicPoint> read_own_arabic(const std::filesystem::path& dir) {
  std::vector<OwnArabicPoint> out;
  std::ifstream f1(own_file(dir, "arabtei1.int", "ARABTEI1.INT"), std::ios::binary);
  std::ifstream f2(own_file(dir, "arabtei2.int", "ARABTEI2.INT"), std::ios::binary);
  if (!f1 || !f2) {
    return out;
  }
  std::string rec(kNameBytes, '\0');
  while (f1.read(rec.data(), static_cast<std::streamsize>(kNameBytes))) {
    const int index = field_int(rec, 0);
    if (index < 0 || index >= kMaxOwnArabic) {
      continue;
    }
    if (static_cast<int>(out.size()) <= index) {
      out.resize(static_cast<std::size_t>(index) + 1);
    }
    std::string name = cp1252_to_utf8(rec.substr(4, 21));
    std::string remark = cp1252_to_utf8(rec.substr(25, 14));
    while (!name.empty() && name.back() == ' ') {
      name.pop_back();
    }
    while (!remark.empty() && remark.back() == ' ') {
      remark.pop_back();
    }
    out[static_cast<std::size_t>(index)].name = name;
    out[static_cast<std::size_t>(index)].remark = remark;
  }
  std::string trm(kTermBytes, '\0');
  while (f2.read(trm.data(), static_cast<std::streamsize>(kTermBytes))) {
    const int jj = field_int(trm, 0);
    const int zg = field_int(trm, 4);
    if (jj >= 0 && jj < static_cast<int>(out.size()) && zg >= 1 && zg <= 3) {
      // his FIELD #2,4 AS jj$,4 AS zz$,4 AS c3$,4 AS c4$
      out[static_cast<std::size_t>(jj)].terms[static_cast<std::size_t>(zg) - 1] = {field_int(trm, 12), field_int(trm, 8)};
    }
  }
  return out;
}

bool append_own_arabic_name(const std::filesystem::path& dir, int index, const std::string& name, const std::string& remark) {
  std::ofstream f(own_file(dir, "arabtei1.int", "ARABTEI1.INT"), std::ios::binary | std::ios::app);
  if (!f) {
    return false;
  }
  // his LSET jj$ = STR$(jj&,4), LSET an$ (21), LSET bem$ (14)
  const auto pad = [](std::string s, std::size_t n) {
    s = utf8_to_cp1252(s);
    s.resize(n, ' ');
    return s;
  };
  char num[8];
  std::snprintf(num, sizeof(num), "%4d", index);
  f << num << pad(name, 21) << pad(remark, 14);
  return static_cast<bool>(f);
}

bool append_own_arabic_term(const std::filesystem::path& dir, int index, int term, const OwnArabicTerm& t) {
  std::ofstream f(own_file(dir, "arabtei2.int", "ARABTEI2.INT"), std::ios::binary | std::ios::app);
  if (!f) {
    return false;
  }
  char buf[20];
  std::snprintf(buf, sizeof(buf), "%4d%4d%4d%4d", index, term, t.value, t.kind);
  f << buf;
  return static_cast<bool>(f);
}

void delete_own_arabic(const std::filesystem::path& dir) {
  std::error_code ec;
  std::filesystem::remove(own_file(dir, "arabtei1.int", "ARABTEI1.INT"), ec);
  std::filesystem::remove(own_file(dir, "arabtei2.int", "ARABTEI2.INT"), ec);
}

}  // namespace horcom
