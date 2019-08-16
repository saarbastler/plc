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

  using VariableDescriptionType = std::unordered_map<std::string, Variable>;
  using EquationType = std::unordered_map<std::string, plc::Expression>;

  void clear()
  {
    variableDescription_.clear();
    equations_.clear();
  }

  bool variableExists(const std::string& name)
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

  const VariableDescriptionType& variableDescription() const
  {
    return variableDescription_;
  }

  unsigned countVariableOfType(Variable::Type t) const
  {
    unsigned count = 0;
    for (auto it = variableDescription_.begin(); it != variableDescription_.end(); it++)
      if (it->second.type() == t)
        ++count;

    return count;
  }

  bool equationExists(const std::string& name) const
  {
    return equations_.find(name) != equations_.end();
  }

  void addEquation(const std::string& name, plc::Expression& expression)
  {
    if (equationExists(name))
      throw PlcAstException("Equation for '%s' already exists.", name.c_str());

    //std::cout << name << " = " << expression << std::endl;

    equations_[name] = std::move(expression);
  }

  const EquationType& equations() const
  {
    return equations_;
  }

  const plc::Expression resolveDependencies(const std::string& name, std::unordered_map<std::string,unsigned> *toSkip= nullptr) const
  {
    if (!equationExists(name))
      throw PlcAstException("Equation %s does not exist.", name.c_str());

    plc::Expression all(equations_.at(name));

    resolveDependencies(all, toSkip);

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
        auto var = equations_.find(it->identifier());
        if (toSkip && var != equations_.end())
        {
          auto skipFound = toSkip->find(it->identifier());
          if (skipFound == toSkip->end())
          {
            (*toSkip)[it->identifier()] = 1;
          }
          else
          {
            skipFound->second++;
            found = false;
          }
        }

        if (found)
        {
          if (var != equations_.end())
          {
            std::unique_ptr<plc::Expression> dep(new plc::Expression(var->second));
            dep->signalName_.clear();
            plc::Term term(dep);

            const Variable& variable = variableDescription_.at(it->identifier());
            if (variable.type() == Variable::Type::Monoflop)
              it->setExpression(new plc::Expression(term, plc::Expression::Operator::Timer, variable.index(), it->identifier()));
            else
              it->setExpression(new plc::Expression(term, it->identifier()));
          }
        }
        break;
      }
    }


  }

  VariableDescriptionType variableDescription_;
  EquationType equations_;
};

#endif // _INCLUDE_PLC_AST_H_

