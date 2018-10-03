
#include "plc2svg.h"
#include "svgHelper.h"

constexpr const char *BOX = "box";
constexpr const char *VARIABLE = "variable";
constexpr const char *LINK = "link";

constexpr const unsigned TEXT_HEIGHT = 20;
constexpr const unsigned TEXT_LENGTH = 100;
constexpr const unsigned GATE_WIDTH = 100;

unsigned Plc2svg::convert(unsigned ypos, unsigned level, const plc::Expression& expression)
{
  unsigned index = 1;
  unsigned width = (maxLevel - level) * (TEXT_LENGTH + GATE_WIDTH) + TEXT_LENGTH;

  for (const plc::Term& term : expression.terms())
  {
    unsigned y = (index + ypos) * TEXT_HEIGHT;

    switch (term.type())
    {
    case plc::Term::Type::Identifier:
      out << svg::Text(xstart, y, term.identifier(), VARIABLE) << svg::Line(xstart, y, xstart + width, y, LINK);
      index++;
      break;

    case plc::Term::Type::Expression:
      out << svg::Line(xstart + width - (TEXT_LENGTH + GATE_WIDTH), y, xstart + width, y, LINK);

      index = convert(index + 2, level + 1, *term.expression());
      break;

    //default:
    }
  }

  out << svg::Rect(xstart + width, ypos * TEXT_HEIGHT, GATE_WIDTH, (unsigned(expression.terms().size()) + 1) * TEXT_HEIGHT, BOX)
    << svg::Text(xstart + width, (1+ypos) * TEXT_HEIGHT, expression.op() == plc::Expression::Operator::And ? "&" : ">=1");

  return index + 1;
}

void Plc2svg::convert(const plc::Expression& expression)
{
  maxLevel = expression.countLevels();
  convert(2, 1, expression);
  /*unsigned x = 10;
  if (expression.isSimple())
  {
    unsigned y = 100 - TEXT_HEIGHT;

    out << svg::Rect(x + TEXT_LENGTH, y, GATE_WIDTH, (1 + int(expression.terms().size())) * TEXT_HEIGHT, BOX);

    for (auto& it : expression.terms())
    {
      y += TEXT_HEIGHT;

      out << svg::Text(x, y, it.identifier(), VARIABLE) << svg::Line(x, y, x + TEXT_LENGTH, y, LINK);
    }
  }*/
}
