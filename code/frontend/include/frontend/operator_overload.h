#ifndef BASED_FRONTEND_OPERATOR_OVERLOAD_H
#define BASED_FRONTEND_OPERATOR_OVERLOAD_H

#include <hlir/hlir.h>
#include <source/source_span.h>

namespace benson
{

  class Frontend;

  class Unary_operator_overload
  {
  public:
    virtual ~Unary_operator_overload() = default;

    virtual bool matches(hlir::Type *operand_type) const = 0;

    virtual hlir::Type *result_type(hlir::Type *operand_type) const = 0;

    virtual hlir::Operand compile(
      Frontend &ctx,
      hlir::Operand operand,
      Source_span location
    ) const = 0;
  };

  class Binary_operator_overload
  {
  public:
    virtual ~Binary_operator_overload() = default;

    virtual hlir::Type *lhs_type() const = 0;

    virtual hlir::Type *rhs_type() const = 0;

    virtual hlir::Type *result_type() const = 0;

    virtual hlir::Operand compile(
      Frontend &ctx,
      hlir::Operand lhs,
      hlir::Operand rhs,
      Source_span location
    ) const = 0;
  };

} // namespace benson

#endif
