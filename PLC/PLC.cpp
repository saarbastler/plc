#ifndef PARSER_TESTS

#include <iostream>
#include <fstream>

#include "PlcParser.h"

std::ostream& operator << (std::ostream& out, const plc::Term& term)
{
  out << "Term(";
  if (term.unary() == plc::Term::Unary::Not)
    out << " ! ";

  switch (term.type())
  {
  case plc::Term::Type::Empty:
    out << "Empty)";
    break;
  case plc::Term::Type::Expression:
    out << term.expression() << " )";
    break;
  case plc::Term::Type::Identifier:
    out << term.identifier() << " )";
    break;
  default:
    out << "Undefined Type: " << int(term.type()) << " )";
  }

  return out;
}

std::ostream& operator << (std::ostream& out, const std::unique_ptr<plc::Expression>& expression)
{
  out << "Expression(";
  if (expression)
  {
    std::string opText;

    for (auto& it : expression->terms())
    {
      if (opText.empty())
      {
        if (expression->op() != plc::Expression::Operator::None)
          switch (expression->op())
          {
          case plc::Expression::Operator::And:
            opText = " & ";
            break;
          case plc::Expression::Operator::Or:
            opText = " | ";
            break;
          default:
            opText = " undefined Operator: " + int(expression->op());
          }
      }
      else
      {
        out << opText;
      }

      out << it;
    }

    out << " )";
  }
  else
  {
    out << "Empty)";
  }

  return out;
}

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

  ParserInput<2> parserInput(in);
  Parser<2,2> parser(parserInput);
  PlcParser<2,2> plcParser(parser);

  try
  {
    plcParser.parse();
  }
  catch (std::exception& ex)
  {
    std::cout << "Exception: " << ex.what() << std::endl;
  }
}

#endif
