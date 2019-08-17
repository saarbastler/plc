#ifndef _INCLUDE_VARIABLE_H_
#define _INCLUDE_VARIABLE_H_

#include <string>
#include <memory>

namespace plc
{
  class Expression;
}

class Variable
{
public:
  enum class Type
  {
    Input, Output, Monoflop, Flag
  };

  Variable(const std::string& name, Type type, unsigned index) : name_(name), type_(type), index_(index) 
  {
  }

  Variable(const Variable& other)
  {
    name_ = other.name_;
    type_ = other.type_;
    index_ = other.index_;
  }

  void swap(Variable&& other)
  {
    name_.swap(other.name_);
    type_ = other.type_;
    std::swap(index_,other.index_);
  }

  const std::string& name() const
  {
    return name_;
  }

  Type type() const
  {
    return type_;
  }

  unsigned index() const
  {
    return index_;
  }

  const std::unique_ptr<plc::Expression>& expression() const
  {
    return expression_;
  }

  std::unique_ptr<plc::Expression>& expression()
  {
    return expression_;
  }

private:

  std::unique_ptr<plc::Expression> expression_;

  std::string name_;
  Type type_;
  unsigned index_;
};

#endif // !_INCLUDE_VARIABLE_H_

