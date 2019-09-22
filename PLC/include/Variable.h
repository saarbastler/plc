#ifndef _INCLUDE_VARIABLE_H_
#define _INCLUDE_VARIABLE_H_

#include <string>
#include <memory>

class Var
{
public:
  enum class Category
  {
    Input, Output, Monoflop, Flag
  };
};

template<typename ExpressiontType>
class Variable : Var
{
public:

  Variable(const std::string& name, Category category, unsigned index) : name_(name), category_(category), index_(index) 
  {
  }

  Variable(const std::string& name, Category category, unsigned index, unsigned time) : Variable(name, category, index)
  {
    time_ = time;
  }

  Variable(const Variable& other)
  {
    name_ = other.name_;
    category_ = other.category_;
    index_ = other.index_;
    time_ = other.time_;
  }

  void swap(Variable&& other)
  {
    name_.swap(other.name_);
    category_ = other.category_;
    std::swap(index_, other.index_); 
    std::swap(time_, other.time_);
  }

  const std::string& name() const
  {
    return name_;
  }

  Category category() const
  {
    return category_;
  }

  unsigned index() const
  {
    return index_;
  }

  unsigned time() const
  {
    return time_;
  }

  const std::unique_ptr<ExpressiontType>& expression() const
  {
    return expression_;
  }

  std::unique_ptr<ExpressiontType>& expression()
  {
    return expression_;
  }

private:

  std::unique_ptr<ExpressiontType> expression_;

  std::string name_;
  Category category_;
  unsigned index_;
  unsigned time_= 0;
};

#endif // !_INCLUDE_VARIABLE_H_

