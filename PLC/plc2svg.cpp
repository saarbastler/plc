
#include "plc2svg.h"
#include "svgHelper.h"

constexpr const char *BOX = "box";
constexpr const char *VARIABLE = "variable";
constexpr const char *LINK = "link";
constexpr const char *INVERT = "invert";
constexpr const char *JOIN = "join";

constexpr const unsigned XSTART = 10;
constexpr const unsigned TEXT_HEIGHT = 20;
constexpr const unsigned LINE_LENGTH = 50;
constexpr const unsigned GATE_WIDTH = 75;
constexpr const unsigned INVERT_RADIUS = 4;
constexpr const unsigned CROSSING_WIDTH = 10;
constexpr const unsigned JOIN_RADIUS = 2;

void Plc2svg::convertInput(const plc::Term& term, unsigned y, unsigned width)
{
  unsigned x = XSTART;
  auto input = inputPosition.find(term.identifier());
  if (input == inputPosition.end())
  {
    crossingCount++;
    inputPosition.emplace(term.identifier(), COORD{ XSTART + crossingCount * CROSSING_WIDTH, y });

    out << svg::Text(x, y, term.identifier(), VARIABLE);
  }
  else
  {
    x = input->second.x;
    out << svg::Circle(x, input->second.y, JOIN_RADIUS, JOIN)
      << svg::Line(x, input->second.y, x, y, LINK);
  }

  if (term.unary() == plc::Term::Unary::None)
    out << svg::Line(x, y, crossingWidth + width, y, LINK);
  else
    out << svg::Line(x, y, crossingWidth + width - 2 * INVERT_RADIUS, y, LINK)
    << svg::Circle(crossingWidth + width - INVERT_RADIUS, y, INVERT_RADIUS, INVERT);
}

unsigned Plc2svg::convert(unsigned ypos, unsigned level, const plc::Expression& expression)
{
  unsigned size = 0;
  unsigned index = 1;
  unsigned width = (maxLevel - level) * (LINE_LENGTH + GATE_WIDTH) + LINE_LENGTH;

  for (const plc::Term& term : expression.terms())
  {
    unsigned y = (index + ypos) * TEXT_HEIGHT;

    switch (term.type())
    {
    case plc::Term::Type::Identifier:
      convertInput(term, y, width);

      index++;
      size++;
      break;

    case plc::Term::Type::Expression:
      out << svg::Line(crossingWidth + width - LINE_LENGTH, y + TEXT_HEIGHT, crossingWidth + width, y + TEXT_HEIGHT, LINK);
      {
        unsigned subSize= 2 + convert(index + ypos, level + 1, *term.expression());
        index += subSize;
        size += subSize;
      }
      break;

    //default:
    }
  }

  out << svg::Rect(crossingWidth + width, ypos * TEXT_HEIGHT, GATE_WIDTH, (size + 1) * TEXT_HEIGHT, BOX)
    << svg::Text(crossingWidth + width, (1+ypos) * TEXT_HEIGHT, expression.op() == plc::Expression::Operator::And ? "&" : ">=1");

  return size;
}

void Plc2svg::convert(const plc::Expression& expression)
{
  std::unordered_map<std::string, unsigned> inputs;
  expression.countInputs(inputs);

  unsigned crossings = 0;
  for (auto it = inputs.begin(); it != inputs.end(); it++)
    if (it->second > 1)
      crossings++;

  crossingWidth = crossings * CROSSING_WIDTH;
  maxLevel = expression.countLevels();

  convert(2, 1, expression);
}
