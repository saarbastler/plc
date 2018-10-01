#ifdef PARSER_TESTS

#include <boost/test/unit_test.hpp>
#include "../PlcParser.h"

class PlcParserMock : public PlcParser<2, 3>
{
public:

  PlcParserMock(Parser<2, 3>& parser, PlcAst& plcAst) : PlcParser(parser, plcAst) {}

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
  PlcAst plcAst;
  PlcParserMock plcParser(parser, plcAst);

  plcAst.addVariable(Variable("abc", Variable::Type::Input, 0));
  plcAst.addVariable(Variable("xyz", Variable::Type::Input, 1));

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
  PlcAst plcAst;
  PlcParserMock plcParser(parser, plcAst);

  plcAst.addVariable(Variable("a", Variable::Type::Input, 0));
  plcAst.addVariable(Variable("b", Variable::Type::Input, 1));
  plcAst.addVariable(Variable("c", Variable::Type::Input, 2));

  std::unique_ptr<plc::Expression> expression(new plc::Expression());

  BOOST_CHECK(!*expression);

  plcParser.parseExpressionMock(expression);
  BOOST_CHECK(*expression);

  BOOST_CHECK_EQUAL(expression->terms().size(), 2);
  BOOST_CHECK(expression->terms()[0]);
  BOOST_CHECK(expression->terms()[0].type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->terms()[0].identifier(), "a");
  BOOST_CHECK(expression->op() == plc::Expression::Operator::Or);
  BOOST_CHECK(expression->terms()[1]);
  BOOST_CHECK(expression->terms()[1].expression());
  BOOST_CHECK(*expression->terms()[1].expression());
  BOOST_CHECK_EQUAL(expression->terms()[1].expression()->terms().size(),2);
  BOOST_CHECK(expression->terms()[1].expression()->terms()[0]);
  BOOST_CHECK(expression->terms()[1].expression()->terms()[0].type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->terms()[1].expression()->terms()[0].identifier(), "b");
  BOOST_CHECK(expression->terms()[1].expression()->op() == plc::Expression::Operator::And);
  BOOST_CHECK(expression->terms()[1].expression()->terms()[1].type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->terms()[1].expression()->terms()[1].identifier(), "c");
}

BOOST_AUTO_TEST_CASE(PlcParser_Expression_Precedence2)
{
  std::istringstream in("(a | b) & c;");
  ParserInput<2> parserInput(in);
  Parser<2, 3> parser(parserInput);
  PlcAst plcAst;
  PlcParserMock plcParser(parser, plcAst);

  plcAst.addVariable(Variable("a", Variable::Type::Input, 0));
  plcAst.addVariable(Variable("b", Variable::Type::Input, 1));
  plcAst.addVariable(Variable("c", Variable::Type::Input, 2));

  std::unique_ptr<plc::Expression> expression(new plc::Expression());

  BOOST_CHECK(!*expression);

  plcParser.parseExpressionMock(expression);
  BOOST_CHECK(*expression);

  BOOST_CHECK_EQUAL(expression->terms().size(), 2);
  BOOST_CHECK(expression->terms()[0]);
  BOOST_CHECK(expression->terms()[0].type() == plc::Term::Type::Expression);
  BOOST_CHECK(expression->terms()[0]);
  BOOST_CHECK(expression->terms()[0].expression());
  BOOST_CHECK(*expression->terms()[0].expression());
  BOOST_CHECK_EQUAL(expression->terms()[0].expression()->terms().size(), 2);
  BOOST_CHECK(expression->terms()[0].expression()->terms()[0]);
  BOOST_CHECK(expression->terms()[0].expression()->terms()[0].type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->terms()[0].expression()->terms()[0].identifier(), "a");
  BOOST_CHECK(expression->terms()[0].expression()->op() == plc::Expression::Operator::Or);
  BOOST_CHECK(expression->terms()[0].expression()->terms()[1].type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->terms()[0].expression()->terms()[1].identifier(), "b");
  BOOST_CHECK(expression->op() == plc::Expression::Operator::And);
  BOOST_CHECK(expression->terms()[1].type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->terms()[1].identifier(), "c");
}

BOOST_AUTO_TEST_CASE(PlcParser_Expression_Precedence3)
{
  std::istringstream in("a & b | c;");
  ParserInput<2> parserInput(in);
  Parser<2, 3> parser(parserInput);
  PlcAst plcAst;
  PlcParserMock plcParser(parser, plcAst);

  plcAst.addVariable(Variable("a", Variable::Type::Input, 0));
  plcAst.addVariable(Variable("b", Variable::Type::Input, 1));
  plcAst.addVariable(Variable("c", Variable::Type::Input, 2));

  std::unique_ptr<plc::Expression> expression(new plc::Expression());

  BOOST_CHECK(!*expression);

  plcParser.parseExpressionMock(expression);
  BOOST_CHECK(*expression);

  BOOST_CHECK_EQUAL(expression->terms().size(), 2);
  BOOST_CHECK(expression->terms()[0]);
  BOOST_CHECK(expression->terms()[0].type() == plc::Term::Type::Expression);
  BOOST_CHECK(expression->terms()[0].expression());
  BOOST_CHECK(*expression->terms()[0].expression());
  BOOST_CHECK_EQUAL(expression->terms()[0].expression()->terms().size(), 2);
  BOOST_CHECK(expression->terms()[0].expression()->terms()[0]);
  BOOST_CHECK(expression->terms()[0].expression()->terms()[0].type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->terms()[0].expression()->terms()[0].identifier(), "a");
  BOOST_CHECK(expression->terms()[0].expression()->op() == plc::Expression::Operator::And);
  BOOST_CHECK(expression->terms()[0].expression()->terms()[1].type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->terms()[0].expression()->terms()[1].identifier(), "b");
  BOOST_CHECK(expression->op() == plc::Expression::Operator::Or);
  BOOST_CHECK(expression->terms()[1].type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->terms()[1].identifier(), "c");
}

BOOST_AUTO_TEST_CASE(PlcParser_Expression_Precedence4)
{
  std::istringstream in("a | (b & c);");
  ParserInput<2> parserInput(in);
  Parser<2, 3> parser(parserInput);
  PlcAst plcAst;
  PlcParserMock plcParser(parser, plcAst);

  plcAst.addVariable(Variable("a", Variable::Type::Input, 0));
  plcAst.addVariable(Variable("b", Variable::Type::Input, 1));
  plcAst.addVariable(Variable("c", Variable::Type::Input, 2));

  std::unique_ptr<plc::Expression> expression(new plc::Expression());

  BOOST_CHECK(!*expression);

  plcParser.parseExpressionMock(expression);
  BOOST_CHECK(*expression);

  BOOST_CHECK_EQUAL(expression->terms().size(), 2);
  BOOST_CHECK(expression->terms()[0]);
  BOOST_CHECK(expression->terms()[0].type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->terms()[0].identifier(), "a");
  BOOST_CHECK(expression->op() == plc::Expression::Operator::Or);
  BOOST_CHECK(expression->terms()[1]);
  BOOST_CHECK(expression->terms()[1].expression());
  BOOST_CHECK(*expression->terms()[1].expression());
  BOOST_CHECK_EQUAL(expression->terms()[1].expression()->terms().size(), 2);
  BOOST_CHECK(expression->terms()[1].expression()->terms()[0]);
  BOOST_CHECK(expression->terms()[1].expression()->terms()[0].type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->terms()[1].expression()->terms()[0].identifier(), "b");
  BOOST_CHECK(expression->terms()[1].expression()->op() == plc::Expression::Operator::And);
  BOOST_CHECK(expression->terms()[1].expression()->terms()[1].type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->terms()[1].expression()->terms()[1].identifier(), "c");
}

BOOST_AUTO_TEST_CASE(PlcParser_Expression_Precedence5)
{
  std::istringstream in("a | b & c & d;");
  ParserInput<2> parserInput(in);
  Parser<2, 3> parser(parserInput);
  PlcAst plcAst;
  PlcParserMock plcParser(parser, plcAst);

  plcAst.addVariable(Variable("a", Variable::Type::Input, 0));
  plcAst.addVariable(Variable("b", Variable::Type::Input, 1));
  plcAst.addVariable(Variable("c", Variable::Type::Input, 2));
  plcAst.addVariable(Variable("d", Variable::Type::Input, 3));

  std::unique_ptr<plc::Expression> expression(new plc::Expression());

  BOOST_CHECK(!*expression);

  plcParser.parseExpressionMock(expression);
  BOOST_CHECK(*expression);

  BOOST_CHECK_EQUAL(expression->terms().size(), 2);
  BOOST_CHECK(expression->terms()[0]);
  BOOST_CHECK(expression->terms()[0].type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->terms()[0].identifier(), "a");
  BOOST_CHECK(expression->op() == plc::Expression::Operator::Or);
  BOOST_CHECK(expression->terms()[1]);
  BOOST_CHECK(expression->terms()[1].expression());
  BOOST_CHECK(*expression->terms()[1].expression());
  BOOST_CHECK_EQUAL(expression->terms()[1].expression()->terms().size(), 3);
  BOOST_CHECK(expression->terms()[1].expression()->terms()[0]);
  BOOST_CHECK(expression->terms()[1].expression()->terms()[0].type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->terms()[1].expression()->terms()[0].identifier(), "b");
  BOOST_CHECK(expression->terms()[1].expression()->op() == plc::Expression::Operator::And);
  BOOST_CHECK(expression->terms()[1].expression()->terms()[1].type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->terms()[1].expression()->terms()[1].identifier(), "c");
  BOOST_CHECK(expression->terms()[1].expression()->terms()[2].type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->terms()[1].expression()->terms()[2].identifier(), "d");
}

BOOST_AUTO_TEST_CASE(PlcParser_Expression_Precedence6)
{
  std::istringstream in("a | b & c | d;");
  ParserInput<2> parserInput(in);
  Parser<2, 3> parser(parserInput);
  PlcAst plcAst;
  PlcParserMock plcParser(parser, plcAst);

  plcAst.addVariable(Variable("a", Variable::Type::Input, 0));
  plcAst.addVariable(Variable("b", Variable::Type::Input, 1));
  plcAst.addVariable(Variable("c", Variable::Type::Input, 2));
  plcAst.addVariable(Variable("d", Variable::Type::Input, 3));

  std::unique_ptr<plc::Expression> expression(new plc::Expression());

  BOOST_CHECK(!*expression);

  plcParser.parseExpressionMock(expression);
  BOOST_CHECK(*expression);

  BOOST_CHECK_EQUAL(expression->terms().size(), 2);
  BOOST_CHECK(expression->terms()[0]);
  BOOST_CHECK(expression->terms()[0].type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->terms()[0].identifier(), "a");
  BOOST_CHECK(expression->op() == plc::Expression::Operator::Or);
  BOOST_CHECK(expression->terms()[1]);
  BOOST_CHECK(expression->terms()[1].expression());
  BOOST_CHECK(*expression->terms()[1].expression());
  BOOST_CHECK_EQUAL(expression->terms()[1].expression()->terms().size(), 2);

  BOOST_CHECK(expression->terms()[1].expression()->terms()[0]);
  BOOST_CHECK(expression->terms()[1].expression()->terms()[0].type() == plc::Term::Type::Expression);
  BOOST_CHECK(expression->terms()[1].expression()->terms()[0].expression());
  BOOST_CHECK(*expression->terms()[1].expression()->terms()[0].expression());
  BOOST_CHECK_EQUAL(expression->terms()[1].expression()->terms()[0].expression()->terms().size(),2);
  BOOST_CHECK_EQUAL(expression->terms()[1].expression()->terms()[0].expression()->terms()[0].identifier(), "b");
  BOOST_CHECK(expression->terms()[1].expression()->terms()[0].expression()->op() == plc::Expression::Operator::And);
  BOOST_CHECK(expression->terms()[1].expression()->terms()[0].expression()->terms()[1].type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->terms()[1].expression()->terms()[0].expression()->terms()[1].identifier(), "c");

  BOOST_CHECK(expression->terms()[1].expression()->terms()[1]);
  BOOST_CHECK(expression->terms()[1].expression()->terms()[1].type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->terms()[1].expression()->terms()[1].identifier(), "d");
}

#endif

