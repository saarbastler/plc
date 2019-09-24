#ifndef _INCLUDE_AWL_COMPILER_H_
#define _INCLUDE_AWL_COMPILER_H_

#include <AwlAst.h>
#include <PlcSimulator.h>

#include <AwlExpression.h>


namespace awl
{
  class Expr2Instr
  {
    using ei = Expression::Instruction;
    using vc = Var::Category;
    using pi = plc::Instruction;

  public:

    static plc::Instruction instruction(ei instruction, vc category)
    {
      static const int matrix[1 + int(ei::Reset)][1 + int(vc::Flag)]
      { {int(pi::ReadInput), int(pi::ReadOutput), int(pi::ReadTimer), int(pi::ReadFlag)}  // Read
        ,{-1, int(pi::WriteOutput), -1, int(pi::WriteFlag)} // Write
        ,{-1, int(pi::SetOutput), int(pi::SetTimer), int(pi::SetFlag)} // Set
        ,{-1, int(pi::ResetOutput), int(pi::ResetTimer), int(pi::ResetFlag)} // Reset
      };

      int r = matrix[int(instruction)][int(category)];

      if (r < 0)
        throw PlcException("undefined combination Instruction %d Category %d", int(instruction), int(category));

      return static_cast<pi>(r);
    }
  };

  inline void compile(const Expression& expression, std::vector<plc::Operation>& instructions)
  {

    if (int(expression.instruction()) <= int(Expression::Instruction::Reset))
    {
      if (expression.instruction() != Expression::Instruction::Read && expression.inversion())
        instructions.emplace_back(plc::Operation{plc::Instruction::OperationNot, 0});

      instructions.emplace_back(plc::Operation{ Expr2Instr::instruction(expression.instruction(), expression.argument()->category()), expression.argument()->index() });

      if (expression.instruction() == Expression::Instruction::Read && expression.inversion())
        instructions.emplace_back(plc::Operation{ plc::Instruction::OperationNot, 0 });
    }
    else
    {
      if (expression.argument() != nullptr)
      {
        instructions.emplace_back(plc::Operation{ Expr2Instr::instruction(Expression::Instruction::Read, expression.argument()->category()), expression.argument()->index() });

        if (expression.inversion())
          instructions.emplace_back(plc::Operation{ plc::Instruction::OperationNot, 0 });
      }
      switch (expression.instruction())
      {
      case Expression::Instruction::And:
        instructions.emplace_back(plc::Operation{ plc::Instruction::OperationAnd, 0 });
        break;
      case Expression::Instruction::Or:
        instructions.emplace_back(plc::Operation{ plc::Instruction::OperationOr, 0 });
        break;
      case Expression::Instruction::Not:
        instructions.emplace_back(plc::Operation{ plc::Instruction::OperationNot, 0 });
        break;
      default:
        throw PlcException("undefined Instruction: %d", expression.instruction());
      }

    }
  }

  inline void compile(const AwlAst& awlAst, std::vector<plc::Operation>& instructions)
  {
    for (auto it = awlAst.expressions().begin(); it != awlAst.expressions().end(); ++it)
      compile(*it, instructions);
  }


}

#endif // !_INCLUDE_AWL_COMPILER_H_

