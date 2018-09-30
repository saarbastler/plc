#ifdef PARSER_TESTS

#define BOOST_TEST_MODULE PARSER_TESTS

#include <boost/test/unit_test.hpp>
#include "../Stack.h"

BOOST_AUTO_TEST_CASE(Stack_Tests)
{
  Stack<char, 2> stack;

  BOOST_CHECK(stack.empty());
  BOOST_CHECK(!stack.full());
  BOOST_CHECK(!stack);

  stack.push('a');
  BOOST_CHECK(!stack.empty());
  BOOST_CHECK(!stack.full());
  BOOST_CHECK(stack);

  stack.push('b');
  BOOST_CHECK(!stack.empty());
  BOOST_CHECK(stack.full());
  BOOST_CHECK(stack);

  BOOST_CHECK_THROW(stack.push('c'), StackOverflowException);

  BOOST_CHECK_EQUAL(stack.pop(), 'b');
  BOOST_CHECK(!stack.empty());
  BOOST_CHECK(!stack.full());
  BOOST_CHECK(stack);

  BOOST_CHECK_EQUAL(stack.pop(), 'a');
  BOOST_CHECK(stack.empty());
  BOOST_CHECK(!stack.full());
  BOOST_CHECK(!stack);

  BOOST_CHECK_THROW(stack.pop(), StackEmptyException);
}

BOOST_AUTO_TEST_CASE(Stack_Test_Class)
{
  Stack<std::string, 2> stack;

  BOOST_CHECK(stack.empty());
  BOOST_CHECK(!stack.full());
  BOOST_CHECK(!stack);

  stack.push("Text1");
  BOOST_CHECK(!stack.empty());
  BOOST_CHECK(!stack.full());
  BOOST_CHECK(stack);

  std::string t("Text2");
  stack.push(t);
  BOOST_CHECK(t.empty());
  BOOST_CHECK(!stack.empty());
  BOOST_CHECK(stack.full());
  BOOST_CHECK(stack);

  BOOST_CHECK_THROW(stack.push("Text3"), StackOverflowException);

  BOOST_CHECK_EQUAL(stack.pop(), "Text2");
  BOOST_CHECK(!stack.empty());
  BOOST_CHECK(!stack.full());
  BOOST_CHECK(stack);

  BOOST_CHECK_EQUAL(stack.pop(), "Text1");
  BOOST_CHECK(stack.empty());
  BOOST_CHECK(!stack.full());
  BOOST_CHECK(!stack);

  BOOST_CHECK_THROW(stack.pop(), StackEmptyException);
}
#endif // PARSER_TESTS
