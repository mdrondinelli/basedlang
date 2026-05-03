#ifndef BASED_FRONTEND_OPERATOR_OVERLOAD_H
#define BASED_FRONTEND_OPERATOR_OVERLOAD_H

#include <ir/hlir.h>
#include <source/source_span.h>

namespace benson
{

  class Frontend;

  class Unary_operator_overload
  {
  public:
    virtual ~Unary_operator_overload() = default;

    virtual bool matches(ir::Type *operand_type) const = 0;

    virtual ir::Type *result_type(ir::Type *operand_type) const = 0;

    virtual ir::Operand
    compile(Frontend &ctx, ir::Operand operand, Source_span location) const = 0;
  };

  class Binary_operator_overload
  {
  public:
    virtual ~Binary_operator_overload() = default;

    virtual ir::Type *lhs_type() const = 0;

    virtual ir::Type *rhs_type() const = 0;

    virtual ir::Type *result_type() const = 0;

    virtual ir::Operand compile(
      Frontend &ctx,
      ir::Operand lhs,
      ir::Operand rhs,
      Source_span location
    ) const = 0;
  };

} // namespace benson

#endif
