#ifdef PARSER_TESTS

#include <ostream>

#include <boost/test/unit_test.hpp>

#include <AwlCompiler.h>

using V = Variable<awl::Expression>;
using Cat = Var::Category;
using Instr = awl::Expression::Instruction;

namespace plc
{
  std::ostream& boost_test_print_type(std::ostream& out, const plc::Instruction& instr)
  {
    switch (instr)
    {
    case plc::Instruction::ReadOutput:
      out << "ReadOutput";
      break;
    case plc::Instruction::ReadFlag:
      out << "ReadFlag";
      break;
    case plc::Instruction::ReadTimer:
      out << "ReadTimer";
      break;
    case plc::Instruction::ReadInput:
      out << "ReadInput";
      break;
    case plc::Instruction::WriteOutput:
      out << "WriteOutput";
      break;
    case plc::Instruction::WriteFlag:
      out << "WriteFlag";
      break;
    case plc::Instruction::SetOutput:
      out << "SetOutput";
      break;
    case plc::Instruction::SetFlag:
      out << "SetFlag";
      break;
    case plc::Instruction::SetTimer:
      out << "SetTimer";
      break;
    case plc::Instruction::ResetOutput:
      out << "ResetOutput";
      break;
    case plc::Instruction::ResetFlag:
      out << "ResetFlag";
      break;
    case plc::Instruction::ResetTimer:
      out << "ResetTimer";
      break;
    case plc::Instruction::OperationAnd:
      out << "OperationAnd";
      break;
    case plc::Instruction::OperationOr:
      out << "OperationOr";
      break;
    case plc::Instruction::OperationNot:
      out << "OperationNot";
      break;
    default:
      out << "<<UNDEFINED>>";
    }

    return out;
  }
}

void prepareVars(AwlAst& awlAst)
{
  awlAst.addVariable(V("input0", Cat::Input, 0));
  awlAst.addVariable(V("input1", Cat::Input, 1));
  awlAst.addVariable(V("input2", Cat::Input, 2));
  awlAst.addVariable(V("input3", Cat::Input, 3));

  awlAst.addVariable(V("output0", Cat::Output, 0));
  awlAst.addVariable(V("output1", Cat::Output, 1));
  awlAst.addVariable(V("output2", Cat::Output, 2));
  awlAst.addVariable(V("output3", Cat::Output, 3));

  awlAst.addVariable(V("timer0", Cat::Monoflop, 0));
  awlAst.addVariable(V("timer1", Cat::Monoflop, 1));
  awlAst.addVariable(V("timer2", Cat::Monoflop, 2));
  awlAst.addVariable(V("timer3", Cat::Monoflop, 3));

  awlAst.addVariable(V("flag0", Cat::Flag, 0));
  awlAst.addVariable(V("flag1", Cat::Flag, 1));
  awlAst.addVariable(V("flag2", Cat::Flag, 2));
  awlAst.addVariable(V("flag3", Cat::Flag, 3));
}

BOOST_AUTO_TEST_CASE(AwlCompiler_ReadTypes)
{
  AwlAst awlAst;
  prepareVars(awlAst);

  awlAst.addExpression(Instr::Read, &awlAst.getVariable("input0"), false);
  awlAst.addExpression(Instr::Read, &awlAst.getVariable("output1"), false);
  awlAst.addExpression(Instr::Read, &awlAst.getVariable("timer2"), false);
  awlAst.addExpression(Instr::Read, &awlAst.getVariable("flag3"), false);
  awlAst.addExpression(Instr::And, nullptr, false);
  awlAst.addExpression(Instr::Or, nullptr, false);
  awlAst.addExpression(Instr::And, nullptr, false);
  awlAst.addExpression(Instr::Write, &awlAst.getVariable("output3"), false);

  std::vector<plc::Operation> ins;

  awl::compile(awlAst, ins);

  BOOST_REQUIRE_EQUAL(ins.size(), 8);
  BOOST_CHECK_EQUAL(ins[0].instruction, plc::Instruction::ReadInput);
  BOOST_CHECK_EQUAL(ins[0].argument, 0);
  BOOST_CHECK_EQUAL(ins[1].instruction, plc::Instruction::ReadOutput);
  BOOST_CHECK_EQUAL(ins[1].argument, 1);
  BOOST_CHECK_EQUAL(ins[2].instruction, plc::Instruction::ReadTimer);
  BOOST_CHECK_EQUAL(ins[2].argument, 2);
  BOOST_CHECK_EQUAL(ins[3].instruction, plc::Instruction::ReadFlag);
  BOOST_CHECK_EQUAL(ins[3].argument, 3);
  BOOST_CHECK_EQUAL(ins[4].instruction, plc::Instruction::OperationAnd);
  BOOST_CHECK_EQUAL(ins[4].argument, 0);
  BOOST_CHECK_EQUAL(ins[5].instruction, plc::Instruction::OperationOr);
  BOOST_CHECK_EQUAL(ins[5].argument, 0);
  BOOST_CHECK_EQUAL(ins[6].instruction, plc::Instruction::OperationAnd);
  BOOST_CHECK_EQUAL(ins[6].argument, 0);
  BOOST_CHECK_EQUAL(ins[7].instruction, plc::Instruction::WriteOutput);
  BOOST_CHECK_EQUAL(ins[7].argument, 3);
}

BOOST_AUTO_TEST_CASE(AwlCompiler_ReadInverted)
{
  AwlAst awlAst;
  prepareVars(awlAst);

  awlAst.addExpression(Instr::Read, &awlAst.getVariable("input3"), true);
  awlAst.addExpression(Instr::Write, &awlAst.getVariable("flag2"), false);

  std::vector<plc::Operation> ins;
  awl::compile(awlAst, ins);

  BOOST_REQUIRE_EQUAL(ins.size(), 3);
  BOOST_CHECK_EQUAL(ins[0].instruction, plc::Instruction::ReadInput);
  BOOST_CHECK_EQUAL(ins[0].argument, 3);
  BOOST_CHECK_EQUAL(ins[1].instruction, plc::Instruction::OperationNot);
  BOOST_CHECK_EQUAL(ins[1].argument, 0);
  BOOST_CHECK_EQUAL(ins[2].instruction, plc::Instruction::WriteFlag);
  BOOST_CHECK_EQUAL(ins[2].argument, 2);
}

BOOST_AUTO_TEST_CASE(AwlCompiler_SetInverted)
{
  AwlAst awlAst;
  prepareVars(awlAst);

  awlAst.addExpression(Instr::Read, &awlAst.getVariable("timer1"), false);
  awlAst.addExpression(Instr::Set, &awlAst.getVariable("timer2"), true);

  std::vector<plc::Operation> ins;
  awl::compile(awlAst, ins);

  BOOST_REQUIRE_EQUAL(ins.size(), 3);
  BOOST_CHECK_EQUAL(ins[0].instruction, plc::Instruction::ReadTimer);
  BOOST_CHECK_EQUAL(ins[0].argument, 1);
  BOOST_CHECK_EQUAL(ins[1].instruction, plc::Instruction::OperationNot);
  BOOST_CHECK_EQUAL(ins[1].argument, 0);
  BOOST_CHECK_EQUAL(ins[2].instruction, plc::Instruction::SetTimer);
  BOOST_CHECK_EQUAL(ins[2].argument, 2);
}

BOOST_AUTO_TEST_CASE(AwlCompiler_MixedOp)
{
  AwlAst awlAst;
  prepareVars(awlAst);

  awlAst.addExpression(Instr::Read, &awlAst.getVariable("input0"), true);
  awlAst.addExpression(Instr::And, &awlAst.getVariable("input1"), false);
  awlAst.addExpression(Instr::Or, &awlAst.getVariable("input2"), false);
  awlAst.addExpression(Instr::Or, &awlAst.getVariable("input3"), true);
  awlAst.addExpression(Instr::Reset, &awlAst.getVariable("flag2"), false);

  std::vector<plc::Operation> ins;
  awl::compile(awlAst, ins);

  BOOST_REQUIRE_EQUAL(ins.size(), 10);
  BOOST_CHECK_EQUAL(ins[0].instruction, plc::Instruction::ReadInput);
  BOOST_CHECK_EQUAL(ins[0].argument, 0);
  BOOST_CHECK_EQUAL(ins[1].instruction, plc::Instruction::OperationNot);
  BOOST_CHECK_EQUAL(ins[1].argument, 0);
  BOOST_CHECK_EQUAL(ins[2].instruction, plc::Instruction::ReadInput);
  BOOST_CHECK_EQUAL(ins[2].argument, 1);
  BOOST_CHECK_EQUAL(ins[3].instruction, plc::Instruction::OperationAnd);
  BOOST_CHECK_EQUAL(ins[3].argument, 0);
  BOOST_CHECK_EQUAL(ins[4].instruction, plc::Instruction::ReadInput);
  BOOST_CHECK_EQUAL(ins[4].argument, 2);
  BOOST_CHECK_EQUAL(ins[5].instruction, plc::Instruction::OperationOr);
  BOOST_CHECK_EQUAL(ins[5].argument, 0);
  BOOST_CHECK_EQUAL(ins[6].instruction, plc::Instruction::ReadInput);
  BOOST_CHECK_EQUAL(ins[6].argument, 3);
  BOOST_CHECK_EQUAL(ins[7].instruction, plc::Instruction::OperationNot);
  BOOST_CHECK_EQUAL(ins[7].argument, 0);
  BOOST_CHECK_EQUAL(ins[8].instruction, plc::Instruction::OperationOr);
  BOOST_CHECK_EQUAL(ins[8].argument, 0);
  BOOST_CHECK_EQUAL(ins[9].instruction, plc::Instruction::ResetFlag);
  BOOST_CHECK_EQUAL(ins[9].argument, 2);
}

BOOST_AUTO_TEST_CASE(AwlCompiler_MixedOut)
{
  AwlAst awlAst;
  prepareVars(awlAst);

  awlAst.addExpression(Instr::Read, &awlAst.getVariable("input0"), true);
  awlAst.addExpression(Instr::Set, &awlAst.getVariable("output0"), false);
  awlAst.addExpression(Instr::Read, &awlAst.getVariable("input1"), false);
  awlAst.addExpression(Instr::Reset, &awlAst.getVariable("output1"), true);
  awlAst.addExpression(Instr::Read, &awlAst.getVariable("input2"), true);
  awlAst.addExpression(Instr::Write, &awlAst.getVariable("output2"), false);

  std::vector<plc::Operation> ins;
  awl::compile(awlAst, ins);

  BOOST_REQUIRE_EQUAL(ins.size(), 9);
  BOOST_CHECK_EQUAL(ins[0].instruction, plc::Instruction::ReadInput);
  BOOST_CHECK_EQUAL(ins[0].argument, 0);
  BOOST_CHECK_EQUAL(ins[1].instruction, plc::Instruction::OperationNot);
  BOOST_CHECK_EQUAL(ins[1].argument, 0);
  BOOST_CHECK_EQUAL(ins[2].instruction, plc::Instruction::SetOutput);
  BOOST_CHECK_EQUAL(ins[2].argument, 0);
  BOOST_CHECK_EQUAL(ins[3].instruction, plc::Instruction::ReadInput);
  BOOST_CHECK_EQUAL(ins[3].argument, 1);
  BOOST_CHECK_EQUAL(ins[4].instruction, plc::Instruction::OperationNot);
  BOOST_CHECK_EQUAL(ins[4].argument, 0);
  BOOST_CHECK_EQUAL(ins[5].instruction, plc::Instruction::ResetOutput);
  BOOST_CHECK_EQUAL(ins[5].argument, 1);
  BOOST_CHECK_EQUAL(ins[6].instruction, plc::Instruction::ReadInput);
  BOOST_CHECK_EQUAL(ins[6].argument, 2);
  BOOST_CHECK_EQUAL(ins[7].instruction, plc::Instruction::OperationNot);
  BOOST_CHECK_EQUAL(ins[7].argument, 0);
  BOOST_CHECK_EQUAL(ins[8].instruction, plc::Instruction::WriteOutput);
  BOOST_CHECK_EQUAL(ins[8].argument, 2);
}

BOOST_AUTO_TEST_CASE(AwlCompiler_MixedTimer)
{
  AwlAst awlAst;
  prepareVars(awlAst);

  awlAst.addExpression(Instr::Read, &awlAst.getVariable("input0"), false);
  awlAst.addExpression(Instr::Set, &awlAst.getVariable("timer0"), true);
  awlAst.addExpression(Instr::Read, &awlAst.getVariable("input1"), false);
  awlAst.addExpression(Instr::Reset, &awlAst.getVariable("timer1"), true);
  awlAst.addExpression(Instr::Read, &awlAst.getVariable("input2"), false);
  awlAst.addExpression(Instr::Write, &awlAst.getVariable("flag2"), true);

  std::vector<plc::Operation> ins;
  awl::compile(awlAst, ins);

  BOOST_REQUIRE_EQUAL(ins.size(), 9);
  BOOST_CHECK_EQUAL(ins[0].instruction, plc::Instruction::ReadInput);
  BOOST_CHECK_EQUAL(ins[0].argument, 0);
  BOOST_CHECK_EQUAL(ins[1].instruction, plc::Instruction::OperationNot);
  BOOST_CHECK_EQUAL(ins[1].argument, 0);
  BOOST_CHECK_EQUAL(ins[2].instruction, plc::Instruction::SetTimer);
  BOOST_CHECK_EQUAL(ins[2].argument, 0);
  BOOST_CHECK_EQUAL(ins[3].instruction, plc::Instruction::ReadInput);
  BOOST_CHECK_EQUAL(ins[3].argument, 1);
  BOOST_CHECK_EQUAL(ins[4].instruction, plc::Instruction::OperationNot);
  BOOST_CHECK_EQUAL(ins[4].argument, 0);
  BOOST_CHECK_EQUAL(ins[5].instruction, plc::Instruction::ResetTimer);
  BOOST_CHECK_EQUAL(ins[5].argument, 1);
  BOOST_CHECK_EQUAL(ins[6].instruction, plc::Instruction::ReadInput);
  BOOST_CHECK_EQUAL(ins[6].argument, 2);
  BOOST_CHECK_EQUAL(ins[7].instruction, plc::Instruction::OperationNot);
  BOOST_CHECK_EQUAL(ins[7].argument, 0);
  BOOST_CHECK_EQUAL(ins[8].instruction, plc::Instruction::WriteFlag);
  BOOST_CHECK_EQUAL(ins[8].argument, 2);
}

#endif
