#ifdef PARSER_TESTS

#include <boost/test/unit_test.hpp>
#include "../PlcExpressionParser.h"

using VariableType = Variable<plc::Expression>;

class PlcExpressionParserMock : public PlcParser<2, 3, plc::Expression, PlcAst>
{
public:

  PlcExpressionParserMock(Parser<2, 3>& parser, PlcAst& plcAst) : PlcParser(parser, plcAst) {}

  void parseVariablesMock(Var::Category category)
  {
    parseVariables(category);
  }
};


BOOST_AUTO_TEST_CASE(PlcParser_Vars)
{
  std::istringstream in(": abc=0, xyz=1, min=2, no=3;");

  ParserInput<2> parserInput(in);
  Parser<2, 3> parser(parserInput);
  PlcAst plcAst;
  PlcExpressionParserMock plcParser(parser, plcAst);
  ParserResult pr;

  plcParser.parseVariablesMock(Var::Category::Input);

  BOOST_CHECK(parser.next(pr).type() == ParserResult::Type::Empty);
  BOOST_CHECK_EQUAL(plcAst.getVariable("abc").time(), 0);
  BOOST_CHECK_EQUAL(plcAst.getVariable("xyz").time(), 0);
  BOOST_CHECK_EQUAL(plcAst.getVariable("min").time(), 0);
  BOOST_CHECK_EQUAL(plcAst.getVariable("no").time(), 0);
}

BOOST_AUTO_TEST_CASE(PlcParser_Timer_Arg)
{
  std::istringstream in(": abc(10s)=0, xyz(1h)=1, min(10 min)=2, no=3;");

  ParserInput<2> parserInput(in);
  Parser<2, 3> parser(parserInput);
  PlcAst plcAst;
  PlcExpressionParserMock plcParser(parser, plcAst);
  ParserResult pr;

  plcParser.parseVariablesMock(Var::Category::Monoflop);

  BOOST_CHECK(parser.next(pr).type() == ParserResult::Type::Empty);
  BOOST_CHECK_EQUAL(plcAst.getVariable("abc").time(), 5);
  BOOST_CHECK_EQUAL(plcAst.getVariable("xyz").time(), 1800);
  BOOST_CHECK_EQUAL(plcAst.getVariable("min").time(), 300);
  BOOST_CHECK_EQUAL(plcAst.getVariable("no").time(), 30);
}

#endif
