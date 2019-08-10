#ifndef _INCLUDE_PARSER_H_
#define _INCLUDE_PARSER_H_

#include <sstream>
#include <exception>
#include <cstdarg>
#include <stdio.h>

#include "ParserInput.h"
#include "ParserResult.h"

class ParserException : public std::exception
{
public:
  ParserException(const char *format, ...)
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

template<unsigned CHAR_STACKSIZE, unsigned PARSER_STACKSIZE>
class Parser
{
public:

  Parser(ParserInput<CHAR_STACKSIZE>& parserInput) : parserInput(parserInput) {}

  const ParserResult& next(ParserResult& parserResult)
  {
    if (stack)
    {
      parserResult = stack.pop();
      return parserResult;
    }

    char c = parserInput.nextIgnoreBlank();
    while (c == '/')
      c = parseComment();

    if (c)
    {
      if( isalpha(c) || c == '_')
      { 
        std::ostringstream identifier;
        identifier << c;

        for (;;)
        {
          c = parserInput.nextChar();
          if (isalpha(c) || isdigit(c) || c == '_')
            identifier << c;
          else
            break;
        }

        parserInput.pushChar(c);

        parserResult = identifier.str();
      }
      else if (isdigit(c))
      {
        uint64_t result= 0;
        std::string text;
        text.reserve(24);

        while (isdigit(c))
        {
          result *= 10;
          result += (c - '0');

          text += c;
          c = parserInput.nextChar();
        }

        parserInput.pushChar(c);

        ParserResult tmp( result, text );
        parserResult.swap(tmp);
      }
      else
      {
        parserResult = c;
      }
    }
    else
    {
      parserResult.clear();
    }

    return parserResult;
  }

  void push(ParserResult& parserResult)
  {
    stack.push(parserResult);
  }

protected:

  char parseComment()
  {
    char c;
    char c2 = parserInput.nextChar();
    if (c2 == '/')
    {
      do
      {
        c = parserInput.nextChar();
      } while (c != '\n' && c != 0);

      return parserInput.nextIgnoreBlank();
    }
    else if (c2 == '*')
    {
      c = parserInput.nextChar();
      for(;;)
      {
        c2 = parserInput.nextChar();
        if (c2 == 0)
          throw ParserException("EOF in Comment");

        if(c == '*' && c2 == '/')
          return parserInput.nextIgnoreBlank();

        c = c2;
      }
    }
    else
    {
      parserInput.pushChar(c2);
    }

    return '/';
  }

  ParserInput<CHAR_STACKSIZE>& parserInput;
  Stack<ParserResult,PARSER_STACKSIZE> stack;
};
#endif // _INCLUDE_PARSER_H_

