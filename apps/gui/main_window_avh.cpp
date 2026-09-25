// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

// VORGABEN HOROSKOP ÄNDERN, the avh wizard with orbis_asp, orbis_pla,
// punkte_pla, hor_farb and the line style screen of asp_li.

#include <QCheckBox>
#include <QColorDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QRegularExpressionValidator>
#include <QVBoxLayout>
#include <cmath>
#include <functional>
#include <limits>

#include "choice_dialog.hpp"
#include "horcom/chart/bodies.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/render/wheel.hpp"
#include "main_window.hpp"
#include "robert_input.hpp"
#include "theme.hpp"

namespace horcom {

namespace {

// the topics of avh in his order, the line style screen avh13 is a page
// of its own since R there steps back to the histograms
enum Topic {
  kFrame = 0,
  kScreens,
  kSymbols,
  kOrbs,
  kDegrees,
  kBegin,
  kRulers,
  kColours,
  kHistograms,
  kAspects,
  kLines,
  kOuter,
  kTopicEnd
};

// orbe$(1..14), the rows of orbis_asp
constexpr const char* kOrbRows[14] = {
    QT_TRANSLATE_NOOP("horcom::MainWindow", " (1)  KONJUNKTION =>    360  "), QT_TRANSLATE_NOOP("horcom::MainWindow", " (2)  OPPOSITION  =>    180  "), QT_TRANSLATE_NOOP("horcom::MainWindow", " (3)  TRIGON      =>    120  "),
    QT_TRANSLATE_NOOP("horcom::MainWindow", " (4)  QUADRAT     =>     90  "), QT_TRANSLATE_NOOP("horcom::MainWindow", " (5)  QUINTIL     =>     72  "), QT_TRANSLATE_NOOP("horcom::MainWindow", " (6)  SEXTIL      =>     60  "),
    QT_TRANSLATE_NOOP("horcom::MainWindow", " (7)  SEPTIL      =>  51.43  "), QT_TRANSLATE_NOOP("horcom::MainWindow", " (8)  OKTIL       =>     45  "), QT_TRANSLATE_NOOP("horcom::MainWindow", " (9)  NONIL       =>     40  "),
    QT_TRANSLATE_NOOP("horcom::MainWindow", "(10)  DEZIL       =>     36  "), QT_TRANSLATE_NOOP("horcom::MainWindow", "(11)  UNDEZIL     =>  32.73  "), QT_TRANSLATE_NOOP("horcom::MainWindow", "(12)  DODEZIL     =>     30  "),
    QT_TRANSLATE_NOOP("horcom::MainWindow", "SPIEGELUNG an KARDINAL-ACHSEN "), QT_TRANSLATE_NOOP("horcom::MainWindow", "                   HALBSUMMEN ")};

// the ORBES für GLEICH WAHRSCHEINLICHE ASPEKTE, geozentrisch and
// heliozentrisch, index by divisor
constexpr double kEqualGeo[13] = {0.0, 5.4, 6.8, 3.1, 3.0, 1.6, 2.6, 1.0, 1.5, 1.0, 1.5, 0.6, 1.5};
constexpr double kEqualHelio[13] = {0.0, 6.51, 5.36, 3.18, 3.00, 1.58, 2.01, 1.02, 1.51, 1.00, 1.53, 0.58, 1.50};
// the HORCOM count, twelve degrees over the divisor, or$(1..12)
constexpr double kOrbTwelve = 12.0;
// or$(13) = STR$(2,5,2), or$(14) = STR$(1,5,2)
constexpr double kMirrorOrb = 2.0;
constexpr double kMidpointOrb = 1.0;
// orbis_asp refuses an orb over one and a half the gap to the next
// divisor and over three times the HORCOM count
constexpr double kOverlapShare = 1.5;
constexpr double kTooLargeShare = 3.0;
// IF VAL(et$(i&)) > 500, ZU GROßER ORBIS
constexpr int kMaxWeight = 500;
// orb < 0.1 OR orb > 3 asks the SONSTIGE percentage again
constexpr double kMinFactor = 0.1;
constexpr double kMaxFactor = 3.0;
// his inputbox of SONSTIGE took any number, the loop behind it refuses
// what lies outside kMinFactor and kMaxFactor
constexpr double kFreeEntry = 1.0e6;
// the preset factors 50, 80, 100, 150 and 200 percent
constexpr double kFactors[5] = {0.5, 0.8, 1.0, 1.5, 2.0};
// ZIFFER von 0 bis 9
constexpr int kMaxPoints = 9;
// the colour sample of a hor_farb row, his pboxn from x - 40 to x + 200
constexpr int kSwatchWidth = 60;
constexpr int kSwatchHeight = 16;

double field(const QLineEdit* e) {
  QString t = e->text().trimmed();
  // @komma_pkt$
  t.replace(',', '.');
  return QLocale::c().toDouble(t);
}

// one row of his Fixedsys forms, the static caption left and the edit
// right of it
QLineEdit* form_row(QGridLayout* g, int row, const QString& caption, const QString& value, QWidget* parent) {
  auto* label = new QLabel(caption, parent);
  label->setFont(theme::mono_font());
  auto* edit = new QLineEdit(value, parent);
  edit->setFont(theme::mono_font());
  edit->setFixedWidth(80);
  edit->setAlignment(Qt::AlignRight);
  g->addWidget(label, row, 0);
  g->addWidget(edit, row, 1);
  return edit;
}

// the WEITER, EXIT and ALLE buttons of his forms, the answer lands in
// result, 1 WEITER, 2 ALLE, 3 EXIT
void form_buttons(QDialog& d, QVBoxLayout* column, const QString& all, int& result) {
  auto* weiter = new QPushButton(MainWindow::tr("WEITER"), &d);
  weiter->setDefault(true);
  auto* exit = new QPushButton(MainWindow::tr("EXIT"), &d);
  auto* reset = new QPushButton(all, &d);
  column->addStretch(1);
  column->addWidget(weiter);
  column->addWidget(exit);
  column->addWidget(reset);
  QObject::connect(weiter, &QPushButton::clicked, &d, [&d, &result]() {
    result = 1;
    d.accept();
  });
  QObject::connect(reset, &QPushButton::clicked, &d, [&d, &result]() {
    result = 2;
    d.accept();
  });
  QObject::connect(exit, &QPushButton::clicked, &d, [&d, &result]() {
    result = 3;
    d.accept();
  });
}

// R and PgUp of his forms, zurueck! on WM_KEYUP 33, 82 and 114
class BackKeys final : public QObject {
 public:
  explicit BackKeys(std::function<void()> back) : back_(std::move(back)) {}

 protected:
  bool eventFilter(QObject*, QEvent* e) override {
    if (e->type() != QEvent::KeyPress) {
      return false;
    }
    const int key = static_cast<QKeyEvent*>(e)->key();
    if (key == Qt::Key_R || key == Qt::Key_PageUp) {
      back_();
      return true;
    }
    return false;
  }

 private:
  std::function<void()> back_;
};

QColor qcolor(Rgb c) {
  return QColor(static_cast<int>((c >> 16) & 0xFF), static_cast<int>((c >> 8) & 0xFF), static_cast<int>(c & 0xFF));
}

// the colour names asp_li writes beside the standard lines
QString standard_colour_name(Rgb c) {
  switch (c) {
    case 0xFF0000: return MainWindow::tr("ROT");
    case 0x00C800: return MainWindow::tr("GRÜN");
    case 0x0000C8: return MainWindow::tr("BLAU");
    default: return MainWindow::tr("SCHWARZ");
  }
}

Qt::PenStyle pen_style(Primitive::Style s) {
  switch (s) {
    case Primitive::Style::kDashed: return Qt::DashLine;
    case Primitive::Style::kDotted: return Qt::DotLine;
    case Primitive::Style::kDashDot: return Qt::DashDotLine;
    default: return Qt::SolidLine;
  }
}

// all$(1..23), the rows of the line style screen
constexpr const char* kLineRows[24] = {
    "",
    QT_TRANSLATE_NOOP("horcom::MainWindow", "        OHNE ASPEKT - LINIEN"),
    QT_TRANSLATE_NOOP("horcom::MainWindow", "      TEILER  2 = 180° = OPPOSITION "),
    QT_TRANSLATE_NOOP("horcom::MainWindow", "         ''   3 = 120° = TRIGON  "),
    QT_TRANSLATE_NOOP("horcom::MainWindow", "         ''   4 =  90° = QUADRAT "),
    QT_TRANSLATE_NOOP("horcom::MainWindow", "         ''   5 =  72° = QUINTIL "),
    QT_TRANSLATE_NOOP("horcom::MainWindow", "         ''   6 =  60° = SEXTIL  "),
    QT_TRANSLATE_NOOP("horcom::MainWindow", "         ''   7 =51.4° = SEPTIL  "),
    QT_TRANSLATE_NOOP("horcom::MainWindow", "         ''   8 =  45° = OKTIL = HALBQUADRAT "),
    QT_TRANSLATE_NOOP("horcom::MainWindow", "         ''   9 =  40° = NONIL "),
    QT_TRANSLATE_NOOP("horcom::MainWindow", "         ''  10 =  36° = DEZIL "),
    QT_TRANSLATE_NOOP("horcom::MainWindow", "         ''  11 =32.7° = UNDEZIL "),
    QT_TRANSLATE_NOOP("horcom::MainWindow", "         ''  12 =  30° = DODEZIL = HALBSEXTIL    "),
    QT_TRANSLATE_NOOP("horcom::MainWindow", "2*72    = 144°         = BIQUINTIL               "),
    QT_TRANSLATE_NOOP("horcom::MainWindow", "X*51.4  = 103°,154.3°  = BISEPTIL , TRISEPTIL    "),
    QT_TRANSLATE_NOOP("horcom::MainWindow", "3*45    = 135°         = ANDERTHALBQUADRAT       "),
    QT_TRANSLATE_NOOP("horcom::MainWindow", "X*40    =  80°,160°    = BINONIL,QUATTRONONIL    "),
    QT_TRANSLATE_NOOP("horcom::MainWindow", "3*36    = 108°         = TRIDEZIL                "),
    QT_TRANSLATE_NOOP("horcom::MainWindow", "X*32.73 =65.5°,98.2°,130.9°,163.6°=BIUNDEZIL..   "),
    QT_TRANSLATE_NOOP("horcom::MainWindow", "5*30    = 150°         = QUINCUNX                "),
    QT_TRANSLATE_NOOP("horcom::MainWindow", "        EIGENE EINSTELLUNGEN BENUTZEN"),
    QT_TRANSLATE_NOOP("horcom::MainWindow", "      EIGENE EINSTELLUNGEN NEU FESTLEGEN"),
    QT_TRANSLATE_NOOP("horcom::MainWindow", "      HORCOM - STANDARD WIEDERHERSTELLEN"),
    QT_TRANSLATE_NOOP("horcom::MainWindow", "        WAHLENDE = WEITER = SPEICHERN")};

// the geometry of avh13, rows of 17 from y0 30, bars 10 to 400 wide
constexpr int kLineTop = 30;
constexpr int kLineHeight = 17;
constexpr int kLineLeft = 10;
constexpr int kLineBar = 400;
constexpr int kLineCount = 23;
constexpr int kLineCanvasW = 640;
constexpr int kLineCanvasH = kLineTop + kLineCount * kLineHeight + 10;
// the sample of asp_li, a thick stub of 17 then the thin line to 624 or
// 560, the standard rows carry their colour name after it
constexpr double kSampleFrom = 400.0;
constexpr double kSampleStub = 17.0;
constexpr double kSampleOwnEnd = 624.0;
constexpr double kSampleStandardEnd = 560.0;
constexpr double kSampleStubWidth = 10.0;

// the drawn list of avh13, 23 bars with the sample lines beside them
class LineStyleList final : public QWidget {
 public:
  LineStyleList(Konsta& k, QWidget* parent) : QWidget(parent), k_(k) {
    setMinimumSize(kLineCanvasW, kLineCanvasH);
    setFocusPolicy(Qt::StrongFocus);
  }

  /// the row a click picked, 1 to 23
  std::function<void(int)> picked;
  /// Enter or Space, his CASE 13,32 leaves the whole avh
  std::function<void()> leave;
  /// neu_cl!, the own settings are being edited
  bool editing = false;

 protected:
  void paintEvent(QPaintEvent*) override {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);
    p.scale(width() / static_cast<double>(kLineCanvasW), height() / static_cast<double>(kLineCanvasH));
    p.fillRect(QRectF(0, 0, kLineCanvasW, kLineCanvasH), Qt::white);
    QFont f = theme::mono_font();
    f.setPixelSize(13);
    p.setFont(f);
    QString head;
    if (k_.selbst_cl_st && editing) {
      head = MainWindow::tr(" LINIEN - FARBEN und STILE WÄHLEN bzw. ÄNDERN ( ANKLICKEN ) ! ");
    } else if (k_.selbst_cl_st) {
      head = MainWindow::tr(" Diese SELBSTGEWÄHLTEN LINIEN - FARBEN und STILE sind AKTIV ! ");
    } else {
      head = MainWindow::tr(" Von HORCOM VORDEFINIERTE LINIEN EIN - oder AUSSCHALTEN ( ANKLICKEN ) ! ");
    }
    p.setPen(Qt::black);
    p.drawText(QRectF(0, 6, kLineCanvasW, 18), Qt::AlignHCenter | Qt::AlignVCenter, head);
    // the active mode wrapped in ** **
    const int active = k_.selbst_cl_st ? (editing ? 21 : 20) : 22;
    for (int i = 1; i <= kLineCount; ++i) {
      const QRectF bar(kLineLeft, kLineTop + (i - 1) * kLineHeight, kLineBar - kLineLeft, kLineHeight);
      // balken, RGB($C0,$DC,$C0) for the aspects, green for the modes,
      // red for the two exits
      QColor bg(QString::fromLatin1(theme::kPanelGreen));
      if (i >= 20 && i <= 22) {
        bg = QColor(0, 255, 0);
      } else if (i == 1 || i == kLineCount) {
        bg = QColor(255, 0, 0);
      }
      QColor ink = Qt::black;
      // INVERT, the row the keys walk
      if (i == cursor_) {
        bg = QColor(255 - bg.red(), 255 - bg.green(), 255 - bg.blue());
        ink = Qt::white;
      }
      p.fillRect(bar, bg);
      p.setPen(Qt::black);
      p.drawRect(bar);
      QString text = MainWindow::tr(kLineRows[i]);
      if (i == active) {
        text = "   ** " + text.trimmed() + " **";
      }
      p.setPen(ink);
      p.drawText(bar.adjusted(4, 0, 0, 0), Qt::AlignVCenter | Qt::AlignLeft, text);
      if (i >= 2 && i <= 19) {
        sample(p, i, bar.bottom());
      }
    }
  }

  void mousePressEvent(QMouseEvent* e) override {
    const double x = e->position().x() * kLineCanvasW / width();
    const double y = e->position().y() * kLineCanvasH / height();
    // MOUSEX/gdx > 3 && < 400
    if (x > 3 && x < kLineBar && y > kLineTop) {
      const int row = 1 + static_cast<int>((y - kLineTop) / kLineHeight);
      if (row >= 1 && row <= kLineCount && picked) {
        picked(row);
      }
    }
  }

  void keyPressEvent(QKeyEvent* e) override {
    switch (e->key()) {
      // Tab and Down walk down, Up walks up
      case Qt::Key_Tab:
      case Qt::Key_Down:
        cursor_ = std::min(cursor_ + 1, kLineCount);
        update();
        return;
      case Qt::Key_Up:
        cursor_ = std::max(cursor_ - 1, 1);
        update();
        return;
      case Qt::Key_Return:
      case Qt::Key_Enter:
      case Qt::Key_Space:
        // EXIT IF ex& = 13 OR ex& = 32 and GOTO avh_st1, the keys never
        // pick the row they walk, only the mouse does
        if (leave) {
          leave();
        }
        return;
      default:
        QWidget::keyPressEvent(e);
    }
  }

  bool focusNextPrevChild(bool) override { return false; }

 private:
  // asp_li, the stub and the thin line in the row's colour and style
  void sample(QPainter& p, int row, double bottom) {
    ChordLine line;
    const auto r = static_cast<std::size_t>(row);
    if (k_.selbst_cl_st) {
      // IF aspli%(ml&) > 0 && aspli%(ml&) <> RGB(255,255,255)
      if (k_.aspli_col[r] <= 0 || k_.aspli_col[r] == 0xFFFFFF) {
        return;
      }
      line = {true, rgb_of_colorref(k_.aspli_col[r]), chord_line_style(k_.aspst[r])};
    } else {
      if (k_.aspli_flag[r] <= 0) {
        return;
      }
      line = standard_chord(row);
    }
    const double y = bottom - 8.0;
    const double end = k_.selbst_cl_st ? kSampleOwnEnd : kSampleStandardEnd;
    // DEFLINE l&,10, a wide pen draws solid on Windows
    p.setPen(QPen(qcolor(line.color), kSampleStubWidth, Qt::SolidLine, Qt::FlatCap));
    p.drawLine(QPointF(kSampleFrom, y), QPointF(kSampleFrom + kSampleStub, y));
    p.setPen(QPen(qcolor(line.color), 1.0, pen_style(line.style)));
    p.drawLine(QPointF(kSampleFrom, y), QPointF(end, y));
    if (!k_.selbst_cl_st) {
      p.setPen(Qt::black);
      p.drawText(QPointF(end + 4, y + 5), standard_colour_name(line.color));
    }
  }

  Konsta& k_;
  int cursor_ = 0;
};

}  // namespace

// ported from avh, the wizard runs on from the chosen topic, R steps back
void MainWindow::vorgaben_horoskop() {
  const QString x = tr("EXIT");
  // ue$(0) = "GEWÜNSCHTES THEMA ANKLICKEN ! "
  const int chosen = ChoiceDialog::ask(
      this, tr("GEWÜNSCHTES THEMA ANKLICKEN ! "), {},
      {tr("Bezugs-System EKLIPTIK oder ÄQUATOR ? "), tr("Langsame BILDSCHIRME SPEICHERN ? "),
       tr("GRÖßE der SYMBOLE im Horoskop "), tr("ORBES BESTIMMEN "),
       tr("GRADE und RÜCKLÄUFIGKEIT im Horoskop ANZEIGEN ? "), tr("BEGINN des Horoskops "),
       tr("ZUORDNUNG ZEICHENHERRSCHER "), tr("FARBEN bzw. SCHRAFFUR im HOROSKOP-RING und HISTOGRAMMEN"),
       tr("HISTOGRAMM der ELEMENTE und KARD-FIX-GEM "),
       tr("ASPEKTE bzw. ASPEKT - Linien bzw. HALBSUMMEN im Horoskop "),
       tr("FARBE FESTLEGEN bei DOPPELKREIS ÄUßERE SYMBOLE bzw. RÜCKLÄUFIGKEITS-ANZEIEGE"), x},
      0);
  if (chosen < 0 || chosen > 10) {
    return;
  }
  int topic = chosen == 10 ? kOuter : chosen;
  // his zurueck! goes to the page before, the line style screen and the
  // aspect question both fall back to the histograms
  const auto back_of = [](int t) {
    switch (t) {
      case kFrame: return static_cast<int>(kFrame);
      case kLines: return static_cast<int>(kHistograms);
      default: return t - 1;
    }
  };
  const auto step_of = [](int answer, int exit_index) {
    if (answer == ChoiceDialog::kBack) {
      return WizardStep::kBack;
    }
    return (answer < 0 || answer == exit_index) ? WizardStep::kExit : WizardStep::kNext;
  };
  while (topic < kTopicEnd) {
    WizardStep step = WizardStep::kNext;
    switch (topic) {
      case kFrame: {
        // IF hrg! = FALSE, BEZUGS-SYSTEM ?
        if (current_settings().heliocentric) {
          konsta_.horm = 1;
          break;
        }
        const int bs = ChoiceDialog::ask_step(this, tr("AUSWAHL"),
                                              {tr("         BEZUGS-SYSTEM ?          "),
                                               tr("EKLIPTIK = Normal - HOROSKOP"), tr("ÄQUATOR  ='Mundan'- HOROSKOP")},
                                              {tr("EKLIPTIK"), tr("ÄQUATORIAL = MUNDAN"), x}, mundane_frame_ ? 1 : 0);
        step = step_of(bs, 2);
        if (bs == 0 || bs == 1) {
          konsta_.horm = bs + 1;
          // MUNDAN setzt begz& = 1
          if (bs == 1) {
            konsta_.begz = 1;
          }
          if ((bs == 1) != mundane_frame_) {
            set_mundane_frame(bs == 1);
          }
        }
        break;
      }
      case kScreens: {
        // bdsp! let aspar and his other slow screens offer the last saved
        // bitmap instead of drawing anew. The port draws every screen in
        // an instant, the switch only stays in the settings file and the
        // box says so
        const int es = ChoiceDialog::ask_step(this, tr("Mit '|' BEZEICHNETE BILDSCHIRME SPEICHERN ?"),
                                              {tr("In dieser Version ohne Wirkung, jedes Bild wird neu gezeichnet !")},
                                              {tr("NICHT SPEICHERN ! ( Nur bei KNAPPEM RAM - SPEICHER ! )"),
                                               tr("Auf HARDDISK SPEICHERN ! ( EMPFOHLEN ! )"), x},
                                              konsta_.bdsp ? 1 : 0);
        step = step_of(es, 2);
        if (es == 0 || es == 1) {
          konsta_.bdsp = es == 1;
        }
        break;
      }
      case kSymbols: {
        const int b = ChoiceDialog::ask_step(this, tr("AUSWAHL"), {QString(), tr("KLEIN-SYMBOLE in HOROSKOPEN ?")},
                                             {tr("NORMAL"), tr("KLEIN"), x}, konsta_.klsy ? 1 : 0);
        step = step_of(b, 2);
        if (b == 0 || b == 1) {
          konsta_.klsy = b == 1;
        }
        break;
      }
      case kOrbs:
        step = orb_topic();
        break;
      case kDegrees: {
        const int d = std::clamp(static_cast<int>(konsta_.pziff), 1, 3) - 1;
        const int b = ChoiceDialog::ask_step(this, tr("AUSWAHL"),
                                             {tr("MÖCHTEN SIE IN HOROSKOPEN"), tr("Bei den Planeten die"),
                                              tr("GRADE bzw. RÜCKLÄUFIGKEITEN ANGEGEBEN HABEN ?")},
                                             {tr("GRADE und RÜCKLÄUFIGKEITEN"), tr("NUR die GRADE"),
                                              tr("NICHTS VON BEIDEN"), x},
                                             d);
        step = step_of(b, 3);
        if (b >= 0 && b <= 2) {
          konsta_.pziff = b + 1;
        }
        break;
      }
      case kBegin: {
        // IF horm& = 1, BEGINN HOROSKOP ?
        if (mundane_frame_) {
          konsta_.begz = 1;
          break;
        }
        const int es = ChoiceDialog::ask_step(this, tr("BEGINN HOROSKOP ?"), {},
                                              {tr("ASZENDENT"), tr("MC"), tr("0 WIDDER"), tr("0 WAAGE "),
                                               tr("BELIEBIGER GRAD"), x},
                                              std::clamp(konsta_.begz, 1, 5) - 1);
        step = step_of(es, 5);
        // his CLR begz$ ran before the choice, so EXIT or R dropped the
        // degree of an own beginning and the wheel fell to zero Aries,
        // the port clears it only when another beginning is chosen
        if (es >= 0 && es <= 3) {
          konsta_.begz = es + 1;
          konsta_.begz_name.clear();
        } else if (es == 4) {
          // begz$ = @inputbox$(160,eaz$,"BEGINN-GRAD für HOROSKOP !","")
          const std::optional<double> deg = ask_number(this, tr("ZAHLEN-Eingabe !"), tr("BEGINN-GRAD für HOROSKOP !"),
                                                       0.0, kDegPerCircle, std::numeric_limits<double>::quiet_NaN(), 4);
          if (deg) {
            konsta_.begz = 5;
            konsta_.begz_name = QString::number(*deg, 'f', 4).toStdString();
          }
        }
        break;
      }
      case kRulers: {
        const int b = ChoiceDialog::ask_step(this, tr("AUSWAHL"),
                                             {tr("ZUORDNUNG ZEICHEN-HERRSCHER ?"), QString(),
                                              tr("NEU : SC-PL   AQ-UR   PS-NE"), tr("ALT : SC-MA   AQ-SA   PS-JU")},
                                             {tr("NEU"), tr("ALT"), x}, alt_rulers_ ? 1 : 0);
        step = step_of(b, 2);
        // alt! lives for the session only, his settings file never kept it
        if (b == 0 || b == 1) {
          alt_rulers_ = b == 1;
        }
        break;
      }
      case kColours:
        step = colour_topic();
        break;
      case kHistograms: {
        // HISTOGRAMME für ELEMENTE - und KARD / FIX / GEM ?
        const bool helio = current_settings().heliocentric;
        const int b = ChoiceDialog::ask_step(
            this, tr("AUSWAHL"), {tr("HISTOGRAMME für ELEMENTE") + (helio ? QString() : tr(" - und KARD / FIX / GEM ?"))},
            {tr("Nur für ZEICHEN"), helio ? QStringLiteral(" ") : tr("Für ZEICHEN und HÄUSER"),
             tr("HISTOGRAMME WEGLASSEN !"), x},
            std::clamp(konsta_.elem, 1, 3) - 1, helio ? std::vector<int>{1} : std::vector<int>{});
        step = step_of(b, 3);
        if (b >= 0 && b <= 2) {
          konsta_.elem = b + 1;
          if (b <= 1) {
            step = points_topic();
          }
        }
        break;
      }
      case kAspects:
        step = aspect_topic();
        break;
      case kLines:
        // IF nasp& > 1, the line style screen
        if (aspect_settings_.divisors > 1) {
          step = line_style_screen();
        }
        break;
      case kOuter: {
        const int f = ChoiceDialog::ask_step(this, tr("AUSWAHL"),
                                             {tr("Bei DOPPELKREIS für ÄUßERE SYMBOLE"),
                                              tr("Und bei RÜCKLÄUFIGKEITS-ANZEIGE"), tr("Die FARBE FESTLEGEN !")},
                                             {tr("ROT"), tr("SCHWARZ"), tr("BLAU")}, std::clamp(outer_color_, 1, 3) - 1);
        step = f == ChoiceDialog::kBack ? WizardStep::kBack : WizardStep::kExit;
        if (f >= 0 && f <= 2) {
          // hard& lives in KONSTA like every other avh answer
          outer_color_ = f + 1;
          konsta_.hard = outer_color_;
        }
        break;
      }
      default:
        break;
    }
    // @param_sp after every topic
    persist_konsta();
    if (step == WizardStep::kExit) {
      break;
    }
    topic = step == WizardStep::kBack ? back_of(topic) : topic + 1;
  }
  recompute();
}

// ORBES BESTIMMEN, avh4 with orbis_asp, avh5 with orbis_pla and the
// factor, R on the factor box starts the topic again
MainWindow::WizardStep MainWindow::orb_topic() {
  const QString x = tr("EXIT");
  for (;;) {
    const bool own = aspect_settings_.equal_probability;
    const int es = ChoiceDialog::ask_step(
        this, tr("ORBES der ASPEKTE EINZELN VORGEBEN ?"), {},
        {tr("ORBES nach HORCOM - ZÄHLUNG : ORB = 12°/ TEILER  z.B 3° für QUADRAT ( TEILER = 4 )"),
         own ? tr("ORBES sind SELBST DEFINIERT") : QStringLiteral("  "), tr("ORBES NEU SELBST DEFINIEREN"), x},
        own ? 1 : 0, own ? std::vector<int>{} : std::vector<int>{1});
    if (es == ChoiceDialog::kBack) {
      return WizardStep::kBack;
    }
    if (es < 0 || es == 3) {
      return WizardStep::kExit;
    }
    if (es == 0) {
      aspect_settings_.equal_probability = false;
    } else {
      aspect_settings_.equal_probability = true;
      // IF nasp& = 16, nasp& = 12
      if (es == 2 && aspect_settings_.divisors > kMaxEqualOrbDivisor) {
        aspect_settings_.divisors = kMaxEqualOrbDivisor;
      }
      if (es == 2 && !orb_table_dialog()) {
        return WizardStep::kExit;
      }
    }
    persist_konsta();
    const WizardStep weights = orb_weight_dialog();
    // IF zurueck!, GOTO avh4, R on orbis_pla asks the orbs again
    if (weights == WizardStep::kBack) {
      continue;
    }
    if (weights == WizardStep::kExit) {
      return WizardStep::kExit;
    }
    persist_konsta();
    // FAKTOR vor ORBIS in PROZENTEN
    int def = 5;
    for (int i = 0; i < 5; ++i) {
      if (aspect_settings_.orb == kFactors[i]) {
        def = i;
      }
    }
    const int fk = ChoiceDialog::ask_step(this, tr("FAKTOR vor ORBIS in PROZENTEN"), {},
                                          {tr(" 50 %"), tr(" 80 %"), tr("100 %"), tr("150 %"), tr("200 %"),
                                           tr("SONSTIGE"), x},
                                          def);
    if (fk == ChoiceDialog::kBack) {
      continue;
    }
    if (fk < 0 || fk == 6) {
      return WizardStep::kExit;
    }
    if (fk < 5) {
      aspect_settings_.orb = kFactors[fk];
      return WizardStep::kNext;
    }
    for (;;) {
      // fk$ = @inputbox$(160,eaz$,"GEWÜNSCHTER PROZENTSATZ !","")
      const std::optional<double> pct =
          ask_number(this, tr("ZAHLEN-Eingabe !"), tr("GEWÜNSCHTER PROZENTSATZ !"), -kFreeEntry, kFreeEntry,
                     std::numeric_limits<double>::quiet_NaN(), 0);
      if (!pct) {
        return WizardStep::kExit;
      }
      // orb = INT(ABS(VAL(fk$))) / 100
      const double orb = std::floor(std::abs(*pct)) / kPercent;
      if (orb >= kMinFactor && orb <= kMaxFactor) {
        aspect_settings_.orb = orb;
        return WizardStep::kNext;
      }
    }
  }
}

// ported from orbis_asp, false after its EXIT
bool MainWindow::orb_table_dialog() {
  const bool helio = current_settings().heliocentric;
  // or$(), the checks measure against the HORCOM count
  std::array<double, 15> base{};
  for (int i = 1; i <= 12; ++i) {
    base[static_cast<std::size_t>(i)] = kOrbTwelve / i;
  }
  base[13] = kMirrorOrb;
  base[14] = kMidpointOrb;
  // IF orb$(i&) = "", orb$(i&) = or$(i&)
  std::array<double, 15> values{};
  for (int i = 1; i <= 14; ++i) {
    const double deg = aspect_settings_.orbe[static_cast<std::size_t>(i)] * kRadToDeg;
    values[static_cast<std::size_t>(i)] = deg > 0.0 ? deg : base[static_cast<std::size_t>(i)];
  }
  for (;;) {
    QDialog d(this);
    d.setWindowTitle(tr("ORBES der GRUND-ASPEKTE EINGEBEN !"));
    auto* v = new QVBoxLayout(&d);
    auto* head = new QLabel(tr("TEILER    ASPEKT             EINGABE ( EDITIEREN ! ) "), &d);
    head->setFont(theme::mono_font());
    v->addWidget(head);
    auto* row = new QHBoxLayout();
    v->addLayout(row);
    auto* g = new QGridLayout();
    row->addLayout(g);
    std::array<QLineEdit*, 15> edits{};
    for (int i = 1; i <= 14; ++i) {
      // d& = hc& for row 13 and 2 * hc& for row 14
      const int r = i <= 12 ? i : 2 * i - 12;
      edits[static_cast<std::size_t>(i)] = form_row(g, r, tr(kOrbRows[i - 1]),
                                                    QString::asprintf("%5.2f", values[static_cast<std::size_t>(i)]), &d);
    }
    g->setRowMinimumHeight(13, 8);
    g->setRowMinimumHeight(15, 8);
    auto* right = new QVBoxLayout();
    row->addLayout(right);
    // h$ = " GEOZENTRISCH " bzw. "TOPOZENTRISCH" bzw. " HELIOZENTRISCH "
    const QString h = helio ? tr(" HELIOZENTRISCH ")
                            : (current_settings().topocentric_parallax ? tr("TOPOZENTRISCH") : tr(" GEOZENTRISCH "));
    auto* txt = new QLabel(tr("Wenn Sie den folgenden Button anklicken,werden Orbes gewählt,die%1etwa GLEICHE "
                              "Wahrscheinlichkeit für alle Aspekte mit Teiler  1 bis 12 aufweisen ! ( 1930 bis 2050 )"
                              "          Näheres,siehe Erläuterung 4 !")
                               .arg(h),
                           &d);
    txt->setWordWrap(true);
    txt->setMaximumWidth(320);
    right->addWidget(txt);
    auto* equal = new QPushButton(tr("ORBES für GLEICH WAHRSCHEINLICHE ASPEKTE"), &d);
    right->addWidget(equal);
    right->addStretch(1);
    auto* buttons = new QHBoxLayout();
    // wr$ = "Weiter"
    auto* weiter = new QPushButton(tr("Weiter"), &d);
    weiter->setDefault(true);
    auto* exit = new QPushButton(tr("EXIT"), &d);
    buttons->addWidget(weiter);
    buttons->addWidget(exit);
    right->addLayout(buttons);
    int result = 0;
    connect(weiter, &QPushButton::clicked, &d, [&]() {
      result = 1;
      d.accept();
    });
    connect(equal, &QPushButton::clicked, &d, [&]() {
      result = 2;
      d.accept();
    });
    connect(exit, &QPushButton::clicked, &d, [&]() {
      result = 3;
      d.accept();
    });
    // ESC leaves the table as it was and goes on
    if (d.exec() != QDialog::Accepted) {
      return true;
    }
    if (result == 3) {
      return false;
    }
    if (result == 2) {
      for (int i = 1; i <= 12; ++i) {
        values[static_cast<std::size_t>(i)] = helio ? kEqualHelio[i] : kEqualGeo[i];
      }
      values[13] = kMirrorOrb;
      values[14] = kMidpointOrb;
      continue;
    }
    for (int i = 1; i <= 14; ++i) {
      values[static_cast<std::size_t>(i)] = field(edits[static_cast<std::size_t>(i)]);
    }
    // the first offending row stops the check, his GOTO orstt shows the
    // table again with the entries as typed
    bool fine = true;
    for (int i = 1; i <= 12 && fine; ++i) {
      const double val = values[static_cast<std::size_t>(i)];
      QString why;
      // IF i& > 2 && i& < 13, ÜBERDECKENDER ORBIS !
      if (i > 2 && val > kOverlapShare * std::abs(kDegPerCircle / (i + 1) - kDegPerCircle / i)) {
        why = tr("ÜBERDECKENDER ORBIS !");
      } else if (val > kTooLargeShare * base[static_cast<std::size_t>(i)]) {
        why = tr("ZU GROßER ORBIS !");
      }
      if (!why.isEmpty()) {
        ChoiceDialog::ask(this, tr("!! ACHTUNG !!"), {QString(), why, tr("BEI NR. %1").arg(i)}, {tr("NOCHMAL")});
        fine = false;
      }
    }
    if (!fine) {
      continue;
    }
    for (int i = 1; i <= 14; ++i) {
      aspect_settings_.orbe[static_cast<std::size_t>(i)] = std::abs(values[static_cast<std::size_t>(i)]) * kDegToRad;
    }
    return true;
  }
}

// ported from orbis_pla, R on its first box steps back to the orbs
MainWindow::WizardStep MainWindow::orb_weight_dialog() {
  const int b = ChoiceDialog::ask_step(this, tr("AUSWAHL"),
                                       {QString(), tr("GEWICHTUNG der"), tr("PLANETEN-ORBES"), tr("ÄNDERN ?")},
                                       {tr(" NEIN "), tr("JA"), tr("EXIT")}, 0);
  if (b == ChoiceDialog::kBack) {
    return WizardStep::kBack;
  }
  if (b == 0) {
    return WizardStep::kNext;
  }
  if (b != 1) {
    return WizardStep::kExit;
  }
  // orbe$(0..15)
  static constexpr const char* kRows[16] = {
      QT_TR_NOOP("FIXPUNKT     = FP "), QT_TR_NOOP("SONNE        = SO "), QT_TR_NOOP("MOND         = MO "), QT_TR_NOOP("MERKUR       = ME "),
      QT_TR_NOOP("VENUS        = VE "), QT_TR_NOOP("MARS         = MA "), QT_TR_NOOP("JUPITER      = JU "), QT_TR_NOOP("SATURN       = SA "),
      QT_TR_NOOP("URANUS       = UR "), QT_TR_NOOP("NEPTUN       = NE "), QT_TR_NOOP("PLUTO        = PL "), QT_TR_NOOP("MONDKNOTEN N = DR "),
      QT_TR_NOOP("MONDKNOTEN S = DS "), QT_TR_NOOP("ASZENDENT    = AC "), QT_TR_NOOP("MEDIUM COELI = MC "), QT_TR_NOOP("ZUSATZ-PLANETEN   ")};
  constexpr int kExtraRow = 15;
  for (;;) {
    QDialog d(this);
    d.setWindowTitle(tr("ORBES - GEWICHTE der PLANETEN in % EINGEBEN !"));
    auto* v = new QVBoxLayout(&d);
    v->addWidget(new QLabel(tr(" AN CURSOR-POSITION EDITIEREN ! NORMAL = 100 % "), &d));
    auto* row = new QHBoxLayout();
    v->addLayout(row);
    auto* g = new QGridLayout();
    row->addLayout(g);
    std::array<QLineEdit*, 16> edits{};
    // et$(15) shows the last extra body that carries a weight
    int extra = aspect_settings_.weight[kExtraRow];
    for (int s = body::kApogee; s < body::kSlotCount; ++s) {
      if (aspect_settings_.weight[static_cast<std::size_t>(s)] > 0) {
        extra = aspect_settings_.weight[static_cast<std::size_t>(s)];
      }
    }
    for (int l = 0; l <= kExtraRow; ++l) {
      // IF NOT(fixpunkt& = 2 && l& = 0)
      if (l == 0 && fixpunkt_ < 0.0) {
        continue;
      }
      const int w = l < kExtraRow ? aspect_settings_.weight[static_cast<std::size_t>(l)] : extra;
      edits[static_cast<std::size_t>(l)] = form_row(g, l, tr(kRows[l]), QString::asprintf("%3d", w), &d);
    }
    auto* right = new QVBoxLayout();
    row->addLayout(right);
    int result = 0;
    form_buttons(d, right, tr("ALLE auf 100 %"), result);
    // ESC leaves the weights as they were and goes on
    if (d.exec() != QDialog::Accepted) {
      return WizardStep::kNext;
    }
    if (result == 3) {
      return WizardStep::kExit;
    }
    if (result == 2) {
      // his FOR k& = 0 TO 15 left the extras 19 to 40 alone and the next
      // pass showed their old weight again, the port resets all of them
      for (int s = 0; s < body::kSlotCount; ++s) {
        if (s <= kExtraRow || s >= body::kApogee) {
          aspect_settings_.weight[static_cast<std::size_t>(s)] = static_cast<int>(kPercent);
        }
      }
      persist_konsta();
      continue;
    }
    bool fine = true;
    for (int l = 0; l <= kExtraRow; ++l) {
      QLineEdit* e = edits[static_cast<std::size_t>(l)];
      if (e == nullptr) {
        continue;
      }
      const int w = static_cast<int>(field(e));
      if (w > kMaxWeight) {
        // fanz("ZU GROßER ORBIS bei " + p$(i&) + " ")
        const QString tag = l < kExtraRow ? QString::fromUtf8(body::kName[static_cast<std::size_t>(l)].data(),
                                                               static_cast<int>(body::kName[static_cast<std::size_t>(l)].size()))
                                          : tr("ZUSATZ-PLANETEN");
        QMessageBox::information(this, "HORCOM", tr("ZU GROßER ORBIS bei %1 ").arg(tag));
        aspect_settings_.weight[static_cast<std::size_t>(l)] = static_cast<int>(kPercent);
        fine = false;
        break;
      }
      aspect_settings_.weight[static_cast<std::size_t>(l)] = w;
      if (l == kExtraRow) {
        // or&(19..40) = VAL(et$(15))
        for (int s = body::kApogee; s < body::kSlotCount; ++s) {
          aspect_settings_.weight[static_cast<std::size_t>(s)] = w;
        }
      }
    }
    if (fine) {
      return WizardStep::kNext;
    }
  }
}

// ported from punkte_pla
MainWindow::WizardStep MainWindow::points_topic() {
  const int b = ChoiceDialog::ask_step(this, tr("AUSWAHL"),
                                       {tr("GEWICHTUNG für HISTOGRAMM der ELEMENTE und KARD/FIX/GEM ÄNDERN?"),
                                        tr("ZIFFER von 0 bis 9")},
                                       {tr(" NEIN "), tr("JA"), tr("EXIT")}, 0);
  if (b == ChoiceDialog::kBack) {
    return WizardStep::kBack;
  }
  if (b == 0) {
    return WizardStep::kNext;
  }
  if (b != 1) {
    return WizardStep::kExit;
  }
  // punkte$(1..15)
  static constexpr const char* kRows[16] = {
      "", QT_TR_NOOP("SONNE        = SO "), QT_TR_NOOP("MOND         = MO "), QT_TR_NOOP("MERKUR       = ME "), QT_TR_NOOP("VENUS        = VE "),
      QT_TR_NOOP("MARS         = MA "), QT_TR_NOOP("JUPITER      = JU "), QT_TR_NOOP("SATURN       = SA "), QT_TR_NOOP("URANUS       = UR "),
      QT_TR_NOOP("NEPTUN       = NE "), QT_TR_NOOP("PLUTO        = PL "), QT_TR_NOOP("MONDKNOTEN N = DR "), QT_TR_NOOP("MONDKNOTEN S = DS "),
      QT_TR_NOOP("ASZENDENT    = AC "), QT_TR_NOOP("MEDIUM COELI = MC "), QT_TR_NOOP("ZUSATZ-PLANETEN   ")};
  for (;;) {
    QDialog d(this);
    d.setWindowTitle(tr("PUNKTE-WERT 0....9 EINGEBEN !"));
    auto* v = new QVBoxLayout(&d);
    v->addWidget(new QLabel(tr(" AN CURSOR-POSITION EDITIEREN ! "), &d));
    auto* row = new QHBoxLayout();
    v->addLayout(row);
    auto* g = new QGridLayout();
    row->addLayout(g);
    std::array<QLineEdit*, 16> edits{};
    auto* digit = new QRegularExpressionValidator(QRegularExpression(QStringLiteral(" ?[0-9]")), &d);
    for (int i = 1; i <= 15; ++i) {
      QLineEdit* e = form_row(g, i, tr(kRows[i]), QString::asprintf("%2d", konsta_.pn[static_cast<std::size_t>(i)]), &d);
      e->setValidator(digit);
      edits[static_cast<std::size_t>(i)] = e;
    }
    auto* right = new QVBoxLayout();
    row->addLayout(right);
    auto* herr = new QCheckBox(tr("1.GEBURTSHERRSCHER  DOPPELT"), &d);
    herr->setChecked(konsta_.gebherr_dop);
    auto* haus = new QCheckBox(tr("PUNKTE im 1. HAUS DOPPELT"), &d);
    haus->setChecked(konsta_.haus1_dop);
    right->addWidget(herr);
    right->addWidget(haus);
    // the two switches take effect the moment they flip, @kon_dsp
    connect(herr, &QCheckBox::toggled, &d, [this](bool on) {
      konsta_.gebherr_dop = on;
      persist_konsta();
    });
    connect(haus, &QCheckBox::toggled, &d, [this](bool on) {
      konsta_.haus1_dop = on;
      persist_konsta();
    });
    int result = 0;
    form_buttons(d, right, tr("ALLE auf 1"), result);
    bool back = false;
    BackKeys keys([&]() {
      back = true;
      d.reject();
    });
    d.installEventFilter(&keys);
    for (QLineEdit* e : edits) {
      if (e != nullptr) {
        e->installEventFilter(&keys);
      }
    }
    const bool accepted = d.exec() == QDialog::Accepted;
    if (back) {
      return WizardStep::kBack;
    }
    // ESC keeps the points and goes on
    if (!accepted) {
      return WizardStep::kNext;
    }
    if (result == 3) {
      return WizardStep::kExit;
    }
    if (result == 2) {
      for (int i = 1; i <= 15; ++i) {
        konsta_.pn[static_cast<std::size_t>(i)] = 1;
      }
      persist_konsta();
      continue;
    }
    for (int i = 1; i <= 15; ++i) {
      konsta_.pn[static_cast<std::size_t>(i)] =
          std::clamp(static_cast<int>(field(edits[static_cast<std::size_t>(i)])), 0, kMaxPoints);
    }
    return WizardStep::kNext;
  }
}

// FARBEN im HOROSKOP-RING u.HISTOGRAMMEN ?, avh9 with hor_farb
MainWindow::WizardStep MainWindow::colour_topic() {
  // the default follows his IF cascade, the last match wins
  const Konsta& k = konsta_;
  int ze = 1;
  if (k.farbs) ze = 1;
  if (k.farbs && k.eigfarb) ze = 2;
  if (k.farbp && k.eigfarb) ze = 3;
  if (k.weiss) ze = 4;
  if (k.eigfarb && k.farbs && k.nursymb == 2) ze = 5;
  if (k.eigfarb && k.farbp && k.nursymb == 2) ze = 6;
  if (k.weiss && k.eigfarb && k.nursymb == 1) ze = 7;
  if (k.farbs && k.eigfarb && k.nursymb == 1) ze = 8;
  if (k.farbp && k.eigfarb && k.nursymb == 1) ze = 9;
  const int es = ChoiceDialog::ask_step(
      this, tr("FARBEN im HOROSKOP-RING u.HISTOGRAMMEN ?"), {},
      {tr("HOROSKOP-RING und HISTOGRAMME :          SCHRAFFIERT STANDARD-FARBEN"),
       tr("HOROSKOP-RING und HISTOGRAMME :          SCHRAFFIERT EIGENE FARBEN  "),
       tr("HOROSKOP-RING und HISTOGRAMME :          FARBIG PUR  EIGENE FARBEN  "),
       tr("ZEICHEN-SYMBOLE: SCHWARZ   HISTOGRAMME:     WEIß                    "),
       tr("ZEICHEN-SYMBOLE: SCHWARZ   HISTOGRAMME:  SCHRAFFIERT EIGENE FARBEN  "),
       tr("ZEICHEN-SYMBOLE: SCHWARZ   HISTOGRAMME:  FARBIG PUR  EIGENE FARBEN  "),
       tr("ZEICHEN-SYMBOLE: FARBIG    HISTOGRAMME:     WEIß                    "),
       tr("ZEICHEN-SYMBOLE: FARBIG    HISTOGRAMME:  SCHRAFFIERT EIGENE FARBEN  "),
       tr("ZEICHEN-SYMBOLE: FARBIG    HISTOGRAMME:  FARBIG PUR  EIGENE FARBEN  "), tr("EXIT")},
      ze - 1);
  if (es == ChoiceDialog::kBack) {
    return WizardStep::kBack;
  }
  if (es < 0 || es == 9) {
    return WizardStep::kExit;
  }
  Konsta& m = konsta_;
  // the SELECT of avh9, each answer sets its switches and clears the rest
  struct Mode {
    bool farbs, farbp, weiss, eigfarb;
    int nursymb;
    bool own;
  };
  static constexpr Mode kModes[9] = {{true, false, false, false, 0, false}, {true, false, false, true, 0, true},
                                     {false, true, false, true, 0, true},   {false, false, true, false, 0, false},
                                     {true, false, false, true, 2, true},   {false, true, false, true, 2, true},
                                     {false, false, true, true, 1, false},  {true, false, false, true, 1, true},
                                     {false, true, false, true, 1, true}};
  const Mode& mode = kModes[es];
  m.farbs = mode.farbs;
  m.farbp = mode.farbp;
  m.weiss = mode.weiss;
  m.eigfarb = mode.eigfarb;
  m.nursymb = mode.nursymb;
  persist_konsta();
  return mode.own ? ring_colour_dialog() : WizardStep::kNext;
}

// ported from hor_farb, the four element colours picked in the colour
// dialog one after the other. His pboxn painted every element row in its
// colour with the hatch or the brush of the ring, the rows here wear the
// shade the sign band shows
MainWindow::WizardStep MainWindow::ring_colour_dialog() {
  static constexpr const char* kRows[4] = {QT_TR_NOOP("FEUER - ZEICHEN > "), QT_TR_NOOP("ERD   - ZEICHEN > "),
                                           QT_TR_NOOP("LUFT  - ZEICHEN > "), QT_TR_NOOP("WASSER- ZEICHEN > ")};
  // IF cols%(i&) = 0, cols%(i&) = RGB(255,255,255)
  for (int i = 1; i <= 4; ++i) {
    if (konsta_.cols[static_cast<std::size_t>(i)] <= 0) {
      konsta_.cols[static_cast<std::size_t>(i)] = theme::to_colorref(Qt::white);
    }
  }
  // IF (weiss! = 0 OR fill& = 8) && farbp! = 0 the hatch, under farbp! the
  // solid brush
  const RingFill fill = konsta_.farbp ? RingFill::kSolid : RingFill::kShaded;
  for (;;) {
    QStringList rows;
    for (int i = 1; i <= 4; ++i) {
      // g$(i&) = " ( RGB - COLOR : " + STR$(cols%(i&)) + " )"
      rows << tr(kRows[i - 1]) + "  " + tr(" ( RGB - COLOR : %1 )").arg(konsta_.cols[static_cast<std::size_t>(i)]);
    }
    rows << tr("FARBEN NICHT ÄNDERN") << tr("      EXIT");
    int es = -1;
    bool back = false;
    {
      ChoiceDialog d(tr("FARBEN für HOROSKOP - RING FESTLEGEN ! ( ANKLICKEN )"), {}, rows, 0, this);
      const QList<QPushButton*> buttons = d.findChildren<QPushButton*>(Qt::FindDirectChildrenOnly);
      for (int i = 0; i < 4 && i < buttons.size(); ++i) {
        const Rgb shade = ring_fill_color(i + 1, rgb_of_colorref(konsta_.cols[static_cast<std::size_t>(i + 1)]), fill);
        QPixmap swatch(kSwatchWidth, kSwatchHeight);
        swatch.fill(qcolor(shade));
        buttons[i]->setIcon(QIcon(swatch));
        buttons[i]->setIconSize(swatch.size());
      }
      // R and PgUp step back like his zurueck! of hor_farb
      BackKeys keys([&]() {
        back = true;
        d.reject();
      });
      d.installEventFilter(&keys);
      for (QPushButton* b : buttons) {
        b->installEventFilter(&keys);
      }
      d.exec();
      es = d.choice();
    }
    if (back) {
      return WizardStep::kBack;
    }
    // ESC and FARBEN NICHT ÄNDERN go on, EXIT ends the wizard
    if (es < 0 || es == 4) {
      return WizardStep::kNext;
    }
    if (es == 5) {
      return WizardStep::kExit;
    }
    const QColor chosen =
        QColorDialog::getColor(theme::from_colorref(konsta_.cols[static_cast<std::size_t>(es + 1)]), this,
                               tr(" FARBE für %1 ANKLICKEN ! ").arg(tr(kRows[es]).left(15)));
    if (chosen.isValid()) {
      // zero stands for no colour in his cols%, pure black rides one step
      // lighter like the fast schwarz of the line colours
      konsta_.cols[static_cast<std::size_t>(es + 1)] = std::max(theme::to_colorref(chosen), 1);
      persist_konsta();
    }
  }
}

// ASPEKTE bzw. ASPEKT - Linien bzw. HALBSUMMEN, avh12
MainWindow::WizardStep MainWindow::aspect_topic() {
  const bool own = aspect_settings_.equal_probability;
  int ze = 1;
  switch (aspect_settings_.divisors) {
    case 4: ze = 2; break;
    case 8: ze = 3; break;
    case 12: ze = 4; break;
    case 16: ze = 5; break;
    default: break;
  }
  // IF ryt! ze& = 6
  if (konsta_.ryt) {
    ze = 6;
  }
  // a$ = " ASPEKTE ZÄHLEN mit " + tl$ + " 1...", or$ = " SELBST DEFINIERT"
  const QString a = tr(" ASPEKTE ZÄHLEN mit Teiler 1...");
  const QString orx = own ? tr(" SELBST DEFINIERT") : QString();
  const int es = ChoiceDialog::ask_step(this, tr("AUSWERTUNGEN im HOROSKOP-FORMULAR ?"), {},
                                        {tr("OHNE ASPEKT - LINIEN"), a + ".4" + orx, a + ".8" + orx, a + "12" + orx,
                                         own ? QStringLiteral("  ") : a + "16", tr("EXIT")},
                                        ze - 1, own ? std::vector<int>{4} : std::vector<int>{});
  if (es == ChoiceDialog::kBack) {
    return WizardStep::kBack;
  }
  if (es < 0 || es == 5) {
    return WizardStep::kExit;
  }
  static constexpr int kNasp[5] = {1, 4, 8, 12, 16};
  aspect_settings_.divisors = kNasp[es];
  konsta_.ryt = false;
  // HALBSUMMENLISTE HINZUNEHMEN ? = 'KOMPAKT-AUSWERTUNG' !
  const int hs = ChoiceDialog::ask_step(this, tr("AUSWAHL"),
                                        {tr("HALBSUMMENLISTE HINZUNEHMEN ?"), tr(" = 'KOMPAKT-AUSWERTUNG' !")},
                                        {tr("NEIN"), tr("JA")}, konsta_.voll ? 1 : 0);
  // IF zurueck!, GOTO avh11, R steps back to the histograms
  if (hs == ChoiceDialog::kBack) {
    return WizardStep::kBack;
  }
  if (hs == 0 || hs == 1) {
    konsta_.voll = hs == 1;
  }
  return WizardStep::kNext;
}

// ported from avh13 with balken, asp_li and asp_li_upd
MainWindow::WizardStep MainWindow::line_style_screen() {
  QDialog d(this);
  d.setWindowTitle(tr("ASPEKT - LINIEN"));
  auto* v = new QVBoxLayout(&d);
  v->setContentsMargins(0, 0, 0, 0);
  auto* list = new LineStyleList(konsta_, &d);
  list->setObjectName("lineStyleList");
  v->addWidget(list, 1);
  WizardStep result = WizardStep::kNext;
  list->picked = [&](int me) {
    // his IF me& > 4 && nasp& < 12 also caught the mode and exit rows, so
    // WAHLENDE could not leave a Teiler 4 or 8 behind, the port asks it of
    // the aspect rows only
    if (me > 4 && me < 20 && aspect_settings_.divisors < kMaxEqualOrbDivisor) {
      aspect_settings_.divisors = kMaxEqualOrbDivisor;
      persist_konsta();
      QMessageBox::information(&d, "HORCOM", tr("TEILER auf 12 FESTGELEGT !"));
      list->update();
      return;
    }
    const auto r = static_cast<std::size_t>(me);
    if (me == 1) {
      // nasp& = 1, CLR voll!,ryt!
      aspect_settings_.divisors = 1;
      konsta_.voll = false;
      konsta_.ryt = false;
      result = WizardStep::kExit;
      d.accept();
      return;
    }
    if (me >= 2 && me <= 19) {
      if (!konsta_.selbst_cl_st) {
        // aspli|(me&) toggles the standard line, off also clears aspst
        if (konsta_.aspli_flag[r] > 0) {
          konsta_.aspli_flag[r] = 0;
          konsta_.aspst[r] = 0;
        } else {
          konsta_.aspli_flag[r] = me;
        }
      } else if (list->editing) {
        // his tt$ = RIGHT$(all$,lg& - 1) cut the row at the wrong end,
        // the port names the aspect after the last '='
        const QString row = tr(kLineRows[me]).trimmed();
        const QString tt = row.mid(row.lastIndexOf('=') + 1).trimmed();
        const int es = ChoiceDialog::ask(&d, tr("LINIEN - STIL für %1 FESTLEGEN !").arg(tt), {},
                                         {tr("DURCHGEZOGEN"), tr("GESTRICHELT"), tr("STRICHPUNKTIERT "), tr("KEINE LINIE")},
                                         0);
        if (es == 3) {
          konsta_.aspst[r] = 0;
          konsta_.aspli_col[r] = 0;
        } else if (es >= 0) {
          // aspst|(me&) = 0, 2 or 3
          konsta_.aspst[r] = es == 0 ? 0 : es + 1;
          const QColor start = konsta_.aspli_col[r] > 0 ? theme::from_colorref(konsta_.aspli_col[r]) : QColor(Qt::black);
          const QColor chosen =
              QColorDialog::getColor(start, &d, tr(" LINIEN - FARBEN für %1  WÄHLEN bzw. ÄNDERN !").arg(tt));
          // IF coll% = 0, coll% = 1 fast schwarz
          const int coll = chosen.isValid() ? theme::to_colorref(chosen) : std::max(konsta_.aspli_col[r], 0);
          konsta_.aspli_col[r] = std::max(coll, 1);
        }
      }
      persist_konsta();
      list->update();
      return;
    }
    switch (me) {
      case 20:
        //RR Eigenvorgabe benutzen
        konsta_.selbst_cl_st = true;
        list->editing = false;
        break;
      case 21:
        //RR Eigenvorgabe ändern
        konsta_.selbst_cl_st = true;
        list->editing = true;
        break;
      case 22:
        //RR Horcomstandard
        konsta_.selbst_cl_st = false;
        list->editing = false;
        break;
      default:
        // WAHLENDE = WEITER = SPEICHERN
        d.accept();
        return;
    }
    persist_konsta();
    list->update();
  };
  // CASE 13,32, GOTO avh_st1, Enter and Space end the whole avh
  list->leave = [&]() {
    result = WizardStep::kExit;
    d.accept();
  };
  BackKeys keys([&]() {
    result = WizardStep::kBack;
    d.reject();
  });
  d.installEventFilter(&keys);
  list->installEventFilter(&keys);
  list->setFocus();
  d.resize(kLineCanvasW + 120, kLineCanvasH + 40);
  // ESC ends the wizard
  if (d.exec() != QDialog::Accepted && result != WizardStep::kBack) {
    result = WizardStep::kExit;
  }
  return result;
}

}  // namespace horcom
