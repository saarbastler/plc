
#include "plc2svg.h"
#include "svgHelper.h"

constexpr const char *BOX = "box";
constexpr const char *VARIABLE = "variable";
constexpr const char *LINK = "link";
constexpr const char *INVERT = "invert";

constexpr const unsigned TEXT_HEIGHT = 20;
constexpr const unsigned TEXT_LENGTH = 100;
constexpr const unsigned GATE_WIDTH = 100;
constexpr const unsigned INVERT_RADIUS = 4;

unsigned Plc2svg::convert(unsigned ypos, unsigned level, const plc::Expression& expression)
{
  unsigned size = 0;
  unsigned index = 1;
  unsigned width = (maxLevel - level) * (TEXT_LENGTH + GATE_WIDTH) + TEXT_LENGTH;

  for (const plc::Term& term : expression.terms())
  {
    unsigned y = (index + ypos) * TEXT_HEIGHT;

    switch (term.type())
    {
    case plc::Term::Type::Identifier:
      out << svg::Text(xstart, y, term.identifier(), VARIABLE);
      if( term.unary() == plc::Term::Unary::None)
        out << svg::Line(xstart, y, xstart + width, y, LINK);
      else
        out << svg::Line(xstart, y, xstart + width - 2 * INVERT_RADIUS, y, LINK)
          << svg::Circle(xstart + width - INVERT_RADIUS, y, INVERT_RADIUS, INVERT);

      index++;
      size++;
      break;

    case plc::Term::Type::Expression:
      out << svg::Line(xstart + width - TEXT_LENGTH, y + TEXT_HEIGHT, xstart + width, y + TEXT_HEIGHT, LINK);
      {
        unsigned subSize= 2 + convert(index + ypos, level + 1, *term.expression());
        index += subSize;
        size += subSize;
      }
      break;

    //default:
    }
  }

  out << svg::Rect(xstart + width, ypos * TEXT_HEIGHT, GATE_WIDTH, (size + 1) * TEXT_HEIGHT, BOX)
    << svg::Text(xstart + width, (1+ypos) * TEXT_HEIGHT, expression.op() == plc::Expression::Operator::And ? "&" : ">=1");

  return size;
}

void Plc2svg::convert(const plc::Expression& expression)
{
  maxLevel = expression.countLevels();
  convert(2, 1, expression);
}
