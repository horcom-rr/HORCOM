// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

// The DYNAMOGRAMM screens, his huber with hubausg and einzel_bogen_anz.

#include <QApplication>
#include <QMessageBox>
#include <QTimer>

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

#include "choice_dialog.hpp"
#include "horcom/chart/dynamogram.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/render/items.hpp"
#include "main_window.hpp"
#include "robert_input.hpp"
#include "robert_text.hpp"
#include "sheet_view.hpp"

namespace horcom {

namespace {

// f = 0.3, his amplitude factor with norm! = 0
constexpr double kAmplitude = 0.3;
// the zero line and the frame of hubausg
constexpr double kZeroY = 220.0;
constexpr double kLeftX = 40.0;
constexpr double kClipRight = 620.0;
constexpr double kClipBottom = 459.0;
// his RGB(0,156,0) green pen of the mood curve and the benefic arcs, a
// shade darker than the RGB(0,160,0) of the linear graph
constexpr Rgb kGreen = 0x009C00;
// his RGB(0,0,255) pen of the resultant and the arcs of DR, AC and MC
constexpr Rgb kBlue = 0x0000FF;
// the arc screen draws two samples per pixel from x 140
constexpr double kArcLeft = 140.0;
// the step of the arcs drawn one by one like his loop found them
constexpr int kArcStepMs = 150;
// BEEP, DELAY 0.2, BEEP at the end of a pass
constexpr int kBeepGapMs = 200;

// his text and textc, x the left edge and y the bottom line
Primitive text(double x, double bottom, double size, const QString& t, Rgb c = kInkColor) {
  return screen_text(x, bottom, size, t.toStdString(), c);
}

// his textzent and textzentc
Primitive centred(double bottom, double size, const QString& t, Rgb c = kInkColor) {
  return screen_text_centred(bottom, size, t.toStdString(), c);
}

// his texts, reading upward from x, y
Primitive upward(double x, double y, double size, const QString& t, Rgb c = kInkColor) {
  return screen_text_vertical(x, y, size, t.toStdString(), c);
}

// CLIP 0,0,@xk(620),@yk(459), the part of a curve segment inside the
// clip box of hubausg, false when none of it is
bool clip_segment(double& x1, double& y1, double& x2, double& y2) {
  double t0 = 0.0;
  double t1 = 1.0;
  const double dx = x2 - x1;
  const double dy = y2 - y1;
  // the four edges as p * t <= q, left, right, top, bottom
  const double p[4] = {-dx, dx, -dy, dy};
  const double q[4] = {x1, kClipRight - x1, y1, kClipBottom - y1};
  for (int k = 0; k < 4; ++k) {
    if (p[k] == 0.0) {
      if (q[k] < 0.0) {
        return false;
      }
      continue;
    }
    const double t = q[k] / p[k];
    if (p[k] < 0.0) {
      t0 = std::max(t0, t);
    } else {
      t1 = std::min(t1, t);
    }
  }
  if (t0 > t1) {
    return false;
  }
  const double ax = x1 + t0 * dx;
  const double ay = y1 + t0 * dy;
  x2 = x1 + t1 * dx;
  y2 = y1 + t1 * dy;
  x1 = ax;
  y1 = ay;
  return true;
}

// one curve with his pen pattern, the dashes emulated sample by sample
// because every segment is one pixel long
void curve(DisplayList& dl, const std::vector<double>& y, Rgb c, double w, Primitive::Style pen) {
  // on and off lengths of the patterns in pixels
  static constexpr int kDash[2] = {6, 4};
  static constexpr int kDashDot[4] = {6, 3, 1, 3};
  const int* pattern = pen == Primitive::Style::kDashed ? kDash : (pen == Primitive::Style::kDashDot ? kDashDot : nullptr);
  const int parts = pen == Primitive::Style::kDashed ? 2 : 4;
  int period = 0;
  if (pattern != nullptr) {
    for (int k = 0; k < parts; ++k) {
      period += pattern[k];
    }
  }
  for (std::size_t i = 0; i + 1 < y.size(); ++i) {
    if (pattern != nullptr) {
      int at = static_cast<int>(i) % period;
      int k = 0;
      while (at >= pattern[k]) {
        at -= pattern[k];
        ++k;
      }
      if (k % 2 == 1) {
        continue;
      }
    }
    double x1 = kLeftX + static_cast<double>(i);
    double x2 = x1 + 1.0;
    double y1 = y[i];
    double y2 = y[i + 1];
    if (clip_segment(x1, y1, x2, y2)) {
      dl.items.push_back(line_item(x1, y1, x2, y2, c, w));
    }
  }
}

// his day_y, the day of the year
int day_of_year(const CalendarDate& d, Calendar cal) {
  return static_cast<int>(std::floor(julian_day({d.day, d.month, d.year, 0.0, 0.0}, cal) -
                                     julian_day({1, 1, d.year, 0.0, 0.0}, cal) + 0.5)) +
         1;
}

// the arc colours of einzel_bogen_anz
Rgb arc_colour(int pl) {
  switch (pl) {
    case 1:
    case 2:
    case 3:
    case 4:
    case 6:
      return kGreen;
    case 5:
    case 7:
    case 8:
    case 9:
    case 10:
      return kMarkRed;
    default:
      return kBlue;
  }
}

QString tag(int slot) {
  const std::string_view n = body::kName[static_cast<std::size_t>(slot)];
  return QString::fromUtf8(n.data(), static_cast<qsizetype>(n.size()));
}

// the arcs of one pass of his analysis, asp_analy_rad or asp_analy_mund
// in the progressive or the regressive run
struct ArcPass {
  std::size_t first = 0;
  std::size_t end = 0;
};

// the passes in the order huber ran them, the radix pass before the
// mutual one, the regressive pair after the progressive pair
std::vector<ArcPass> arc_passes(const Dynamogram& d, bool regressive) {
  std::vector<std::pair<bool, bool>> order{{true, false}, {false, false}};
  if (regressive) {
    order.emplace_back(true, true);
    order.emplace_back(false, true);
  }
  std::vector<ArcPass> out;
  std::size_t next = 0;
  for (const auto& [radix, back] : order) {
    ArcPass p;
    p.first = next;
    while (next < d.arcs.size() && d.arcs[next].radix == radix && d.arcs[next].regressive == back) {
      ++next;
    }
    p.end = next;
    out.push_back(p);
  }
  return out;
}

}  // namespace

// ported from einzel_bogen_anz, the arcs of a pass from first to k with
// the captions of k
DisplayList MainWindow::dynamo_arc_screen(const Dynamogram& d, std::size_t first, std::size_t k, int lja) const {
  DisplayList dl;
  if (k >= d.arcs.size() || first > k) {
    return dl;
  }
  const DynamogramArc& a = d.arcs[k];
  // "Aspekt " + pl$(pl&) + " - " + pl$(rd&)
  dl.items.push_back(centred(20, 16, tr("Aspekt %1 - %2").arg(tag(a.pl), tag(a.rd))));
  dl.items.push_back(centred(36, 16, tr("Teiler = %1 | Vielfaches = %2").arg(a.na).arg(a.ma)));
  // "Bogenmitte =" + STR$(tb - 25,6,2) + " Beginn - LEBENS-Jahr = " + STR$(lja)
  dl.items.push_back(centred(52, 16, tr("Bogenmitte =%1 Beginn - LEBENS-Jahr = %2").arg(a.tb - 25.0, 6, 'f', 2).arg(lja)));
  const QString run = a.regressive ? tr(" | Regressiv !") : tr(" | Progressiv !");
  dl.items.push_back(centred(
      68, 16, (a.radix ? tr("Laufende über RADIX-Faktoren") : tr("Laufende Faktoren untereinander = MUNDAN")) + run,
      kMarkRed));
  // the axis, thin from 20 to 620 and thick over the window
  dl.items.push_back(line_item(20, kZeroY, 620, kZeroY));
  dl.items.push_back(line_item(kArcLeft, kZeroY, 440, kZeroY, kInkColor, 3.0));
  for (int x = 20; x <= 620; x += 5) {
    const int j = x - 20;
    if (j % 60 == 0) {
      dl.items.push_back(line_item(x, kZeroY - 10, x, kZeroY + 10, kInkColor, 2.0));
      dl.items.push_back(text(x - 8, kZeroY - 10, 13, QString::number(lja + j / 60 - 2).rightJustified(2)));
    } else {
      dl.items.push_back(line_item(x, kZeroY - 5, x, kZeroY + 5));
    }
  }
  dl.items.push_back(text(516, kZeroY - 22, 13, tr("LEBENS-Jahre")));
  for (std::size_t n = first; n <= k; ++n) {
    const DynamogramArc& b = d.arcs[n];
    const Rgb c = arc_colour(b.pl);
    // pl$(pl&) + "/" + pl$(rd&) + " " + STR$(na&) + "/" + STR$(ma&) at the
    // arc centre before the arc, his texts font reading upward. His texts
    // wrote OPAQUE over nine blanks first, the label stands on its own
    // white ground and a later one covers it. A label off both edges of
    // his screen never showed
    const int im = (b.i1 + b.i2) / 2;
    const double peak = b.bog[static_cast<std::size_t>(im - b.i1)];
    const double lx = 144 + (im - kDynamogramWindowStart) / 2.0;
    const double ly = kZeroY - peak + 48;
    const Primitive label = upward(lx, ly, 10, QString("%1/%2 %3/%4").arg(tag(b.pl), tag(b.rd)).arg(b.na).arg(b.ma));
    if (lx > 0.0 && lx - label.size < kCanvasWidth) {
      constexpr double kBlanks = 9.0;
      const double span = std::max(kBlanks, static_cast<double>(QString::fromStdString(label.text).size())) * text_advance(label);
      Primitive ground;
      ground.kind = Primitive::Kind::kRect;
      ground.x1 = label.x1;
      ground.y1 = ly - 0.5 * span;
      ground.r1 = 0.5 * label.size;
      ground.r2 = 0.5 * span;
      ground.fill = kPaperColor;
      dl.items.push_back(ground);
      dl.items.push_back(label);
    }
    // FOR i& = i1& TO i2&, the last segment falls back to the empty
    // bog(i2& + 1) on the axis
    for (int i = b.i1; i <= b.i2; ++i) {
      const double v1 = b.bog[static_cast<std::size_t>(i - b.i1)];
      const double v2 = i < b.i2 ? b.bog[static_cast<std::size_t>(i + 1 - b.i1)] : 0.0;
      const double x1 = kArcLeft + (i - kDynamogramWindowStart) / 2.0;
      if (x1 < 0.0 || x1 > kCanvasWidth) {
        continue;
      }
      dl.items.push_back(line_item(x1, kZeroY - v1, x1 + 0.5, kZeroY - v2, c));
    }
  }
  dl.items.push_back(line_item(40, 414, 100, 414, kGreen));
  dl.items.push_back(text(104, 418, 16, tr(" Planeten SO,MO,ME,VE,JU")));
  dl.items.push_back(line_item(40, 430, 100, 430, kMarkRed));
  dl.items.push_back(text(104, 434, 16, tr(" Planeten MA,SA,UR,PL")));
  dl.items.push_back(line_item(40, 446, 100, 446, kBlue));
  dl.items.push_back(text(104, 450, 16, tr(" DR,AC,MC")));
  return dl;
}

// ported from hubausg
DisplayList MainWindow::dynamo_graph(const Dynamogram& d, int lja, const Chart& radix) const {
  DisplayList dl;
  std::vector<double> exist;
  std::vector<double> mood;
  std::vector<double> sum;
  for (int i = kDynamogramWindowStart; i <= kDynamogramWindowEnd + 1; ++i) {
    const double e = kAmplitude * d.existential[static_cast<std::size_t>(i)];
    const double m = kAmplitude * d.mood[static_cast<std::size_t>(i)];
    exist.push_back(kZeroY - e);
    mood.push_back(kZeroY - m);
    sum.push_back(kZeroY - (e + m));
  }
  // DEFLINE 3,1 red, DEFLINE 1,1 green, DEFLINE 0,2 blue
  curve(dl, exist, kMarkRed, 1.0, Primitive::Style::kDashDot);
  curve(dl, mood, kGreen, 1.0, Primitive::Style::kDashed);
  curve(dl, sum, kBlue, 2.0, Primitive::Style::kSolid);
  // the date axis, year lines with the year and month ticks with letters
  const Calendar cal = current_settings().calendar;
  const CalendarDate birth = calendar_date(radix.jd_ut, cal);
  const int j = day_of_year(birth, cal);
  const double tja = radix.ta.tropical_year_days;
  int k = birth.month;
  // jd = jd(1,ze) + CINT(tja * lja)
  int jj = calendar_date(radix.jd_ut + std::lround(tja * lja), cal).year;
  if (j < 240) {
    //RR genug Platz ?
    dl.items.push_back(text(44 + (-32 + 120 * (tja - j) / tja) / 2, 58, 16, QString::number(jj)));
  }
  static constexpr const char* kMonth[12] = {"J", "F", "M", "A", "M", "J", "J", "A", "S", "O", "N", "D"};
  const int shift = static_cast<int>(std::lround(j * kDynamogramPerYear / tja));
  // the date axis still stands under his CLIP, its lines and letters end
  // at 620 like the curves
  const double letter = text_advance(text(0, 0, 9, kMonth[0]));
  for (int i = 0; i <= kDynamogramWindowEnd - kDynamogramWindowStart; ++i) {
    if (kLeftX + i > kClipRight) {
      break;
    }
    if ((i + shift) % kDynamogramPerYear == 0) {
      ++jj;
      dl.items.push_back(line_item(40 + i, 60, 40 + i, 360));
      if (100 + i - 16 < 596) {
        dl.items.push_back(text(100 + i - 16, 58, 16, QString::number(jj)));
      } else if (i + 42 < 596) {
        dl.items.push_back(text(i + 42, 58, 16, QString::number(jj)));
      }
      k = 0;
    }
    if ((i + shift) % 10 == 0) {
      dl.items.push_back(line_item(40 + i, 215, 40 + i, 225));
      ++k;
      if (k >= 1 && k <= 12 && 42 + i + letter <= kClipRight) {
        dl.items.push_back(text(42 + i, 218, 9, kMonth[k - 1]));
      }
    }
  }
  dl.items.push_back(text(4, 60, 13, QString("%1").arg(150, 4)));
  dl.items.push_back(text(4, 370, 13, QString("%1").arg(-150, 4)));
  // DYNAMOGRAMM nach K.E.KRAFFT / F.G.GOERNER
  dl.items.push_back(centred(20, 16, tr("DYNAMOGRAMM nach K.E.KRAFFT / F.G.GOERNER")));
  const QString name = QString("%1 %2").arg(QString::fromStdString(record_.surname).trimmed(),
                                          QString::fromStdString(record_.given).trimmed()).trimmed();
  dl.items.push_back(centred(36, 16,
                             tr("%1 | %2 |Geb.- Datum: %3").arg(name, QString::fromStdString(record_.place).trimmed(),
                                                                datum3_text(calendar_date(radix.jd_ut, cal)))));
  dl.items.push_back(line_item(2, 38, 628, 38));
  dl.items.push_back(line_item(628, 2, 628, 478));
  dl.items.push_back(line_item(40, 380, 100, 380, kMarkRed));
  dl.items.push_back(text(104, 386, 13, tr(" 1 Existentielle Situation ")));
  dl.items.push_back(line_item(40, 396, 100, 396, kGreen));
  dl.items.push_back(text(104, 402, 13, tr(" 2 Grundstimmung ")));
  dl.items.push_back(line_item(40, 412, 100, 412, kBlue, 2.0));
  dl.items.push_back(text(104, 418, 13, tr(" Resultierende Energie ")));
  dl.items.push_back(line_item(40, kZeroY, 620, kZeroY));
  dl.items.push_back(line_item(40, 60, 40, 360));
  //RR @texts(36,190,16,"Positiv"), @texts(36,316,16,"Negativ")
  dl.items.push_back(upward(36, 190, 16, tr("Positiv")));
  dl.items.push_back(upward(36, 316, 16, tr("Negativ")));
  // @textzentrl(438,16,1," MITTEL über 50 Tage ( = Jahre ) = " + STR$(mittel% / 6000,7,0))
  add_screen_text_boxed(
      dl, 438, 16, tr(" MITTEL über 50 Tage ( = Jahre ) = %1").arg(std::lround(dynamogram_mean(d, kAmplitude)), 7).toStdString());
  return dl;
}

// ported from huber, his question chain, the two screens and the loop
// over the intervals of four years
void MainWindow::dynamogram_view() {
  if (!last_chart_) {
    return;
  }
  //RR IF od <> 1, KEIN RADIX-DATENSATZ !
  if (active_is_solar_) {
    QMessageBox::information(this, tr(" Information "), tr("KEIN RADIX-DATENSATZ !"));
    return;
  }
  // inputbox$(160," LEBENSJAHR für BEGINN eingeben !"," Ca. 4 JAHRE DANACH werden dargestellt","")
  const std::optional<double> age =
      ask_number(this, tr(" LEBENSJAHR für BEGINN eingeben !"), tr(" Ca. 4 JAHRE DANACH werden dargestellt"), -200.0, 200.0,
                 std::numeric_limits<double>::quiet_NaN(), 0);
  if (!age) {
    return;
  }
  int lja = static_cast<int>(std::floor(*age));
  DynamogramOptions opt;
  //RR NEGATIVE ( REGRESSIVE ) ZEIT-RICHTUNG miteinbeziehen ?
  const int neg = ChoiceDialog::ask(this, tr("ENTSCHEIDUNG !"), {tr("NEGATIVE ( REGRESSIVE ) ZEIT-RICHTUNG miteinbeziehen ?")},
                                    {tr(" JA "), tr(" NEIN "), tr("ABBRUCH")}, 0);
  if (neg < 0 || neg == 2) {
    return;
  }
  opt.regressive = neg == 0;
  //RR MOND LAUFEND HINZUNEHMEN ?
  const int moon = ChoiceDialog::ask(this, tr("ENTSCHEIDUNG !"), {tr("MOND LAUFEND HINZUNEHMEN ?")},
                                     {tr(" NEIN "), tr("JA"), tr("ABBRUCH")}, 0);
  if (moon < 0 || moon == 2) {
    return;
  }
  opt.with_moon = moon == 1;
  //RR EINZEL - BÖGEN BEOBACHTEN ?
  const int single = ChoiceDialog::ask(this, tr("ENTSCHEIDUNG !"), {tr("EINZEL - BÖGEN BEOBACHTEN ?")},
                                       {tr("JA"), tr(" NEIN "), tr("ABBRUCH")}, 1);
  if (single < 0 || single == 2) {
    return;
  }
  //RR AUSWERTUNG : Mit COSINUS - BÖGEN oder Mittels GAUß - KURVEN ( GLOCKEN-KURVEN )
  const int shape = ChoiceDialog::ask(
      this, tr("ENTSCHEIDUNG !"),
      {tr("AUSWERTUNG : "), tr("Mit COSINUS - BÖGEN"), tr("oder"), tr("Mittels GAUß - KURVEN ( GLOCKEN-KURVEN )")},
      {tr("COSINUS-BÖGEN"), tr("GAUß-KURVEN"), tr("ABBRUCH")}, 1);
  if (shape < 0 || shape == 2) {
    return;
  }
  opt.gauss = shape == 1;
  // par = 2, appa& = 1, hrg! = 0, the run forces its own frame
  SearchContext ctx = make_context();
  ctx.settings.topocentric_parallax = false;
  ctx.settings.heliocentric = false;
  ctx.settings.apparent = ApparentMode::kLightTime;
  const Chart radix = compute_chart(radix_input(), ctx.settings, vsop_, eph_);
  if (!radix.ok) {
    return;
  }
  for (;;) {
    opt.from_age = lja;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    const Dynamogram d = dynamogram(radix, opt, ctx);
    QApplication::restoreOverrideCursor();
    if (single == 0) {
      // asp_analy_rad and asp_analy_mund each cleared the screen, drew
      // their arcs as they were found and closed on two beeps and a key
      for (const ArcPass& pass : arc_passes(d, opt.regressive)) {
        dynamo_arc_pass(d, pass.first, pass.end, lja);
      }
    }
    SheetView graph(this);
    mark_output(&graph, menu_item::kSecondary);
    graph.setWindowTitle(tr(" DYNAMOGRAMM nach KRAFFT-GOERNER "));
    graph.canvas()->set_plain_list(dynamo_graph(d, lja, radix));
    graph.resize(size());
    if (graph.exec() != QDialog::Accepted) {
      return;
    }
    // ds$ = na$ + " " + ta + "." + mo + "." + ja + " | " + go$
    const QString ds = QString("%1 %2.%3.%4 | %5")
                           .arg(QString::fromStdString(record_.surname).trimmed())
                           .arg(record_.day, 2)
                           .arg(record_.month, 2)
                           .arg(record_.year, 5)
                           .arg(QString::fromStdString(record_.place).trimmed());
    const int re = ChoiceDialog::ask(this, tr("ENTSCHEIDUNG !"),
                                     {tr("NÄCHSTES INTERVALL ?"), tr("VORHERGEHENDES INTERVALL ?"), tr("Mit dem DATENSATZ :"), ds},
                                     {tr("NÄCHSTES"), tr("VORHERGEHENDES"), tr("Programm BEENDEN")}, 0);
    if (re == 0) {
      // ADD lja,4
      lja += 4;
    } else if (re == 1) {
      lja -= 4;
    } else {
      return;
    }
  }
}

// one pass of EINZEL - BÖGEN, his asp_analy_rad or asp_analy_mund with
// the arcs of einzel_bogen_anz, the beeps and the wart at its end
void MainWindow::dynamo_arc_pass(const Dynamogram& d, std::size_t first, std::size_t end, int lja) {
  SheetView arcs(this);
  mark_output(&arcs, menu_item::kSecondary);
  // hub_tit
  arcs.setWindowTitle(tr(" DYNAMOGRAMM nach KRAFFT-GOERNER | WEITER mit LEERTASTE"));
  std::size_t k = first;
  QTimer step;
  QTimer second_beep;
  second_beep.setSingleShot(true);
  connect(&second_beep, &QTimer::timeout, &arcs, []() { QApplication::beep(); });
  // BEEP, DELAY 0.2, BEEP once every arc of the pass stands
  const auto done = [&second_beep]() {
    QApplication::beep();
    second_beep.start(kBeepGapMs);
  };
  const auto show = [&]() {
    arcs.canvas()->set_plain_list(dynamo_arc_screen(d, first, k, lja));
    if (k + 1 < end) {
      ++k;
    } else {
      step.stop();
      done();
    }
  };
  connect(&step, &QTimer::timeout, &arcs, show);
  if (first < end) {
    show();
    if (first + 1 < end) {
      step.start(kArcStepMs);
    }
  } else {
    // a pass without an arc in the window left his screen empty
    done();
  }
  // Space shows every arc of the pass at once, then goes on
  arcs.set_step([&]() {
    if (step.isActive()) {
      step.stop();
      k = end - 1;
      arcs.canvas()->set_plain_list(dynamo_arc_screen(d, first, k, lja));
      done();
      return true;
    }
    return false;
  });
  arcs.resize(size());
  arcs.exec();
}

}  // namespace horcom
