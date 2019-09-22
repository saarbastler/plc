#ifndef _INCLUDE_PLC_AST_EXCEPTION_H_
#define _INCLUDE_PLC_AST_EXCEPTION_H_

#include <exception>
#include <string>
#include <cstdarg>
#include <stdio.h>

class PlcAstException : public std::exception
{
public:
  PlcAstException(const char *format, ...)
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


#endif // !_INCLUDE_PLC_AST_EXCEPTION_H_

