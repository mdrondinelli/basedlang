#ifndef BASEDHLIR_OPERATOR_FUNCTORS_H
#define BASEDHLIR_OPERATOR_FUNCTORS_H

#include <cassert>
#include <stdexcept>
#include <variant>

#include "ir/compile.h"
#include "primitive_type_traits.h"

namespace benson::ir
{

  template <
    typename OperandT,
    typename ResultT,
    typename InstructionT,
    typename Fn
  >
  class Primitive_unary_operator_overload: public Unary_operator_overload
  {
  public:
    explicit Primitive_unary_operator_overload(Type_pool *type_pool)
        : _operand_type{Primitive_type_traits<OperandT>::get(type_pool)},
          _result_type{Primitive_type_traits<ResultT>::get(type_pool)}
    {
    }

    bool matches(Type *operand_type) const override
    {
      return operand_type == _operand_type;
    }

    Type *result_type(Type *) const override
    {
      return _result_type;
    }

    Operand compile(Compilation_context &ctx, Operand operand) const override
    {
      if (auto const cv = std::get_if<Constant_value>(&operand))
      {
        return Operand{Constant_value{_fn(std::get<OperandT>(*cv))}};
      }
      else
      {
        auto const result = ctx.allocate_register(_result_type);
        ctx.emit(
          Instruction{InstructionT{.result = result, .operand = operand}}
        );
        return Operand{result};
      }
    }

  private:
    Type *_operand_type;
    Type *_result_type;
    [[no_unique_address]] Fn _fn;
  };

  template <
    typename LhsT,
    typename RhsT,
    typename ResultT,
    typename InstructionT,
    typename Fn
  >
  class Primitive_binary_operator_overload: public Binary_operator_overload
  {
  public:
    explicit Primitive_binary_operator_overload(Type_pool *type_pool)
        : _lhs_type{Primitive_type_traits<LhsT>::get(type_pool)},
          _rhs_type{Primitive_type_traits<RhsT>::get(type_pool)},
          _result_type{Primitive_type_traits<ResultT>::get(type_pool)}
    {
    }

    Type *lhs_type() const override
    {
      return _lhs_type;
    }

    Type *rhs_type() const override
    {
      return _rhs_type;
    }

    Type *result_type() const override
    {
      return _result_type;
    }

    Operand
    compile(Compilation_context &ctx, Operand lhs, Operand rhs) const override
    {
      auto const lhs_cv = std::get_if<Constant_value>(&lhs);
      auto const rhs_cv = std::get_if<Constant_value>(&rhs);
      if (lhs_cv != nullptr && rhs_cv != nullptr)
      {
        return Operand{
          Constant_value{_fn(std::get<LhsT>(*lhs_cv), std::get<RhsT>(*rhs_cv))}
        };
      }
      else
      {
        auto const result = ctx.allocate_register(_result_type);
        ctx.emit(
          Instruction{InstructionT{.result = result, .lhs = lhs, .rhs = rhs}}
        );
        return Operand{result};
      }
    }

  private:
    Type *_lhs_type;
    Type *_rhs_type;
    Type *_result_type;
    [[no_unique_address]] Fn _fn;
  };

  template <typename T>
  struct Negate_fn
  {
    auto operator()(T x) const -> T
    {
      return static_cast<T>(-x);
    }
  };

  template <typename T>
  struct Add_fn
  {
    auto operator()(T a, T b) const -> T
    {
      return static_cast<T>(a + b);
    }
  };

  template <typename T>
  struct Sub_fn
  {
    auto operator()(T a, T b) const -> T
    {
      return static_cast<T>(a - b);
    }
  };

  template <typename T>
  struct Mul_fn
  {
    auto operator()(T a, T b) const -> T
    {
      return static_cast<T>(a * b);
    }
  };

  template <typename T>
  struct Div_fn
  {
    auto operator()(T a, T b) const -> T
    {
      return static_cast<T>(a / b);
    }
  };

  template <typename T>
  struct Mod_fn
  {
    auto operator()(T a, T b) const -> T
    {
      return static_cast<T>(a % b);
    }
  };

  template <typename T>
  struct Equal_fn
  {
    auto operator()(T a, T b) const -> bool
    {
      return a == b;
    }
  };

  template <typename T>
  struct Not_equal_fn
  {
    auto operator()(T a, T b) const -> bool
    {
      return a != b;
    }
  };

  template <typename T>
  struct Less_fn
  {
    auto operator()(T a, T b) const -> bool
    {
      return a < b;
    }
  };

  template <typename T>
  struct Less_eq_fn
  {
    auto operator()(T a, T b) const -> bool
    {
      return a <= b;
    }
  };

  template <typename T>
  struct Greater_fn
  {
    auto operator()(T a, T b) const -> bool
    {
      return a > b;
    }
  };

  template <typename T>
  struct Greater_eq_fn
  {
    auto operator()(T a, T b) const -> bool
    {
      return a >= b;
    }
  };

  template <typename T>
  using Negate = Primitive_unary_operator_overload<
    T,
    T,
    Negate_instruction<T>,
    Negate_fn<T>
  >;

  template <typename T>
  using Add =
    Primitive_binary_operator_overload<T, T, T, Add_instruction<T>, Add_fn<T>>;

  template <typename T>
  using Subtract = Primitive_binary_operator_overload<
    T,
    T,
    T,
    Subtract_instruction<T>,
    Sub_fn<T>
  >;

  template <typename T>
  using Multiply = Primitive_binary_operator_overload<
    T,
    T,
    T,
    Multiply_instruction<T>,
    Mul_fn<T>
  >;

  template <typename T>
  using Divide = Primitive_binary_operator_overload<
    T,
    T,
    T,
    Divide_instruction<T>,
    Div_fn<T>
  >;

  template <typename T>
  using Modulo = Primitive_binary_operator_overload<
    T,
    T,
    T,
    Modulo_instruction<T>,
    Mod_fn<T>
  >;

  template <typename T>
  using Equal = Primitive_binary_operator_overload<
    T,
    T,
    bool,
    Equal_instruction<T>,
    Equal_fn<T>
  >;

  template <typename T>
  using Not_equal = Primitive_binary_operator_overload<
    T,
    T,
    bool,
    Not_equal_instruction<T>,
    Not_equal_fn<T>
  >;

  template <typename T>
  using Less = Primitive_binary_operator_overload<
    T,
    T,
    bool,
    Less_instruction<T>,
    Less_fn<T>
  >;

  template <typename T>
  using Less_eq = Primitive_binary_operator_overload<
    T,
    T,
    bool,
    Less_eq_instruction<T>,
    Less_eq_fn<T>
  >;

  template <typename T>
  using Greater = Primitive_binary_operator_overload<
    T,
    T,
    bool,
    Greater_instruction<T>,
    Greater_fn<T>
  >;

  template <typename T>
  using Greater_eq = Primitive_binary_operator_overload<
    T,
    T,
    bool,
    Greater_eq_instruction<T>,
    Greater_eq_fn<T>
  >;

  class Unary_plus final: public Unary_operator_overload
  {
  public:
    explicit Unary_plus(Type_pool *type_pool)
        : _int8_type{type_pool->int8_type()},
          _int16_type{type_pool->int16_type()},
          _int32_type{type_pool->int32_type()},
          _int64_type{type_pool->int64_type()},
          _float32_type{type_pool->float32_type()},
          _float64_type{type_pool->float64_type()}
    {
    }

    bool matches(Type *operand_type) const override
    {
      return operand_type == _int8_type || operand_type == _int16_type ||
             operand_type == _int32_type || operand_type == _int64_type ||
             operand_type == _float32_type || operand_type == _float64_type;
    }

    Type *result_type(Type *operand_type) const override
    {
      return operand_type;
    }

    Operand compile(Compilation_context &, Operand operand) const override
    {
      return operand;
    }

  private:
    Type *_int8_type;
    Type *_int16_type;
    Type *_int32_type;
    Type *_int64_type;
    Type *_float32_type;
    Type *_float64_type;
  };

  class Pointer_to final: public Unary_operator_overload
  {
  public:
    explicit Pointer_to(Type_pool *type_pool)
        : _type{type_pool->type_type()}, _type_pool{type_pool}
    {
    }

    bool matches(Type *operand_type) const override
    {
      return operand_type == _type;
    }

    Type *result_type(Type *) const override
    {
      return _type;
    }

    Operand compile(Compilation_context &, Operand operand) const override
    {
      auto const cv = std::get_if<Constant_value>(&operand);
      assert(cv != nullptr);
      return Operand{Constant_value{Type_value{
        _type_pool->pointer_type(std::get<Type_value>(*cv).type, false)
      }}};
    }

  private:
    Type *_type;
    Type_pool *_type_pool;
  };

  class Pointer_to_mut final: public Unary_operator_overload
  {
  public:
    explicit Pointer_to_mut(Type_pool *type_pool)
        : _type{type_pool->type_type()}, _type_pool{type_pool}
    {
    }

    bool matches(Type *operand_type) const override
    {
      return operand_type == _type;
    }

    Type *result_type(Type *) const override
    {
      return _type;
    }

    Operand compile(Compilation_context &, Operand operand) const override
    {
      auto const cv = std::get_if<Constant_value>(&operand);
      assert(cv != nullptr);
      return Operand{Constant_value{Type_value{
        _type_pool->pointer_type(std::get<Type_value>(*cv).type, true)
      }}};
    }

  private:
    Type *_type;
    Type_pool *_type_pool;
  };

  class Dereference final: public Unary_operator_overload
  {
  public:
    bool matches(Type *operand_type) const override
    {
      return std::holds_alternative<Pointer_type>(operand_type->data);
    }

    Type *result_type(Type *operand_type) const override
    {
      return std::get<Pointer_type>(operand_type->data).pointee;
    }

    Operand compile(Compilation_context &, Operand) const override
    {
      throw std::runtime_error{"dereference is not implemented"};
    }
  };

} // namespace benson::ir

#endif
