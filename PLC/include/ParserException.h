#ifndef _INCLUDE_PARSER_EXCEPTION_H_
#define _INCLUDE_PARSER_EXCEPTION_H_

#include <string>
#include <exception>
#include <cstdarg>
#include <stdio.h>

class ParserException : public std::exception
{
public:

  ParserException(unsigned lineNo, const char *format, ...)
  {
    constexpr const std::size_t BufferSize = 1000;
    char buffer[BufferSize+1];

    std::va_list list;
    va_start(list, format);
    int n= vsnprintf(buffer, BufferSize, format, list);
    va_end(list);

    if (n >= 0 && n < BufferSize)
      snprintf(buffer + n, BufferSize - n, " in line %d", lineNo);

    message = buffer;
  }

  virtual char const* what() const noexcept
  {
    return message.c_str();
  }

private:

  std::string message;
};


#endif // !_INCLUDE_PARSER_EXCEPTION_H_
