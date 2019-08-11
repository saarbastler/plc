#ifndef _INCLUDE_PLC_AST_H_
#define _INCLUDE_PLC_AST_H_

#include <exception>
#include <cstdarg>
//#include <iostream>
#include <unordered_map>
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

  const plc::Expression resolveDependencies(const std::string& name)
  {
    if (!equationExists(name))
      throw PlcAstException("Equation %s does not exist.", name.c_str());

    plc::Expression all(equations_[name]);

    resolveDependencies(all);

    return all;
  }

protected:

  void resolveDependencies(plc::Expression& expression)
  {
    for (auto it = expression.terms_.begin(); it != expression.terms_.end(); it++)
    {
      switch (it->type())
      {
      case plc::Term::Type::Expression:
        resolveDependencies(*it->expression().get());
        break;

      case plc::Term::Type::Identifier:
        auto var = equations_.find(it->identifier());
        if (var != equations_.end())
        {
          std::unique_ptr<plc::Expression> dep(new plc::Expression(var->second));
          plc::Term term(dep);

          it->setExpression(new plc::Expression(term, plc::Expression::Operator::Timer));
        }
        break;
      }
    }


  }

  VariableDescriptionType variableDescription_;
  EquationType equations_;
};

#endif // _INCLUDE_PLC_AST_H_

