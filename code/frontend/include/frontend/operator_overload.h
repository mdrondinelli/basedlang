#ifndef BASED_FRONTEND_OPERATOR_OVERLOAD_H
#define BASED_FRONTEND_OPERATOR_OVERLOAD_H

#include <ir/hlir.h>

namespace benson::ir
{

  class Compilation_context;

  class Unary_operator_overload
  {
  public:
    virtual ~Unary_operator_overload() = default;

    virtual bool matches(Type *operand_type) const = 0;

    virtual Type *result_type(Type *operand_type) const = 0;

    virtual Operand
    compile(Compilation_context &ctx, Operand operand) const = 0;
  };

  class Binary_operator_overload
  {
  public:
    virtual ~Binary_operator_overload() = default;

    virtual Type *lhs_type() const = 0;

    virtual Type *rhs_type() const = 0;

    virtual Type *result_type() const = 0;

    virtual Operand
    compile(Compilation_context &ctx, Operand lhs, Operand rhs) const = 0;
  };

} // namespace benson::ir

#endif
