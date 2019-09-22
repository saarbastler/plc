#ifndef _INCLUDE_PLC_ABSTRACT_SYNTAX_TREE_H_
#define _INCLUDE_PLC_ABSTRACT_SYNTAX_TREE_H_

#include <unordered_map>
#include <unordered_set>

#include "Variable.h"
#include "PlcAstException.h"

template<typename ExpressionType>
class PlcAbstractSyntaxTree
{
public:

  using VariableType = Variable<ExpressionType>;
  using VariableDescriptionType = std::unordered_map<std::string, VariableType>;

  void swap(PlcAbstractSyntaxTree& other)
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

  void addVariable(VariableType&& variable)
  {
    if (variableExists(variable.name()))
      throw PlcAstException("Variable '%s' already declared", variable.name().c_str());

    variableDescription_.emplace(variable.name(), std::move(variable));
  }

  const VariableType& getVariable(const std::string& name) const
  {
    auto it = variableDescription_.find(name);
    if( it == variableDescription_.end())
      throw PlcAstException("Variable '%s' does not exist", name.c_str());

    return it->second;
  }

  VariableType& getVariable(const std::string& name)
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

  unsigned maxVariableIndexOfType(Var::Category cat) const
  {
    unsigned index = 0;
    for (auto it = variableDescription_.begin(); it != variableDescription_.end(); it++)
      if (it->second.category() == cat && it->second.index() > index)
        index= it->second.index();

    return index;
  }

protected:

  VariableDescriptionType variableDescription_;
};

#endif // _INCLUDE_PLC_ABSTRACT_SYNTAX_TREE_H_

