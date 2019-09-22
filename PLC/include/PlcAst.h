#ifndef _INCLUDE_PLC_AST_H_
#define _INCLUDE_PLC_AST_H_

#include <PlcAbstractSyntaxTree.h>

#include "PlcExpression.h"

class PlcAst : public PlcAbstractSyntaxTree<plc::Expression>
{
public:

  using Op = plc::Expression::Operator;

  const plc::Expression resolveDependencies(const std::string& name, std::unordered_map<std::string, unsigned> *toSkip = nullptr) const
  {
    const VariableType& variable = getVariable(name);

    if (!variable.expression().operator bool())
      throw PlcAstException("Expression %s does not exist.", name.c_str());

    plc::Expression all(*variable.expression());

    resolveDependencies(all, toSkip);

    all.pullupFirstTerm();

    return all;
  }

protected:

  void resolveDependencies(plc::Expression& expression, std::unordered_map<std::string, unsigned> *toSkip) const
  {
    for (auto it = expression.terms_.begin(); it != expression.terms_.end(); it++)
    {
      bool found = true;

      switch (it->type())
      {
      case plc::Term::Type::Expression:
        resolveDependencies(*it->expression().get(), toSkip);
        break;

      case plc::Term::Type::Identifier:
        const plc::Expression *pExpression = it->variable()->expression().get();
        if (toSkip && pExpression)
        {
          auto skipFound = toSkip->find(it->variable()->name());
          if (skipFound == toSkip->end())
          {
            (*toSkip)[it->variable()->name()] = 1;
          }
          else
          {
            skipFound->second++;
            found = false;
          }
        }

        if (found && pExpression)
        {
          if (pExpression->terms().size() == 1)
          {
            plc::Term copyTerm(pExpression->terms()[0]);

            plc::Expression::Operator op = (it->variable()->category() == Var::Category::Monoflop) ? Op::Timer : Op::None;

            plc::Expression *expression = new plc::Expression(copyTerm, op, pExpression->id());
            expression->setVariable(pExpression->variable());
            it->setExpression(expression);
          }
          else
          {
            std::unique_ptr<plc::Expression> dep(new plc::Expression(pExpression->op(), pExpression->terms()));
            plc::Term term(dep);

            plc::Expression *expression = nullptr;

            if (it->variable()->category() == Var::Category::Monoflop)
              expression = new plc::Expression(term, Op::Timer, it->variable()->index());
            else
              expression = new plc::Expression(term);

            expression->setVariable(it->variable());
            it->setExpression(expression);
          }

        }
        break;
      }
    }
  }
};

#endif // _INCLUDE_PLC_AST_H_


