#include "plc.h"

#include "PlcExpressionParser.h"
#include "plc2svg.h"
#include <PlcCompiler.h>

void plcParse(std::istream& in, PlcAst& plcAst)
{
  ParserInput<2> parserInput(in);
  Parser<2, 2> parser(parserInput);
  PlcExpressionParser<2, 2> plcParser(parser, plcAst);

  plcParser.parse();
}

void plcParse(const char *text, PlcAst& plcAst)
{
  std::istringstream in(text);

  plcParse(in, plcAst);
}

void convert2svg(const PlcAst& plcAst, const plc::Expression& expression, const std::string& name
  , std::ostream& out, const std::initializer_list<SVGOption> options)
{
  Plc2svg plc2svg(plcAst, out, options);

  plc2svg.convert(expression, name);
}

void convert2svg(const PlcAst& plcAst, std::ostream& out, const std::initializer_list<SVGOption> options)
{
  Plc2svg plc2svg(plcAst, out, options);

  std::vector<std::string> names;
  for (auto it = plcAst.variableDescription().begin(); it != plcAst.variableDescription().end(); it++)
    if (plcAst.getVariable(it->first).category() == Var::Category::Output && it->second.expression().operator bool())
      names.emplace_back(it->first);

  plc2svg.convertMultiple(names);
}

void translate2Avr(const PlcAst& plcAst, std::vector<uint8_t>& avrplc)
{
  std::vector<plc::Operation> instructions;
  plc::compile(plcAst, instructions);

  avrplc.clear();
  plc::translateAvr(instructions, avrplc);
}
