#ifndef _INCLUDE_PLC_EXPRESSION_H_
#define _INCLUDE_PLC_EXPRESSION_H_

#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>

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
    Term(const Term& other);

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

    void operator=(Term& other)
    {
      swap(other);
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
  //           | Term [op Term ]*
  class Expression
  {
  public:

    // ordered by precedence, highest last
    enum class Operator
    {
      None, Or, And
    };

    Expression() : id_(idCounter++) {}

    Expression(Expression& other)
    {
      swap(other);
    }

    Expression(Term& term) : Expression()
    {
      addTerm(term);
    }

    Expression(Term& term, Operator op) : Expression(term)
    {      
      operator_ = op;
    }

    void swap(Expression& other)
    {
      Operator tmp = operator_;
      operator_ = other.operator_;
      other.operator_ = tmp;

      std::swap(id_, other.id_);
      std::swap(terms_, other.terms_);
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
      for(const Term& term : terms_)
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
          auto count = inputs.find(term.identifier());
          if (count == inputs.end())
            inputs[term.identifier()] = 1;
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

  private:

    Operator operator_= Operator::None;
    std::vector<Term> terms_;    
    /// <summary>
    /// The unique identifier
    /// </summary>
    unsigned id_;
    static unsigned idCounter;
  };
}

std::ostream& operator << (std::ostream& out, const plc::Expression& expression);
std::ostream& operator << (std::ostream& out, const plc::Term& term);

#endif // !_INCLUDE_PLC_EXPRESSION_H_

