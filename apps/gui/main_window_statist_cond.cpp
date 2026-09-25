// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

// STATISTIK, the conditions of stat2. One factor of ausw_pl_hs, the
// object of obj_wahl, the window of such_wo with the inputs of stat_ausw
// and the join of od_un.

#include <limits>
#include <optional>
#include <utility>
#include <vector>

#include "choice_dialog.hpp"
#include "horcom/chart/bodies.hpp"
#include "horcom/chart/signs.hpp"
#include "horcom/core/constants.hpp"
#include "main_window_statist.hpp"
#include "robert_input.hpp"

namespace horcom {

namespace {

// the list rows of ausw_pl_hs past the bodies, his anm& to dnm&
constexpr int kRowHouse = 100;
constexpr int kRowRuler = 101;
constexpr int kRowLights = 102;
constexpr int kRowAll = 103;

}  // namespace

// ported from ausw_pl_hs with pl_h and the numw boxes
bool MainWindow::stat_pick(const StatSession& ss, bool groups, const QStringList& notes, StatPick& out,
                           const QString& running) {
  const bool helio = ss.helio;
  // ltp$(1) to ltp$(18), dashes for what the hrg mode lacks
  static constexpr const char* kFixed[kStatCuspSlotLast + 1] = {
      nullptr,
      QT_TRANSLATE_NOOP("horcom::MainWindow", " SONNE"),
      QT_TRANSLATE_NOOP("horcom::MainWindow", " MOND"),
      QT_TRANSLATE_NOOP("horcom::MainWindow", " MERKUR"),
      QT_TRANSLATE_NOOP("horcom::MainWindow", " VENUS"),
      QT_TRANSLATE_NOOP("horcom::MainWindow", " MARS"),
      QT_TRANSLATE_NOOP("horcom::MainWindow", " JUPITER"),
      QT_TRANSLATE_NOOP("horcom::MainWindow", " SATURN"),
      QT_TRANSLATE_NOOP("horcom::MainWindow", " URANUS"),
      QT_TRANSLATE_NOOP("horcom::MainWindow", " NEPTUN"),
      QT_TRANSLATE_NOOP("horcom::MainWindow", " PLUTO"),
      QT_TRANSLATE_NOOP("horcom::MainWindow", "  MONDKNOTEN N"),
      QT_TRANSLATE_NOOP("horcom::MainWindow", "  MONDKNOTEN S"),
      QT_TRANSLATE_NOOP("horcom::MainWindow", "  ASZENDENT"),
      QT_TRANSLATE_NOOP("horcom::MainWindow", "     MC"),
      QT_TRANSLATE_NOOP("horcom::MainWindow", "  HAUS 2"),
      QT_TRANSLATE_NOOP("horcom::MainWindow", "  HAUS 3"),
      QT_TRANSLATE_NOOP("horcom::MainWindow", "  HAUS 5"),
      QT_TRANSLATE_NOOP("horcom::MainWindow", "  HAUS 6")};
  // the extras at their nk& place
  static constexpr const char* kExtra[kStatExtraCount + 1] = {
      nullptr,
      QT_TRANSLATE_NOOP("horcom::MainWindow", "  SCHWARZER MOND  "),
      QT_TRANSLATE_NOOP("horcom::MainWindow", " CHIRON         a=13.61  e=0.38  i=6.94°  T=50 Jahre "),
      QT_TRANSLATE_NOOP("horcom::MainWindow", "    TRANSPLUTO  HYPOTHETISCH"),
      QT_TRANSLATE_NOOP("horcom::MainWindow", "  GLÜCKSPUNKT"),
      QT_TRANSLATE_NOOP("horcom::MainWindow", " CERES"),
      QT_TRANSLATE_NOOP("horcom::MainWindow", " PALLAS"),
      QT_TRANSLATE_NOOP("horcom::MainWindow", " JUNO"),
      QT_TRANSLATE_NOOP("horcom::MainWindow", " VESTA"),
      QT_TRANSLATE_NOOP("horcom::MainWindow", "    CUPIDO    HYPOTHETISCH"),
      QT_TRANSLATE_NOOP("horcom::MainWindow", "    HADES     HYP."),
      QT_TRANSLATE_NOOP("horcom::MainWindow", "    ZEUS      HYP."),
      QT_TRANSLATE_NOOP("horcom::MainWindow", "    KRONOS    HYP."),
      QT_TRANSLATE_NOOP("horcom::MainWindow", "    APOLLON   HYP."),
      QT_TRANSLATE_NOOP("horcom::MainWindow", "    ADMETOS   HYP."),
      QT_TRANSLATE_NOOP("horcom::MainWindow", "    VULKANUS  HYP."),
      QT_TRANSLATE_NOOP("horcom::MainWindow", "    POSEIDON  HYP."),
      QT_TRANSLATE_NOOP("horcom::MainWindow", " QUAOAR         a=43.25 e=0.035 i=  7.99°   T=284 Jr."),
      QT_TRANSLATE_NOOP("horcom::MainWindow", " KOMET HALLEY   a=17.94 e=0.97  i=162.24°   T= 76 Jr."),
      QT_TRANSLATE_NOOP("horcom::MainWindow", " PHOLUS         a=20.23 e=0.57  i= 24.70°   T= 91 Jr."),
      QT_TRANSLATE_NOOP("horcom::MainWindow", " DAMOKLES       a=11.82 e=0.87  i= 61.84°   T= 41 Jr."),
      QT_TRANSLATE_NOOP("horcom::MainWindow", " NESSUS         a=24.46 e=0.52  i= 15.66°   T=121 Jr."),
      QT_TRANSLATE_NOOP("horcom::MainWindow", " XENA           a=67.66 e=0.44  i= 44.12°   T=557 Jr.")};
  std::vector<std::pair<int, QString>> rows;
  for (int slot = body::kSun; slot <= kStatCuspSlotLast; ++slot) {
    const bool dash = helio && (slot == body::kSun || slot >= body::kNodeAsc);
    if (dash) {
      rows.emplace_back(-1, QString());
    } else if (helio && slot == body::kMoon) {
      rows.emplace_back(slot, tr(" TERRA"));
    } else {
      rows.emplace_back(slot, tr(kFixed[slot]));
    }
  }
  for (int k = 1; k <= kStatExtraCount; ++k) {
    if (ss.set.params.nk[static_cast<std::size_t>(k)] <= 0) {
      continue;
    }
    // the Black Moon and the Part of Fortune stay dashes under hrg!
    const bool dash = helio && (k == 1 || k == 4);
    rows.emplace_back(dash ? -1 : extra_slot(k), dash ? QString() : tr(kExtra[k]));
  }
  rows.emplace_back(helio ? -1 : kRowHouse, tr("   HAUS NR.     "));
  rows.emplace_back(helio ? -1 : kRowRuler, tr(" HERR v. HAUS NR."));
  if (groups) {
    rows.emplace_back(helio ? -1 : kRowLights, tr(" SO / MO / AC  "));
    rows.emplace_back(kRowAll, tr(" ALLE PLANETEN"));
  }
  const std::optional<int> pick = ask_object(this, rows, notes);
  if (!pick) {
    return false;
  }
  StatPick p;
  if (*pick == kRowHouse || *pick == kRowRuler) {
    // hf& = @numw("HAUS NR. ? ",1,12), hh& = @numw("HERR von HAUS NR.?",1,12)
    const QString prompt = *pick == kRowHouse ? tr("HAUS NR. ? ") : tr("HERR von HAUS NR.?");
    const QString line = running.trimmed().isEmpty() ? prompt : QString(running + "\n" + prompt);
    const std::optional<int> h = ask_digit(this, line, 1, 12);
    if (!h) {
      return false;
    }
    p.kind = *pick == kRowHouse ? StatPick::Kind::kHouse : StatPick::Kind::kRuler;
    p.house = *h;
    // pl$(anm&) = "H" + STR$(hf&), pl$(bnm&) = "HR.v.H" + STR$(hh&)
    p.tag = (*pick == kRowHouse ? QStringLiteral("H") : QStringLiteral("HR.v.H")) + QString::number(*h);
  } else if (*pick == kRowLights) {
    p.kind = StatPick::Kind::kLights;
    p.tag = QStringLiteral("SO/MO/AC");
  } else if (*pick == kRowAll) {
    p.kind = StatPick::Kind::kAll;
    p.tag = QStringLiteral("ALLE PL.");
  } else {
    p.slot = *pick;
    if (stat_cusp_slot(p.slot)) {
      // pl$(15) to pl$(18), H2, H3, H5 and H6
      p.tag = "H" + QString::number(kStatCuspHouse[static_cast<std::size_t>(p.slot - kStatCuspSlotFirst)]);
    } else if (helio && p.slot == body::kMoon) {
      p.tag = QString::fromUtf8(body::kEarthName.data(), static_cast<int>(body::kEarthName.size()));
    } else {
      const std::string_view n = body::kName[static_cast<std::size_t>(p.slot)];
      p.tag = QString::fromUtf8(n.data(), static_cast<int>(n.size()));
    }
  }
  out = p;
  return true;
}

// ported from obj_wahl, such_wo and stat_ausw with the run of one
// condition over the dataset
bool MainWindow::stat_condition(StatSession& ss) {
  const bool helio = ss.helio;
  const int n = ss.conditions;
  const QString file_line = tr(" DATEI : ") + stat_file_label(ss.sta) + " ";
  const QString helio_line = tr(" Die Datei ist HELIOZENTRISCH !");
  // m$ = "Wenn Sie eine ALPHABETISCHE Liste wollen geben Sie bei 'NAME' ein BLANK ein !"
  QStringList info{tr("Wenn Sie eine ALPHABETISCHE Liste wollen geben Sie bei 'NAME' ein BLANK ein !")};
  if (!ss.odu.isEmpty()) {
    info << ss.odu;
  }
  info << ss.condition_lines() << file_line;
  if (helio) {
    info << helio_line;
  }
  const QStringList rows{helio ? tr("PLANET") : tr("PLANET / HÄUSERSPITZE"),
                         helio ? QStringLiteral(" ") : tr("HERRSCHER von HAUS NR. ?"),
                         tr("HALBSUMME"),
                         tr("ASPEKT (mit EINZEL-PLANET oder HALBSUMME)"),
                         helio ? tr("SPIEGELPUNKT   PLANET") : tr("SPIEGELPUNKT   PLANET / HÄUSERSPITZE"),
                         tr("NAME    ( BUCHSTABENFOLGE )"),
                         helio ? QStringLiteral(" ") : tr("ARABISCHE TEILE ( SENSITIVE PUNKTE )"),
                         tr("ABBRUCH")};
  // ue$(0) = STR$(anzb&,2) + ". OBJEKT WÄHLEN !", helio greys 2 and 7
  const int es = ChoiceDialog::ask_with_disabled(this, QString::asprintf("%2d", n) + tr(". OBJEKT WÄHLEN !"), info,
                                                 rows, 0, helio ? std::vector<int>{1, 6} : std::vector<int>{});
  if (es < 0 || es == 7) {
    return false;
  }
  const int obj = es + 1;
  StatQuery q;
  q.classic_rulers = alt_rulers_;
  q.combine_and = ss.join == StatJoin::kAndInclusive || ss.join == StatJoin::kAndExclusive;
  std::array<StatPick, 4> m{};
  const auto factor_note = [&](int zp) {
    // fa$ + STR$(zp&,2) + " = " + pl$(m&)
    return tr("Faktor ") + QString::asprintf("%2d", zp) + " = " + m[static_cast<std::size_t>(zp - 1)].tag;
  };
  QString obj_text;
  QString p_text;
  QString aspi;
  bool aspect_mid = false;
  int spg = 3;
  bool traditional = true;
  switch (obj) {
    case kStatObjBody: {
      //RR PLANET
      // ueb1$ = "Planeten bzw. Faktor auswählen !"
      if (!stat_pick(ss, true, {tr("Planeten bzw. Faktor auswählen !")}, m[0])) {
        return false;
      }
      switch (m[0].kind) {
        case StatPick::Kind::kLights:
          q.object = StatObject::kLights;
          obj_text = ss.odu + "SO,MO,AC";
          p_text = "SO,MO,AC";
          break;
        case StatPick::Kind::kAll:
          q.object = StatObject::kAllBodies;
          obj_text = ss.odu + tr("ALLE PLAN.");
          p_text = tr("Alle Plan.");
          break;
        case StatPick::Kind::kHouse:
          q.object = StatObject::kBody;
          q.a = m[0].operand();
          obj_text = ss.odu + tr("HAUS ") + QString::number(m[0].house);
          p_text = tr("Haus ") + QString::number(m[0].house);
          break;
        case StatPick::Kind::kRuler:
          q.object = StatObject::kBody;
          q.a = m[0].operand();
          obj_text = ss.odu + tr("HR.v.HAUS ") + QString::number(m[0].house);
          p_text = tr("Hr.v.Haus ") + QString::number(m[0].house);
          break;
        default:
          q.object = StatObject::kBody;
          q.a = m[0].operand();
          obj_text = ss.odu + m[0].tag;
          p_text = m[0].tag;
          break;
      }
      break;
    }
    case kStatObjRuler: {
      //RR HERR v.HAUS NR.
      const std::optional<int> hh = ask_digit(this, tr("HERR von HAUS NR.?"), 1, 12);
      if (!hh) {
        return false;
      }
      q.object = StatObject::kHouseRuler;
      q.a.house = *hh;
      m[0].kind = StatPick::Kind::kRuler;
      m[0].house = *hh;
      m[0].tag = "HR.v.H" + QString::number(*hh);
      obj_text = ss.odu + tr("HERR v. HAUS  ") + QString::number(*hh);
      p_text = tr("Herr v. Haus ") + QString::number(*hh);
      break;
    }
    case kStatObjMidpoint: {
      //RR HALBSUM
      const QString head = tr("Halbsumme = (Faktor 1 + Faktor 2)/2");
      if (!stat_pick(ss, false, {head}, m[0]) || !stat_pick(ss, false, {head, factor_note(1)}, m[1])) {
        return false;
      }
      q.object = StatObject::kMidpoint;
      q.a = m[0].operand();
      q.b = m[1].operand();
      obj_text = ss.odu + tr("HALBS. ") + m[0].tag + "-" + m[1].tag;
      p_text = tr("HALBSUMME ") + m[0].tag + "-" + m[1].tag;
      break;
    }
    case kStatObjAspect: {
      //RR ASPEKT
      if (!stat_pick(ss, false, {tr("1. Faktor des Aspekts ?")}, m[0])) {
        return false;
      }
      const int has = ChoiceDialog::ask(this, tr("AUSWAHL"), {},
                                        {tr("ASPEKT von %1 mit EINZELNEM FAKTOR").arg(m[0].tag),
                                         tr("ASPEKT von %1 mit HALBSUMME").arg(m[0].tag)},
                                        0);
      if (has < 0) {
        return false;
      }
      aspect_mid = has == 1;
      const QString first = tr("1. Faktor des Aspekts ?") + " = " + m[0].tag;
      if (!aspect_mid) {
        // ueb3$ = "2. Faktor des Aspekts ?"
        if (!stat_pick(ss, false, {first, tr("Aspekt von %1").arg(m[0].tag), tr("2. Faktor des Aspekts ?")}, m[1])) {
          return false;
        }
      } else {
        // ueb3$ = "Mit Halbsumme = (Faktor 2 + Faktor 3)/2"
        if (!stat_pick(ss, false, {tr("Aspekt von %1").arg(m[0].tag), tr("Mit Halbsumme = (Faktor 2 + Faktor 3)/2")},
                       m[1]) ||
            !stat_pick(ss, false, {tr("Aspekt von %1").arg(m[0].tag), tr("Mit Halbsumme = %1 + Faktor 3)/2").arg(m[1].tag)},
                       m[2])) {
          return false;
        }
      }
      const int as = ChoiceDialog::ask(this, tr("AUSWAHL"), {}, {tr("EINZEL-ASPEKT ?"), tr("ALLE ASPEKTE bis TEILER...")}, 0);
      if (as < 0) {
        return false;
      }
      if (as == 0) {
        // nas& = @numw("TEILER (z.B QUADRAT=4) ?",1,12), obas = orbe(nas&) * up
        const std::optional<int> nas = ask_digit(this, tr("TEILER (z.B QUADRAT=4) ?"), 1, 12);
        if (!nas) {
          return false;
        }
        q.asp_low = *nas;
        q.asp_high = *nas;
        // his own orb box stayed commented out, the orbe row of the
        // divisor serves without his orb factor
        //RR überflüssig
        q.asp_orb = aspect_settings_.orbe[static_cast<std::size_t>(*nas)];
        // aspi$ = " " + STR$(360 / naspe&) + "(+-" + STR$(obas,4,1) + ")"
        aspi = " " + QString::number(kDegPerCircle / *nas, 'g', 10) + "(+-" +
               QString::asprintf("%4.1f", q.asp_orb * kRadToDeg) + ")";
      } else {
        // nas& = @numw("MAX. TEILER ? (NICHT ZU GROß !)",1,8), naspe& = 1
        const std::optional<int> nas = ask_digit(this, tr("MAX. TEILER ? (NICHT ZU GROß !)"), 1, 8);
        if (!nas) {
          return false;
        }
        q.asp_low = 1;
        q.asp_high = *nas;
        q.asp_orb = 0.0;
        aspi = tr(" bis TEILER ") + QString::number(*nas) + " ";
      }
      q.a = m[0].operand();
      q.b = m[1].operand();
      if (aspect_mid) {
        q.object = StatObject::kMidpointAspect;
        q.c = m[2].operand();
        // his CASE 2 wrote pl$(1), the Sun, for the first factor and built
        // the label twice
        obj_text = ss.odu + tr("ASPEKT ") + m[0].tag + tr("/HS. ") + m[1].tag + "-" + m[2].tag + aspi;
        p_text = tr("ASPEKT ") + m[0].tag + tr("/ HS. ") + m[1].tag + "-" + m[2].tag + aspi;
      } else {
        q.object = StatObject::kAspect;
        obj_text = ss.odu + tr("ASPEKT  ") + m[0].tag + "/" + m[1].tag + aspi;
        p_text = tr("ASPEKT ") + m[0].tag + "/" + m[1].tag + aspi;
      }
      break;
    }
    case kStatObjMirror: {
      //RR MIT SPIEG.
      // a$ = "SPIEGELUNG an ACHSE "
      const int s2 = ChoiceDialog::ask(
          this, tr("AUSWAHL"),
          {QString(), tr("SPIEGELUNG an ACHSE AR-LI ?"), tr("SPIEGELUNG an ACHSE CN-CP ?"), tr("BEIDE ?")},
          {tr("AR-LI"), tr("CN-CP"), tr("BEIDE")}, 0);
      if (s2 < 0) {
        return false;
      }
      spg = s2 + 1;
      // ueb1$ = "SPIEGELPUNKT : PLANETEN auswählen !"
      if (!stat_pick(ss, false, {tr("SPIEGELPUNKT : PLANETEN auswählen !")}, m[0])) {
        return false;
      }
      q.object = StatObject::kMirror;
      q.a = m[0].operand();
      q.mirror = static_cast<MirrorAxis>(spg);
      static constexpr const char* kAxis[3] = {" / AR-LI", " / CN-CP", " AR-LI/CN-CP"};
      static constexpr const char* kAxisBox[3] = {" /AR-LI", " /CN-CP", " AR-LI/CN-CP"};
      obj_text = ss.odu + tr("SPIEGELP. ") + m[0].tag + kAxis[spg - 1];
      p_text = tr("SPIEGELP.") + m[0].tag + kAxisBox[spg - 1];
      break;
    }
    case kStatObjName: {
      // strin$ = @inputbox$(180,eas$,"NAME oder BUCHSTABENFOLGE EINGEBEN !","")
      const std::optional<QString> strin = ask_text(this, tr("STRING-Eingabe !"), tr("NAME oder BUCHSTABENFOLGE EINGEBEN !"));
      if (!strin || strin->isEmpty()) {
        return false;
      }
      q.object = StatObject::kName;
      // the names stand upper case in the file, his INSTR compared the
      // typed case and a lower case entry never found anything
      q.name = strin->toUpper().toStdString();
      obj_text = ss.odu + tr("NAME  ") + *strin;
      p_text = tr("FOLGE: ") + *strin;
      break;
    }
    case kStatObjArabic: {
      //RR ARABT
      // "TRADITIONELLE FORMEL ?","","IMMER : FAKTOR 1 + FAKTOR 2 - FAKTOR 3 ?"
      const int ara = ChoiceDialog::ask(this, tr("AUSWAHL"),
                                        {QString(), tr("TRADITIONELLE FORMEL ?"), QString(),
                                         tr("IMMER : FAKTOR 1 + FAKTOR 2 - FAKTOR 3 ?")},
                                        {tr("TRADITIONELL"), tr("IMMER F1 + F2 - F3")}, 0);
      if (ara < 0) {
        return false;
      }
      traditional = ara == 0;
      QStringList notes{tr("Drei Faktoren nacheinander eingeben !")};
      if (traditional) {
        notes << tr("Taggeburt   : Faktor 1 + Faktor 3 - Faktor 2  ") << tr("Nachtgeburt : Faktor 1 + Faktor 2 - Faktor 3  ")
              << tr(" Z.B. GLÜCKSPUNKT,Taggeburt : AC + MO - SO");
      } else {
        notes << tr("Feste Vorgabe : ") << tr("Faktor 1 + Faktor 2 - Faktor 3");
      }
      for (int zp = 1; zp <= 3; ++zp) {
        QStringList with = notes;
        for (int k = 1; k < zp; ++k) {
          with << factor_note(k);
        }
        if (!stat_pick(ss, false, with, m[static_cast<std::size_t>(zp - 1)])) {
          return false;
        }
      }
      q.object = StatObject::kArabicPart;
      q.a = m[0].operand();
      q.b = m[1].operand();
      q.c = m[2].operand();
      q.arabic_day_night = traditional;
      if (traditional) {
        obj_text = ss.odu + tr("ARAB. TEILE TRAD. ") + m[0].tag + "/" + m[1].tag + "/" + m[2].tag;
        p_text = tr("ARABT. TRADIT. ") + m[0].tag + "/" + m[1].tag + "/" + m[2].tag;
      } else {
        obj_text = ss.odu + tr("ARAB. TEILE ") + m[0].tag + "+" + m[1].tag + "-" + m[2].tag;
        p_text = tr("ARABT. ") + m[0].tag + "+" + m[1].tag + "-" + m[2].tag;
      }
      break;
    }
    default:
      return false;
  }
  // obja$(anzb&) = " " + obj$(obj&) + " "
  ss.objects << " " + obj_text + " ";
  ss.windows << QString();
  // numw, the running condition " " + obja$ + suca$ over the digit box
  const auto running = [&](const QString& suca) -> QString { return " " + ss.objects.last() + suca; };
  const auto digit = [&](const QString& suca, const QString& prompt, int from, int to) {
    return ask_digit(this, running(suca) + "\n" + prompt, from, to);
  };

  // such_wo for the objects that stand somewhere
  int suc = 0;
  const bool lights = obj == kStatObjBody && m[0].kind == StatPick::Kind::kLights;
  const bool all = obj == kStatObjBody && m[0].kind == StatPick::Kind::kAll;
  if (obj == kStatObjBody || obj == kStatObjRuler || obj == kStatObjMidpoint || obj == kStatObjMirror ||
      obj == kStatObjArabic) {
    const QString l = tr("LAGE ");
    QStringList where{tr("SUCHE OBJEKT : ") + obj_text};
    where << ss.condition_lines() << tr(" Datei : ") + stat_file_label(ss.sta) + " ";
    if (helio) {
      where << helio_line;
    }
    if (all) {
      // ze$(0) = l$ + "Bei GRAD : NENNWERT"
      const int es2 = ChoiceDialog::ask(this, tr("Wo soll das OBJEKT GESUCHT werden ?"), where,
                                        {l + tr("Bei GRAD : NENNWERT"), tr("ABBRUCH")}, 0);
      if (es2 != 0) {
        return false;
      }
      suc = kStatSucDegree;
    } else if (lights) {
      const int es2 = ChoiceDialog::ask(this, tr("Wo soll das OBJEKT GESUCHT werden ?"), where,
                                        {l + tr("Bei GRAD : NENNWERT"), l + tr("IN ZEICHEN"), tr("ABBRUCH")}, 0);
      if (es2 < 0 || es2 == 2) {
        return false;
      }
      suc = es2 + 1;
    } else {
      std::vector<int> grey;
      // the angles, the stored cusps and HAUS NR. stand in no house, his
      // hrg! asked again after In HAUS and greys it here
      const bool cusp = obj == kStatObjBody && ((m[0].slot >= body::kAscendant && m[0].slot <= kStatCuspSlotLast) ||
                                                m[0].kind == StatPick::Kind::kHouse);
      if (cusp || helio) {
        grey.push_back(2);
      }
      // OHNE EINSCHRÄNKUNG only for the first condition and one mirror
      // axis. His ELSE IF chain stopped at the cusp test, a cusp as a
      // later condition kept it and ran the whole file past UND
      if ((obj == kStatObjMirror && spg == 3) || !ss.odu.isEmpty()) {
        grey.push_back(4);
      }
      const int es2 = ChoiceDialog::ask_with_disabled(
          this, tr("Wo soll das OBJEKT GESUCHT werden ?"), where,
          {l + tr("Bei GRAD..ORBIS.."), l + tr("In ZEICHEN"), helio ? QStringLiteral("  ") : l + tr("In HAUS"),
           helio ? l + tr("Bei PLANET") : l + tr("Bei PLANET/HÄUSERSPITZE"), tr("OHNE EINSCHRÄNKUNG ( 0-360° )"),
           tr("ABBRUCH")},
          1, grey);
      if (es2 < 0 || es2 == 5) {
        return false;
      }
      suc = es2 + 1;
    }
  }

  // the inputs of stat_ausw
  QString window_text;
  QString c_text;
  int framed = 0;
  switch (suc) {
    case kStatSucDegree: {
      //RR LAGE BEI GRAD
      // pgd$ = @inputbox$(160,eaz$,obj$(obj&) + ": LAGE in " + d$ + " ! (z.B. 125.5)",fixpunkt$)
      const std::optional<double> p =
          ask_number(this, tr("ZAHLEN-Eingabe !"), obj_text + tr(": LAGE in DEZIMAL-GRAD ! (z.B. 125.5)"), 0.0,
                     kDegPerCircle, fixpunkt_ >= 0.0 ? fixpunkt_ * kRadToDeg : std::numeric_limits<double>::quiet_NaN(), 3);
      if (!p) {
        return false;
      }
      // suc$(1) + STR$(p * up,5,1)
      window_text = tr("bei GRAD ") + QString::asprintf("%5.1f", *p);
      // nw& = @numw("ORBIS in GRAD ?",1,10), ALLE PLANETEN to three
      const std::optional<int> orb = digit(window_text, tr("ORBIS in GRAD ?"), 1, all ? 3 : 10);
      if (!orb) {
        return false;
      }
      q.window = StatWindow::kAtDegree;
      q.degree = *p * kDegToRad;
      q.orb = *orb * kDegToRad;
      // suc$(1) + " +- " + STR$(up * obg,4,1)
      window_text += " +- " + QString::asprintf("%4.1f", static_cast<double>(*orb));
      c_text = tr("Bei ") + QString::number(*p) + "° +- " + QString::asprintf("%4.1f", static_cast<double>(*orb));
      break;
    }
    case kStatSucSign: {
      //RR LAGE IN ZEICHEN
      // zei& = @ze_pl_wa(1,12,0,-1), the list of zod_zeich_alph
      const std::optional<int> sign = pick_sign(this);
      if (!sign) {
        return false;
      }
      const int zei = *sign + 1;
      // suc$(2) + zei$(zei&)
      window_text = tr("in ") + kSignTag[*sign];
      const std::optional<int> orb = digit(window_text, tr("ORBIS in GRAD ?"), 0, 10);
      if (!orb) {
        return false;
      }
      q.window = StatWindow::kInSign;
      q.sign = zei;
      q.orb = *orb;
      c_text = tr("in ") + kSignTag[*sign];
      framed = zei;
      break;
    }
    case kStatSucHouse: {
      //RR LAGE IN HAUS
      // ha& = @numw("LAGE in HAUS NR. ? ",1,12), obp = @numw("ORBIS in % der Länge des HAUSES ?",0,10)
      const std::optional<int> ha = digit(tr("in HAUS "), tr("LAGE in HAUS NR. ? "), 1, 12);
      if (!ha) {
        return false;
      }
      window_text = tr("in HAUS ") + QString::number(*ha);
      const std::optional<int> obp = digit(window_text, tr("ORBIS in % der Länge des HAUSES ?"), 0, 10);
      if (!obp) {
        return false;
      }
      q.window = StatWindow::kInHouse;
      q.house = *ha;
      q.house_orb_pct = *obp;
      c_text = tr("in Haus ") + QString::number(*ha);
      framed = *ha;
      break;
    }
    case kStatSucBody: {
      //RR BEI PL..
      if (!stat_pick(ss, false, {tr("SUCHE OBJEKT : ") + obj_text}, m[3], running(tr("bei ")))) {
        return false;
      }
      window_text = tr("bei ") + m[3].tag;
      const std::optional<int> orb = digit(window_text, tr("ORBIS in GRAD ?"), 1, 10);
      if (!orb) {
        return false;
      }
      q.window = StatWindow::kNearBody;
      q.near_body = m[3].operand();
      q.orb = *orb * kDegToRad;
      c_text = tr("Bei ") + m[3].tag + " +- " + QString::asprintf("%4.1f", static_cast<double>(*orb)) + "°";
      break;
    }
    case kStatSucAll:
      //RR 0...360
      q.window = StatWindow::kAnywhere;
      window_text = tr(" 0-360 ° ");
      c_text = tr("Im Bereich 0..360°");
      break;
    default:
      q.window = StatWindow::kAnywhere;
      break;
  }
  ss.windows.last() = window_text;

  // the run of stat_ausw_1 over every record
  const StatEvalResult res = evaluate_statistics(ss.set, q, aspect_settings_, ss.mask);
  ss.list->add(res, n, ss.join);
  ss.obj = obj;
  ss.suc = suc;
  ss.groups = lights || all;
  ss.angle_mc = obj == kStatObjBody && m[0].slot == body::kAscendant;
  // inf_box1, b$ = "Länge " + p$ for a planet, p$ + " " else
  ss.object_line = obj == kStatObjBody ? tr("Länge ") + p_text : p_text + " ";
  ss.window_line = c_text;
  // inf_box20, the tag over the bars
  switch (obj) {
    case kStatObjBody:
      if (m[0].kind == StatPick::Kind::kHouse) {
        ss.object_tag = "H" + QString::number(m[0].house) + " ";
      } else if (m[0].kind == StatPick::Kind::kRuler) {
        ss.object_tag = "HR.v.H" + QString::number(m[0].house) + " ";
      } else if (lights) {
        ss.object_tag = tr("SO..AC ");
      } else if (all) {
        ss.object_tag = tr("Alle PL ");
      } else {
        ss.object_tag = " " + m[0].tag + " ";
      }
      break;
    case kStatObjRuler:
      ss.object_tag = m[0].tag + " ";
      break;
    case kStatObjMidpoint:
      ss.object_tag = m[0].tag + "-" + m[1].tag + " ";
      break;
    case kStatObjMirror:
      ss.object_tag = tr("Spiegp.");
      break;
    case kStatObjArabic:
      ss.object_tag = tr("Arabt.");
      break;
    default:
      ss.object_tag = "  SO ";
      break;
  }
  ss.sums = res.distribution;
  ss.framed = framed;
  return true;
}

// ported from od_un
int MainWindow::stat_join(StatSession& ss) {
  QStringList info;
  if (ss.conditions > 1) {
    // e$ = " " + STR$(anzb&,2) + " OBJEKTE SCHON GEWÄHLT ! "
    info << " " + QString::asprintf("%2d", ss.conditions) + tr(" OBJEKTE SCHON GEWÄHLT ! ");
  }
  info << ss.condition_lines() << tr(" DATEI : ") + stat_file_label(ss.sta) + " ";
  if (ss.helio) {
    info << tr(" Die Datei ist HELIOZENTRISCH !");
  }
  std::vector<int> grey;
  int preset = 3;
  const bool chained = ss.odu == QLatin1String("UND  ");
  if (ss.odu.isEmpty()) {
    preset = ss.weit ? 0 : 3;
  } else if (ss.odu == QLatin1String("ODER ")) {
    preset = 0;
  } else if (chained && ss.join != StatJoin::kAndExclusive) {
    grey = {0};
    preset = 1;
  } else if (chained) {
    grey = {0, 1};
    preset = 2;
  }
  // IF weit!, a3& = 4, the list stands already
  if (ss.weit) {
    grey.push_back(3);
  }
  // his arrays end at the twelfth condition, only the list remains
  if (ss.conditions >= kStatMostConditions) {
    grey = {0, 1, 2};
    preset = 3;
  }
  // ue$(0) = "* WEITERE BEDINGUNG ? *"
  const int es = ChoiceDialog::ask_with_disabled(this, tr("* WEITERE BEDINGUNG ? *"), info,
                                                 {tr("ODER"), tr("UND  INKLUSIV"), tr("UND  EXKLUSIV"),
                                                  tr("AUSGABE-LISTE"), tr("ABBRUCH")},
                                                 preset, grey);
  ss.weit = false;
  switch (es) {
    case 0:
      //RR ODER
      ss.odu = "ODER ";
      ss.join = StatJoin::kOr;
      return 1;
    case 1:
      //RR UND INKL
      ss.odu = "UND  ";
      ss.join = StatJoin::kAndInclusive;
      return 2;
    case 2:
      //RR UND EXKL
      ss.odu = "UND  ";
      ss.join = StatJoin::kAndExclusive;
      return 3;
    case 3:
      //RR !AUSG
      return 4;
    default:
      return -1;
  }
}

}  // namespace horcom
