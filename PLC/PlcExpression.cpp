#include "PlcExpression.h"


unsigned plc::Expression::idCounter = 0;

std::ostream& operator<<(std::ostream& out, const plc::Term& term)
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
      out << *term.expression() << " )";
      break;
    case plc::Term::Type::Identifier:
      out << term.variable()->name() << " )";
      break;
    default:
      out << "Undefined Type: " << int(term.type()) << " )";
  }

  return out;
}

std::ostream& operator<<(std::ostream& out, const plc::Expression& expression)
{
  out << "Expression(";
  out << expression.id() << ',';
  if (expression.variable())
    out << expression.variable()->name() << ',';

  std::string opText;

  for (auto& it : expression.terms())
  {
    if (opText.empty())
    {
      if (expression.op() != plc::Expression::Operator::None)
        switch (expression.op())
        {
          case plc::Expression::Operator::And:
            opText = " & ";
            break;
          case plc::Expression::Operator::Or:
            opText = " | ";
            break;
          default:
            opText = " undefined Operator: " + int(expression.op());
        }
    }
    else
    {
      out << opText;
    }

    out << it;
  }

  out << " )";

  return out;
}

namespace plc
{
  void Term::operator=(const Term& other)
  {
    unary_ = other.unary_;
    type_ = other.type_;
    if (other.expression_)
      expression_.reset(new Expression(*other.expression_));
    //identifier_ = other.identifier_;
    variable_ = other.variable_;
  }

}