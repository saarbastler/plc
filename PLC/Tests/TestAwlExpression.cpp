#ifdef PARSER_TESTS

#include <boost/test/unit_test.hpp>

#include <AwlExpression.h>

using Instr = awl::Expression::Instruction;
using Cat = Var::Category;

BOOST_AUTO_TEST_CASE(AwlExpression_ArgumentMandatory)
{
  BOOST_CHECK(awl::Expression::argumentMandatory(Instr::Read));
  BOOST_CHECK(awl::Expression::argumentMandatory(Instr::Write));
  BOOST_CHECK(awl::Expression::argumentMandatory(Instr::Set));
  BOOST_CHECK(awl::Expression::argumentMandatory(Instr::Reset));
  BOOST_CHECK(!awl::Expression::argumentMandatory(Instr::And));
  BOOST_CHECK(!awl::Expression::argumentMandatory(Instr::Or));
  BOOST_CHECK(!awl::Expression::argumentMandatory(Instr::Not));
}

BOOST_AUTO_TEST_CASE(AwlExpression_ArgumentOptional)
{
  BOOST_CHECK(awl::Expression::argumentOptional(Instr::Read));
  BOOST_CHECK(awl::Expression::argumentOptional(Instr::Write));
  BOOST_CHECK(awl::Expression::argumentOptional(Instr::Set));
  BOOST_CHECK(awl::Expression::argumentOptional(Instr::Reset));
  BOOST_CHECK(awl::Expression::argumentOptional(Instr::And));
  BOOST_CHECK(awl::Expression::argumentOptional(Instr::Or));
  BOOST_CHECK(awl::Expression::argumentOptional(Instr::Not));
}

BOOST_AUTO_TEST_CASE(AwlExpression_AllowedInstructionCategoryRead)
{
  BOOST_CHECK(awl::Expression::allowedInstructionArgumentCategory(Instr::Read, Cat::Input ));
  BOOST_CHECK(awl::Expression::allowedInstructionArgumentCategory(Instr::Read, Cat::Output));
  BOOST_CHECK(awl::Expression::allowedInstructionArgumentCategory(Instr::Read, Cat::Monoflop));
  BOOST_CHECK(awl::Expression::allowedInstructionArgumentCategory(Instr::Read, Cat::Flag));
}

BOOST_AUTO_TEST_CASE(AwlExpression_AllowedInstructionCategoryWrite)
{
  BOOST_CHECK(!awl::Expression::allowedInstructionArgumentCategory(Instr::Write, Cat::Input));
  BOOST_CHECK(awl::Expression::allowedInstructionArgumentCategory(Instr::Write, Cat::Output));
  BOOST_CHECK(!awl::Expression::allowedInstructionArgumentCategory(Instr::Write, Cat::Monoflop));
  BOOST_CHECK(awl::Expression::allowedInstructionArgumentCategory(Instr::Write, Cat::Flag));
}

BOOST_AUTO_TEST_CASE(AwlExpression_AllowedInstructionCategorySet)
{
  BOOST_CHECK(!awl::Expression::allowedInstructionArgumentCategory(Instr::Set, Cat::Input));
  BOOST_CHECK(awl::Expression::allowedInstructionArgumentCategory(Instr::Set, Cat::Output));
  BOOST_CHECK(awl::Expression::allowedInstructionArgumentCategory(Instr::Set, Cat::Monoflop));
  BOOST_CHECK(awl::Expression::allowedInstructionArgumentCategory(Instr::Set, Cat::Flag));
}

BOOST_AUTO_TEST_CASE(AwlExpression_AllowedInstructionCategoryReset)
{
  BOOST_CHECK(!awl::Expression::allowedInstructionArgumentCategory(Instr::Reset, Cat::Input));
  BOOST_CHECK(awl::Expression::allowedInstructionArgumentCategory(Instr::Reset, Cat::Output));
  BOOST_CHECK(awl::Expression::allowedInstructionArgumentCategory(Instr::Reset, Cat::Monoflop));
  BOOST_CHECK(awl::Expression::allowedInstructionArgumentCategory(Instr::Reset, Cat::Flag));
}

BOOST_AUTO_TEST_CASE(AwlExpression_AllowedInstructionCategoryAnd)
{
  BOOST_CHECK(awl::Expression::allowedInstructionArgumentCategory(Instr::And, Cat::Input));
  BOOST_CHECK(awl::Expression::allowedInstructionArgumentCategory(Instr::And, Cat::Output));
  BOOST_CHECK(awl::Expression::allowedInstructionArgumentCategory(Instr::And, Cat::Monoflop));
  BOOST_CHECK(awl::Expression::allowedInstructionArgumentCategory(Instr::And, Cat::Flag));
}

BOOST_AUTO_TEST_CASE(AwlExpression_AllowedInstructionCategoryOr)
{
  BOOST_CHECK(awl::Expression::allowedInstructionArgumentCategory(Instr::Or, Cat::Input));
  BOOST_CHECK(awl::Expression::allowedInstructionArgumentCategory(Instr::Or, Cat::Output));
  BOOST_CHECK(awl::Expression::allowedInstructionArgumentCategory(Instr::Or, Cat::Monoflop));
  BOOST_CHECK(awl::Expression::allowedInstructionArgumentCategory(Instr::Or, Cat::Flag));
}

BOOST_AUTO_TEST_CASE(AwlExpression_AllowedInstructionCategoryNot)
{
  BOOST_CHECK(awl::Expression::allowedInstructionArgumentCategory(Instr::Not, Cat::Input));
  BOOST_CHECK(awl::Expression::allowedInstructionArgumentCategory(Instr::Not, Cat::Output));
  BOOST_CHECK(awl::Expression::allowedInstructionArgumentCategory(Instr::Not, Cat::Monoflop));
  BOOST_CHECK(awl::Expression::allowedInstructionArgumentCategory(Instr::Not, Cat::Flag));
}


#endif
