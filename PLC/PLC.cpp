#ifndef PARSER_TESTS

#include <iostream>
#include <fstream>

#include "PlcParser.h"
#include "plc2svg.h"

std::ostream& operator<<(std::ostream& out, ParserResult& parserResult)
{
  out << "ParserResult: ";
  switch (parserResult.type())
  {
  case ParserResult::Type::Empty:
    out << "Empty";
    break;
  case ParserResult::Type::Char:
    out << "Char: " << parserResult.text();
    break;
  case ParserResult::Type::Identifier:
    out << "Identifier: " << parserResult.text();
    break;
  case ParserResult::Type::Integer:
    out << "Int: " << parserResult.intValue();
    break;
  default:
    out << "Unknown Type" << static_cast<int>(parserResult.type());
  }

  return out;
}

int main()
{
  //std::ifstream in("C:\\HardwareProjekte\\Haus\\AussenBeleuchtung\\plc.txt");

  PlcAst plcAst;

  plcAst.addVariable(Variable("out", Variable::Type::Output, 0));

  plcAst.addVariable(Variable("a", Variable::Type::Input, 0));
  plcAst.addVariable(Variable("b", Variable::Type::Input, 1));
  plcAst.addVariable(Variable("c", Variable::Type::Input, 2));
  plcAst.addVariable(Variable("d", Variable::Type::Input, 3));
  plcAst.addVariable(Variable("e", Variable::Type::Input, 4));
  plcAst.addVariable(Variable("f", Variable::Type::Input, 5));
  plcAst.addVariable(Variable("g", Variable::Type::Input, 6));
  plcAst.addVariable(Variable("h", Variable::Type::Input, 7));

  try
  {
    //plcParse(in, plcAst);
    plcParse("out = (a & !b | c & d) & (!a & b | c & !d);", plcAst);

    const plc::Expression& equation = plcAst.equations().at("out");

    std::ofstream out("C:\\HardwareProjekte\\Haus\\out.svg");

    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>" << std::endl;
    out << "<svg xmlns:svg=\"http://www.w3.org/2000/svg\" xmlns=\"http://www.w3.org/2000/svg\" "
      "width=\"800\" height=\"800\">" << std::endl;
    out << "<style type=\"text/css\">" << std::endl
      << "<![CDATA[" << std::endl
      << "rect.box { stroke:#000; fill:none; }" << std::endl
      << "circle.invert { stroke:#000; fill:none; }" << std::endl
      << "circle.join { stroke:#000; fill:#000; }" << std::endl
      << "line.link { stroke:#000; stroke-width:1px; }" << std::endl
      << "]]>" << std::endl
      << "</style>" << std::endl;

    Plc2svg plc2svg(out);
    plc2svg.convert(equation);

    out << "</svg>" << std::endl;
  }
  catch (std::exception& ex)
  {
    std::cout << "Exception: " << ex.what() << std::endl;
  }
}

#endif
