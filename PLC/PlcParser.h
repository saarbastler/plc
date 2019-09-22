#ifndef _INCLUDE_PLC_PARSER_H_
#define _INCLUDE_PLC_PARSER_H_

#include <iostream>
#include <unordered_map>
#include <functional>

#include <boost/algorithm/string.hpp>

#include "Parser.h"

#include <PlcAbstractSyntaxTree.h>

template<unsigned CHAR_STACKSIZE, unsigned PARSER_STACKSIZE, typename ExpressionType, typename ASTType>
class PlcParser
{
public:
  using VariableType = Variable<ExpressionType>;
  using ParseRemaining = std::function<void(const std::string&)>;

  PlcParser(Parser<CHAR_STACKSIZE, PARSER_STACKSIZE>& parser, ASTType& ast) : parser(parser), ast(ast) {}

  void parseVariables(ParseRemaining&& parseRemaining)
  {
    ParserResult parserResult;
    for (;;)
    {
      if (parser.next(parserResult).type() == ParserResult::Type::Identifier)
      {
        if (boost::algorithm::iequals(parserResult.text(), "inputs"))
          parseVariables(Var::Category::Input);
        else if (boost::algorithm::iequals(parserResult.text(), "outputs"))
          parseVariables(Var::Category::Output);
        else if (boost::algorithm::iequals(parserResult.text(), "monoflops") || boost::algorithm::iequals(parserResult.text(), "timer"))
          parseVariables(Var::Category::Monoflop);
        else if (boost::algorithm::iequals(parserResult.text(), "flags"))
          parseVariables(Var::Category::Flag);
        else
          parseRemaining(parserResult.text());
      }
      else if (!parserResult)
      {
        break;
      }
      else
      {
        throw ParserException(parser.getLineNo(), "Syntax Error");
      }
    }
  }

protected:

  void parseVariables(Var::Category category)
  {
    ParserResult parserResult;
    if (!parser.next(parserResult).is(ParserResult::Type::Char, ':'))
      throw ParserException(parser.getLineNo(), "missing ':' after in variable declaration");

    for (;;)
    {
      if (parser.next(parserResult).type() != ParserResult::Type::Identifier)
        throw ParserException(parser.getLineNo(), "missing identifier in variable declaration");

      if (ast.variableExists(parserResult.text()))
        throw ParserException(parser.getLineNo(), "Variable '%s' already declared", parserResult.text().c_str());

      uint64_t timeArg = (category == Var::Category::Monoflop) ? 30 : 0;
      ParserResult parserResult2;
      if (parser.next(parserResult2).is(ParserResult::Type::Char, '(') && category == Var::Category::Monoflop)
      {
        if (parser.next(parserResult2).type() != ParserResult::Type::Integer)
          throw ParserException(parser.getLineNo(), "missing Integer Value after Variable '%s' (", parserResult.text().c_str());

        timeArg = parserResult2.intValue();

        if (parser.next(parserResult2).is(ParserResult::Type::Identifier, "s"))
          timeArg /= 2;
        else if (parserResult2.is(ParserResult::Type::Identifier, "min"))
          timeArg *= 30;
        else if (parserResult2.is(ParserResult::Type::Identifier, "h"))
          timeArg *= 1800;
        else
          throw ParserException(parser.getLineNo(), "missing Time unit (s,min,h) after Variable '%s' (%d", parserResult.text().c_str(), timeArg);

        if (timeArg > 65535)
          throw ParserException(parser.getLineNo(), "Variable '%s' Time value overflow, max is 131071 s", parserResult.text().c_str());

        if (!parser.next(parserResult2).is(ParserResult::Type::Char, ')'))
          throw ParserException(parser.getLineNo(), "missing ')' after Variable '%s'", parserResult.text().c_str());

        parser.next(parserResult2);
      }

      if (!parserResult2.is(ParserResult::Type::Operator, ParserResult::Operator::Assign))
        throw ParserException(parser.getLineNo(), "missing '=' after Variable '%s'", parserResult.text().c_str());
      if (parser.next(parserResult2).type() != ParserResult::Type::Integer)
        throw ParserException(parser.getLineNo(), "missing integer value after Variable '%s'=", parserResult.text().c_str());

      ast.addVariable(VariableType(parserResult.text(), category, unsigned(parserResult2.intValue()), unsigned(timeArg)));

      if (parser.next(parserResult).is(ParserResult::Type::Char, ','))
        continue;

      if (parserResult.is(ParserResult::Type::Char, ';'))
        break;

      throw ParserException(parser.getLineNo(), "missing ',' in Variable list");
    }
  }

  Parser<CHAR_STACKSIZE, PARSER_STACKSIZE>& parser;
  ASTType& ast;
};

#endif // _INCLUDE_PLC_PARSER_H_
