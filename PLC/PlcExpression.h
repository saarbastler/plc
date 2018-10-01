#ifndef _INCLUDE_PLC_EXPRESSION_H_
#define _INCLUDE_PLC_EXPRESSION_H_

#include <memory>
#include <string>
#include <sstream>

namespace plc
{
  class Expression;

  // Term= '(' Expression ')'
  //     | !Expression
  //     | Identifier
  class Term
  {
  public:
    enum class Type
    {
      Empty, Expression, Identifier
    };

    enum class Unary
    {
      None, Not
    };

    Term() { }
    Term(std::unique_ptr<Expression>& expression)
    {
      *this = expression;
    }

    void swap(Term& other)
    {
      Type tmp = type_;
      type_ = other.type_;
      other.type_ = tmp;

      Unary utmp = unary_;
      unary_ = other.unary_;
      other.unary_ = utmp;

      std::swap(expression_, other.expression_);
      std::swap(identifier_, other.identifier_);
    }

    void clear()
    {
      type_ = Type::Empty;
      unary_ = Unary::None;
      expression_.reset();
      identifier_.clear();
    }

    void operator=(const std::string& identifier)
    {
      unary_ = Unary::None;
      type_ = Type::Identifier;
      identifier_ = identifier;
    }

    void operator=(std::unique_ptr<Expression>& expression)
    {
      unary_ = Unary::None;
      type_ = Type::Expression;
      expression_.swap(expression);
    }

    operator bool() const
    {
      return type_ != Type::Empty;
    }

    void reverseUnary()
    {
      unary_ = (unary_ == Unary::None) ? Unary::Not : Unary::None;
    }

    Unary unary() const
    {
      return unary_;
    }

    Type type() const 
    {
      return type_;
    }

    const std::unique_ptr<Expression>& expression() const
    {
      return expression_;
    }

    const std::string& identifier() const
    {
      return identifier_;
    }

  private:

    Unary unary_= Unary::None;
    Type type_= Type::Empty;
    std::unique_ptr<Expression> expression_;
    std::string identifier_;
  };

  // Expression= Term
  //           | Term op Term
  class Expression
  {
  public:

    // ordered by precedence, highest last
    enum class Operator
    {
      None, Or, And
    };

    Expression() {}
    Expression(Term& left)
    {
      terms_.emplace_back(std::move(left));
    }

    Expression(Term& left, Operator op)
    {
      terms_.emplace_back(std::move(left));
      operator_ = op;
    }

    void clear()
    {
      terms_.clear();
      operator_ = Operator::None;
    }

    operator bool() const
    {
      return !terms_.empty() & (operator_ != Operator::None);
    }

    std::vector<Term>& terms()
    {
      return terms_;
    }

    Operator& op()
    {
      return operator_;
    }

  private:

    Operator operator_= Operator::None;
    std::vector<Term> terms_;

  };
}

std::ostream& operator << (std::ostream& out, const std::unique_ptr<plc::Expression>& expression);
std::ostream& operator << (std::ostream& out, const plc::Term& term);

#endif // !_INCLUDE_PLC_EXPRESSION_H_

