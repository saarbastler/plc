#ifndef _INCLUDE_PLC_AST_H_
#define _INCLUDE_PLC_AST_H_

#include <exception>
#include <cstdarg>
#include <unordered_map>
#include <unordered_set>
#include <stdio.h>

#include "PlcExpression.h"
#include "Variable.h"

class PlcAstException : public std::exception
{
public:
  PlcAstException(const char *format, ...)
  {
    char buffer[1000];

    std::va_list list;
    va_start(list, format);
    vsnprintf(buffer, sizeof(buffer) / sizeof(buffer[0]), format, list);
    va_end(list);

    message = buffer;
  }

  virtual char const* what() const noexcept
  {
    return message.c_str();
  }

private:

  std::string message;
};


class PlcAst
{
public:

  using Op = plc::Expression::Operator;
  using VariableDescriptionType = std::unordered_map<std::string, Variable>;

  void swap(PlcAst& other)
  {
    std::swap(variableDescription_, other.variableDescription_);
  }

  void clear()
  {
    variableDescription_.clear();
  }

  bool variableExists(const std::string& name) const
  {
    return variableDescription_.find(name) != variableDescription_.end();
  }

  void addVariable(Variable&& variable)
  {
    if (variableExists(variable.name()))
      throw PlcAstException("Variable '%s' already declared", variable.name().c_str());

    variableDescription_.emplace(variable.name(), std::move(variable));
  }

  const Variable& getVariable(const std::string& name) const
  {
    auto it = variableDescription_.find(name);
    if( it == variableDescription_.end())
      throw PlcAstException("Variable '%s' does not exist", name.c_str());

    return it->second;
  }

  Variable& getVariable(const std::string& name)
  {
    auto it = variableDescription_.find(name);
    if (it == variableDescription_.end())
      throw PlcAstException("Variable '%s' does not exist", name.c_str());

    return it->second;
  }

  const VariableDescriptionType& variableDescription() const
  {
    return variableDescription_;
  }

  unsigned maxVariableIndexOfType(Variable::Type t) const
  {
    unsigned index = 0;
    for (auto it = variableDescription_.begin(); it != variableDescription_.end(); it++)
      if (it->second.type() == t && it->second.index() > index)
        index= it->second.index();

    return index;
  }

  const plc::Expression resolveDependencies(const std::string& name, std::unordered_map<std::string,unsigned> *toSkip= nullptr) const
  {
    const Variable& variable = getVariable(name);

    if (!variable.expression().operator bool())
      throw PlcAstException("Expression %s does not exist.", name.c_str());

    plc::Expression all(*variable.expression());

    resolveDependencies(all, toSkip);

    all.pullupFirstTerm();

    return all;
  }

protected:

  void resolveDependencies(plc::Expression& expression, std::unordered_map<std::string, unsigned> *toSkip) const
  {
    for (auto it = expression.terms_.begin(); it != expression.terms_.end(); it++)
    {
      bool found = true;

      switch (it->type())
      {
      case plc::Term::Type::Expression:
        resolveDependencies(*it->expression().get(), toSkip);
        break;

      case plc::Term::Type::Identifier:
        const plc::Expression *pExpression = it->variable()->expression().get();        
        if (toSkip && pExpression)
        {
          auto skipFound = toSkip->find(it->variable()->name());
          if (skipFound == toSkip->end())
          {
            (*toSkip)[it->variable()->name()] = 1;
          }
          else
          {
            skipFound->second++;
            found = false;
          }
        }

        if (found && pExpression)
        {
          if (pExpression->terms().size() == 1)
          {
            plc::Term copyTerm(pExpression->terms()[0]);

            plc::Expression::Operator op = (it->variable()->type() == Variable::Type::Monoflop) ? Op::Timer : Op::None;

            plc::Expression *expression = new plc::Expression(copyTerm, op, pExpression->id());
            expression->setVariable(pExpression->variable());
            it->setExpression(expression);
          }
          else
          {
            std::unique_ptr<plc::Expression> dep(new plc::Expression(pExpression->op(), pExpression->terms()));
            plc::Term term(dep);

            plc::Expression *expression = nullptr;
            
            if (it->variable()->type() == Variable::Type::Monoflop)
              expression = new plc::Expression(term, Op::Timer, it->variable()->index());
            else
              expression = new plc::Expression(term);

            expression->setVariable(it->variable());
            it->setExpression(expression);
          }

        }
        break;
      }
    }
  }

  VariableDescriptionType variableDescription_;
};

#endif // _INCLUDE_PLC_AST_H_

