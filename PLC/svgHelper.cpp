#include "svgHelper.h"

namespace svg
{
  const char X[] = "x";
  const char Y[] = "y";
  const char X1[] = "x1";
  const char Y1[] = "y1";
  const char X2[] = "x2";
  const char Y2[] = "y2";
  const char CX[] = "cx";
  const char CY[] = "cy";
  const char R[] = "r";

  const char WIDTH[] = "width";
  const char HEIGHT[] = "height";
  const char SVG_CLASS[] = "class";
  const char ID[] = "id";

  const char RECT[] = "rect";
  const char LINE[] = "line";
  const char TEXT[] = "text";
  const char CIRCLE[] = "circle";
}

std::ostream& operator << (std::ostream& out, const svg::Rect& elem)
{
  elem.print(out);

  return out;
}

std::ostream& operator << (std::ostream& out, const svg::Text& elem)
{
  elem.print(out);

  return out;
}

std::ostream& operator << (std::ostream& out, const svg::Line& elem)
{
  elem.print(out);

  return out;
}

std::ostream& operator << (std::ostream& out, const svg::Circle& elem)
{
  elem.print(out);

  return out;
}