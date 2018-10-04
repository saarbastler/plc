#ifndef PARSER_TESTS

#include <iostream>
#include <fstream>

#include "PlcParser.h"
#include "plc2svg.h"
#include "PlcCompiler.h"

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
  plcAst.addVariable(Variable("input", Variable::Type::Input, 4));
  /*plcAst.addVariable(Variable("f", Variable::Type::Input, 5));
  plcAst.addVariable(Variable("g", Variable::Type::Input, 6));
  plcAst.addVariable(Variable("h", Variable::Type::Input, 7));*/

  try
  {
    //plcParse(in, plcAst);
    plcParse("out = (a & !b | c & d) & input & (!a & b | c & !d);", plcAst);

    const plc::Expression& equation = plcAst.equations().at("out");

    std::ofstream out("C:\\HardwareProjekte\\Haus\\out.svg");

    Plc2svg plc2svg(plcAst, out);
    plc2svg.convert(equation);

    std::vector<plc::Operation> instructions;
    plc::compile(plcAst, equation, instructions);

    PlcSimulator plcSimulator(5, 0, 0, 0);

    for (unsigned bitvector = 0; bitvector < 32; bitvector++)
    {
      for (int i = 4; i >= 0; i--)
      {
        plcSimulator.io(PlcSimulator::IOType::Input, i) = bitvector & (1 << i);

        std::cout << (plcSimulator.io(PlcSimulator::IOType::Input, i) ? 1 : 0) << ' ';
      }

      std::cout << " = " << (plcSimulator.execute<5>(instructions) ? 1 : 0) << std::endl;
    }

    std::vector<uint8_t> avrplc;
    plc::translateAvr(instructions, avrplc);

    std::cout.width(2);
    std::cout.fill('0');
    for (uint8_t op : avrplc)
      std::cout << std::hex << unsigned(op) << ' ';

    std::cout << std::endl;
  }
  catch (std::exception& ex)
  {
    std::cout << "Exception: " << ex.what() << std::endl;
  }
}

#endif
