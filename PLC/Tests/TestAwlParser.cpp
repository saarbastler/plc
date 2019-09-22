#ifdef PARSER_TESTS

#include <boost/test/unit_test.hpp>

#include "../PlcAwlParser.h"

using VariableType = Variable<awl::Expression>;
using Instr = awl::Expression::Instruction;
using Cat = Var::Category;

class AwlParserMock : public PlcAWLParser<2, 3>
{
public:

  AwlParserMock(Parser<2, 3>& parser, AwlAst& awlAst) : PlcAWLParser(parser, awlAst) {}

  void mockParseInstruction(const std::string& name)
  {
    parseInstruction(name);
  }
};

void testParseInstruction(const std::string& varname, const Cat category, const std::string& instructionText, const Instr instruction)
{
  std::istringstream in(varname + ";");
  ParserInput<2> parserInput(in);
  Parser<2, 3> parser(parserInput);
  AwlAst awlAst;
  AwlParserMock awlParser(parser, awlAst);

  awlAst.addVariable(VariableType(varname, category, 0));

  awlParser.mockParseInstruction(instructionText);

  BOOST_REQUIRE_EQUAL(awlAst.expressions().size(), 1);
  BOOST_CHECK_EQUAL(awlAst[0].instruction(), instruction);

  BOOST_CHECK_EQUAL(awlAst[0].argument(), &awlAst.getVariable(varname));
}

BOOST_AUTO_TEST_CASE(AwlParser_ReadInput)
{
  testParseInstruction("input0", Cat::Input, "read", Instr::Read);
}

BOOST_AUTO_TEST_CASE(AwlParser_ReadOutput)
{
  testParseInstruction("relais", Cat::Output, "read", Instr::Read);
}

BOOST_AUTO_TEST_CASE(AwlParser_ReadTimer)
{
  testParseInstruction("timer2", Cat::Monoflop, "read", Instr::Read);
}

BOOST_AUTO_TEST_CASE(AwlParser_ReadFlag)
{
  testParseInstruction("flag32", Cat::Flag, "read", Instr::Read);
}

BOOST_AUTO_TEST_CASE(AwlParser_WriteInput)
{
  BOOST_CHECK_THROW(testParseInstruction("input0", Cat::Input, "Write", Instr::Write), ParserException);
}

BOOST_AUTO_TEST_CASE(AwlParser_WriteOutput)
{
  testParseInstruction("relais", Cat::Output, "Write", Instr::Write);
}

BOOST_AUTO_TEST_CASE(AwlParser_WriteTimer)
{
  BOOST_CHECK_THROW(testParseInstruction("timer2", Cat::Monoflop, "Write", Instr::Write), ParserException);
}

BOOST_AUTO_TEST_CASE(AwlParser_WriteFlag)
{
  testParseInstruction("flag32", Cat::Flag, "Write", Instr::Write);
}

BOOST_AUTO_TEST_CASE(AwlParser_SetInput)
{
  BOOST_CHECK_THROW(testParseInstruction("input0", Cat::Input, "Set", Instr::Set), ParserException);
}

BOOST_AUTO_TEST_CASE(AwlParser_SetOutput)
{
  testParseInstruction("relais", Cat::Output, "Set", Instr::Set);
}

BOOST_AUTO_TEST_CASE(AwlParser_SetTimer)
{
  testParseInstruction("timer2", Cat::Monoflop, "Set", Instr::Set);
}

BOOST_AUTO_TEST_CASE(AwlParser_SetFlag)
{
  testParseInstruction("flag32", Cat::Flag, "Set", Instr::Set);
}

BOOST_AUTO_TEST_CASE(AwlParser_ResetInput)
{
  BOOST_CHECK_THROW(testParseInstruction("input0", Cat::Input, "Reset", Instr::Reset), ParserException);
}

BOOST_AUTO_TEST_CASE(AwlParser_ResetOutput)
{
  testParseInstruction("relais", Cat::Output, "Reset", Instr::Reset);
}

BOOST_AUTO_TEST_CASE(AwlParser_ResetTimer)
{
  testParseInstruction("timer2", Cat::Monoflop, "Reset", Instr::Reset);
}

BOOST_AUTO_TEST_CASE(AwlParser_ResetFlag)
{
  testParseInstruction("flag32", Cat::Flag, "Reset", Instr::Reset);
}

BOOST_AUTO_TEST_CASE(AwlParser_AndInput)
{
  testParseInstruction("input0", Cat::Input, "And", Instr::And);
}

BOOST_AUTO_TEST_CASE(AwlParser_AndOutput)
{
  testParseInstruction("relais", Cat::Output, "And", Instr::And);
}

BOOST_AUTO_TEST_CASE(AwlParser_AndTimer)
{
  testParseInstruction("timer2", Cat::Monoflop, "And", Instr::And);
}

BOOST_AUTO_TEST_CASE(AwlParser_AndFlag)
{
  testParseInstruction("flag32", Cat::Flag, "And", Instr::And);
}

BOOST_AUTO_TEST_CASE(AwlParser_OrInput)
{
  testParseInstruction("input0", Cat::Input, "Or", Instr::Or);
}

BOOST_AUTO_TEST_CASE(AwlParser_OrOutput)
{
  testParseInstruction("relais", Cat::Output, "Or", Instr::Or);
}

BOOST_AUTO_TEST_CASE(AwlParser_OrTimer)
{
  testParseInstruction("timer2", Cat::Monoflop, "Or", Instr::Or);
}

BOOST_AUTO_TEST_CASE(AwlParser_OrFlag)
{
  testParseInstruction("flag32", Cat::Flag, "Or", Instr::Or);
}

BOOST_AUTO_TEST_CASE(AwlParser_NotInput)
{
  testParseInstruction("input0", Cat::Input, "Not", Instr::Not);
}

BOOST_AUTO_TEST_CASE(AwlParser_NotOutput)
{
  testParseInstruction("relais", Cat::Output, "Not", Instr::Not);
}

BOOST_AUTO_TEST_CASE(AwlParser_NotTimer)
{
  testParseInstruction("timer2", Cat::Monoflop, "Not", Instr::Not);
}

BOOST_AUTO_TEST_CASE(AwlParser_NotFlag)
{
  testParseInstruction("flag32", Cat::Flag, "Not", Instr::Not);
}

BOOST_AUTO_TEST_CASE(AwlParser_Program)
{
  std::istringstream in(
  R"~~~(Inputs:
  SensorLampeHaustuer=0,
  SensorWerkstattUnten=1,
  SensorWerkstattOben=2;

Outputs:
  LampenUnten=0,
  LampeMitte=1,
  LampenOben=2;

Timer:
  MonoflopUnten(5min)=0,
  MonoflopMitte(5min)=1,
  MonoflopOben(5min)=2;

  read SensorLampeHaustuer;
  and !SensorWerkstattUnten;  
  set LampenUnten;
  not SensorWerkstattOben;
  reset LampenOben;
)~~~");

  ParserInput<2> parserInput(in);
  Parser<2, 3> parser(parserInput);
  AwlAst awlAst;
  PlcAWLParser<2, 3> awlParser(parser, awlAst);

  awlParser.parse();

  BOOST_REQUIRE_EQUAL(awlAst.expressions().size(), 5);
  BOOST_CHECK_EQUAL(awlAst[0].instruction(), Instr::Read);
  BOOST_CHECK_EQUAL(awlAst[0].argument(), &awlAst.getVariable("SensorLampeHaustuer"));
  BOOST_CHECK_EQUAL(awlAst[0].inversion(), false);
  BOOST_CHECK_EQUAL(awlAst[1].instruction(), Instr::And);
  BOOST_CHECK_EQUAL(awlAst[1].argument(), &awlAst.getVariable("SensorWerkstattUnten"));
  BOOST_CHECK_EQUAL(awlAst[1].inversion(), true);
  BOOST_CHECK_EQUAL(awlAst[2].instruction(), Instr::Set);
  BOOST_CHECK_EQUAL(awlAst[2].argument(), &awlAst.getVariable("LampenUnten"));
  BOOST_CHECK_EQUAL(awlAst[2].inversion(), false);
  BOOST_CHECK_EQUAL(awlAst[3].instruction(), Instr::Not);
  BOOST_CHECK_EQUAL(awlAst[3].argument(), &awlAst.getVariable("SensorWerkstattOben"));
  BOOST_CHECK_EQUAL(awlAst[3].inversion(), false);
  BOOST_CHECK_EQUAL(awlAst[4].instruction(), Instr::Reset);
  BOOST_CHECK_EQUAL(awlAst[4].argument(), &awlAst.getVariable("LampenOben"));
  BOOST_CHECK_EQUAL(awlAst[4].inversion(), false);
}

#endif
