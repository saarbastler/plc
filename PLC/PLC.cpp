#include "plc.h"

#include "PlcParser.h"
#include "plc2svg.h"

void plcParse(std::istream& in, PlcAst& plcAst)
{
  ParserInput<2> parserInput(in);
  Parser<2, 2> parser(parserInput);
  PlcParser<2, 2> plcParser(parser, plcAst);

  plcParser.parse();
}

void plcParse(const char *text, PlcAst& plcAst)
{
  std::istringstream in(text);

  plcParse(in, plcAst);
}

void convert2svg(const PlcAst& plcAst, const plc::Expression& expression, std::ostream& out)
{
  Plc2svg plc2svg(plcAst, out, {});

  plc2svg.convert(expression);
}