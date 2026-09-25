// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "robert_input.hpp"

#include <cmath>

#include <QCoreApplication>
#include <QDialog>
#include <QRegularExpressionValidator>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QLocale>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include "horcom/chart/signs.hpp"
#include "horcom/core/constants.hpp"
#include "auto_advance.hpp"

namespace horcom {

namespace {

// the sign names of his zod_zeich_alph
constexpr const char* kSignNames[kSignCount] = {
    QT_TRANSLATE_NOOP("RobertInput", "WIDDER"),     QT_TRANSLATE_NOOP("RobertInput", "STIER"),
    QT_TRANSLATE_NOOP("RobertInput", "ZWILLING"),   QT_TRANSLATE_NOOP("RobertInput", "KREBS"),
    QT_TRANSLATE_NOOP("RobertInput", "LÖWE"),       QT_TRANSLATE_NOOP("RobertInput", "JUNGFRAU"),
    QT_TRANSLATE_NOOP("RobertInput", "WAAGE"),      QT_TRANSLATE_NOOP("RobertInput", "SKORPION"),
    QT_TRANSLATE_NOOP("RobertInput", "SCHÜTZE"),    QT_TRANSLATE_NOOP("RobertInput", "STEINBOCK"),
    QT_TRANSLATE_NOOP("RobertInput", "WASSERMANN"), QT_TRANSLATE_NOOP("RobertInput", "FISCHE")};

// the translation context of his input boxes, lupdate reads the
// RobertInput::tr calls
struct RobertInput {
  Q_DECLARE_TR_FUNCTIONS(RobertInput)
};

// his number fields read like VAL, an empty box is zero
double field_value(const QLineEdit* e) {
  QString t = e->text().trimmed();
  t.replace(',', '.');
  return t.isEmpty() ? 0.0 : QLocale::c().toDouble(t);
}

// his komma_pkt$ took a comma or a point before VAL. A QDoubleValidator
// follows the system locale and turned a typed 17.5 into 175 under a
// German Windows, the pattern takes both marks
QValidator* number_validator(QObject* parent, bool negative, int decimals) {
  const QString sign = negative ? QStringLiteral("-?") : QString();
  const QString fraction = decimals > 0 ? QString("([.,]\\d{0,%1})?").arg(decimals) : QString();
  return new QRegularExpressionValidator(QRegularExpression("^" + sign + "\\d*" + fraction + "$"), parent);
}

}  // namespace

QLineEdit* small_box(QWidget* parent, int chars, QValidator* validator) {
  auto* e = new QLineEdit(parent);
  e->setAlignment(Qt::AlignRight);
  e->setMaxLength(chars);
  e->setFixedWidth(28 + 10 * chars);
  e->setValidator(validator);
  return e;
}

QLineEdit* number_box(QWidget* parent, const QString& text, int chars, int max_value) {
  QLineEdit* e = small_box(parent, chars);
  if (max_value > 0) {
    e->setValidator(new QIntValidator(0, max_value, e));
  }
  e->setText(text);
  return e;
}

char hemisphere_letter(const QString& text, char first, char second, char fallback) {
  const QString t = text.trimmed().toUpper();
  if (t.startsWith(QChar(first))) {
    return first;
  }
  if (t.startsWith(QChar(second))) {
    return second;
  }
  return fallback;
}

// IF ch$ = "V" OR ch$ = "-" of zuo
bool before_christ_flag(const QString& text) {
  const QString t = text.trimmed().toUpper();
  return t.startsWith('V') || t.startsWith('-');
}

QString sign_name(int sign) {
  return RobertInput::tr(kSignNames[static_cast<std::size_t>(sign)]);
}

// the sign list of ze_pl_wa, a double click picks
std::optional<int> pick_sign(QWidget* parent) {
  QDialog d(parent);
  //RR Mit DOPPELKLICK AUSWÄHLEN !
  d.setWindowTitle(RobertInput::tr("Mit DOPPELKLICK AUSWÄHLEN !"));
  auto* v = new QVBoxLayout(&d);
  auto* list = new QListWidget(&d);
  for (int i = 0; i < kSignCount; ++i) {
    list->addItem(QString("%1 = %2").arg(i + 1, 3).arg(sign_name(i)));
  }
  list->setCurrentRow(0);
  v->addWidget(list);
  int picked = -1;
  QObject::connect(list, &QListWidget::itemActivated, &d, [&picked, list, &d](QListWidgetItem*) {
    picked = list->currentRow();
    d.accept();
  });
  d.resize(300, 420);
  if (d.exec() != QDialog::Accepted || picked < 0) {
    return std::nullopt;
  }
  return picked;
}

// ported from input_grmise_zod
std::optional<double> ask_zodiac_position(QWidget* parent, const QString& title) {
  for (;;) {
    const std::optional<int> sign = pick_sign(parent);
    if (!sign) {
      return std::nullopt;
    }
    QDialog d(parent);
    d.setWindowTitle(title);
    auto* grid = new QGridLayout(&d);
    grid->setContentsMargins(18, 14, 18, 14);
    grid->setHorizontalSpacing(8);
    grid->setVerticalSpacing(12);
    grid->addWidget(new QLabel(RobertInput::tr("ZEICHEN : %1").arg(RobertInput::tr(kSignNames[*sign])), &d), 0, 0, 1, 4);
    grid->addWidget(new QLabel(RobertInput::tr("GRAD-MINUTE-SEKUNDE  EINGEBEN :"), &d), 1, 0);
    auto* row = new QHBoxLayout();
    auto* deg = small_box(&d, 2, new QIntValidator(0, 29, &d));
    auto* min = small_box(&d, 2, new QIntValidator(0, 59, &d));
    auto* sec = small_box(&d, 2, new QIntValidator(0, 59, &d));
    row->addWidget(deg);
    row->addWidget(new QLabel(QStringLiteral("°"), &d));
    row->addWidget(min);
    row->addWidget(new QLabel(QStringLiteral("'"), &d));
    row->addWidget(sec);
    row->addWidget(new QLabel(QStringLiteral("''"), &d));
    row->addStretch(1);
    grid->addLayout(row, 1, 1, 1, 3);
    grid->addWidget(new QLabel(RobertInput::tr("oder"), &d), 2, 0, Qt::AlignHCenter);
    grid->addWidget(new QLabel(RobertInput::tr("DEZIMALGRAD EINGEBEN :"), &d), 3, 0);
    auto* dec = small_box(&d, 11, number_validator(&d, false, 6));
    grid->addWidget(dec, 3, 1);
    // ENTF, back to the sign list
    auto* back = new QPushButton(RobertInput::tr(" ENTF "), &d);
    auto* ok = new QPushButton(RobertInput::tr(" &OK "), &d);
    ok->setDefault(true);
    grid->addWidget(back, 3, 2);
    grid->addWidget(ok, 3, 3);
    bool again = false;
    QObject::connect(back, &QPushButton::clicked, &d, [&again, &d]() {
      again = true;
      d.reject();
    });
    QObject::connect(ok, &QPushButton::clicked, &d, &QDialog::accept);
    chain_fields({{deg, 2}, {min, 2}, {sec, 2}, {dec, 0}});
    deg->setFocus();
    if (d.exec() != QDialog::Accepted) {
      if (again) {
        continue;
      }
      return std::nullopt;
    }
    // IF d > 0 : w = d * pu, else sign, degree, minute and second
    const double decimal = field_value(dec);
    const double w = decimal > 0.0 ? decimal
                                   : *sign * kDegPerSign + field_value(deg) + field_value(min) / 60.0 +
                                         field_value(sec) / 3600.0;
    if (w > 360.0) {
      //RR WINKEL >360°
      QMessageBox::warning(parent, "HORCOM", RobertInput::tr("WINKEL >360° "));
      continue;
    }
    return w * kDegToRad;
  }
}

// ported from zeiteing
std::optional<double> ask_clock(QWidget* parent, const QString& title) {
  QDialog d(parent);
  d.setWindowTitle(title);
  auto* row = new QHBoxLayout(&d);
  row->setContentsMargins(18, 14, 18, 14);
  row->addWidget(new QLabel(RobertInput::tr("STUNDE-MINUTE-SEKUNDE :"), &d));
  auto* hh = small_box(&d, 2, new QIntValidator(0, 23, &d));
  auto* mm = small_box(&d, 2, new QIntValidator(0, 59, &d));
  auto* ss = small_box(&d, 2, new QIntValidator(0, 59, &d));
  row->addWidget(new QLabel("hh", &d));
  row->addWidget(hh);
  row->addWidget(new QLabel("mm", &d));
  row->addWidget(mm);
  row->addWidget(new QLabel("ss", &d));
  row->addWidget(ss);
  auto* ok = new QPushButton(RobertInput::tr("&OK"), &d);
  ok->setDefault(true);
  row->addWidget(ok);
  QObject::connect(ok, &QPushButton::clicked, &d, &QDialog::accept);
  chain_fields({{hh, 2}, {mm, 2}, {ss, 2}});
  hh->setFocus();
  if (d.exec() != QDialog::Accepted) {
    return std::nullopt;
  }
  return field_value(hh) + field_value(mm) / 60.0 + field_value(ss) / 3600.0;
}

// ported from a37dat
std::optional<CalendarDate> ask_date(QWidget* parent, const QString& title, const CalendarDate& start,
                                     const QString& time_label, const QStringList& notes) {
  QDialog d(parent);
  d.setWindowTitle(title);
  auto* outer = new QVBoxLayout(&d);
  outer->setContentsMargins(18, 14, 18, 14);
  for (const QString& n : notes) {
    auto* line = new QLabel(n, &d);
    line->setAlignment(Qt::AlignHCenter);
    outer->addWidget(line);
  }
  auto* grid = new QGridLayout();
  outer->addLayout(grid);
  grid->setHorizontalSpacing(6);
  grid->addWidget(new QLabel(RobertInput::tr(" WENN v.CHR. , 'V' EINGEBEN !"), &d), 0, 0);
  auto* bc = small_box(&d, 1, nullptr);
  grid->addWidget(bc, 0, 1);
  grid->addWidget(new QLabel(RobertInput::tr("DATUM :"), &d), 0, 2);
  auto* dd = small_box(&d, 2, new QIntValidator(1, 31, &d));
  auto* mm = small_box(&d, 2, new QIntValidator(1, 12, &d));
  auto* yy = small_box(&d, 5, new QIntValidator(0, 99999, &d));
  const bool before_christ = start.year <= 0;
  // his CLR ta,mo,ja before a37dat leaves the fields empty
  const bool blank = start.day == 0 && start.month == 0;
  bc->setText(before_christ && !blank ? "V" : QString());
  dd->setText(blank ? QString() : QString::number(start.day));
  mm->setText(blank ? QString() : QString::number(start.month));
  yy->setText(blank ? QString() : QString::number(before_christ ? 1 - start.year : start.year));
  grid->addWidget(new QLabel(RobertInput::tr("TT"), &d), 0, 3);
  grid->addWidget(dd, 0, 4);
  grid->addWidget(new QLabel("MM", &d), 0, 5);
  grid->addWidget(mm, 0, 6);
  grid->addWidget(new QLabel(RobertInput::tr("JJJJ"), &d), 0, 7);
  grid->addWidget(yy, 0, 8);
  QLineEdit* hh = nullptr;
  QLineEdit* mi = nullptr;
  QLineEdit* ss = nullptr;
  if (!time_label.isEmpty()) {
    grid->addWidget(new QLabel(time_label + " :", &d), 1, 0);
    hh = small_box(&d, 2, new QIntValidator(0, 23, &d));
    mi = small_box(&d, 2, new QIntValidator(0, 59, &d));
    ss = small_box(&d, 2, new QIntValidator(0, 59, &d));
    const int seconds = static_cast<int>((start.hour * 60.0 + start.minute) * 60.0 + 0.5);
    hh->setText(QString::number(seconds / 3600));
    mi->setText(QString::number((seconds / 60) % 60));
    ss->setText(QString::number(seconds % 60));
    grid->addWidget(new QLabel("hh", &d), 1, 3);
    grid->addWidget(hh, 1, 4);
    grid->addWidget(new QLabel("mm", &d), 1, 5);
    grid->addWidget(mi, 1, 6);
    grid->addWidget(new QLabel("ss", &d), 1, 7);
    grid->addWidget(ss, 1, 8);
  }
  auto* ok = new QPushButton(RobertInput::tr("&OK"), &d);
  ok->setDefault(true);
  grid->addWidget(ok, time_label.isEmpty() ? 0 : 1, 9);
  QObject::connect(ok, &QPushButton::clicked, &d, &QDialog::accept);
  // eingl| of a37dat, the year completes with four digits
  if (hh != nullptr) {
    chain_fields({{dd, 2}, {mm, 2}, {yy, 4}, {hh, 2}, {mi, 2}, {ss, 2}});
  } else {
    chain_fields({{dd, 2}, {mm, 2}, {yy, 4}});
  }
  dd->setFocus();
  for (;;) {
    if (d.exec() != QDialog::Accepted) {
      return std::nullopt;
    }
    const QString flag = bc->text().trimmed().toUpper();
    const int day = static_cast<int>(field_value(dd));
    const int month = static_cast<int>(field_value(mm));
    // IF VAL(et$(2)) > 31 OR VAL(et$(3)) > 12 ...
    if (day <= 0 || day > 31 || month <= 0 || month > 12 || !(flag.isEmpty() || flag == "V" || flag == "-")) {
      QMessageBox::warning(parent, "HORCOM", RobertInput::tr("Datum INKORREKT !"));
      continue;
    }
    CalendarDate out;
    out.day = day;
    out.month = month;
    const int year = static_cast<int>(field_value(yy));
    // IF ch$ = "V" OR ch$ = "-", the astronomical count
    out.year = before_christ_flag(flag) ? 1 - year : year;
    if (hh != nullptr) {
      out.hour = field_value(hh);
      out.minute = field_value(mi) + field_value(ss) / 60.0;
    }
    return out;
  }
}

namespace {

// his inputbox keeps its 420 and grows with a longer prompt or note,
// the layout minimum rules so no line is ever cut
void fit_box(QVBoxLayout* v) {
  constexpr int kBoxWidth = 420;
  const QMargins m = v->contentsMargins();
  v->addStrut(kBoxWidth - m.left() - m.right());
  v->setSizeConstraint(QLayout::SetMinimumSize);
}

}  // namespace

// ported from inputbox$
std::optional<double> ask_number(QWidget* parent, const QString& title, const QString& prompt, double minimum,
                                 double maximum, double start, int decimals, const QStringList& notes) {
  QDialog d(parent);
  d.setWindowTitle(title);
  auto* v = new QVBoxLayout(&d);
  v->setContentsMargins(18, 14, 18, 14);
  auto* label = new QLabel(prompt, &d);
  label->setWordWrap(true);
  v->addWidget(label);
  auto* row = new QHBoxLayout();
  auto* edit = new QLineEdit(std::isnan(start) ? QString() : QString::number(start, 'f', decimals), &d);
  if (decimals == 0) {
    edit->setValidator(new QIntValidator(static_cast<int>(minimum), static_cast<int>(maximum), &d));
  } else {
    edit->setValidator(number_validator(&d, minimum < 0.0, decimals));
  }
  edit->selectAll();
  row->addWidget(edit, 1);
  auto* ok = new QPushButton(RobertInput::tr("&OK"), &d);
  ok->setDefault(true);
  row->addWidget(ok);
  v->addLayout(row);
  for (const QString& n : notes) {
    auto* line = new QLabel(n, &d);
    line->setAlignment(Qt::AlignHCenter);
    v->addWidget(line);
  }
  QObject::connect(ok, &QPushButton::clicked, &d, &QDialog::accept);
  fit_box(v);
  for (;;) {
    if (d.exec() != QDialog::Accepted) {
      return std::nullopt;
    }
    const double value = field_value(edit);
    if (value >= minimum && value <= maximum) {
      return value;
    }
  }
}

// ported from inputbox$ with eas$
std::optional<QString> ask_text(QWidget* parent, const QString& title, const QString& prompt, const QString& start,
                                int max_len) {
  QDialog d(parent);
  d.setWindowTitle(title);
  auto* v = new QVBoxLayout(&d);
  v->setContentsMargins(18, 14, 18, 14);
  auto* label = new QLabel(prompt, &d);
  label->setWordWrap(true);
  v->addWidget(label);
  auto* row = new QHBoxLayout();
  auto* edit = new QLineEdit(start, &d);
  if (max_len > 0) {
    edit->setMaxLength(max_len);
  }
  edit->selectAll();
  row->addWidget(edit, 1);
  auto* ok = new QPushButton(RobertInput::tr("&OK"), &d);
  ok->setDefault(true);
  row->addWidget(ok);
  v->addLayout(row);
  QObject::connect(ok, &QPushButton::clicked, &d, &QDialog::accept);
  fit_box(v);
  if (d.exec() != QDialog::Accepted) {
    return std::nullopt;
  }
  return max_len > 0 ? edit->text().left(max_len) : edit->text();
}

// ported from ausw_obj_m with the NICHTS GEWÄHLT ! guard of plan_wahl
std::optional<std::vector<int>> ask_objects(QWidget* parent, const std::vector<int>& objects,
                                            const QStringList& names) {
  QDialog d(parent);
  //RR OBJEKTE AUSWÄHLEN !
  d.setWindowTitle(RobertInput::tr("OBJEKTE AUSWÄHLEN !"));
  auto* v = new QVBoxLayout(&d);
  v->setContentsMargins(18, 14, 18, 14);
  auto* list = new QListWidget(&d);
  for (int i = 0; i < names.size(); ++i) {
    auto* item = new QListWidgetItem(names[i], list);
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Unchecked);
  }
  v->addWidget(list, 1);
  auto* ok = new QPushButton(RobertInput::tr("&OK"), &d);
  ok->setDefault(true);
  v->addWidget(ok);
  QObject::connect(ok, &QPushButton::clicked, &d, &QDialog::accept);
  d.resize(320, 460);
  for (;;) {
    if (d.exec() != QDialog::Accepted) {
      return std::nullopt;
    }
    std::vector<int> chosen;
    for (int i = 0; i < list->count() && i < static_cast<int>(objects.size()); ++i) {
      if (list->item(i)->checkState() == Qt::Checked) {
        chosen.push_back(objects[static_cast<std::size_t>(i)]);
      }
    }
    if (!chosen.empty()) {
      return chosen;
    }
    //RR NICHTS GEWÄHLT !
    QMessageBox::information(parent, "HORCOM", RobertInput::tr("NICHTS GEWÄHLT !"));
  }
}
// ported from ausw_pl_hs1, a double click or ENTER takes the row
std::optional<int> ask_object(QWidget* parent, const std::vector<std::pair<int, QString>>& rows,
                              const QStringList& notes, const QString& title) {
  QDialog d(parent);
  //RR EIN OBJEKT AUSWÄHLEN !
  d.setWindowTitle(title.isEmpty() ? RobertInput::tr("EIN OBJEKT AUSWÄHLEN !") : title);
  auto* v = new QVBoxLayout(&d);
  v->setContentsMargins(18, 14, 18, 14);
  QFont note_font(QStringLiteral("Courier New"));
  note_font.setStyleHint(QFont::Monospace);
  for (const QString& n : notes) {
    auto* line = new QLabel(n, &d);
    line->setFont(note_font);
    v->addWidget(line);
  }
  auto* list = new QListWidget(&d);
  QFont mono(QStringLiteral("Courier New"));
  mono.setStyleHint(QFont::Monospace);
  list->setFont(mono);
  for (const auto& [slot, caption] : rows) {
    // s$ = "----------------"
    auto* item = new QListWidgetItem(slot < 0 ? QStringLiteral("----------------") : caption, list);
    item->setData(Qt::UserRole, slot);
    if (slot < 0) {
      item->setFlags(Qt::NoItemFlags);
    }
  }
  v->addWidget(list, 1);
  auto* ok = new QPushButton(RobertInput::tr("&OK"), &d);
  ok->setDefault(true);
  v->addWidget(ok);
  QObject::connect(ok, &QPushButton::clicked, &d, &QDialog::accept);
  QObject::connect(list, &QListWidget::itemActivated, &d, &QDialog::accept);
  d.resize(520, 520);
  for (;;) {
    if (d.exec() != QDialog::Accepted) {
      return std::nullopt;
    }
    const QListWidgetItem* item = list->currentItem();
    if (item != nullptr && item->data(Qt::UserRole).toInt() >= 0) {
      return item->data(Qt::UserRole).toInt();
    }
  }
}

// ported from numw1, the row of STR$(i&,2) buttons
std::optional<int> ask_digit(QWidget* parent, const QString& prompt, int from, int to) {
  QDialog d(parent);
  //RR ZIFFERN-EINGABE
  d.setWindowTitle(RobertInput::tr("ZIFFERN-EINGABE"));
  auto* v = new QVBoxLayout(&d);
  v->setContentsMargins(18, 14, 18, 14);
  auto* label = new QLabel(prompt, &d);
  label->setAlignment(Qt::AlignHCenter);
  v->addWidget(label);
  auto* row = new QHBoxLayout();
  row->setSpacing(2);
  int chosen = from;
  for (int i = from; i <= to; ++i) {
    auto* b = new QPushButton(QString::asprintf("%2d", i), &d);
    b->setFixedWidth(34);
    // defaui&(diha&) = 101 + nu0&, the first value takes ENTER
    if (i == from) {
      b->setDefault(true);
    }
    QObject::connect(b, &QPushButton::clicked, &d, [&chosen, &d, i]() {
      chosen = i;
      d.accept();
    });
    row->addWidget(b);
  }
  v->addLayout(row);
  if (d.exec() != QDialog::Accepted) {
    return std::nullopt;
  }
  return chosen;
}

// ported from labr2, the E W and N S letters first, both letters must
// hold. His test NOT(E/W OR N/S) let one wrong letter through
std::optional<std::pair<double, double>> ask_geo_coordinates(QWidget* parent, const QStringList& notes) {
  QDialog d(parent);
  //RR Eingabe  GEOGRAPHISCHER  Koordinaten
  d.setWindowTitle(RobertInput::tr("Eingabe  GEOGRAPHISCHER  Koordinaten"));
  auto* outer = new QVBoxLayout(&d);
  outer->setContentsMargins(18, 14, 18, 14);
  for (const QString& n : notes) {
    auto* line = new QLabel(n, &d);
    line->setAlignment(Qt::AlignHCenter);
    outer->addWidget(line);
  }
  auto* grid = new QGridLayout();
  outer->addLayout(grid);
  const auto row = [&](int r, const QString& caption, const QString& letter) {
    grid->addWidget(new QLabel(caption, &d), r, 0);
    auto* l = small_box(&d, 1, nullptr);
    l->setText(letter);
    grid->addWidget(l, r, 1);
    std::array<QLineEdit*, 4> f{l, small_box(&d, 3, new QIntValidator(0, 180, &d)),
                                small_box(&d, 2, new QIntValidator(0, 59, &d)),
                                small_box(&d, 2, new QIntValidator(0, 59, &d))};
    grid->addWidget(new QLabel(QStringLiteral("\xC2\xB0"), &d), r, 2);
    grid->addWidget(f[1], r, 3);
    grid->addWidget(new QLabel(QStringLiteral("'"), &d), r, 4);
    grid->addWidget(f[2], r, 5);
    grid->addWidget(new QLabel(QStringLiteral("''"), &d), r, 6);
    grid->addWidget(f[3], r, 7);
    return f;
  };
  // et$(1) = "E", et$(5) = "N"
  const auto lon = row(0, RobertInput::tr("GEOGRAPHISCHE Länge  :"), QStringLiteral("E"));
  const auto lat = row(1, RobertInput::tr("GEOGRAPHISCHE Breite :"), QStringLiteral("N"));
  auto* ok = new QPushButton(RobertInput::tr("&OK"), &d);
  ok->setDefault(true);
  grid->addWidget(ok, 2, 7);
  QObject::connect(ok, &QPushButton::clicked, &d, &QDialog::accept);
  // eingl|(diha&,i&) 1 3 2 2 1 2 2 2
  chain_fields({{lon[0], 1}, {lon[1], 3}, {lon[2], 2}, {lon[3], 2}, {lat[0], 1}, {lat[1], 2}, {lat[2], 2}, {lat[3], 2}});
  lon[0]->setFocus();
  for (;;) {
    if (d.exec() != QDialog::Accepted) {
      return std::nullopt;
    }
    const QString ew = lon[0]->text().trimmed().toUpper();
    const QString ns = lat[0]->text().trimmed().toUpper();
    if (!((ew == "W" || ew == "E") && (ns == "N" || ns == "S"))) {
      continue;
    }
    double gl = field_value(lon[1]) + field_value(lon[2]) / 60.0 + field_value(lon[3]) / 3600.0;
    double gg = field_value(lat[1]) + field_value(lat[2]) / 60.0 + field_value(lat[3]) / 3600.0;
    if (ew == "W") {
      gl = -gl;
    }
    if (ns == "S") {
      gg = -gg;
    }
    return std::make_pair(gl, gg);
  }
}

// ported from input_grmise_ekl_aeq, degree minute second on the left or
// decimal values on the right, a decimal value wins
std::optional<std::pair<double, double>> ask_sky_coordinates(QWidget* parent, bool equatorial, const QString& title) {
  QDialog d(parent);
  d.setWindowTitle(title);
  auto* outer = new QVBoxLayout(&d);
  outer->setContentsMargins(18, 14, 18, 14);
  //RR Links GRAD-MINUTE-SEKUNDE eingeben,Rechts DEZIMALWERTE
  outer->addWidget(new QLabel(RobertInput::tr("Links GRAD-MINUTE-SEKUNDE eingeben,Rechts DEZIMALWERTE"), &d));
  auto* grid = new QGridLayout();
  outer->addLayout(grid);
  const auto row = [&](int r, const QString& caption, QLineEdit* letter, int deg_chars) {
    grid->addWidget(new QLabel(caption, &d), r, 0);
    if (letter != nullptr) {
      grid->addWidget(letter, r, 1);
    }
    std::array<QLineEdit*, 4> f{small_box(&d, deg_chars, nullptr), small_box(&d, 2, nullptr), small_box(&d, 2, nullptr),
                                new QLineEdit(&d)};
    f[3]->setFixedWidth(120);
    grid->addWidget(f[0], r, 2);
    grid->addWidget(new QLabel(QStringLiteral("\xC2\xB0"), &d), r, 3);
    grid->addWidget(f[1], r, 4);
    grid->addWidget(new QLabel(QStringLiteral("'"), &d), r, 5);
    grid->addWidget(f[2], r, 6);
    grid->addWidget(new QLabel(QStringLiteral("''"), &d), r, 7);
    //RR DEZIMAL:
    grid->addWidget(new QLabel(RobertInput::tr("DEZIMAL:"), &d), r, 8);
    grid->addWidget(f[3], r, 9);
    return f;
  };
  // REKTASZENSION : / DEKLINATION   : or EKLIPT. Länge : / EKLIPT. Breite:
  const auto ll = row(0, equatorial ? RobertInput::tr("REKTASZENSION :") : RobertInput::tr("EKLIPT. Länge :"), nullptr, 3);
  auto* ns_box = small_box(&d, 1, nullptr);
  // et$(5) = "N"
  ns_box->setText(QStringLiteral("N"));
  const auto bb = row(1, equatorial ? RobertInput::tr("DEKLINATION   :") : RobertInput::tr("EKLIPT. Breite:"), ns_box, 2);
  auto* ok = new QPushButton(RobertInput::tr(" &OK "), &d);
  ok->setDefault(true);
  grid->addWidget(ok, 2, 9);
  QObject::connect(ok, &QPushButton::clicked, &d, &QDialog::accept);
  // eingl|(diha&,i&), 3 2 2 for the first row, 1 2 2 2 for the second
  chain_fields({{ll[0], 3}, {ll[1], 2}, {ll[2], 2}, {ns_box, 1}, {bb[0], 2}, {bb[1], 2}, {bb[2], 2}, {ll[3], 0}, {bb[3], 0}});
  ll[0]->setFocus();
  for (;;) {
    if (d.exec() != QDialog::Accepted) {
      return std::nullopt;
    }
    const QString letter = ns_box->text().trimmed().toUpper();
    //RR 'N' oder 'S'
    if (!(letter == "N" || letter == "S")) {
      QMessageBox::information(&d, "HORCOM", RobertInput::tr("'N' oder 'S' "));
      continue;
    }
    double lon = std::abs(field_value(ll[0])) + std::abs(field_value(ll[1])) / 60.0 + std::abs(field_value(ll[2])) / 3600.0;
    if (!ll[3]->text().trimmed().isEmpty()) {
      lon = std::abs(field_value(ll[3]));
    }
    // his bb = g + m / 60 + s / 3600 took a negative degree against
    // positive minutes, the port signs the whole value
    double lat = std::abs(field_value(bb[0])) + std::abs(field_value(bb[1])) / 60.0 + std::abs(field_value(bb[2])) / 3600.0;
    if (letter == "S" || field_value(bb[0]) < 0.0) {
      lat = -lat;
    }
    if (!bb[3]->text().trimmed().isEmpty()) {
      lat = field_value(bb[3]);
    }
    // his WINKEL >180° warning on an ecliptic longitude had its retry
    // commented out, longitudes run to 360, the port drops the box
    //RR WINKEL >360°
    if (equatorial && lon > kDegPerCircle) {
      QMessageBox::information(&d, "HORCOM", RobertInput::tr("WINKEL >360° "));
      continue;
    }
    //RR WINKEL >90°
    if (std::abs(lat) > kDegPerCircle / 4.0) {
      QMessageBox::information(&d, "HORCOM", RobertInput::tr("WINKEL >90° "));
      continue;
    }
    return std::make_pair(lon, lat);
  }
}

// ported from the ENTER of rechne1. His decimal to degree split took the
// absolute value and dropped the sign, the way back added positive
// minutes to a negative degree. The port carries the sign both ways and
// the rounded seconds carry into the minutes
std::array<QString, 4> angle_enter(const std::array<QString, 4>& fields) {
  const auto val = [](QString t) {
    // komma_pkt$
    t = t.trimmed().replace(',', '.');
    return t.isEmpty() ? 0.0 : QLocale::c().toDouble(t);
  };
  std::array<QString, 4> out = fields;
  // IF VAL(et$(1)) = 0 && (et$(2) <> "" OR et$(3) <> "" OR et$(4) <> "")
  if (val(fields[0]) == 0.0 && !(fields[1].isEmpty() && fields[2].isEmpty() && fields[3].isEmpty())) {
    const bool negative = fields[1].trimmed().startsWith('-');
    const double v = std::abs(val(fields[1])) + std::abs(val(fields[2])) / 60.0 + std::abs(val(fields[3])) / 3600.0;
    out[0] = QString::asprintf("%13.6f", negative ? -v : v);
    return out;
  }
  const double v = val(fields[0]);
  // STR$(60 * FRAC(FRAC(v) * 60),5,2), hundredths of a second
  const long long hundredths = std::llround(std::abs(v) * 360000.0);
  const long long deg = hundredths / 360000;
  const long long min = (hundredths / 6000) % 60;
  const double sec = static_cast<double>(hundredths % 6000) / 100.0;
  out[1] = (v < 0.0 ? QString("-%1").arg(deg) : QString::number(deg)).rightJustified(3);
  out[2] = QString::asprintf("%3lld", min);
  out[3] = QString::asprintf("%5.2f", sec);
  return out;
}

// ported from rechne1, the box stays until QUIT or ESC, ENTER converts
// whichever side holds a value
void angle_converter(QWidget* parent) {
  QDialog d(parent);
  //RR WINKEL(ZEIT)-UMRECHNUNG  ERGEBNIS :'ENTER'
  d.setWindowTitle(RobertInput::tr("WINKEL(ZEIT)-UMRECHNUNG  ERGEBNIS :'ENTER'"));
  auto* grid = new QGridLayout(&d);
  grid->setContentsMargins(18, 14, 18, 14);
  //RR DEZIMAL-GRAD ( h ) => GRAD ( h ) -MIN-SEK
  grid->addWidget(new QLabel(RobertInput::tr("DEZIMAL-GRAD ( h ) => GRAD ( h ) -MIN-SEK"), &d), 0, 0, 1, 6,
                  Qt::AlignHCenter);
  std::array<QLineEdit*, 4> f{new QLineEdit(&d), new QLineEdit(&d), new QLineEdit(&d), new QLineEdit(&d)};
  grid->addWidget(new QLabel(RobertInput::tr("DEZIMAL-GRAD ( h )"), &d), 1, 0, 1, 2, Qt::AlignRight);
  grid->addWidget(f[0], 1, 2, 1, 2);
  grid->addWidget(new QLabel(RobertInput::tr("GRAD ( h )"), &d), 2, 0);
  grid->addWidget(f[1], 2, 1);
  grid->addWidget(new QLabel(RobertInput::tr("MIN"), &d), 2, 2);
  grid->addWidget(f[2], 2, 3);
  grid->addWidget(new QLabel(RobertInput::tr("SEK"), &d), 2, 4);
  grid->addWidget(f[3], 2, 5);
  auto* clear = new QPushButton(RobertInput::tr("CLEAR"), &d);
  auto* quit = new QPushButton(RobertInput::tr("QUIT"), &d);
  grid->addWidget(clear, 3, 0);
  grid->addWidget(quit, 3, 5);
  for (QLineEdit* e : f) {
    e->setAlignment(Qt::AlignRight);
    QObject::connect(e, &QLineEdit::returnPressed, &d, [f]() {
      const std::array<QString, 4> out = angle_enter({f[0]->text(), f[1]->text(), f[2]->text(), f[3]->text()});
      for (std::size_t i = 0; i < f.size(); ++i) {
        f[i]->setText(out[i]);
      }
    });
  }
  QObject::connect(clear, &QPushButton::clicked, &d, [f]() {
    for (QLineEdit* e : f) {
      e->clear();
    }
    f[0]->setFocus();
  });
  QObject::connect(quit, &QPushButton::clicked, &d, &QDialog::reject);
  // ENTER converts in the fields, never closes the box
  clear->setAutoDefault(false);
  quit->setAutoDefault(false);
  f[0]->setFocus();
  d.exec();
}

}  // namespace horcom
