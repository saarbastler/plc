#ifndef _INCLUDE_PLC_AWL_PARSER_H_
#define _INCLUDE_PLC_AWL_PARSER_H_

#include <array>

#include <boost/algorithm/string.hpp>

#include "Parser.h"
#include "PlcParser.h"
#include "AwlExpression.h"
#include "AwlAst.h"

template<unsigned CHAR_STACKSIZE, unsigned PARSER_STACKSIZE>
class PlcAWLParser : PlcParser<CHAR_STACKSIZE, PARSER_STACKSIZE, awl::Expression, AwlAst>
{
public:
  using VariableType = Variable<awl::Expression>;
  using Instruction_t = awl::Expression::Instruction;
  using Super = PlcParser<CHAR_STACKSIZE, PARSER_STACKSIZE, awl::Expression, AwlAst>;

  PlcAWLParser(Parser<CHAR_STACKSIZE, PARSER_STACKSIZE>& parser, AwlAst& awlAst) : Super(parser, awlAst) {}

  void parse()
  {
    Super::parseVariables([this](const std::string& identifier)
    {
      parseInstruction(identifier);
    });
  }

protected:

  static int text2instruction(const std::string& name)
  {
    static const std::array<const char *, 7> instructionText{ "read", "write", "set", "reset", "and", "or", "not" };

    for (auto it = instructionText.begin(); it != instructionText.end(); it++)
      if (boost::iequals(name, *it))
        return int(it - instructionText.begin());

    return -1;
  }

  void parseInstruction(const std::string& name)
  {
    int instr = text2instruction(name);
    if (instr < 0)
      throw ParserException(Super::parser.getLineNo(), "undefined instruction: %s", name.c_str());

    Instruction_t instruction = static_cast<Instruction_t>(instr);
    ParserResult parserResult;

    bool inversion = false;
    if (Super::parser.next(parserResult).is(ParserResult::Type::Char, '!'))
    {
      inversion = true;
      Super::parser.next(parserResult);
    }

    if (parserResult.type() == ParserResult::Type::Identifier)
    {
      VariableType& variable= Super::ast.getVariable(parserResult.text());

      if (!awl::Expression::argumentOptional(instruction))
        throw ParserException(Super::parser.getLineNo(), "No Argument allowed in instruction %s", name);
      if (!awl::Expression::allowedInstructionArgumentCategory(instruction, variable.category()))
        throw ParserException(Super::parser.getLineNo(), "Category %s not allowed in instruction %s", Var::categoryText(variable.category()), name.c_str());

      Super::ast.addExpression(instruction, &variable, inversion);
      Super::parser.next(parserResult);
    }
    else
    {
      if (!awl::Expression::argumentMandatory(instruction))
        throw ParserException(Super::parser.getLineNo(), "Missing Argument for Instruction %s", name);

      if (inversion)
        throw ParserException(Super::parser.getLineNo(), "Inversion without argument not allowed");

      Super::ast.addExpression(instruction, nullptr, false);
    }

    if(!parserResult.is(ParserResult::Type::Char, ';'))
      throw ParserException(Super::parser.getLineNo(), "missing ';' afer Instruction");
  }
};
#endif // _INCLUDE_PLC_AWL_PARSER_H_

