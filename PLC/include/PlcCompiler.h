#ifndef _INCLUDE_PLC_COMPILER_H_
#define _INCLUDE_PLC_COMPILER_H_

#include <functional>

#include "PlcAst.h"
#include "PlcSimulator.h"
#include "AvrPlc.h"

namespace plc
{
  Instruction readInstruction(Variable::Type type)
  {
    switch (type)
    {
    case Variable::Type::Input:     return Instruction::ReadInput;
    case Variable::Type::Output:    return Instruction::ReadOutput;
    case Variable::Type::Monoflop:  return Instruction::ReadMonoflop;
    case Variable::Type::Flag:      return Instruction::ReadFlag;
    default:
      throw PlcAstException("undefined Variable Type: %d", int(type));
    }
  }

  Instruction writeInstruction(Variable::Type type)
  {
    switch (type)
    {
    case Variable::Type::Output:    return Instruction::WriteOuput;
    case Variable::Type::Monoflop:  return Instruction::WriteMonoflop;
    case Variable::Type::Flag:      return Instruction::WriteFlag;
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
        const Variable& variable = plcAst.getVariable(term.variable()->name());

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

  void compile(const PlcAst& plcAst, const Expression& expression, const Variable& variable, std::vector<Operation>& instructions)
  {
    compile(plcAst, expression, [&instructions](plc::Instruction instruction, unsigned argument) 
    {
      instructions.emplace_back(Operation{ instruction, argument });
    });

    instructions.emplace_back(Operation{ writeInstruction(variable.type()), variable.index() });
  }

  void compile(const PlcAst& plcAst, std::vector<Operation>& instructions)
  {
    for (auto it = plcAst.variableDescription().begin(); it != plcAst.variableDescription().end(); it++)
      if(it->second.expression().operator bool())
        plc::compile(plcAst, *it->second.expression().get(), it->second, instructions);
  }

  void avrArgument(int8_t avrOp, unsigned argument, std::vector<uint8_t>& avrplc)
  {
    if (argument >= avrplc::ARGUMENT_MAXIMUM)
      throw PlcException("argument %d out of bounds, max: %d", argument, avrplc::ARGUMENT_MAXIMUM);

    if (argument < avrplc::ARGUMENT_EXTENDED)
    {
      avrplc.emplace_back(uint8_t(avrOp + argument));
    }
    else
    {
      avrplc.emplace_back(uint8_t(avrOp | avrplc::ARGUMENT_EXTENDED));
      avrplc.emplace_back(uint8_t(argument - avrplc::ARGUMENT_EXTENDED));
    }
  }

  void translateAvr(const std::vector<Operation>& instructions, std::vector<uint8_t>& avrplc)
  {
    for (const Operation& operation : instructions)
    {
      switch (operation.instruction)
      {
      case plc::Instruction::ReadInput:
        avrArgument(avrplc::READ_INPUT, operation.argument, avrplc);
        break;
      case plc::Instruction::ReadOutput:
        avrArgument(avrplc::READ_OUTPUT, operation.argument, avrplc);
        break;
      case plc::Instruction::ReadFlag:
        avrArgument(avrplc::READ_FLAG, operation.argument, avrplc);
        break;
      case plc::Instruction::ReadMonoflop:
        avrArgument(avrplc::READ_MONOFLOP, operation.argument, avrplc);
        break;
      case plc::Instruction::WriteOuput:
        avrArgument(avrplc::WRITE_OUTPUT, operation.argument, avrplc);
        break;
      case plc::Instruction::WriteFlag:
        avrArgument(avrplc::WRITE_FLAG, operation.argument, avrplc);
        break;
      case plc::Instruction::WriteMonoflop:
        avrArgument(avrplc::WRITE_MONOFLOP, operation.argument, avrplc);
        break;
      case plc::Instruction::OperationAnd:
        avrArgument(avrplc::OPERATION, avrplc::AND, avrplc);
        break;
      case plc::Instruction::OperationOr:
        avrArgument(avrplc::OPERATION, avrplc::OR, avrplc);
        break;
      case plc::Instruction::OperationNot:
        avrArgument(avrplc::OPERATION, avrplc::NOT, avrplc);
        break;
      default:
        throw PlcException("undefined Instruction: %d", int(operation.instruction));
      }
    }

  }
};

#endif // !_INCLUDE_PLC_COMPILER_H_

