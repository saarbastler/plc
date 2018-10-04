#ifndef _INCLUDE_PLC_COMPILER_H_
#define _INCLUDE_PLC_COMPILER_H_

#include <functional>

#include "PlcAst.h"
#include "PlcSimulator.h"

namespace plc
{
  Instruction readInstruction(Variable::Type type)
  {
    switch (type)
    {
    case Variable::Type::Input:     return Instruction::ReadInput;
    case Variable::Type::Output:    return Instruction::ReadOutput;
    case Variable::Type::Monoflop:  return Instruction::ReadMonoflop;
    default:
      throw PlcAstException("undefined Variable Type: %d", int(type));
    }
  }

  using Emitter = std::function<void(Instruction,unsigned)>;

  void compile(const PlcAst& plcAst, const Expression& expression, Emitter emitter)
  {
    bool firstTerm = true;
    for (const plc::Term& term : expression.terms())
    {
      if (term.type() == Term::Type::Identifier)
      {
        const Variable& variable = plcAst.getVariable(term.identifier());

        emitter(readInstruction(variable.type()), variable.index());
        if (term.unary() == Term::Unary::Not)
          emitter(Instruction::OperationNot, 0);
      }
      else if (term.type() == Term::Type::Expression)
      {
        compile(plcAst, *term.expression(), emitter);
      }
      else
        throw PlcAstException("empty Term");

      if (firstTerm)
        firstTerm = false;
      else
        emitter(expression.op() == Expression::Operator::And ? Instruction::OperationAnd : Instruction::OperationOr, 0);
    }
  }

  void compile(const PlcAst& plcAst, const Expression& expression, std::vector<Operation>& instructions)
  {
    compile(plcAst, expression, [&instructions](plc::Instruction instruction, unsigned argument) 
    {
      instructions.emplace_back(Operation{ instruction, argument });
    });
  }

};

#endif // !_INCLUDE_PLC_COMPILER_H_

