#ifndef _INCLUDE_STACK_H_
#define _INCLUDE_STACK_H_

#include <array>
#include <exception>

class StackOverflowException : public std::exception
{
  virtual char const* what() const
  {
    return "StackOverflow";
  }
};

class StackEmptyException : public std::exception
{
  virtual char const* what() const
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
    return size == 0;
  }

  bool full() const
  {
    return size == SIZE;
  }

  operator bool() const
  {
    return !empty();
  }

  void push(T const& data)
  {
    if (size < SIZE)
      stack[size++]= data;
    else
      throw StackOverflowException();
  }

  void push(T& data)
  {
    if (size < SIZE)
      std::swap(stack[size++], data);
    else
      throw StackOverflowException();
  }

  T& pop()
  {
    if (empty())
      throw StackEmptyException();
    else
      return stack[--size];
  }

private:

  unsigned size = 0;
  std::array<T, SIZE> stack;
  std::vector<int> a;
};

#endif // !_INCLUDE_STACK_H_

