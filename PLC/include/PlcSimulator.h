#ifndef _INCLUDE_PLC_SIMULATOR_H_
#define _INCLUDE_PLC_SIMULATOR_H_

#include <array>
#include <vector>

#include "Stack.h"
#include "PlcException.h"

class IO
{
public:
  
  virtual operator bool() const
  {
    return value_;
  }

  virtual void operator =(bool value)
  {
    value_ = value;
  }

private:
  bool value_ = 0;
};

namespace plc
{
  enum class Instruction
  {
    ReadOutput, ReadFlag, ReadTimer, ReadInput,
    WriteOutput, WriteFlag, 
    SetOutput, SetFlag, SetTimer, 
    ResetOutput, ResetFlag, ResetTimer,
    OperationAnd, OperationOr, OperationNot
  };

  struct Operation
  {
    Instruction instruction;
    unsigned argument;
  };
}

class PlcSimulator
{
public:

  enum class IOType
  {
    Input, Output, Flag, Monoflop
  };

  PlcSimulator(unsigned inputs, unsigned outputs, unsigned flags, unsigned monoflops)
  {
    processImage[unsigned(IOType::Input)].resize(inputs);
    processImage[unsigned(IOType::Output)].resize(outputs);
    processImage[unsigned(IOType::Flag)].resize(flags);
    processImage[unsigned(IOType::Monoflop)].resize(monoflops);
  }

  IO& io(IOType type, unsigned index)
  {
    std::vector<std::unique_ptr<IO>>& ios = processImage[unsigned(type)];

    if (index >= ios.size())
      throw PlcException("index %d for type %d out out bounds (%d)"
        , index, unsigned(type), ios.size());

    std::unique_ptr<IO>& io = ios[index];
    if (!io)
    {
      // Factory!
      io.reset(new IO());
    }

    return *io;
  }

  void resetAll()
  {
    for (auto& it : processImage)
      for (auto& io : it)
        if (io)
          *io = false;
  }

  template<unsigned STACKSIZE>
  bool execute(const std::vector<plc::Operation>& instructions)
  {

    Stack<bool, STACKSIZE> stack;
    for (const plc::Operation& operation : instructions)
    {
      bool outValue = false;
      switch (operation.instruction)
      {
      case plc::Instruction::ReadInput:
        outValue = io(IOType::Input, operation.argument);
        stack.push(outValue);
        break;
      case plc::Instruction::ReadOutput:
        outValue= io(IOType::Output, operation.argument);
        stack.push(outValue);
        break;
      case plc::Instruction::ReadFlag:
        outValue= io(IOType::Flag, operation.argument);
        stack.push(outValue);
        break;
      case plc::Instruction::ReadMonoflop:
        outValue= io(IOType::Monoflop, operation.argument);
        stack.push(outValue);
        break;
      case plc::Instruction::WriteOuput:
        outValue = stack.pop();
        io(IOType::Output, operation.argument) = outValue;
        break;
      case plc::Instruction::WriteFlag:
        outValue = stack.pop();
        io(IOType::Flag, operation.argument) = outValue;
        break;
      //case plc::Instruction::WriteMonoflop:
      //  outValue = stack.pop();
      //  io(IOType::Monoflop, operation.argument) = outValue;
      //  break;
      case plc::Instruction::SetOuput:
        outValue = stack.pop();
        if (outValue)
          io(IOType::Output, operation.argument) = outValue;
        break;
      case plc::Instruction::SetFlag:
        outValue = stack.pop();
        if (outValue)
          io(IOType::Flag, operation.argument) = outValue;
        break;
      case plc::Instruction::SetTimer:
        outValue = stack.pop();
        if (outValue)
          io(IOType::Monoflop, operation.argument) = outValue;
        break;
      case plc::Instruction::ResetOuput:
        outValue = stack.pop();
        if (outValue)
          io(IOType::Output, operation.argument) = !outValue;
        break;
      case plc::Instruction::ResetFlag:
        outValue = stack.pop();
        if (outValue)
          io(IOType::Flag, operation.argument) = !outValue;
        break;
      case plc::Instruction::ResetTimer:
        outValue = stack.pop();
        if (outValue)
          io(IOType::Monoflop, operation.argument) = !outValue;
        break;
      case plc::Instruction::OperationAnd:
        outValue = stack.pop() & stack.pop();
        stack.push(outValue);
        break;
      case plc::Instruction::OperationOr:
        outValue = stack.pop() | stack.pop();
        stack.push(outValue);
        break;
      case plc::Instruction::OperationNot:
        outValue = !stack.pop();
        stack.push(outValue);
        break;
      default:
        std::cout << "UNDEFINED: " << int(operation.instruction) << std::endl;
      }
    }

    if (stack.size() != 1)
      throw PlcException("internal Error, stack size= %d", stack.size());

    return stack.pop();
  }

private:

  std::array<std::vector<std::unique_ptr<IO>>, 4> processImage;
};

#endif // !_INCLUDE_PLC_SIMULATOR_H_

