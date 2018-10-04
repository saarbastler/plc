#ifndef _INCLUDE_PLC_2_SVG_H_
#define _INCLUDE_PLC_2_SVG_H_

#include <ostream>
#include <sstream>
#include <unordered_map>

#include "PlcAst.h"

class Plc2svg
{
public:

  Plc2svg(const PlcAst& plcAst, std::ostream& out) : plcAst(plcAst), out(out) {}

  void convert(const plc::Expression& expression);

private:

  unsigned convert(unsigned ypos, unsigned level, const plc::Expression& expression);
  void convertInput(const plc::Term& term, unsigned y, unsigned width);
  
  static char variableTypeIdentifier(Variable::Type type);
  const char *variableCssClass(const Variable& variable);
  const char *gateCssClass(const plc::Expression& expression);

  std::string tmpCssClass;

  void expressionJsEquation(const plc::Expression& expression);

  unsigned maxLevel = 0;
  unsigned crossingWidth = 0;
  unsigned crossingCount = 0;
  unsigned textWidth = 0;

  const PlcAst& plcAst;
  std::ostream& out;

  std::ostringstream svgOut;
  std::ostringstream jsOut;

  struct COORD
  {
    unsigned x;
    unsigned y;
  };

  std::unordered_map<std::string, COORD> inputPosition;
};

#endif // !_INCLUDE_PLC_2_SVG_H_

