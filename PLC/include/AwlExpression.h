#ifndef _INCLUDE_AWL_EXPRESSION_H_
#define _INCLUDE_AWL_EXPRESSION_H_

#include <array>
#include <ostream>

#include <Variable.h>

namespace awl
{

  class Expression
  {
  public:
    using VariableType = Variable<Expression>;

    enum class Instruction
    {
      Read,
      Write,
      Set,
      Reset,
      And,
      Or,
      Not
    };

    Expression(Instruction instruction, const VariableType *argument, bool inversion) 
      : instruction_(instruction), argument_(argument), inversion_(inversion) {}

    Instruction instruction() const 
    {
      return instruction_;
    }

    const VariableType *argument() const
    {
      return argument_;
    }

    const bool inversion() const
    {
      return inversion_;
    }

    static bool argumentMandatory(Instruction instruction)
    {
      static const std::array<bool, 7> instructionHasMandatoryArgument{true, true, true, true, false, false, false};

      return instructionHasMandatoryArgument[int(instruction)];
    }

    static bool argumentOptional(Instruction instruction)
    {
      static const std::array<bool, 7> instructionHasOptionalArgument{ true, true, true, true, true, true, true };

      return instructionHasOptionalArgument[int(instruction)];
    }

    static bool allowedInstructionArgumentCategory(Instruction instruction, Var::Category category)
    {
      static constexpr unsigned BitInput = 1 << unsigned(Var::Category::Input);
      static constexpr unsigned BitOutput = 1 << unsigned(Var::Category::Output);
      static constexpr unsigned BitTimer = 1 << unsigned(Var::Category::Monoflop);
      static constexpr unsigned BitFlag = 1 << unsigned(Var::Category::Flag);

      static const std::array<unsigned, 7> instructionArgumentCategoryBits
      { BitInput | BitOutput | BitTimer | BitFlag, BitOutput | BitFlag, BitOutput | BitTimer | BitFlag, BitOutput | BitTimer | BitFlag,
      BitInput | BitOutput | BitTimer | BitFlag ,BitInput | BitOutput | BitTimer | BitFlag ,BitInput | BitOutput | BitTimer | BitFlag };

      return instructionArgumentCategoryBits[int(instruction)] & (1 << unsigned(category));
    }

    friend std::ostream& operator << (std::ostream& out, Instruction instruction)
    {
      static const std::array<const char *, 7> instructionText{ "Read", "Write", "Set", "Reset", "And", "Or", "Not" };

      out << instructionText[int(instruction)];

      return out;
    }

  protected:

    Instruction instruction_;
    bool inversion_;
    const VariableType *argument_;
  };

}

#endif // !_INCLUDE_AWL_EXPRESSION_H_

