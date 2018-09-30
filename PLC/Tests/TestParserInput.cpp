#ifdef PARSER_TESTS

#include <boost/test/unit_test.hpp>
#include "../ParserInput.h"

#include <sstream>

BOOST_AUTO_TEST_CASE(ParserInput_nextChar)
{
  std::istringstream in("a b\tc");
  ParserInput<2> parserInput(in);

  BOOST_CHECK_EQUAL(parserInput.nextChar(), 'a');
  BOOST_CHECK_EQUAL(parserInput.nextChar(), ' ');
  BOOST_CHECK_EQUAL(parserInput.nextChar(), 'b');
  BOOST_CHECK_EQUAL(parserInput.nextChar(), '\t');
  BOOST_CHECK_EQUAL(parserInput.nextChar(), 'c');
  BOOST_CHECK_EQUAL(parserInput.nextChar(), '\0');
  BOOST_CHECK_EQUAL(parserInput.nextChar(), '\0');
  BOOST_CHECK_EQUAL(parserInput.nextChar(), '\0');
  BOOST_CHECK_EQUAL(parserInput.nextChar(), '\0');
  BOOST_CHECK_EQUAL(parserInput.nextChar(), '\0');
}

BOOST_AUTO_TEST_CASE(ParserInput_nextIgnoreBlank)
{
  std::istringstream in("a    b\t\t\tc de");
  ParserInput<2> parserInput(in);

  BOOST_CHECK_EQUAL(parserInput.nextIgnoreBlank(), 'a');
  BOOST_CHECK_EQUAL(parserInput.nextIgnoreBlank(), 'b');
  BOOST_CHECK_EQUAL(parserInput.nextIgnoreBlank(), 'c');
  BOOST_CHECK_EQUAL(parserInput.nextIgnoreBlank(), 'd');
  BOOST_CHECK_EQUAL(parserInput.nextIgnoreBlank(), 'e');
  BOOST_CHECK_EQUAL(parserInput.nextIgnoreBlank(), '\0');
  BOOST_CHECK_EQUAL(parserInput.nextIgnoreBlank(), '\0');
}

BOOST_AUTO_TEST_CASE(ParserInput_push)
{
  std::istringstream in("a bc");
  ParserInput<2> parserInput(in);

  BOOST_CHECK_EQUAL(parserInput.nextChar(), 'a');
  BOOST_CHECK_EQUAL(parserInput.nextChar(), ' ');
  BOOST_CHECK_EQUAL(parserInput.nextChar(), 'b');
  parserInput.pushChar('b');
  BOOST_CHECK_EQUAL(parserInput.nextChar(), 'b');
  BOOST_CHECK_EQUAL(parserInput.nextChar(), 'c');
  parserInput.pushChar('c');
  parserInput.pushChar('b');
  BOOST_CHECK_EQUAL(parserInput.nextChar(), 'b');
  BOOST_CHECK_EQUAL(parserInput.nextChar(), 'c');
  BOOST_CHECK_EQUAL(parserInput.nextChar(), '\0');
  BOOST_CHECK_EQUAL(parserInput.nextChar(), '\0');
}
#endif
