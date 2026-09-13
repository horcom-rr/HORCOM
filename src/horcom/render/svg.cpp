// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/render/svg.hpp"

#include <cmath>
#include <cstdio>
#include <sstream>

namespace horcom {

namespace {

std::string hex(Rgb c) {
  char buf[8];
  std::snprintf(buf, sizeof(buf), "#%06X", c & 0xFFFFFF);
  return buf;
}

std::string num(double v) {
  char buf[32];
  std::snprintf(buf, sizeof(buf), "%.2f", v);
  return buf;
}

const char* dash(Primitive::Style s) {
  switch (s) {
    case Primitive::Style::kDashed: return " stroke-dasharray=\"6 4\"";
    case Primitive::Style::kDotted: return " stroke-dasharray=\"1 4\"";
    default: return "";
  }
}

std::string escape(const std::string& t) {
  std::string out;
  for (char ch : t) {
    switch (ch) {
      case '<': out += "&lt;"; break;
      case '>': out += "&gt;"; break;
      case '&': out += "&amp;"; break;
      default: out += ch; break;
    }
  }
  return out;
}

// a point on the wheel, SVG y axis equals the canvas y axis
std::string sector_path(const Primitive& p) {
  const double x1i = p.x1 + p.r1 * std::cos(-p.a1);
  const double y1i = p.y1 + p.r1 * std::sin(-p.a1);
  const double x1o = p.x1 + p.r2 * std::cos(-p.a1);
  const double y1o = p.y1 + p.r2 * std::sin(-p.a1);
  const double x2i = p.x1 + p.r1 * std::cos(-p.a2);
  const double y2i = p.y1 + p.r1 * std::sin(-p.a2);
  const double x2o = p.x1 + p.r2 * std::cos(-p.a2);
  const double y2o = p.y1 + p.r2 * std::sin(-p.a2);
  std::ostringstream d;
  d << "M " << num(x1i) << ' ' << num(y1i)
    << " L " << num(x1o) << ' ' << num(y1o)
    << " A " << num(p.r2) << ' ' << num(p.r2) << " 0 0 0 " << num(x2o) << ' ' << num(y2o)
    << " L " << num(x2i) << ' ' << num(y2i)
    << " A " << num(p.r1) << ' ' << num(p.r1) << " 0 0 1 " << num(x1i) << ' ' << num(y1i)
    << " Z";
  return d.str();
}

}  // namespace

std::string to_svg(const DisplayList& dl) {
  std::ostringstream s;
  s << "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 " << num(dl.width) << ' ' << num(dl.height)
    << "\" font-family=\"'Segoe UI Symbol', 'Noto Sans Symbols', sans-serif\">\n";
  s << "<rect width=\"" << num(dl.width) << "\" height=\"" << num(dl.height) << "\" fill=\"#FFFFFF\"/>\n";
  for (const Primitive& p : dl.items) {
    switch (p.kind) {
      case Primitive::Kind::kCircle:
        s << "<circle cx=\"" << num(p.x1) << "\" cy=\"" << num(p.y1) << "\" r=\"" << num(p.r1)
          << "\" fill=\"none\" stroke=\"" << hex(p.color) << "\" stroke-width=\"" << num(p.width) << "\"/>\n";
        break;
      case Primitive::Kind::kLine:
        s << "<line x1=\"" << num(p.x1) << "\" y1=\"" << num(p.y1) << "\" x2=\"" << num(p.x2) << "\" y2=\""
          << num(p.y2) << "\" stroke=\"" << hex(p.color) << "\" stroke-width=\"" << num(p.width) << "\"" << dash(p.style)
          << "/>\n";
        break;
      case Primitive::Kind::kSector:
        s << "<path d=\"" << sector_path(p) << "\" fill=\"" << hex(p.fill) << "\" fill-opacity=\"0.25\" stroke=\""
          << hex(p.color) << "\" stroke-width=\"0.5\"/>\n";
        break;
      case Primitive::Kind::kGlyph:
      case Primitive::Kind::kText:
        s << "<text x=\"" << num(p.x1) << "\" y=\"" << num(p.y1) << "\" font-size=\"" << num(p.size)
          << "\" text-anchor=\"middle\" dominant-baseline=\"middle\" fill=\"" << hex(p.color) << "\">"
          << escape(p.text) << "</text>\n";
        break;
      case Primitive::Kind::kDot:
        s << "<circle cx=\"" << num(p.x1) << "\" cy=\"" << num(p.y1) << "\" r=\"" << num(p.r1) << "\" fill=\""
          << hex(p.color) << "\"/>\n";
        break;
    }
  }
  s << "</svg>\n";
  return s.str();
}

}  // namespace horcom
