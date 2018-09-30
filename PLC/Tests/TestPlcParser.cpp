#ifdef PARSER_TESTS

#include <boost/test/unit_test.hpp>
#include "../PlcParser.h"

class PlcParserMock : public PlcParser<2, 3>
{
public:

  PlcParserMock(Parser<2, 3>& parser) : PlcParser(parser) {}

  void parseTermMock(plc::Term& term)
  {
    parseTerm(term);
  }

  static plc::Expression::Operator toOperatorMock(ParserResult& parserResult)
  {
    return toOperator(parserResult);
  }

  void parseExpressionMock(std::unique_ptr<plc::Expression>& expression)
  {
    parseExpression(expression);
  }

  void parseEquationMock(const std::string& left)
  {
    parseEquation(left);
  }

  void parseVariablesMock(Variable::Type type)
  {
    parseVariables(type);
  }

  VariableDescription& getVariableDescription()
  {
    return variableDescription;
  }

};

BOOST_AUTO_TEST_CASE(PlcParser_toOperator)
{
  std::string dummy;
  ParserResult parseResultInt(4711ull, dummy);

  BOOST_CHECK_THROW(PlcParserMock::toOperatorMock(parseResultInt), ParserException);

  ParserResult parserResult;
  BOOST_CHECK_THROW(PlcParserMock::toOperatorMock(parserResult), ParserException);

  parserResult = "text";
  BOOST_CHECK_THROW(PlcParserMock::toOperatorMock(parserResult), ParserException);

  parserResult = 'a';
  BOOST_CHECK_THROW(PlcParserMock::toOperatorMock(parserResult), ParserException);

  parserResult = '&';
  BOOST_CHECK(PlcParserMock::toOperatorMock(parserResult) == plc::Expression::Operator::And);
  parserResult = '|';
  BOOST_CHECK(PlcParserMock::toOperatorMock(parserResult) == plc::Expression::Operator::Or);
}

BOOST_AUTO_TEST_CASE(PlcParser_Term)
{
  std::istringstream in("abc !xyz 123");
  ParserInput<2> parserInput(in);
  Parser<2, 3> parser(parserInput);
  PlcParserMock plcParser(parser);

  plcParser.getVariableDescription().emplace("abc", Variable("abc", Variable::Type::Input, 0));
  plcParser.getVariableDescription().emplace("xyz", Variable("xyz", Variable::Type::Input, 1));

  plc::Term term;
  BOOST_CHECK(!term);

  plcParser.parseTermMock(term);
  BOOST_CHECK(term.type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(term.identifier(), "abc");
  BOOST_CHECK(term.unary() == plc::Term::Unary::None);

  plcParser.parseTermMock(term);
  BOOST_CHECK(term.type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(term.identifier(), "xyz");
  BOOST_CHECK(term.unary() == plc::Term::Unary::Not);

  BOOST_CHECK_THROW(plcParser.parseTermMock(term), ParserException);
}

BOOST_AUTO_TEST_CASE(PlcParser_Expression_Precedence1)
{
  std::istringstream in("a | b & c;");
  ParserInput<2> parserInput(in);
  Parser<2, 3> parser(parserInput);
  PlcParserMock plcParser(parser);

  plcParser.getVariableDescription().emplace("a", Variable("a", Variable::Type::Input, 0));
  plcParser.getVariableDescription().emplace("b", Variable("b", Variable::Type::Input, 1));
  plcParser.getVariableDescription().emplace("c", Variable("c", Variable::Type::Input, 2));

  std::unique_ptr<plc::Expression> expression(new plc::Expression());

  BOOST_CHECK(!*expression);

  plcParser.parseExpressionMock(expression);
  BOOST_CHECK(*expression);

  BOOST_CHECK(expression->left());
  BOOST_CHECK(expression->left().type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->left().identifier(), "a");
  BOOST_CHECK(expression->op() == plc::Expression::Operator::Or);
  BOOST_CHECK(expression->right());
  BOOST_CHECK(expression->right().expression());
  BOOST_CHECK(*expression->right().expression());
  BOOST_CHECK(expression->right().expression()->left());
  BOOST_CHECK(expression->right().expression()->left().type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->right().expression()->left().identifier(), "b");
  BOOST_CHECK(expression->right().expression()->op() == plc::Expression::Operator::And);
  BOOST_CHECK(expression->right().expression()->right().type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->right().expression()->right().identifier(), "c");
}

BOOST_AUTO_TEST_CASE(PlcParser_Expression_Precedence2)
{
  std::istringstream in("(a | b) & c;");
  ParserInput<2> parserInput(in);
  Parser<2, 3> parser(parserInput);
  PlcParserMock plcParser(parser);

  plcParser.getVariableDescription().emplace("a", Variable("a", Variable::Type::Input, 0));
  plcParser.getVariableDescription().emplace("b", Variable("b", Variable::Type::Input, 1));
  plcParser.getVariableDescription().emplace("c", Variable("c", Variable::Type::Input, 2));

  std::unique_ptr<plc::Expression> expression(new plc::Expression());

  BOOST_CHECK(!*expression);

  plcParser.parseExpressionMock(expression);
  BOOST_CHECK(*expression);

  BOOST_CHECK(expression->left());
  BOOST_CHECK(expression->left().expression());
  BOOST_CHECK(*expression->left().expression());
  BOOST_CHECK(expression->left().expression()->left());
  BOOST_CHECK(expression->left().expression()->left().type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->left().expression()->left().identifier(), "a");
  BOOST_CHECK(expression->left().expression()->op() == plc::Expression::Operator::Or);
  BOOST_CHECK(expression->left().expression()->right().type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->left().expression()->right().identifier(), "b");
  BOOST_CHECK(expression->op() == plc::Expression::Operator::And);
  BOOST_CHECK(expression->right());
  BOOST_CHECK(expression->right().type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->right().identifier(), "c");
}

BOOST_AUTO_TEST_CASE(PlcParser_Expression_Precedence3)
{
  std::istringstream in("a & b | c;");
  ParserInput<2> parserInput(in);
  Parser<2, 3> parser(parserInput);
  PlcParserMock plcParser(parser);

  plcParser.getVariableDescription().emplace("a", Variable("a", Variable::Type::Input, 0));
  plcParser.getVariableDescription().emplace("b", Variable("b", Variable::Type::Input, 1));
  plcParser.getVariableDescription().emplace("c", Variable("c", Variable::Type::Input, 2));

  std::unique_ptr<plc::Expression> expression(new plc::Expression());

  BOOST_CHECK(!*expression);

  plcParser.parseExpressionMock(expression);
  BOOST_CHECK(*expression);

  BOOST_CHECK(expression->left());
  BOOST_CHECK(expression->left().expression());
  BOOST_CHECK(*expression->left().expression());
  BOOST_CHECK(expression->left().expression()->left());
  BOOST_CHECK(expression->left().expression()->left().type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->left().expression()->left().identifier(), "a");
  BOOST_CHECK(expression->left().expression()->op() == plc::Expression::Operator::And);
  BOOST_CHECK(expression->left().expression()->right().type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->left().expression()->right().identifier(), "b");
  BOOST_CHECK(expression->op() == plc::Expression::Operator::Or);
  BOOST_CHECK(expression->right());
  BOOST_CHECK(expression->right().type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->right().identifier(), "c");
}

BOOST_AUTO_TEST_CASE(PlcParser_Expression_Precedence4)
{
  std::istringstream in("a | (b & c);");
  ParserInput<2> parserInput(in);
  Parser<2, 3> parser(parserInput);
  PlcParserMock plcParser(parser);

  plcParser.getVariableDescription().emplace("a", Variable("a", Variable::Type::Input, 0));
  plcParser.getVariableDescription().emplace("b", Variable("b", Variable::Type::Input, 1));
  plcParser.getVariableDescription().emplace("c", Variable("c", Variable::Type::Input, 2));

  std::unique_ptr<plc::Expression> expression(new plc::Expression());

  BOOST_CHECK(!*expression);

  plcParser.parseExpressionMock(expression);
  BOOST_CHECK(*expression);

  BOOST_CHECK(expression->left());
  BOOST_CHECK(expression->left().type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->left().identifier(), "a");
  BOOST_CHECK(expression->op() == plc::Expression::Operator::Or);
  BOOST_CHECK(expression->right());
  BOOST_CHECK(expression->right().expression());
  BOOST_CHECK(*expression->right().expression());
  BOOST_CHECK(expression->right().expression()->left());
  BOOST_CHECK(expression->right().expression()->left().type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->right().expression()->left().identifier(), "b");
  BOOST_CHECK(expression->right().expression()->op() == plc::Expression::Operator::And);
  BOOST_CHECK(expression->right().expression()->right().type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->right().expression()->right().identifier(), "c");
}

BOOST_AUTO_TEST_CASE(PlcParser_Expression_Precedence5)
{
  std::istringstream in("a | b & c & d;");
  ParserInput<2> parserInput(in);
  Parser<2, 3> parser(parserInput);
  PlcParserMock plcParser(parser);

  plcParser.getVariableDescription().emplace("a", Variable("a", Variable::Type::Input, 0));
  plcParser.getVariableDescription().emplace("b", Variable("b", Variable::Type::Input, 1));
  plcParser.getVariableDescription().emplace("c", Variable("c", Variable::Type::Input, 2));
  plcParser.getVariableDescription().emplace("d", Variable("d", Variable::Type::Input, 3));

  std::unique_ptr<plc::Expression> expression(new plc::Expression());

  BOOST_CHECK(!*expression);

  plcParser.parseExpressionMock(expression);
  BOOST_CHECK(*expression);

  BOOST_CHECK(expression->left());
  BOOST_CHECK(expression->left().type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->left().identifier(), "a");
  BOOST_CHECK(expression->op() == plc::Expression::Operator::Or);
  BOOST_CHECK(expression->right());
  BOOST_CHECK(expression->right().expression());
  BOOST_CHECK(*expression->right().expression());
  BOOST_CHECK(expression->right().expression()->left());
  BOOST_CHECK(expression->right().expression()->left().type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->right().expression()->left().identifier(), "b");
  BOOST_CHECK(expression->right().expression()->op() == plc::Expression::Operator::And);
  BOOST_CHECK(expression->right().expression()->right().type() == plc::Term::Type::Expression);
  BOOST_CHECK(expression->right().expression()->right().expression());
  BOOST_CHECK(*expression->right().expression()->right().expression());
  BOOST_CHECK(expression->right().expression()->right().expression()->left());
  BOOST_CHECK(expression->right().expression()->right().expression()->left().type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->right().expression()->right().expression()->left().identifier(), "c");
  BOOST_CHECK(expression->right().expression()->right().expression()->op() == plc::Expression::Operator::And);
  BOOST_CHECK(expression->right().expression()->right().expression()->right());
  BOOST_CHECK(expression->right().expression()->right().expression()->right().type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->right().expression()->right().expression()->right().identifier(), "d");
}

BOOST_AUTO_TEST_CASE(PlcParser_Expression_Precedence6)
{
  std::istringstream in("a | b & c | d;");
  ParserInput<2> parserInput(in);
  Parser<2, 3> parser(parserInput);
  PlcParserMock plcParser(parser);

  plcParser.getVariableDescription().emplace("a", Variable("a", Variable::Type::Input, 0));
  plcParser.getVariableDescription().emplace("b", Variable("b", Variable::Type::Input, 1));
  plcParser.getVariableDescription().emplace("c", Variable("c", Variable::Type::Input, 2));
  plcParser.getVariableDescription().emplace("d", Variable("d", Variable::Type::Input, 3));

  std::unique_ptr<plc::Expression> expression(new plc::Expression());

  BOOST_CHECK(!*expression);

  plcParser.parseExpressionMock(expression);
  BOOST_CHECK(*expression);

  BOOST_CHECK(expression->left());
  BOOST_CHECK(expression->left().type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->left().identifier(), "a");
  BOOST_CHECK(expression->op() == plc::Expression::Operator::Or);
  BOOST_CHECK(expression->right());
  BOOST_CHECK(expression->right().expression());
  BOOST_CHECK(*expression->right().expression());
  BOOST_CHECK(expression->right().expression()->left());
  BOOST_CHECK(expression->right().expression()->left().type() == plc::Term::Type::Expression);
  BOOST_CHECK(expression->right().expression()->left().expression());
  BOOST_CHECK(*expression->right().expression()->left().expression());
  BOOST_CHECK(expression->right().expression()->left().expression()->left());
  BOOST_CHECK(expression->right().expression()->left().expression()->left().type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->right().expression()->left().expression()->left().identifier(), "b");
  BOOST_CHECK(expression->right().expression()->left().expression()->op() == plc::Expression::Operator::And);
  BOOST_CHECK(expression->right().expression()->left().expression()->right().type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->right().expression()->left().expression()->right().identifier(), "c");
  BOOST_CHECK(expression->right().expression()->op() == plc::Expression::Operator::Or);
  BOOST_CHECK(expression->right().expression()->right().type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->right().expression()->right().identifier(), "d");
}


#endif

