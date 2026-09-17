// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/chart/arabic.hpp"

#include <fstream>

#include "horcom/chart/bodies.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/data/encoding.hpp"
#include "horcom/data/statist_eval.hpp"

namespace horcom {

namespace {

// one moving term of a formula, mirroring the c4 codes of his own
// point files, zero a body, twelve a cusp, thirteen the ruler of a
// house, fourteen a fixed ecliptic degree
struct Term {
  int kind = 0;
  int value = 0;
};

std::string tag_of(int slot) {
  return std::string(body::kName[static_cast<std::size_t>(slot)]);
}

// resolves one term against the chart, label as his columns spelled it
double term_value(const Chart& c, const Term& t, std::string& label) {
  switch (t.kind) {
    case 12:
      label = "H" + std::to_string(t.value);
      return c.houses.cusp[static_cast<std::size_t>(t.value)];
    case 13: {
      label = "Hv" + std::to_string(t.value);
      const int kp = sign_ruler(c.houses.cusp[static_cast<std::size_t>(t.value)], false);
      return kp > 0 ? c.b[static_cast<std::size_t>(kp)].el : 0.0;
    }
    case 14:
      label = std::to_string(t.value) + "\xC2\xB0";
      return t.value * kDegToRad;
    default:
      label = tag_of(t.value);
      return c.b[static_cast<std::size_t>(t.value)].el;
  }
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
std::vector<ArabicPart> arabic_parts(const Chart& chart, ArabicFormula af, const std::filesystem::path& own_dir) {
  std::vector<ArabicPart> out;
  const double ac = chart.houses.cusp[1];
  const double dc = chart.houses.cusp[7];
  const double mc = chart.houses.cusp[10];
  const auto el = [&chart](int slot) { return chart.b[static_cast<std::size_t>(slot)].el; };
  const auto ruler = [&chart](int house, std::string& label) {
    label = "Hv" + std::to_string(house);
    const int kp = sign_ruler(chart.houses.cusp[static_cast<std::size_t>(house)], false);
    return kp > 0 ? chart.b[static_cast<std::size_t>(kp)].el : 0.0;
  };
  const auto body_part = [&](const std::string& name, const std::string& remark, const std::string& base_label, double base, int j, int k) {
    out.push_back(make_part(chart, af, name, remark, base_label, base, tag_of(j), el(j), tag_of(k), el(k)));
  };
  const char* mahl = "Von B.MAHL";
  //RR die Tabelle von arabt, Basis, erstes und zweites Glied
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

  // the user's own points, three terms per record from the two files
  if (!own_dir.empty()) {
    std::ifstream f1(own_dir / "arabtei1.int", std::ios::binary);
    std::ifstream f2(own_dir / "arabtei2.int", std::ios::binary);
    if (!f1) {
      f1.open(own_dir / "ARABTEI1.INT", std::ios::binary);
    }
    if (!f2) {
      f2.open(own_dir / "ARABTEI2.INT", std::ios::binary);
    }
    if (f1 && f2) {
      std::vector<std::pair<std::string, std::string>> names;
      std::string rec(39, '\0');
      while (f1.read(rec.data(), 39)) {
        names.emplace_back(cp1252_to_utf8(rec.substr(4, 21)), cp1252_to_utf8(rec.substr(25, 14)));
      }
      std::vector<std::array<Term, 4>> defs(names.size());
      std::string trm(16, '\0');
      while (f2.read(trm.data(), 16)) {
        const int jj = field_int(trm, 0);
        const int zg = field_int(trm, 4);
        if (jj >= 0 && jj < static_cast<int>(defs.size()) && zg >= 1 && zg <= 3) {
          defs[static_cast<std::size_t>(jj)][static_cast<std::size_t>(zg)] = {field_int(trm, 12), field_int(trm, 8)};
        }
      }
      for (std::size_t i = 0; i < names.size(); ++i) {
        std::string bl;
        std::string al;
        std::string blb;
        const double base = term_value(chart, defs[i][1], bl);
        const double a = term_value(chart, defs[i][2], al);
        const double b = term_value(chart, defs[i][3], blb);
        ArabicPart p = make_part(chart, af, names[i].first, names[i].second, bl, base, al, a, blb, b);
        out.push_back(std::move(p));
      }
    }
  }
  return out;
}

}  // namespace horcom
