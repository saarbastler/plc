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

    char ch = 0;
    if (in)
    {
      int i = in.get();
      if (i > 0)
      {
        ch = char(i);

        if (ch == '\n')
          ++lineNo;
      }
    }

    return ch;
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

  unsigned getLineNo()
  {
    return lineNo;
  }

private:

  unsigned lineNo = 1;

  std::istream& in;

  Stack<char, STACKSIZE> stack;
};
#endif // !_INCLUDE_PARSER_INPUT_H_

