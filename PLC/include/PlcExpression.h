#ifndef _INCLUDE_PLC_EXPRESSION_H_
#define _INCLUDE_PLC_EXPRESSION_H_

#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <algorithm>

#include <jsConsts.h>
#include <Variable.h>

class PlcAst;
namespace plc
{
  class Expression;
  using VariableType = Variable<Expression>;

  // Term= '(' Expression ')'
  //     | !Term
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

    Term()
    {
    }

    Term(const Term& other)
    {
      *this = other;
    }

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
      std::swap(variable_, other.variable_);
    }

    void clear()
    {
      type_ = Type::Empty;
      unary_ = Unary::None;
      expression_.reset();
      variable_ = nullptr;
    }

    void operator=(const VariableType& variable)
    {
      unary_ = Unary::None;
      type_ = Type::Identifier;
      variable_ = &variable;
    }

    void operator=(std::unique_ptr<Expression>& expression)
    {
      unary_ = Unary::None;
      type_ = Type::Expression;
      expression_.swap(expression);
    }

    void operator=(const Term& other);

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

    const VariableType *variable() const
    {
      return variable_;
    }

    void setExpression(Expression *expression)
    {
      expression_.reset(expression);
      type_ = Type::Expression;
      variable_ = nullptr;
    }

  private:

    friend class Expression;
    void swapExpression(std::unique_ptr<Expression>& expression)
    {
      expression.swap(expression_);
    }

    Unary unary_ = Unary::None;
    Type type_ = Type::Empty;
    std::unique_ptr<Expression> expression_;
    const VariableType *variable_ = nullptr;
  };

  // Expression= Term
  //           | Term [op Term ]*

  class Expression
  {
  public:

    enum class Assignment
    {
      Assign,
      Set,
      Reset
    };

    // ordered by precedence, highest last
    enum class Operator
    {
      None, Or, And, Timer
    };

    Expression() : id_(Expression::nextId())
    {
    }

    Expression(unsigned id, Assignment assign = Assignment::Assign) : id_(id), assign_(assign)
    {
    }

    Expression(Term& term) : Expression()
    {
      addTerm(term);
    }

    Expression(Operator op, const std::vector<Term>& terms) : Expression()
    {
      operator_ = op;
      for (auto it = terms.begin(); it != terms.end(); it++)
      {
        plc::Term t(*it);
        addTerm(t);
      }
    }

    Expression(Term& term, Operator op) : Expression(term)
    {
      operator_ = op;
    }

    Expression(Term& term, Operator op, unsigned id) : Expression(term, op)
    {
      id_ = id;
    }

    Expression(const Expression& other)
    {
      operator_ = other.operator_;

      terms_.reserve(other.terms_.size());
      std::for_each(other.terms_.begin(), other.terms_.end(), [this](const Term& otherTerm)
      {
        terms_.emplace_back(Term(otherTerm));
      });

      id_= other.id_;
      variable_ = other.variable_;
    }

    Expression(Expression&& other)
    {
      swap(other);
    }

    void swap(Expression& other)
    {
      std::swap(assign_, other.assign_);
      std::swap(operator_, other.operator_);
      std::swap(id_, other.id_);
      std::swap(terms_, other.terms_);
      std::swap(variable_, other.variable_);
    }

    void clear()
    {
      terms_.clear();
      operator_ = Operator::None;
    }

    operator bool() const
    {
      return (terms_.size() == 1 && operator_ == Operator::None) || (!terms_.empty() && operator_ != Operator::None);
    }

    const std::vector<Term>& terms() const
    {
      return terms_;
    }

    Operator& op()
    {
      return operator_;
    }

    Operator op() const
    {
      return operator_;
    }

    void addTerm(Term& term)
    {
      terms_.emplace_back();
      terms_.back().swap(term);
    }

    Assignment assignment() const
    {
      return assign_;
    }

    void pullupFirstTerm()
    {
      if (operator_ == Operator::None && terms_.size() == 1 && terms_[0].type() == Term::Type::Expression && terms_[0].unary() == Term::Unary::None)
      {
        Term tmp;
        tmp.swap(terms_[0]);

        std::unique_ptr<Expression> tmpExpr;
        tmp.swapExpression(tmpExpr);

        tmpExpr->swap(*this);

        id_ = tmpExpr->id();
        assign_ = tmpExpr->assignment();
        variable_ = tmpExpr->variable();
      }
    }

    /// <summary>
    /// A Term is simple, if there is no sub expression
    /// </summary>
    /// <returns>
    ///   <c>true</c> if this instance is simple; otherwise, <c>false</c>.
    /// </returns>
    bool isSimple() const
    {
      for (auto&it : terms_)
        if (it.type() != Term::Type::Identifier)
          return false;

      return true;
    }

    /// <summary>
    /// Counts the number Expression levels.
    /// </summary>
    /// <returns>The enumber of levels</returns>
    unsigned countLevels() const
    {
      unsigned level = 0;
      for (const Term& term : terms_)
        if (term.type() == Term::Type::Expression)
        {
          unsigned termLevel = term.expression()->countLevels();
          if (termLevel > level)
            level = termLevel;
        }

      return 1 + level;
    }

    /// <summary>
    /// Counts the inputs, the map keys the input name to the number of occurence.
    /// </summary>
    /// <param name="inputs">The input count.</param>
    void countInputs(std::unordered_map<std::string, unsigned>& inputs) const
    {
      for (const Term& term : terms_)
        if (term.type() == Term::Type::Identifier)
        {
          auto count = inputs.find(term.variable()->name());
          if (count == inputs.end())
            inputs[term.variable()->name()] = 1;
          else
            (count->second)++;
        }
        else if (term.type() == Term::Type::Expression)
        {
          term.expression()->countInputs(inputs);
        }
    }

    unsigned id() const
    {
      return id_;
    }

    static unsigned lastId()
    {
      return idCounter;
    }

    const VariableType *variable() const
    {
      return variable_;
    }

    void setVariable(const VariableType *variable)
    {
      variable_ = variable;
    }

    static const char *assignmentText(plc::Expression::Assignment assignment);

  private:

    static unsigned nextId()
    {
      return ++idCounter;
    }

    friend class ::PlcAst;
    
    Assignment assign_;

    Operator operator_ = Operator::None;
    std::vector<Term> terms_;
    /// <summary>
    /// The unique identifier
    /// </summary>
    unsigned id_;
    static unsigned idCounter;

    const VariableType *variable_ = nullptr;
  };
}

std::ostream& operator<<(std::ostream& out, const plc::Expression& expression);
std::ostream& operator<<(std::ostream& out, const plc::Term& term);

#endif // !_INCLUDE_PLC_EXPRESSION_H_

