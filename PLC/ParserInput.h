#ifndef _INCLUDE_PARSER_INPUT_H_

#include <istream>

#include "Stack.h"

template<unsigned STACKSIZE>
class ParserInput
{
public:

  ParserInput(std::istream& in) : in(in) {}

  char nextChar()
  {
    if (stack)
      return stack.pop();

    if (in)
    {
      int i = in.get();
      if (i > 0)
        return char(i);
    }

    return 0;
  }

  char nextIgnoreBlank()
  {
    char c;
    do
    {
      c = nextChar();
    } while (isspace(c));

    return c;
  }

  void pushChar(char c)
  {
    stack.push(c);
  }

private:

  std::istream& in;

  Stack<char, STACKSIZE> stack;
};
#endif // !_INCLUDE_PARSER_INPUT_H_

