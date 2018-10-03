#ifndef _INCLUDE_PLC_2_SVG_H_
#define _INCLUDE_PLC_2_SVG_H_

#include <ostream>
#include <unordered_map>

#include "PlcAst.h"

class Plc2svg
{
public:

  Plc2svg(std::ostream& out) : out(out) {}

  void convert(const plc::Expression& expression);

private:

  unsigned convert(unsigned ypos, unsigned level, const plc::Expression& expression);
  void convertInput(const plc::Term& term, unsigned y, unsigned width);

  unsigned maxLevel = 0;
  unsigned crossingWidth = 0;
  unsigned crossingCount = 0;

  std::ostream& out;

  struct COORD
  {
    unsigned x;
    unsigned y;
  };

  std::unordered_map<std::string, COORD> inputPosition;
};

#endif // !_INCLUDE_PLC_2_SVG_H_

