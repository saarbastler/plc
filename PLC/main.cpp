#if !defined(PARSER_TESTS) && !defined(LIBRARY)

#include <iostream>
#include <fstream>

#include "PlcParser.h"
#include "plc2svg.h"
#include "PlcCompiler.h"
#include "plc.h"

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
  std::ifstream in("C:\\HardwareProjekte\\Haus\\plcProgram\\plc.txt");

  PlcAst plcAst;
  try
  {
    plcParse(in, plcAst);

    const plc::Expression& equation = plcAst.equations().begin()->second;

    std::ofstream out("C:\\HardwareProjekte\\Haus\\out.svg");

    Plc2svg plc2svg(plcAst, out);
    plc2svg.convert(equation);

    std::vector<plc::Operation> instructions;
    plc::compile(plcAst, equation, "output", instructions);

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

  return 0;
}

int mainY()
{
  std::ifstream in("C:\\HardwareProjekte\\Haus\\AussenBeleuchtung\\plc.txt");

  PlcAst plcAst;
  try
  {
    plcParse(in, plcAst);

    std::vector<plc::Operation> instructions;
    plc::compile(plcAst, instructions);

    std::vector<uint8_t> avrplc;
    plc::translateAvr(instructions, avrplc);

    std::cout << "AVR Size: " << avrplc.size() << std::endl;
    std::cout.width(2);
    std::cout.fill('0');
    for (uint8_t op : avrplc)
      std::cout << std::hex << "0x" << unsigned(op) << ", ";

    std::cout << std::endl;
  }
  catch (std::exception& ex)
  {
    std::cout << "Exception: " << ex.what() << std::endl;
  }

  return 0;
}

#endif
