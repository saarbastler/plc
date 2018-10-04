#ifndef _INCLUDE_PLC_SIMULATOR_H_
#define _INCLUDE_PLC_SIMULATOR_H_

#include <exception>
#include <cstdarg>
#include <array>
#include <vector>

#include "Stack.h"

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

class PlcSimulatorException : public std::exception
{
public:
  PlcSimulatorException(const char *format, ...)
  {
    char buffer[1000];

    std::va_list list;
    va_start(list, format);
    vsnprintf_s(buffer, sizeof(buffer) / sizeof(buffer[0]), format, list);
    va_end(list);

    message = buffer;
  }

  virtual char const* what() const
  {
    return message.c_str();
  }

private:

  std::string message;
};

namespace plc
{
  enum class Instruction
  {
    ReadInput, ReadOutput, ReadFlag, ReadMonoflop,
    WriteOuput, WriteFlag, WriteMonoflop,
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
      throw PlcSimulatorException("index %d for type %d out out bounds (%d)"
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
      switch (operation.instruction)
      {
      case plc::Instruction::ReadInput:
        stack.push(io(IOType::Input, operation.argument));
        break;
      case plc::Instruction::ReadOutput:
        stack.push(io(IOType::Output, operation.argument));
        break;
      case plc::Instruction::ReadFlag:
        stack.push(io(IOType::Flag, operation.argument));
        break;
      case plc::Instruction::ReadMonoflop:
        stack.push(io(IOType::Monoflop, operation.argument));
        break;
      case plc::Instruction::WriteOuput:
        io(IOType::Output, operation.argument) = stack.pop();
        break;
      case plc::Instruction::WriteFlag:
        io(IOType::Flag, operation.argument) = stack.pop();
        break;
      case plc::Instruction::WriteMonoflop:
        io(IOType::Monoflop, operation.argument) = stack.pop();
        break;
      case plc::Instruction::OperationAnd:
        stack.push(stack.pop() & stack.pop());
        break;
      case plc::Instruction::OperationOr:
        stack.push(stack.pop() | stack.pop());
        break;
      case plc::Instruction::OperationNot:
        stack.push(!stack.pop());
        break;
      default:
        std::cout << "UNDEFINED: " << int(operation.instruction) << std::endl;
      }
    }

    if (stack.size() != 1)
      throw PlcSimulatorException("internal Error, stack size= %d", stack.size());
    return stack.pop();
  }

private:

  std::array<std::vector<std::unique_ptr<IO>>, 4> processImage;
};

#endif // !_INCLUDE_PLC_SIMULATOR_H_

