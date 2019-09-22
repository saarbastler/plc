#ifdef PARSER_TESTS

#include <boost/test/unit_test.hpp>
#include "../PlcExpressionParser.h"

using VariableType = Variable<plc::Expression>;

class PlcParserMock : public PlcExpressionParser<2, 3>
{
public:

  PlcParserMock(Parser<2, 3>& parser, PlcAst& plcAst) : PlcExpressionParser(parser, plcAst) {}

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

  plcAst.addVariable(VariableType("abc", Var::Category::Input, 0));
  plcAst.addVariable(VariableType("xyz", Var::Category::Input, 1));

  plc::Term term;
  BOOST_CHECK(!term);

  plcParser.parseTermMock(term);
  BOOST_CHECK(term.type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(term.variable()->name(), "abc");
  BOOST_CHECK(term.unary() == plc::Term::Unary::None);

  plcParser.parseTermMock(term);
  BOOST_CHECK(term.type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(term.variable()->name(), "xyz");
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

  plcAst.addVariable(VariableType("a", Var::Category::Input, 0));
  plcAst.addVariable(VariableType("b", Var::Category::Input, 1));
  plcAst.addVariable(VariableType("c", Var::Category::Input, 2));

  std::unique_ptr<plc::Expression> expression(new plc::Expression());

  BOOST_CHECK(!*expression);

  plcParser.parseExpressionMock(expression);
  BOOST_CHECK(*expression);

  BOOST_CHECK_EQUAL(expression->terms().size(), 2);
  BOOST_CHECK(expression->terms()[0]);
  BOOST_CHECK(expression->terms()[0].type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->terms()[0].variable()->name(), "a");
  BOOST_CHECK(expression->op() == plc::Expression::Operator::Or);
  BOOST_CHECK(expression->terms()[1]);
  BOOST_CHECK(expression->terms()[1].expression());
  BOOST_CHECK(*expression->terms()[1].expression());
  BOOST_CHECK_EQUAL(expression->terms()[1].expression()->terms().size(),2);
  BOOST_CHECK(expression->terms()[1].expression()->terms()[0]);
  BOOST_CHECK(expression->terms()[1].expression()->terms()[0].type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->terms()[1].expression()->terms()[0].variable()->name(), "b");
  BOOST_CHECK(expression->terms()[1].expression()->op() == plc::Expression::Operator::And);
  BOOST_CHECK(expression->terms()[1].expression()->terms()[1].type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->terms()[1].expression()->terms()[1].variable()->name(), "c");
}

BOOST_AUTO_TEST_CASE(PlcParser_Expression_Precedence2)
{
  std::istringstream in("(a | b) & c;");
  ParserInput<2> parserInput(in);
  Parser<2, 3> parser(parserInput);
  PlcAst plcAst;
  PlcParserMock plcParser(parser, plcAst);

  plcAst.addVariable(VariableType("a", Var::Category::Input, 0));
  plcAst.addVariable(VariableType("b", Var::Category::Input, 1));
  plcAst.addVariable(VariableType("c", Var::Category::Input, 2));

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
  BOOST_CHECK_EQUAL(expression->terms()[0].expression()->terms()[0].variable()->name(), "a");
  BOOST_CHECK(expression->terms()[0].expression()->op() == plc::Expression::Operator::Or);
  BOOST_CHECK(expression->terms()[0].expression()->terms()[1].type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->terms()[0].expression()->terms()[1].variable()->name(), "b");
  BOOST_CHECK(expression->op() == plc::Expression::Operator::And);
  BOOST_CHECK(expression->terms()[1].type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->terms()[1].variable()->name(), "c");
}

BOOST_AUTO_TEST_CASE(PlcParser_Expression_Precedence3)
{
  std::istringstream in("a & b | c;");
  ParserInput<2> parserInput(in);
  Parser<2, 3> parser(parserInput);
  PlcAst plcAst;
  PlcParserMock plcParser(parser, plcAst);

  plcAst.addVariable(VariableType("a", Var::Category::Input, 0));
  plcAst.addVariable(VariableType("b", Var::Category::Input, 1));
  plcAst.addVariable(VariableType("c", Var::Category::Input, 2));

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
  BOOST_CHECK_EQUAL(expression->terms()[0].expression()->terms()[0].variable()->name(), "a");
  BOOST_CHECK(expression->terms()[0].expression()->op() == plc::Expression::Operator::And);
  BOOST_CHECK(expression->terms()[0].expression()->terms()[1].type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->terms()[0].expression()->terms()[1].variable()->name(), "b");
  BOOST_CHECK(expression->op() == plc::Expression::Operator::Or);
  BOOST_CHECK(expression->terms()[1].type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->terms()[1].variable()->name(), "c");
}

BOOST_AUTO_TEST_CASE(PlcParser_Expression_Precedence4)
{
  std::istringstream in("a | (b & c);");
  ParserInput<2> parserInput(in);
  Parser<2, 3> parser(parserInput);
  PlcAst plcAst;
  PlcParserMock plcParser(parser, plcAst);

  plcAst.addVariable(VariableType("a", Var::Category::Input, 0));
  plcAst.addVariable(VariableType("b", Var::Category::Input, 1));
  plcAst.addVariable(VariableType("c", Var::Category::Input, 2));

  std::unique_ptr<plc::Expression> expression(new plc::Expression());

  BOOST_CHECK(!*expression);

  plcParser.parseExpressionMock(expression);
  BOOST_CHECK(*expression);

  BOOST_CHECK_EQUAL(expression->terms().size(), 2);
  BOOST_CHECK(expression->terms()[0]);
  BOOST_CHECK(expression->terms()[0].type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->terms()[0].variable()->name(), "a");
  BOOST_CHECK(expression->op() == plc::Expression::Operator::Or);
  BOOST_CHECK(expression->terms()[1]);
  BOOST_CHECK(expression->terms()[1].expression());
  BOOST_CHECK(*expression->terms()[1].expression());
  BOOST_CHECK_EQUAL(expression->terms()[1].expression()->terms().size(), 2);
  BOOST_CHECK(expression->terms()[1].expression()->terms()[0]);
  BOOST_CHECK(expression->terms()[1].expression()->terms()[0].type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->terms()[1].expression()->terms()[0].variable()->name(), "b");
  BOOST_CHECK(expression->terms()[1].expression()->op() == plc::Expression::Operator::And);
  BOOST_CHECK(expression->terms()[1].expression()->terms()[1].type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->terms()[1].expression()->terms()[1].variable()->name(), "c");
}

BOOST_AUTO_TEST_CASE(PlcParser_Expression_Precedence5)
{
  std::istringstream in("a | b & c & d;");
  ParserInput<2> parserInput(in);
  Parser<2, 3> parser(parserInput);
  PlcAst plcAst;
  PlcParserMock plcParser(parser, plcAst);

  plcAst.addVariable(VariableType("a", Var::Category::Input, 0));
  plcAst.addVariable(VariableType("b", Var::Category::Input, 1));
  plcAst.addVariable(VariableType("c", Var::Category::Input, 2));
  plcAst.addVariable(VariableType("d", Var::Category::Input, 3));

  std::unique_ptr<plc::Expression> expression(new plc::Expression());

  BOOST_CHECK(!*expression);

  plcParser.parseExpressionMock(expression);
  BOOST_CHECK(*expression);

  BOOST_CHECK_EQUAL(expression->terms().size(), 2);
  BOOST_CHECK(expression->terms()[0]);
  BOOST_CHECK(expression->terms()[0].type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->terms()[0].variable()->name(), "a");
  BOOST_CHECK(expression->op() == plc::Expression::Operator::Or);
  BOOST_CHECK(expression->terms()[1]);
  BOOST_CHECK(expression->terms()[1].expression());
  BOOST_CHECK(*expression->terms()[1].expression());
  BOOST_CHECK_EQUAL(expression->terms()[1].expression()->terms().size(), 3);
  BOOST_CHECK(expression->terms()[1].expression()->terms()[0]);
  BOOST_CHECK(expression->terms()[1].expression()->terms()[0].type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->terms()[1].expression()->terms()[0].variable()->name(), "b");
  BOOST_CHECK(expression->terms()[1].expression()->op() == plc::Expression::Operator::And);
  BOOST_CHECK(expression->terms()[1].expression()->terms()[1].type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->terms()[1].expression()->terms()[1].variable()->name(), "c");
  BOOST_CHECK(expression->terms()[1].expression()->terms()[2].type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->terms()[1].expression()->terms()[2].variable()->name(), "d");
}

BOOST_AUTO_TEST_CASE(PlcParser_Expression_Precedence6)
{
  std::istringstream in("a | b & c | d;");
  ParserInput<2> parserInput(in);
  Parser<2, 3> parser(parserInput);
  PlcAst plcAst;
  PlcParserMock plcParser(parser, plcAst);

  plcAst.addVariable(VariableType("a", Var::Category::Input, 0));
  plcAst.addVariable(VariableType("b", Var::Category::Input, 1));
  plcAst.addVariable(VariableType("c", Var::Category::Input, 2));
  plcAst.addVariable(VariableType("d", Var::Category::Input, 3));

  std::unique_ptr<plc::Expression> expression(new plc::Expression());

  BOOST_CHECK(!*expression);

  plcParser.parseExpressionMock(expression);
  BOOST_CHECK(*expression);

  BOOST_CHECK_EQUAL(expression->terms().size(), 2);
  BOOST_CHECK(expression->terms()[0]);
  BOOST_CHECK(expression->terms()[0].type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->terms()[0].variable()->name(), "a");
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
  BOOST_CHECK_EQUAL(expression->terms()[1].expression()->terms()[0].expression()->terms()[0].variable()->name(), "b");
  BOOST_CHECK(expression->terms()[1].expression()->terms()[0].expression()->op() == plc::Expression::Operator::And);
  BOOST_CHECK(expression->terms()[1].expression()->terms()[0].expression()->terms()[1].type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->terms()[1].expression()->terms()[0].expression()->terms()[1].variable()->name(), "c");

  BOOST_CHECK(expression->terms()[1].expression()->terms()[1]);
  BOOST_CHECK(expression->terms()[1].expression()->terms()[1].type() == plc::Term::Type::Identifier);
  BOOST_CHECK_EQUAL(expression->terms()[1].expression()->terms()[1].variable()->name(), "d");
}

BOOST_AUTO_TEST_CASE(PlcParser_Expression_Levels1)
{
  std::istringstream in("a;");
  ParserInput<2> parserInput(in);
  Parser<2, 3> parser(parserInput);
  PlcAst plcAst;
  PlcParserMock plcParser(parser, plcAst);

  plcAst.addVariable(VariableType("a", Var::Category::Input, 0));

  std::unique_ptr<plc::Expression> expression(new plc::Expression());

  plcParser.parseExpressionMock(expression);
  BOOST_CHECK(*expression);
  BOOST_CHECK_EQUAL(expression->countLevels(), 1u);
}

BOOST_AUTO_TEST_CASE(PlcParser_Expression_Levels2)
{
  std::istringstream in("a | b | c;");
  ParserInput<2> parserInput(in);
  Parser<2, 3> parser(parserInput);
  PlcAst plcAst;
  PlcParserMock plcParser(parser, plcAst);

  plcAst.addVariable(VariableType("a", Var::Category::Input, 0));
  plcAst.addVariable(VariableType("b", Var::Category::Input, 1));
  plcAst.addVariable(VariableType("c", Var::Category::Input, 2));

  std::unique_ptr<plc::Expression> expression(new plc::Expression());

  BOOST_CHECK(!*expression);

  plcParser.parseExpressionMock(expression);
  BOOST_CHECK(*expression);
  BOOST_CHECK_EQUAL(expression->countLevels(), 1u);
}

BOOST_AUTO_TEST_CASE(PlcParser_Expression_Levels3)
{
  std::istringstream in("a | b & c;");
  ParserInput<2> parserInput(in);
  Parser<2, 3> parser(parserInput);
  PlcAst plcAst;
  PlcParserMock plcParser(parser, plcAst);

  plcAst.addVariable(VariableType("a", Var::Category::Input, 0));
  plcAst.addVariable(VariableType("b", Var::Category::Input, 1));
  plcAst.addVariable(VariableType("c", Var::Category::Input, 2));

  std::unique_ptr<plc::Expression> expression(new plc::Expression());

  BOOST_CHECK(!*expression);

  plcParser.parseExpressionMock(expression);
  BOOST_CHECK(*expression);
  BOOST_CHECK_EQUAL(expression->countLevels(), 2u);
}

BOOST_AUTO_TEST_CASE(PlcParser_Expression_Levels4)
{
  std::istringstream in("a | b & c | d;");
  ParserInput<2> parserInput(in);
  Parser<2, 3> parser(parserInput);
  PlcAst plcAst;
  PlcParserMock plcParser(parser, plcAst);

  plcAst.addVariable(VariableType("a", Var::Category::Input, 0));
  plcAst.addVariable(VariableType("b", Var::Category::Input, 1));
  plcAst.addVariable(VariableType("c", Var::Category::Input, 2));
  plcAst.addVariable(VariableType("d", Var::Category::Input, 3));

  std::unique_ptr<plc::Expression> expression(new plc::Expression());

  BOOST_CHECK(!*expression);

  plcParser.parseExpressionMock(expression);
  BOOST_CHECK(*expression);
  BOOST_CHECK_EQUAL(expression->countLevels(), 3u);
}

BOOST_AUTO_TEST_CASE(PlcParser_Expression_Levels5)
{
  std::istringstream in("(a | b) & (c | d) & (e | f);");
  ParserInput<2> parserInput(in);
  Parser<2, 3> parser(parserInput);
  PlcAst plcAst;
  PlcParserMock plcParser(parser, plcAst);

  plcAst.addVariable(VariableType("a", Var::Category::Input, 0));
  plcAst.addVariable(VariableType("b", Var::Category::Input, 1));
  plcAst.addVariable(VariableType("c", Var::Category::Input, 2));
  plcAst.addVariable(VariableType("d", Var::Category::Input, 3));
  plcAst.addVariable(VariableType("e", Var::Category::Input, 4));
  plcAst.addVariable(VariableType("f", Var::Category::Input, 5));

  std::unique_ptr<plc::Expression> expression(new plc::Expression());

  BOOST_CHECK(!*expression);

  plcParser.parseExpressionMock(expression);
  BOOST_CHECK(*expression);
  BOOST_CHECK_EQUAL(expression->countLevels(), 2u);
}


#endif

