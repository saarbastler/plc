#ifndef PARSER_TESTS

#include <iostream>
#include <fstream>

#include "PlcParser.h"

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
  std::ifstream in("C:\\HardwareProjekte\\Haus\\AussenBeleuchtung\\plc.txt");

  PlcAst plcAst;
  //plcAst.addVariable(Variable("a", Variable::Type::Input, 0));

  try
  {
    plcParse(in, plcAst);
  }
  catch (std::exception& ex)
  {
    std::cout << "Exception: " << ex.what() << std::endl;
  }
}

#endif
