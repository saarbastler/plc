#ifndef _INCLUDE_PARSER_RESULT_H_
#define _INCLUDE_PARSER_RESULT_H_

#include <string>

class ParserResult
{
public:

  enum class Type
  {
    Empty,
    Identifier,
    Char,
    Integer
  };

  ParserResult()
  {
    clear();
  }

  ParserResult(const ParserResult& other)
  {
    *this = other;
  }

  ParserResult(ParserResult&& other)
  {
    swap(other);
  }

  ParserResult(uint64_t intValue, std::string& text)
  {
    type_ = Type::Integer;
    intValue_ = intValue;
    text_.swap(text);
  }

  ParserResult& operator =(const ParserResult& other)
  {
    type_ = other.type_;
    text_ = other.text_;
    intValue_ = other.intValue_;

    return *this;
  }

  ParserResult& operator =(const std::string& string)
  {
    text_ = string;
    type_ = Type::Identifier;

    return *this;
  }

  ParserResult& operator =(std::string&& string)
  {
    string.swap(text_);
    type_ = Type::Identifier;

    return *this;
  }

  ParserResult& operator =(char c)
  {
    text_= c;
    type_ = Type::Char;
    intValue_ = c;

    return *this;
  }

  void swap(ParserResult& other)
  {
    if (this != &other)
    {
      Type tmp = type_;
      type_= other.type_;
      other.type_ = tmp;

      text_.swap(other.text_);
      std::swap(intValue_, other.intValue_);
    }
  }

  void clear()
  {
    type_ = Type::Empty;
  }

  Type type() const
  {
    return type_;
  }

  const std::string& text() const
  {
    return text_;
  }

  uint64_t intValue() const
  {
    return intValue_;
  }

  operator bool() const
  {
    return type_ != Type::Empty;
  }

  bool is(Type type, const char *text) const
  {
    return type == type_ && text == text_;
  }

  bool is(Type type, const std::string& text) const
  {
    return type == type_ && text == text_;
  }

  bool is(Type type, uint64_t ui) const
  {
    return type == type_ && intValue_ == ui;
  }

private:

  Type type_;
  std::string text_;
  uint64_t intValue_;
};

std::ostream& operator<<(std::ostream& out, ParserResult& parserResult);

#endif // !_INCLUDE_PARSER_RESULT_H_

