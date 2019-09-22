#ifndef _INCLUDE_PLC_EXPRESSION_PARSER_H_
#define _INCLUDE_PLC_EXPRESSION_PARSER_H_

#include <iostream>
#include <unordered_map>

#include <boost/algorithm/string.hpp>

#include "Parser.h"
#include "Variable.h"
#include "PlcAst.h"
#include "PlcParser.h"

template<unsigned CHAR_STACKSIZE, unsigned PARSER_STACKSIZE>
class PlcExpressionParser : PlcParser<CHAR_STACKSIZE, PARSER_STACKSIZE, plc::Expression>
{
public:
  using Super = PlcParser<CHAR_STACKSIZE, PARSER_STACKSIZE, plc::Expression>;
  using VariableType = Variable<plc::Expression>;

  PlcExpressionParser(Parser<CHAR_STACKSIZE,PARSER_STACKSIZE>& parser, PlcAst& plcAst) : Super(parser, plcAst) {}

  void parse()
  {
    ParserResult parserResult;
    for (;;)
    {
      if (Super::parser.next(parserResult).type() == ParserResult::Type::Identifier)
      {
        if (boost::algorithm::iequals(parserResult.text(), "inputs"))
          Super::parseVariables(Var::Category::Input);
        else if (boost::algorithm::iequals(parserResult.text(), "outputs"))
          Super::parseVariables(Var::Category::Output);
        else if (boost::algorithm::iequals(parserResult.text(), "monoflops") || boost::algorithm::iequals(parserResult.text(), "timer"))
          Super::parseVariables(Var::Category::Monoflop);
        else if (boost::algorithm::iequals(parserResult.text(), "flags"))
          Super::parseVariables(Var::Category::Flag);
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
    if (Super::parser.next(parserResult).type() == ParserResult::Type::Identifier)
    {
      term = Super::plcAst.getVariable(parserResult.text());
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

      if (!Super::parser.next(parserResult).is(ParserResult::Type::Char, ')'))
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

      if (Super::parser.next(parserResult).type() == ParserResult::Type::Char &&
        (parserResult.intValue() == ';' || parserResult.intValue() == ')'))
      {
        expression->addTerm(term);

        Super::parser.push(parserResult);
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
    plc::Expression::Assignment assignment;

    ParserResult parserResult;
    if (Super::parser.next(parserResult).type() == ParserResult::Type::Operator)
      switch (parserResult.intValue())
      {
        case int(ParserResult::Operator::Assign) :
          assignment = plc::Expression::Assignment::Assign;
          break;
        case int(ParserResult::Operator::Set) :
          assignment = plc::Expression::Assignment::Set;
          break;
        case int(ParserResult::Operator::Reset) :
          assignment = plc::Expression::Assignment::Reset;
          break;
        default:
          throw ParserException("missing '=', '+=' or '-=' after variable '%s' equation", name.c_str());
      }
    else
      throw ParserException("missing '=', '+=' or '-=' after variable '%s' equation", name.c_str());

    VariableType& variable = Super::plcAst.getVariable(name);
    std::unique_ptr<plc::Expression> expression(new plc::Expression(variable.index(), assignment));
    parseExpression(expression);
    if (!Super::parser.next(parserResult).is(ParserResult::Type::Char, ';'))
      throw ParserException("missing ';' after expression");

    if (variable.expression().operator bool())
      throw ParserException("Two Expressions are assigned to Variable '%s'", name);

    variable.expression().swap(expression);
    variable.expression()->setVariable(&variable);
  }

};

#endif // _INCLUDE_PLC_EXPRESSION_PARSER_H_
