#ifndef _INCLUDE_PLC_PARSER_H_
#define _INCLUDE_PLC_PARSER_H_

#include <iostream>
#include <unordered_map>

#include <boost/algorithm/string.hpp>

#include "Parser.h"
#include "Variable.h"
#include "PlcAst.h"

template<unsigned CHAR_STACKSIZE, unsigned PARSER_STACKSIZE>
class PlcParser
{
public:
  PlcParser(Parser<CHAR_STACKSIZE,PARSER_STACKSIZE>& parser, PlcAst& plcAst) : parser(parser), plcAst(plcAst) {}

  void parse()
  {
    ParserResult parserResult;
    for (;;)
    {
      if (parser.next(parserResult).type() == ParserResult::Type::Identifier)
      {
        if (boost::algorithm::iequals(parserResult.text(), "inputs"))
          parseVariables(Variable::Type::Input);
        else if (boost::algorithm::iequals(parserResult.text(), "outputs"))
          parseVariables(Variable::Type::Output);
        else if (boost::algorithm::iequals(parserResult.text(), "monoflops") || boost::algorithm::iequals(parserResult.text(), "timer"))
          parseVariables(Variable::Type::Monoflop);
        else if (boost::algorithm::iequals(parserResult.text(), "flags"))
          parseVariables(Variable::Type::Flag);
        else
          parseEquation(parserResult.text());
      }
      else if (!parserResult)
      {
        break;
      }
      else
      {
        throw ParserException("Syntax Error");
      }
    }
  }

protected:

  void parseTerm(plc::Term& term)
  {
    ParserResult parserResult;
    if (parser.next(parserResult).type() == ParserResult::Type::Identifier)
    {
      term = plcAst.getVariable(parserResult.text());
    }
    else if (parserResult.is(ParserResult::Type::Char, '!'))
    {
      parseTerm(term);
      term.reverseUnary();
    }
    else if (parserResult.is(ParserResult::Type::Char, '('))
    {
      std::unique_ptr<plc::Expression> expression(new plc::Expression());
      parseExpression(expression);

      if (!parser.next(parserResult).is(ParserResult::Type::Char, ')'))
        throw ParserException("missing closing ')'");

      term = expression;
    }
    else
      throw ParserException("Syntax Error");
  }

  static plc::Expression::Operator toOperator(ParserResult& parserResult)
  {
    if (parserResult.type() != ParserResult::Type::Char)
      throw ParserException("Syntax Error, Operator expected, but got '%s'", parserResult.text().c_str());

    switch (parserResult.intValue())
    {
    case '&': return plc::Expression::Operator::And;
    case '|': return plc::Expression::Operator::Or;
    default:
      throw ParserException("Syntax Error, undefined Operator: '%s'", parserResult.text().c_str());
    }
  }

  void parseExpression(std::unique_ptr<plc::Expression>& expression)
  {
    ParserResult parserResult;
    for (;;)
    {
      plc::Term term;
      parseTerm(term);

      if (parser.next(parserResult).type() == ParserResult::Type::Char &&
        (parserResult.intValue() == ';' || parserResult.intValue() == ')'))
      {
        expression->addTerm(term);

        parser.push(parserResult);
        break;
      }

      plc::Expression::Operator op = toOperator(parserResult);
      if (!*expression || expression->op() == op)
      {
        expression->op() = op;
        expression->addTerm(term);
      }
      else if(expression->op() < op)
      {
        std::unique_ptr<plc::Expression> tmp(new plc::Expression(term, op));

        parseExpression(tmp);

        plc::Term otherTerm(tmp);
        expression->addTerm(otherTerm);
        break;
      }
      else
      {
        expression->addTerm(term);

        plc::Term firstTerm(expression);
        expression.reset(new plc::Expression(firstTerm));

        expression->op() = op;
      }
    }
  }

  void parseEquation(const std::string& name)
  {
    ParserResult parserResult;
    if (!parser.next(parserResult).is(ParserResult::Type::Char, '='))
      throw ParserException("missing '=' after variable '%s' equation", name.c_str());

    Variable& variable = plcAst.getVariable(name);
    std::unique_ptr<plc::Expression> expression(new plc::Expression(variable.index()));
    parseExpression(expression);
    if (!parser.next(parserResult).is(ParserResult::Type::Char, ';'))
      throw ParserException("missing ';' after expression");

    if (variable.expression().operator bool())
      throw ParserException("Two Expressions are assigned to Variable '%s'", name);

    variable.expression().swap(expression);
    variable.expression()->setVariable(&variable);
  }

  void parseVariables(Variable::Type type)
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

      uint64_t timeArg = (type == Variable::Type::Monoflop) ? 30 : 0;
      ParserResult parserResult2;
      if (parser.next(parserResult2).is(ParserResult::Type::Char, '(') && type == Variable::Type::Monoflop)
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

      if(!parserResult2.is(ParserResult::Type::Char,'='))
        throw ParserException("missing '=' after Variable '%s'", parserResult.text().c_str());
      if (parser.next(parserResult2).type() != ParserResult::Type::Integer)
        throw ParserException("missing integer value after Variable '%s'=", parserResult.text().c_str());

      plcAst.addVariable( Variable(parserResult.text(), type, unsigned( parserResult2.intValue()), unsigned(timeArg)));

      if (parser.next(parserResult).is(ParserResult::Type::Char, ','))
        continue;

      if (parserResult.is(ParserResult::Type::Char, ';'))
        break;

      throw ParserException("missing ',' in Variable list");
    }

  }
  
  Parser<CHAR_STACKSIZE,PARSER_STACKSIZE>& parser;
  PlcAst& plcAst;
};

#endif // _INCLUDE_PLC_PARSER_H_


