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

  Variable(const std::string& name, Type type, unsigned index, unsigned time) : Variable(name, type, index)
  {
    time_ = time;
  }

  Variable(const Variable& other)
  {
    name_ = other.name_;
    type_ = other.type_;
    index_ = other.index_;
    time_ = other.time_;
  }

  void swap(Variable&& other)
  {
    name_.swap(other.name_);
    type_ = other.type_;
    std::swap(index_, other.index_); 
    std::swap(time_, other.time_);
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

  unsigned time() const
  {
    return time_;
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
  unsigned time_= 0;
};

#endif // !_INCLUDE_VARIABLE_H_

