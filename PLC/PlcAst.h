#ifndef _INCLUDE_PLC_AST_H_
#define _INCLUDE_PLC_AST_H_

#include <exception>
#include <cstdarg>
#include <iostream>
#include <unordered_map>

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
    vsnprintf_s(buffer, sizeof(buffer) / sizeof(buffer[0]), format, list);
    va_end(list);

    message = buffer;
  }

  virtual char const* what() const
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
    if (it == variableDescription_.end())
      throw PlcAstException("Variable '%s' does not exist", name.c_str());

    return it->second;
  }

  const VariableDescriptionType& variableDescription() const
  {
    return variableDescription_;
  }

  bool equationExists(const std::string& name) const
  {
    return equations_.find(name) != equations_.end();
  }

  void addEquation(const std::string& name, plc::Expression& expression)
  {
    if (equationExists(name))
      throw PlcAstException("Equation for '%s' already exists.", name.c_str());

    std::cout << name << " = " << expression << std::endl;

    equations_[name] = std::move(expression);
  }

  const EquationType& equations() const
  {
    return equations_;
  }

protected:

  VariableDescriptionType variableDescription_;
  EquationType equations_;
};

#endif // _INCLUDE_PLC_AST_H_

