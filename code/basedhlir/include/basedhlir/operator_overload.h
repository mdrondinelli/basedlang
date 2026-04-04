#ifndef BASEDHLIR_OPERATOR_OVERLOAD_H
#define BASEDHLIR_OPERATOR_OVERLOAD_H

#include "constant_value.h"
#include "type.h"

namespace basedhlir
{

  class Unary_operator_overload
  {
  public:
    virtual ~Unary_operator_overload() = default;

    virtual bool matches(Type *operand_type) const = 0;

    virtual Type *result_type(Type *operand_type) const = 0;

    virtual Constant_value evaluate(Constant_value operand) const = 0;
  };

  class Binary_operator_overload
  {
  public:
    virtual ~Binary_operator_overload() = default;

    virtual Type *lhs_type() const = 0;

    virtual Type *rhs_type() const = 0;

    virtual Type *result_type() const = 0;

    virtual Constant_value
    evaluate(Constant_value lhs, Constant_value rhs) const = 0;
  };

} // namespace basedhlir

#endif
