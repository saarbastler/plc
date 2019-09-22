#ifndef _INCLUDE_PLC_PARSER_H_
#define _INCLUDE_PLC_PARSER_H_

#include <iostream>
#include <unordered_map>

#include <boost/algorithm/string.hpp>

#include "Parser.h"

#include <PlcAbstractSyntaxTree.h>

template<unsigned CHAR_STACKSIZE, unsigned PARSER_STACKSIZE, typename ExpressionType>
class PlcParser
{
public:
  using VariableType = Variable<ExpressionType>;

  PlcParser(Parser<CHAR_STACKSIZE, PARSER_STACKSIZE>& parser, PlcAbstractSyntaxTree<ExpressionType>& plcAst) : parser(parser), plcAst(plcAst) {}

protected:

  void parseVariables(Var::Category category)
  {
    ParserResult parserResult;
    if (!parser.next(parserResult).is(ParserResult::Type::Char, ':'))
      throw ParserException("missing ':' after in variable declaration");

    for (;;)
    {
      if (parser.next(parserResult).type() != ParserResult::Type::Identifier)
        throw ParserException("missing identifier in variable declaration");

      if (plcAst.variableExists(parserResult.text()))
        throw ParserException("Variable '%s' already declared", parserResult.text().c_str());

      uint64_t timeArg = (category == Var::Category::Monoflop) ? 30 : 0;
      ParserResult parserResult2;
      if (parser.next(parserResult2).is(ParserResult::Type::Char, '(') && category == Var::Category::Monoflop)
      {
        if (parser.next(parserResult2).type() != ParserResult::Type::Integer)
          throw ParserException("missing Integer Value after Variable '%s' (", parserResult.text().c_str());

        timeArg = parserResult2.intValue();

        if (parser.next(parserResult2).is(ParserResult::Type::Identifier, "s"))
          timeArg /= 2;
        else if (parserResult2.is(ParserResult::Type::Identifier, "min"))
          timeArg *= 30;
        else if (parserResult2.is(ParserResult::Type::Identifier, "h"))
          timeArg *= 1800;
        else
          throw ParserException("missing Time unit (s,min,h) after Variable '%s' (%d", parserResult.text().c_str(), timeArg);

        if (timeArg > 65535)
          throw ParserException("Variable '%s' Time value overflow, max is 131071 s", parserResult.text().c_str());

        if (!parser.next(parserResult2).is(ParserResult::Type::Char, ')'))
          throw ParserException("missing ')' after Variable '%s'", parserResult.text().c_str());

        parser.next(parserResult2);
      }

      if (!parserResult2.is(ParserResult::Type::Operator, ParserResult::Operator::Assign))
        throw ParserException("missing '=' after Variable '%s'", parserResult.text().c_str());
      if (parser.next(parserResult2).type() != ParserResult::Type::Integer)
        throw ParserException("missing integer value after Variable '%s'=", parserResult.text().c_str());

      plcAst.addVariable(VariableType(parserResult.text(), category, unsigned(parserResult2.intValue()), unsigned(timeArg)));

      if (parser.next(parserResult).is(ParserResult::Type::Char, ','))
        continue;

      if (parserResult.is(ParserResult::Type::Char, ';'))
        break;

      throw ParserException("missing ',' in Variable list");
    }
  }

  Parser<CHAR_STACKSIZE, PARSER_STACKSIZE>& parser;
  PlcAbstractSyntaxTree<ExpressionType>& plcAst;
};

#endif // _INCLUDE_PLC_PARSER_H_
