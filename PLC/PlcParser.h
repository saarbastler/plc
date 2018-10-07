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
        else if (boost::algorithm::iequals(parserResult.text(), "monoflops"))
          parseVariables(Variable::Type::Monoflop);
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
      if (!plcAst.variableExists(parserResult.text()))
        throw ParserException("Variable '%s' does not exist", parserResult.text().c_str());

      term = parserResult.text();
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

    std::unique_ptr<plc::Expression> expression(new plc::Expression());
    parseExpression(expression);
    if (!parser.next(parserResult).is(ParserResult::Type::Char, ';'))
      throw ParserException("missing ';' after expression");

    plc::Expression tmp(*expression.release());
    plcAst.addEquation(name, tmp);
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

      ParserResult parserResult2;
      if(!parser.next(parserResult2).is(ParserResult::Type::Char,'='))
        throw ParserException("missing '=' after Variable '%s'", parserResult.text().c_str());
      if (parser.next(parserResult2).type() != ParserResult::Type::Integer)
        throw ParserException("missing integer value after Variable '%s'=", parserResult.text().c_str());

      plcAst.addVariable( Variable(parserResult.text(), type, unsigned( parserResult2.intValue())));

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


