#ifndef _INCLUDE_AWL_AST_H_
#define _INCLUDE_AWL_AST_H_

#include <vector>
#include <utility>
#include <PlcAbstractSyntaxTree.h>

#include <AwlExpression.h>

class AwlAst : public PlcAbstractSyntaxTree<awl::Expression>
{
public:

  template<class... _Valty>
  void addExpression(_Valty&&... val)
  {
    expressions_.emplace_back(std::forward<_Valty>(val)...);
  }

  const std::vector<awl::Expression>& expressions() const
  {
    return expressions_;
  }

  const awl::Expression& operator [] (std::size_t index) const
  {
    return expressions_[index];
  }

protected:

  std::vector<awl::Expression> expressions_;
};

#endif // !_INCLUDE_AWL_AST_H_

