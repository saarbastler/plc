#ifndef _INCLUDE_STACK_H_
#define _INCLUDE_STACK_H_

#include <array>
#include <exception>

class StackOverflowException : public std::exception
{
  virtual char const* what() const noexcept
  {
    return "StackOverflow";
  }
};

class StackEmptyException : public std::exception
{
  virtual char const* what() const noexcept
  {
    return "StackEmpty";
  }
};

template<typename T, unsigned SIZE>
class Stack
{
public:

  bool empty() const
  {
    return size_ == 0;
  }

  bool full() const
  {
    return size_ == SIZE;
  }

  operator bool() const
  {
    return !empty();
  }

  void push(T const& data)
  {
    if (size_ < SIZE)
      stack[size_++]= data;
    else
      throw StackOverflowException();
  }

  void push(T& data)
  {
    if (size_ < SIZE)
      std::swap(stack[size_++], data);
    else
      throw StackOverflowException();
  }

  T& pop()
  {
    if (empty())
      throw StackEmptyException();
    else
      return stack[--size_];
  }

  unsigned size() const
  {
    return size_;
  }

private:

  unsigned size_ = 0;
  std::array<T, SIZE> stack;
  std::vector<int> a;
};

#endif // !_INCLUDE_STACK_H_

