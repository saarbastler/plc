#ifndef _INCLUDE_PLC_2_SVG_H_
#define _INCLUDE_PLC_2_SVG_H_

#include <ostream>

#include "PlcAst.h"

class Plc2svg
{
public:

  Plc2svg(std::ostream& out) : out(out) {}

  void convert(const plc::Expression& expression);

private:

  unsigned convert(unsigned ypos, unsigned level, const plc::Expression& expression);

  unsigned maxLevel = 0;

  unsigned xstart = 10;

  std::ostream& out;
};

#endif // !_INCLUDE_PLC_2_SVG_H_

