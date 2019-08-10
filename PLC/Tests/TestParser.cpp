#ifdef PARSER_TESTS

#include <boost/test/unit_test.hpp>
#include "../Parser.h"

#include <sstream>

BOOST_AUTO_TEST_CASE(Parser_parse)
{
  std::istringstream in("abc:def , x y\nnewline");
  ParserInput<2> parserInput(in);
  Parser<2,3> parser(parserInput);

  ParserResult parserResult;
  BOOST_CHECK(parser.next(parserResult).is(ParserResult::Type::Identifier, "abc"));
  BOOST_CHECK(parser.next(parserResult).is(ParserResult::Type::Char, ":"));
  BOOST_CHECK(parser.next(parserResult).is(ParserResult::Type::Identifier, "def"));
  BOOST_CHECK(parser.next(parserResult).is(ParserResult::Type::Char, ","));
  BOOST_CHECK(parser.next(parserResult).is(ParserResult::Type::Identifier, "x"));
  BOOST_CHECK(parser.next(parserResult).is(ParserResult::Type::Identifier, "y"));
  BOOST_CHECK(parser.next(parserResult).is(ParserResult::Type::Identifier, "newline"));
  BOOST_CHECK(!parser.next(parserResult).operator bool());
  BOOST_CHECK(!parser.next(parserResult).operator bool());
  BOOST_CHECK(!parser.next(parserResult).operator bool());
}

BOOST_AUTO_TEST_CASE(Parser_Comments1)
{
  std::istringstream in(
    "value=0;//until end of line Windows\r\n"
    "xyz;  // linux \n"
    "q  // /* other comment\n"
    "lastline");

  ParserInput<2> parserInput(in);
  Parser<2,3> parser(parserInput);

  ParserResult parserResult;
  BOOST_CHECK(parser.next(parserResult).is(ParserResult::Type::Identifier, "value"));
  BOOST_CHECK(parser.next(parserResult).is(ParserResult::Type::Char, "="));
  BOOST_CHECK(parser.next(parserResult).is(ParserResult::Type::Integer, 0ull));
  BOOST_CHECK(parser.next(parserResult).is(ParserResult::Type::Char, ";"));
  BOOST_CHECK(parser.next(parserResult).is(ParserResult::Type::Identifier, "xyz"));
  BOOST_CHECK(parser.next(parserResult).is(ParserResult::Type::Char, ";"));
  BOOST_CHECK(parser.next(parserResult).is(ParserResult::Type::Identifier, "q"));
  BOOST_CHECK(parser.next(parserResult).is(ParserResult::Type::Identifier, "lastline"));
  BOOST_CHECK(!parser.next(parserResult).operator bool());
  BOOST_CHECK(!parser.next(parserResult).operator bool());
  BOOST_CHECK(!parser.next(parserResult).operator bool());
}

BOOST_AUTO_TEST_CASE(Parser_Comments2)
{
  std::istringstream in(
    "oneline;//until end of line Windows\r\n"
  );

  ParserInput<2> parserInput(in);
  Parser<2,3> parser(parserInput);

  ParserResult parserResult;
  BOOST_CHECK(parser.next(parserResult).is(ParserResult::Type::Identifier, "oneline"));
  BOOST_CHECK(parser.next(parserResult).is(ParserResult::Type::Char, ";"));
  BOOST_CHECK(!parser.next(parserResult).operator bool());
  BOOST_CHECK(!parser.next(parserResult).operator bool());
  BOOST_CHECK(!parser.next(parserResult).operator bool());
}

BOOST_AUTO_TEST_CASE(Parser_Comments3)
{
  std::istringstream in(
    "multi/*comment*/;\n"
    "multiple  /* 1\r\n"
    "2\n"
    "3 */lines\n"
    "1/* // abc\n"
    "2*/\n"
    "3"
  );

  ParserInput<2> parserInput(in);
  Parser<2,3> parser(parserInput);

  ParserResult parserResult;
  BOOST_CHECK(parser.next(parserResult).is(ParserResult::Type::Identifier, "multi"));
  BOOST_CHECK(parser.next(parserResult).is(ParserResult::Type::Char, ";"));
  BOOST_CHECK(parser.next(parserResult).is(ParserResult::Type::Identifier, "multiple"));
  BOOST_CHECK(parser.next(parserResult).is(ParserResult::Type::Identifier, "lines"));
  BOOST_CHECK(parser.next(parserResult).is(ParserResult::Type::Integer, 1ull));
  BOOST_CHECK(parser.next(parserResult).is(ParserResult::Type::Integer, 3ull));
  BOOST_CHECK(!parser.next(parserResult).operator bool());
  BOOST_CHECK(!parser.next(parserResult).operator bool());
  BOOST_CHECK(!parser.next(parserResult).operator bool());
}

BOOST_AUTO_TEST_CASE(Parser_Comments4)
{
  std::istringstream in(
    "nocomment;\n"
    "// single line\r\n"
    "// single line\r\n"
    "2\n"
  );

  ParserInput<2> parserInput(in);
  Parser<2, 3> parser(parserInput);

  ParserResult parserResult;
  BOOST_CHECK(parser.next(parserResult).is(ParserResult::Type::Identifier, "nocomment"));
  BOOST_CHECK(parser.next(parserResult).is(ParserResult::Type::Char, ";"));
  BOOST_CHECK(parser.next(parserResult).is(ParserResult::Type::Integer, 2ull));
  BOOST_CHECK(!parser.next(parserResult).operator bool());
  BOOST_CHECK(!parser.next(parserResult).operator bool());
  BOOST_CHECK(!parser.next(parserResult).operator bool());
}

BOOST_AUTO_TEST_CASE(Parser_Comments5)
{
  std::istringstream in(
    "nocomment;\n"
    "/* multi\n"
    "line */\n"
    "// single line\r\n"
    "// single line\r\n"
    "/* multi\n"
    "line */\n"
    "2\n"
  );

  ParserInput<2> parserInput(in);
  Parser<2, 3> parser(parserInput);

  ParserResult parserResult;
  BOOST_CHECK(parser.next(parserResult).is(ParserResult::Type::Identifier, "nocomment"));
  BOOST_CHECK(parser.next(parserResult).is(ParserResult::Type::Char, ";"));
  BOOST_CHECK(parser.next(parserResult).is(ParserResult::Type::Integer, 2ull));
  BOOST_CHECK(!parser.next(parserResult).operator bool());
  BOOST_CHECK(!parser.next(parserResult).operator bool());
  BOOST_CHECK(!parser.next(parserResult).operator bool());
}


BOOST_AUTO_TEST_CASE(Parser_Integers)
{
  std::istringstream in(
    "4711;123456789\n"
    "007-32767"
  );

  ParserInput<2> parserInput(in);
  Parser<2,3> parser(parserInput);

  ParserResult parserResult;
  BOOST_CHECK(parser.next(parserResult).is(ParserResult::Type::Integer, 4711ull));
  BOOST_CHECK(parser.next(parserResult).is(ParserResult::Type::Char, ";"));
  BOOST_CHECK(parser.next(parserResult).is(ParserResult::Type::Integer, 123456789ull));
  BOOST_CHECK(parser.next(parserResult).is(ParserResult::Type::Integer, 7ull));
  BOOST_CHECK_EQUAL(parserResult.text(), "007");
  BOOST_CHECK(parser.next(parserResult).is(ParserResult::Type::Char, "-"));
  BOOST_CHECK(parser.next(parserResult).is(ParserResult::Type::Integer, 32767));
}

#endif
