#include <cassert>
#include <charconv>
#include <cstdint>
#include <format>
#include <limits>
#include <memory>
#include <utility>
#include <variant>
#include <vector>

#include <basedast/source_span.h>

#include "basedhlir/compile.h"
#include "basedhlir/interpret.h"

namespace basedhlir
{

  template <typename T>
  class Scoped_assign
  {
  public:
    explicit Scoped_assign(T &target, T value)
        : _target{target}, _previous{target}
    {
      _target = std::move(value);
    }

    ~Scoped_assign()
    {
      _target = _previous;
    }

    Scoped_assign(Scoped_assign const &) = delete;

    Scoped_assign &operator=(Scoped_assign const &) = delete;

  private:
    T &_target;
    T _previous;
  };

  template <typename T>
  struct Primitive_hlir_type;

  template <>
  struct Primitive_hlir_type<std::int8_t>
  {
    static constexpr std::string_view name = "Int8";

    static auto get(Type_pool *type_pool) -> Type *
    {
      return type_pool->int8_type();
    }
  };

  template <>
  struct Primitive_hlir_type<std::int16_t>
  {
    static constexpr std::string_view name = "Int16";

    static auto get(Type_pool *type_pool) -> Type *
    {
      return type_pool->int16_type();
    }
  };

  template <>
  struct Primitive_hlir_type<std::int32_t>
  {
    static constexpr std::string_view name = "Int32";

    static auto get(Type_pool *type_pool) -> Type *
    {
      return type_pool->int32_type();
    }
  };

  template <>
  struct Primitive_hlir_type<std::int64_t>
  {
    static constexpr std::string_view name = "Int64";

    static auto get(Type_pool *type_pool) -> Type *
    {
      return type_pool->int64_type();
    }
  };

  template <>
  struct Primitive_hlir_type<float>
  {
    static constexpr std::string_view name = "Float32";

    static auto get(Type_pool *type_pool) -> Type *
    {
      return type_pool->float32_type();
    }
  };

  template <>
  struct Primitive_hlir_type<double>
  {
    static constexpr std::string_view name = "Float64";

    static auto get(Type_pool *type_pool) -> Type *
    {
      return type_pool->float64_type();
    }
  };

  template <>
  struct Primitive_hlir_type<bool>
  {
    static constexpr std::string_view name = "Bool";

    static auto get(Type_pool *type_pool) -> Type *
    {
      return type_pool->bool_type();
    }
  };

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
        : _operand_type{Primitive_hlir_type<OperandT>::get(type_pool)},
          _result_type{Primitive_hlir_type<ResultT>::get(type_pool)}
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
        : _lhs_type{Primitive_hlir_type<LhsT>::get(type_pool)},
          _rhs_type{Primitive_hlir_type<RhsT>::get(type_pool)},
          _result_type{Primitive_hlir_type<ResultT>::get(type_pool)}
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
  using Add = Primitive_binary_operator_overload<
    T,
    T,
    T,
    Add_instruction<T>,
    Add_fn<T>
  >;

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

  using Bool_equal = Primitive_binary_operator_overload<
    bool,
    bool,
    bool,
    Bool_equal_instruction,
    Equal_fn<bool>
  >;

  using Bool_not_equal = Primitive_binary_operator_overload<
    bool,
    bool,
    bool,
    Bool_not_equal_instruction,
    Not_equal_fn<bool>
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

  Compilation_context::Compilation_context(Type_pool *type_pool)
      : _type_pool{type_pool}
  {
    assert(_type_pool != nullptr);
    _symbol_table.declare_value("Int8", Type_value{_type_pool->int8_type()});
    _symbol_table.declare_value("Int16", Type_value{_type_pool->int16_type()});
    _symbol_table.declare_value("Int32", Type_value{_type_pool->int32_type()});
    _symbol_table.declare_value("Int64", Type_value{_type_pool->int64_type()});
    _symbol_table.declare_value("Bool", Type_value{_type_pool->bool_type()});
    _symbol_table.declare_value("Void", Type_value{_type_pool->void_type()});
    _symbol_table.declare_value(
      "Float32", Type_value{_type_pool->float32_type()}
    );
    _symbol_table.declare_value(
      "Float64", Type_value{_type_pool->float64_type()}
    );
    _symbol_table.declare_value("true", true);
    _symbol_table.declare_value("false", false);
    _unary_overloads[basedast::Operator::unary_plus].push_back(
      std::make_unique<Unary_plus>(_type_pool)
    );
    _unary_overloads[basedast::Operator::unary_minus].push_back(
      std::make_unique<Negate<std::int8_t>>(_type_pool)
    );
    _unary_overloads[basedast::Operator::unary_minus].push_back(
      std::make_unique<Negate<std::int16_t>>(_type_pool)
    );
    _unary_overloads[basedast::Operator::unary_minus].push_back(
      std::make_unique<Negate<std::int32_t>>(_type_pool)
    );
    _unary_overloads[basedast::Operator::unary_minus].push_back(
      std::make_unique<Negate<std::int64_t>>(_type_pool)
    );
    _unary_overloads[basedast::Operator::unary_minus].push_back(
      std::make_unique<Negate<float>>(_type_pool)
    );
    _unary_overloads[basedast::Operator::unary_minus].push_back(
      std::make_unique<Negate<double>>(_type_pool)
    );
    _unary_overloads[basedast::Operator::pointer_to].push_back(
      std::make_unique<Pointer_to>(_type_pool)
    );
    _unary_overloads[basedast::Operator::pointer_to_mut].push_back(
      std::make_unique<Pointer_to_mut>(_type_pool)
    );
    _unary_overloads[basedast::Operator::dereference].push_back(
      std::make_unique<Dereference>()
    );
    _binary_overloads[basedast::Operator::add].push_back(
      std::make_unique<Add<std::int8_t>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::add].push_back(
      std::make_unique<Add<std::int16_t>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::add].push_back(
      std::make_unique<Add<std::int32_t>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::add].push_back(
      std::make_unique<Add<std::int64_t>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::subtract].push_back(
      std::make_unique<Subtract<std::int8_t>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::subtract].push_back(
      std::make_unique<Subtract<std::int16_t>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::subtract].push_back(
      std::make_unique<Subtract<std::int32_t>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::subtract].push_back(
      std::make_unique<Subtract<std::int64_t>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::multiply].push_back(
      std::make_unique<Multiply<std::int8_t>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::multiply].push_back(
      std::make_unique<Multiply<std::int16_t>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::multiply].push_back(
      std::make_unique<Multiply<std::int32_t>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::multiply].push_back(
      std::make_unique<Multiply<std::int64_t>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::divide].push_back(
      std::make_unique<Divide<std::int8_t>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::divide].push_back(
      std::make_unique<Divide<std::int16_t>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::divide].push_back(
      std::make_unique<Divide<std::int32_t>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::divide].push_back(
      std::make_unique<Divide<std::int64_t>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::modulo].push_back(
      std::make_unique<Modulo<std::int8_t>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::modulo].push_back(
      std::make_unique<Modulo<std::int16_t>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::modulo].push_back(
      std::make_unique<Modulo<std::int32_t>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::modulo].push_back(
      std::make_unique<Modulo<std::int64_t>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::equal].push_back(
      std::make_unique<Equal<std::int8_t>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::equal].push_back(
      std::make_unique<Equal<std::int16_t>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::equal].push_back(
      std::make_unique<Equal<std::int32_t>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::equal].push_back(
      std::make_unique<Equal<std::int64_t>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::not_equal].push_back(
      std::make_unique<Not_equal<std::int8_t>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::not_equal].push_back(
      std::make_unique<Not_equal<std::int16_t>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::not_equal].push_back(
      std::make_unique<Not_equal<std::int32_t>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::not_equal].push_back(
      std::make_unique<Not_equal<std::int64_t>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::less].push_back(
      std::make_unique<Less<std::int8_t>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::less].push_back(
      std::make_unique<Less<std::int16_t>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::less].push_back(
      std::make_unique<Less<std::int32_t>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::less].push_back(
      std::make_unique<Less<std::int64_t>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::less_eq].push_back(
      std::make_unique<Less_eq<std::int8_t>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::less_eq].push_back(
      std::make_unique<Less_eq<std::int16_t>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::less_eq].push_back(
      std::make_unique<Less_eq<std::int32_t>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::less_eq].push_back(
      std::make_unique<Less_eq<std::int64_t>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::greater].push_back(
      std::make_unique<Greater<std::int8_t>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::greater].push_back(
      std::make_unique<Greater<std::int16_t>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::greater].push_back(
      std::make_unique<Greater<std::int32_t>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::greater].push_back(
      std::make_unique<Greater<std::int64_t>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::greater_eq].push_back(
      std::make_unique<Greater_eq<std::int8_t>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::greater_eq].push_back(
      std::make_unique<Greater_eq<std::int16_t>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::greater_eq].push_back(
      std::make_unique<Greater_eq<std::int32_t>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::greater_eq].push_back(
      std::make_unique<Greater_eq<std::int64_t>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::add].push_back(
      std::make_unique<Add<float>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::add].push_back(
      std::make_unique<Add<double>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::subtract].push_back(
      std::make_unique<Subtract<float>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::subtract].push_back(
      std::make_unique<Subtract<double>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::multiply].push_back(
      std::make_unique<Multiply<float>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::multiply].push_back(
      std::make_unique<Multiply<double>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::divide].push_back(
      std::make_unique<Divide<float>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::divide].push_back(
      std::make_unique<Divide<double>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::equal].push_back(
      std::make_unique<Equal<float>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::equal].push_back(
      std::make_unique<Equal<double>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::not_equal].push_back(
      std::make_unique<Not_equal<float>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::not_equal].push_back(
      std::make_unique<Not_equal<double>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::less].push_back(
      std::make_unique<Less<float>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::less].push_back(
      std::make_unique<Less<double>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::less_eq].push_back(
      std::make_unique<Less_eq<float>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::less_eq].push_back(
      std::make_unique<Less_eq<double>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::greater].push_back(
      std::make_unique<Greater<float>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::greater].push_back(
      std::make_unique<Greater<double>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::greater_eq].push_back(
      std::make_unique<Greater_eq<float>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::greater_eq].push_back(
      std::make_unique<Greater_eq<double>>(_type_pool)
    );
    _binary_overloads[basedast::Operator::equal].push_back(
      std::make_unique<Bool_equal>(_type_pool)
    );
    _binary_overloads[basedast::Operator::not_equal].push_back(
      std::make_unique<Bool_not_equal>(_type_pool)
    );
  }

  Translation_unit
  Compilation_context::compile(basedast::Translation_unit const &ast)
  {
    for (auto const &decl : ast.let_statements)
    {
      try
      {
        compile_statement(decl);
      }
      catch (Compilation_error const &)
      {
      }
    }
    if (!_diagnostics.empty())
    {
      throw Compilation_failure{std::move(_diagnostics)};
    }
    return std::move(_translation_unit);
  }

  Type *Compilation_context::type_of_constant(Constant_value const &value)
  {
    return std::visit(
      [this](auto const &v) -> Type *
      {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, std::int8_t>)
        {
          return _type_pool->int8_type();
        }
        else if constexpr (std::is_same_v<T, std::int16_t>)
        {
          return _type_pool->int16_type();
        }
        else if constexpr (std::is_same_v<T, std::int32_t>)
        {
          return _type_pool->int32_type();
        }
        else if constexpr (std::is_same_v<T, std::int64_t>)
        {
          return _type_pool->int64_type();
        }
        else if constexpr (std::is_same_v<T, float>)
        {
          return _type_pool->float32_type();
        }
        else if constexpr (std::is_same_v<T, double>)
        {
          return _type_pool->float64_type();
        }
        else if constexpr (std::is_same_v<T, bool>)
        {
          return _type_pool->bool_type();
        }
        else if constexpr (std::is_same_v<T, Void_value>)
        {
          return _type_pool->void_type();
        }
        else if constexpr (std::is_same_v<T, Type_value>)
        {
          return _type_pool->type_type();
        }
        else if constexpr (std::is_same_v<T, Function_value>)
        {
          return v.function->type;
        }
      },
      value
    );
  }

  [[noreturn]] void
  Compilation_context::emit_error(std::string message, Source_span location)
  {
    _diagnostics.push_back(
      Diagnostic{
        .message = std::move(message),
        .location = location,
        .notes = {},
      }
    );
    throw Compilation_error{};
  }

  Symbol *
  Compilation_context::try_lookup_identifier(basedlex::Lexeme const &identifier)
  {
    return _symbol_table.lookup(identifier.text);
  }

  Symbol *
  Compilation_context::lookup_identifier(basedlex::Lexeme const &identifier)
  {
    auto const sym = try_lookup_identifier(identifier);
    if (sym == nullptr)
    {
      emit_error(
        std::format("undefined identifier: {}", identifier.text),
        identifier
      );
    }
    return sym;
  }

  Unary_operator_overload *Compilation_context::find_unary_overload(
    basedast::Operator op,
    Type *operand_type
  )
  {
    auto const it = _unary_overloads.find(op);
    if (it != _unary_overloads.end())
    {
      for (auto const &overload : it->second)
      {
        if (overload->matches(operand_type))
        {
          return overload.get();
        }
      }
    }
    return nullptr;
  }

  Binary_operator_overload *Compilation_context::find_binary_overload(
    basedast::Operator op,
    Type *lhs_type,
    Type *rhs_type
  )
  {
    auto const it = _binary_overloads.find(op);
    if (it != _binary_overloads.end())
    {
      for (auto const &overload : it->second)
      {
        if (overload->lhs_type() == lhs_type &&
            overload->rhs_type() == rhs_type)
        {
          return overload.get();
        }
      }
    }
    return nullptr;
  }

  bool Compilation_context::is_type_compatible(
    Type *parameter_type,
    Type *argument_type
  )
  {
    if (parameter_type == argument_type)
    {
      return true;
    }
    auto const param_ptr = std::get_if<Pointer_type>(&parameter_type->data);
    auto const arg_ptr = std::get_if<Pointer_type>(&argument_type->data);
    if (param_ptr != nullptr && arg_ptr != nullptr)
    {
      if (param_ptr->is_mutable && !arg_ptr->is_mutable)
      {
        return false;
      }
      return is_type_compatible(param_ptr->pointee, arg_ptr->pointee);
    }
    if (param_ptr != nullptr)
    {
      auto const param_unsized =
        std::get_if<Unsized_array_type>(&param_ptr->pointee->data);
      auto const arg_sized =
        std::get_if<Sized_array_type>(&argument_type->data);
      if (param_unsized != nullptr && arg_sized != nullptr)
      {
        return is_type_compatible(param_unsized->element, arg_sized->element);
      }
    }
    return false;
  }

  Type *
  Compilation_context::compile_type_expression(basedast::Expression const &expr)
  {
    auto const value = evaluate_constant_expression(expr);
    auto const tv = std::get_if<Type_value>(&value);
    if (tv == nullptr)
    {
      emit_error("expected a type expression", expr);
    }
    return tv->type;
  }

  Constant_value Compilation_context::evaluate_constant_expression(
    basedast::Expression const &expr
  )
  {
    auto result = Operand{};
    try
    {
      result = compile_expression(expr);
    }
    catch (Not_a_constant_error const &e)
    {
      _diagnostics.push_back(
        Diagnostic{
          .message = "expression is not a compile-time constant",
          .location = e.expression_location.value_or(basedast::span_of(expr)),
          .notes = {},
        }
      );
      throw Compilation_error{};
    }
    catch (Fuel_exhausted_error const &)
    {
      _diagnostics.push_back(
        Diagnostic{
          .message = "compile-time evaluation ran out of fuel",
          .location = basedast::span_of(expr),
          .notes = {},
        }
      );
      throw Compilation_error{};
    }
    auto const cv = std::get_if<Constant_value>(&result);
    if (cv == nullptr)
    {
      emit_error("expression is not a compile-time constant", expr);
    }
    return *cv;
  }

  bool Compilation_context::is_top_level() const
  {
    return _current_function == nullptr;
  }

  Basic_block *Compilation_context::new_block()
  {
    assert(_current_function != nullptr);
    auto block = std::make_unique<Basic_block>();
    auto const ptr = block.get();
    _current_function->blocks.push_back(std::move(block));
    return ptr;
  }

  void Compilation_context::set_current_block(Basic_block *block)
  {
    _current_block = block;
  }

  Register Compilation_context::allocate_register(Type *type)
  {
    if (_current_function == nullptr)
    {
      throw Not_a_constant_error{};
    }
    auto const reg = Register{_current_function->register_count++};
    _register_types.push_back(type);
    return reg;
  }

  Type *Compilation_context::type_of_register(Register r) const
  {
    assert(*r >= 0 && *r < static_cast<std::int32_t>(_register_types.size()));
    return _register_types[*r];
  }

  Type *Compilation_context::type_of_operand(Operand const &operand)
  {
    if (auto const r = std::get_if<Register>(&operand))
    {
      return type_of_register(*r);
    }
    return type_of_constant(std::get<Constant_value>(operand));
  }

  void Compilation_context::emit(Instruction instruction)
  {
    if (_current_block == nullptr)
    {
      throw Not_a_constant_error{};
    }
    assert(!_current_block->has_terminator());
    _current_block->instructions.push_back(std::move(instruction));
  }

  void Compilation_context::emit(Terminator terminator)
  {
    if (_current_block == nullptr)
    {
      throw Not_a_constant_error{};
    }
    assert(!_current_block->has_terminator());
    _current_block->terminator = std::move(terminator);
  }

  Operand
  Compilation_context::compile_expression(basedast::Expression const &expr)
  {
    try
    {
      auto const result = std::visit(
        [this](auto const &e) -> Operand
        {
          return compile_expression(e);
        },
        expr.value
      );
      if (_current_block != nullptr)
      {
        // Postcondition: compiling an expression should not leave the current
        // block in a terminated state
        assert(!_current_block->has_terminator());
      }
      return result;
    }
    catch (Not_a_constant_error &e)
    {
      if (!e.expression_location.has_value())
      {
        e.expression_location = basedast::span_of(expr);
      }
      throw;
    }
  }

  Operand Compilation_context::compile_expression(
    basedast::Int_literal_expression const &expr
  )
  {
    return compile_int_literal(expr.literal.text, false, expr.literal);
  }

  Operand Compilation_context::compile_expression(
    basedast::Float_literal_expression const &expr
  )
  {
    return compile_float_literal(expr.literal.text, expr.literal);
  }

  Operand Compilation_context::compile_expression(
    basedast::Identifier_expression const &expr
  )
  {
    auto const sym = lookup_identifier(expr.identifier);
    auto const ob = std::get_if<Object_binding>(&sym->data);
    if (ob != nullptr)
    {
      return ob->reg;
    }
    auto const cv = std::get_if<Constant_value>(&sym->data);
    assert(cv != nullptr);
    return *cv;
  }

  Operand Compilation_context::compile_expression(
    basedast::Recurse_expression const &expr
  )
  {
    if (is_top_level())
    {
      emit_error("'recurse' used outside of a function body", expr.kw_recurse);
    }
    return Constant_value{Function_value{.function = _current_function}};
  }

  Operand
  Compilation_context::compile_expression(basedast::Fn_expression const &expr)
  {
    return Constant_value{Function_value{.function = compile_function(expr)}};
  }

  Operand Compilation_context::compile_expression(
    basedast::Paren_expression const &expr
  )
  {
    return compile_expression(*expr.inner);
  }

  Operand Compilation_context::compile_expression(
    basedast::Prefix_expression const &expr
  )
  {
    auto const op = basedast::get_prefix_operator(expr.op.token);
    assert(op.has_value());
    // Special case: unary minus before int literal expression
    if (*op == basedast::Operator::unary_minus &&
        std::holds_alternative<basedast::Int_literal_expression>(
          expr.operand->value
        ))
    {
      auto const &literal =
        std::get<basedast::Int_literal_expression>(expr.operand->value);
      return compile_int_literal(literal.literal.text, true, literal.literal);
    }
    // General case
    else
    {
      auto const operand_result = compile_expression(*expr.operand);
      auto const operand_type = type_of_operand(operand_result);
      auto const overload = find_unary_overload(*op, operand_type);
      assert(overload != nullptr);
      return overload->compile(*this, operand_result);
    }
  }

  Operand Compilation_context::compile_expression(
    basedast::Postfix_expression const &expr
  )
  {
    emit_error("dereference is not supported in this context", expr.op);
  }

  Operand Compilation_context::compile_expression(
    basedast::Binary_expression const &expr
  )
  {
    auto const lhs_result = compile_expression(*expr.left);
    auto const rhs_result = compile_expression(*expr.right);
    auto const op = basedast::get_binary_operator(expr.op.token);
    assert(op.has_value());
    auto const lhs_type = type_of_operand(lhs_result);
    auto const rhs_type = type_of_operand(rhs_result);
    auto const overload = find_binary_overload(*op, lhs_type, rhs_type);
    assert(overload != nullptr);
    return overload->compile(*this, lhs_result, rhs_result);
  }

  Operand
  Compilation_context::compile_expression(basedast::Call_expression const &expr)
  {
    auto const callee_result = compile_expression(*expr.callee);
    auto const callee_type = type_of_operand(callee_result);
    auto const ft = std::get_if<Function_type>(&callee_type->data);
    if (ft == nullptr)
    {
      emit_error("expression is not callable", expr.lparen);
    }
    auto const callee_cv = std::get_if<Constant_value>(&callee_result);
    auto const fv =
      callee_cv != nullptr ? std::get_if<Function_value>(callee_cv) : nullptr;
    if (fv == nullptr)
    {
      emit_error("callee must be a known function", *expr.callee);
    }
    if (expr.arguments.size() != ft->parameter_types.size())
    {
      emit_error(
        std::format(
          "expected {} arguments, got {}",
          ft->parameter_types.size(),
          expr.arguments.size()
        ),
        expr.lparen
      );
    }
    auto arg_results = std::vector<Operand>{};
    arg_results.reserve(expr.arguments.size());
    auto all_constant = bool{true};
    for (auto i = std::size_t{}; i < expr.arguments.size(); ++i)
    {
      auto const arg_result = compile_expression(expr.arguments[i]);
      auto const arg_type = type_of_operand(arg_result);
      if (!is_type_compatible(ft->parameter_types[i], arg_type))
      {
        emit_error(
          std::format(
            "argument {} is not compatible with parameter type",
            i + 1
          ),
          expr.arguments[i]
        );
      }
      if (!std::holds_alternative<Constant_value>(arg_result))
      {
        all_constant = false;
      }
      arg_results.push_back(arg_result);
    }
    if (all_constant)
    {
      auto const_args = std::vector<Constant_value>{};
      const_args.reserve(arg_results.size());
      for (auto const &r : arg_results)
      {
        const_args.push_back(std::get<Constant_value>(r));
      }
      return interpret(*fv->function, const_args);
    }
    auto const result = allocate_register(ft->return_type);
    emit(
      Instruction{Call_instruction{
        .result = result,
        .callee = fv->function,
        .arguments = std::move(arg_results),
      }}
    );
    return result;
  }

  Operand
  Compilation_context::compile_expression(basedast::Index_expression const &)
  {
    throw std::runtime_error{"index expressions are not implemented"};
  }

  Operand Compilation_context::compile_expression(
    basedast::Prefix_bracket_expression const &expr
  )
  {
    auto const element_result = compile_expression(*expr.operand);
    auto const element_cv = std::get_if<Constant_value>(&element_result);
    if (element_cv == nullptr)
    {
      emit_error("expected a type expression", *expr.operand);
    }
    auto const tv = std::get_if<Type_value>(element_cv);
    if (tv == nullptr)
    {
      emit_error("expected a type expression", *expr.operand);
    }
    if (expr.size == nullptr)
    {
      return Constant_value{
        Type_value{_type_pool->unsized_array_type(tv->type)}
      };
    }
    auto const size_result = compile_expression(*expr.size);
    if (type_of_operand(size_result) != _type_pool->int32_type())
    {
      emit_error("array size must be an integer", *expr.size);
    }
    auto const size_cv = std::get_if<Constant_value>(&size_result);
    if (size_cv == nullptr)
    {
      emit_error("array size must be a compile-time constant", *expr.size);
    }
    auto const size = std::get<std::int32_t>(*size_cv);
    if (size <= 0)
    {
      emit_error("array size must be positive", *expr.size);
    }
    return Constant_value{
      Type_value{_type_pool->sized_array_type(tv->type, size)}
    };
  }

  Operand Compilation_context::compile_expression(
    basedast::Block_expression const &expr
  )
  {
    _symbol_table.push_scope();
    for (auto const &stmt : expr.statements)
    {
      compile_statement(stmt);
    }
    auto const result = expr.tail ? compile_expression(*expr.tail)
                                  : Operand{Constant_value{Void_value{}}};
    _symbol_table.pop_scope();
    return result;
  }

  Operand
  Compilation_context::compile_expression(basedast::If_expression const &expr)
  {
    auto const cond_result = compile_expression(*expr.condition);
    auto const cond_type = type_of_operand(cond_result);
    if (cond_type != _type_pool->bool_type())
    {
      emit_error("condition must be a Bool", *expr.condition);
    }
    // Constant condition folding
    auto const cond_cv = std::get_if<Constant_value>(&cond_result);
    if (cond_cv != nullptr && std::get<bool>(*cond_cv))
    {
      return compile_expression(expr.then_block);
    }
    // If condition is constant false, fold greedily through else-if chain
    // until a non-constant condition is hit, then fall through to runtime.
    auto runtime_cond = cond_result;
    auto runtime_then = &expr.then_block;
    auto runtime_else_if_start = std::size_t{};
    if (cond_cv != nullptr)
    {
      auto folded_all = bool{true};
      for (auto i = std::size_t{}; i < expr.else_if_parts.size(); ++i)
      {
        auto const &part = expr.else_if_parts[i];
        auto const ei_result = compile_expression(*part.condition);
        if (auto const ei_cv = std::get_if<Constant_value>(&ei_result))
        {
          if (std::get<bool>(*ei_cv))
          {
            return compile_expression(part.body);
          }
        }
        else
        {
          runtime_cond = ei_result;
          runtime_then = &part.body;
          runtime_else_if_start = i + 1;
          folded_all = false;
          break;
        }
      }
      if (folded_all)
      {
        return expr.else_part.has_value()
                 ? compile_expression(expr.else_part->body)
                 : Operand{Constant_value{Void_value{}}};
      }
    }
    // Runtime condition path
    auto const merge_block = new_block();
    auto const then_block = new_block();
    auto const first_else_target =
      runtime_else_if_start < expr.else_if_parts.size() ||
          expr.else_part.has_value()
        ? new_block()
        : merge_block;
    emit(
      Terminator{Branch_terminator{
        .condition = runtime_cond,
        .then_target = then_block,
        .then_arguments = {},
        .else_target = first_else_target,
        .else_arguments = {},
      }}
    );
    // Compile then block
    set_current_block(then_block);
    auto const then_result = compile_expression(*runtime_then);
    auto const then_type = type_of_operand(then_result);
    auto const merge_param =
      then_type != _type_pool->void_type()
        ? merge_block->parameters.emplace_back(allocate_register(then_type))
        : Register{};
    auto const emit_jump_to_merge = [&](Operand const &result)
    {
      emit(
        Terminator{Jump_terminator{
          .target = merge_block,
          .arguments =
            merge_param ? std::vector<Operand>{result} : std::vector<Operand>{},
        }}
      );
    };
    emit_jump_to_merge(then_result);
    // Compile else-if chain
    auto current_else_block = first_else_target;
    for (auto i = runtime_else_if_start; i < expr.else_if_parts.size(); ++i)
    {
      auto const &part = expr.else_if_parts[i];
      set_current_block(current_else_block);
      auto const ei_cond_result = compile_expression(*part.condition);
      auto const ei_then = new_block();
      auto const ei_else =
        i + 1 < expr.else_if_parts.size() || expr.else_part.has_value()
          ? new_block()
          : merge_block;
      emit(
        Terminator{Branch_terminator{
          .condition = ei_cond_result,
          .then_target = ei_then,
          .then_arguments = {},
          .else_target = ei_else,
          .else_arguments = {},
        }}
      );
      set_current_block(ei_then);
      auto const ei_result = compile_expression(part.body);
      auto const ei_type = type_of_operand(ei_result);
      if (ei_type != then_type)
      {
        emit_error(
          "else-if branch type does not match then branch type",
          part.body.lbrace
        );
      }
      emit_jump_to_merge(ei_result);
      current_else_block = ei_else;
    }
    // Compile else block
    if (expr.else_part.has_value())
    {
      set_current_block(current_else_block);
      auto const else_result = compile_expression(expr.else_part->body);
      auto const else_type = type_of_operand(else_result);
      if (else_type != then_type)
      {
        emit_error(
          "else branch type does not match then branch type",
          expr.else_part->body.lbrace
        );
      }
      emit_jump_to_merge(else_result);
    }
    set_current_block(merge_block);
    if (merge_param)
    {
      return merge_param;
    }
    return Constant_value{Void_value{}};
  }

  void Compilation_context::compile_statement(basedast::Statement const &stmt)
  {
    std::visit(
      [this](auto const &s)
      {
        compile_statement(s);
      },
      stmt.value
    );
  }

  void Compilation_context::compile_statement(
    basedast::Let_statement const &stmt
  )
  {
    auto const is_mutable = stmt.kw_mut.has_value();
    if (is_mutable && is_top_level())
    {
      emit_error("top-level bindings cannot be mutable", *stmt.kw_mut);
    }
    auto result = Operand{};
    try
    {
      result = compile_expression(stmt.initializer);
    }
    catch (Not_a_constant_error const &e)
    {
      _diagnostics.push_back(
        Diagnostic{
          .message = "expression is not a compile-time constant",
          .location =
            e.expression_location.value_or(basedast::span_of(stmt.initializer)),
          .notes = {Diagnostic_note{
            .message = "required by this top-level binding",
            .location = basedast::span_of(stmt),
          }},
        }
      );
      throw Compilation_error{};
    }
    catch (Fuel_exhausted_error const &)
    {
      _diagnostics.push_back(
        Diagnostic{
          .message = "compile-time evaluation ran out of fuel",
          .location = basedast::span_of(stmt.initializer),
          .notes = {Diagnostic_note{
            .message = "required by this top-level binding",
            .location = basedast::span_of(stmt),
          }},
        }
      );
      throw Compilation_error{};
    }
    auto const type = type_of_operand(result);
    auto const is_object = is_object_type(type);
    if (is_mutable && !is_object)
    {
      emit_error("mutable binding requires an object type", *stmt.kw_mut);
    }
    if (is_top_level() || !is_object)
    {
      auto const cv = std::get_if<Constant_value>(&result);
      if (cv == nullptr)
      {
        _diagnostics.push_back(
          Diagnostic{
            .message = "expression is not a compile-time constant",
            .location = basedast::span_of(stmt.initializer),
            .notes = {Diagnostic_note{
              .message = "required by this top-level binding",
              .location = basedast::span_of(stmt),
            }},
          }
        );
        throw Compilation_error{};
      }
      _symbol_table.declare_value(stmt.name.text, *cv);
      if (auto const fv = std::get_if<Function_value>(cv))
      {
        _translation_unit.function_table[stmt.name.text] = fv->function;
      }
    }
    else if (is_mutable)
    {
      emit_error("mutable bindings are not implemented", *stmt.kw_mut);
    }
    else
    {
      if (auto const cv = std::get_if<Constant_value>(&result))
      {
        _symbol_table.declare_value(stmt.name.text, *cv);
      }
      else
      {
        auto const reg = std::get<Register>(result);
        _symbol_table.declare_object(stmt.name.text, type, false, reg);
      }
    }
  }

  void Compilation_context::compile_statement(
    basedast::While_statement const &stmt
  )
  {
    emit_error(
      "while statements are not supported without mutation",
      stmt.kw_while
    );
  }

  void Compilation_context::compile_statement(
    basedast::Return_statement const &stmt
  )
  {
    auto const result = compile_expression(stmt.value);
    auto const result_type = type_of_operand(result);
    auto const return_type =
      std::get<Function_type>(_current_function->type->data).return_type;
    if (!is_type_compatible(return_type, result_type))
    {
      emit_error(
        "return value type does not match function return type",
        stmt.value
      );
    }
    emit(Terminator{Return_terminator{.value = result}});
    // Start a new (dead) block for any subsequent code
    set_current_block(new_block());
  }

  void Compilation_context::compile_statement(
    basedast::Expression_statement const &stmt
  )
  {
    compile_expression(stmt.expression);
  }

  Function *
  Compilation_context::compile_function(basedast::Fn_expression const &expr)
  {
    auto parameter_types = std::vector<Type *>{};
    for (auto const &param : expr.parameters)
    {
      auto const param_type = compile_type_expression(*param.type);
      if (!is_object_type(param_type))
      {
        emit_error("parameter type must be an object type", *param.type);
      }
      parameter_types.push_back(param_type);
    }
    auto const return_type =
      compile_type_expression(*expr.return_type_specifier.type);
    auto const fn_type =
      _type_pool->function_type(std::move(parameter_types), return_type);
    auto const ft = std::get<Function_type>(fn_type->data);
    auto func = std::make_unique<Function>(Function{
      .type = fn_type,
      .blocks = {},
      .register_count = 0,
    });
    auto const func_ptr = func.get();
    _translation_unit.functions.push_back(std::move(func));
    auto const saved_function = Scoped_assign{_current_function, func_ptr};
    auto const saved_block =
      Scoped_assign{_current_block, static_cast<Basic_block *>(nullptr)};
    auto const saved_register_types =
      Scoped_assign{_register_types, std::vector<Type *>{}};
    auto const entry = new_block();
    set_current_block(entry);
    _symbol_table.push_scope(true);
    for (auto i = std::size_t{}; i < expr.parameters.size(); ++i)
    {
      auto const reg = allocate_register(ft.parameter_types[i]);
      entry->parameters.push_back(reg);
      _symbol_table.declare_object(
        expr.parameters[i].name.text,
        ft.parameter_types[i],
        false,
        reg
      );
    }
    // Compile the body
    auto const body_result = compile_expression(*expr.body);
    auto const body_type = type_of_operand(body_result);
    if (body_type != _type_pool->void_type() &&
        !is_type_compatible(ft.return_type, body_type))
    {
      emit_error("body type does not match return type", *expr.body);
    }
    emit(Terminator{Return_terminator{.value = body_result}});
    _symbol_table.pop_scope();
    for (auto const &block : func_ptr->blocks)
    {
      assert(block->has_terminator());
    }
    return func_ptr;
  }

  Translation_unit
  compile(basedast::Translation_unit const &ast, Type_pool *type_pool)
  {
    auto ctx = Compilation_context{type_pool};
    return ctx.compile(ast);
  }

  std::optional<std::uint64_t>
  validate_int_literal(std::string_view digits, std::uint64_t max_value)
  {
    auto value = std::uint64_t{0};
    for (auto const ch : digits)
    {
      assert(ch >= '0' && ch <= '9');
      auto const digit = static_cast<std::uint64_t>(ch - '0');
      if (value > (std::numeric_limits<std::uint64_t>::max() - digit) / 10u)
      {
        return std::nullopt;
      }
      value = value * 10u + digit;
    }
    if (value > max_value)
    {
      return std::nullopt;
    }
    return value;
  }

  Operand Compilation_context::compile_int_literal(
    std::string_view text,
    bool negate,
    basedlex::Lexeme const &token
  )
  {
    auto const suffix_pos = text.rfind('i');
    auto const suffix =
      suffix_pos != std::string_view::npos ? text.substr(suffix_pos) : "";
    auto const digits = text.substr(0, suffix_pos);
    auto const compile_typed = [&]<typename T>() -> Operand
    {
      auto constexpr max =
        static_cast<std::uint64_t>(std::numeric_limits<T>::max());
      auto constexpr max_magnitude = max + 1u;
      auto const limit = negate ? max_magnitude : max;
      auto const value = validate_int_literal(digits, limit);
      if (!value.has_value())
      {
        emit_error(
          std::format(
            "integer literal is out of range for {}",
            Primitive_hlir_type<T>::name
          ),
          token
        );
      }
      if (negate)
      {
        return Constant_value{
          *value < max_magnitude ? static_cast<T>(-static_cast<T>(*value))
                                 : std::numeric_limits<T>::min()
        };
      }
      return Constant_value{static_cast<T>(*value)};
    };
    if (suffix == "i8")
    {
      return compile_typed.operator()<std::int8_t>();
    }
    else if (suffix == "i16")
    {
      return compile_typed.operator()<std::int16_t>();
    }
    else if (suffix == "i32")
    {
      return compile_typed.operator()<std::int32_t>();
    }
    else if (suffix.empty())
    {
      auto constexpr max_i32 =
        static_cast<std::uint64_t>(std::numeric_limits<std::int32_t>::max());
      auto constexpr max_magnitude_i32 = max_i32 + 1u;
      auto const limit_i32 = negate ? max_magnitude_i32 : max_i32;
      return validate_int_literal(digits, limit_i32).has_value()
               ? compile_typed.operator()<std::int32_t>()
               : compile_typed.operator()<std::int64_t>();
    }
    else if (suffix == "i64")
    {
      return compile_typed.operator()<std::int64_t>();
    }
    emit_error(
      std::format("unknown integer literal suffix '{}'", suffix),
      token
    );
  }

  Operand Compilation_context::compile_float_literal(
    std::string_view text,
    basedlex::Lexeme const &token
  )
  {
    auto const suffix_pos = text.rfind('f');
    auto const suffix =
      suffix_pos != std::string_view::npos ? text.substr(suffix_pos) : "";
    auto const digits = text.substr(0, suffix_pos);
    auto const compile_typed = [&]<typename T>() -> Operand
    {
      T value{};
      auto const result =
        std::from_chars(digits.data(), digits.data() + digits.size(), value);
      if (result.ec != std::errc{})
      {
        emit_error(
          std::format(
            "float literal is out of range for {}",
            Primitive_hlir_type<T>::name
          ),
          token
        );
      }
      return Constant_value{value};
    };
    if (suffix == "f32")
    {
      return compile_typed.operator()<float>();
    }
    else if (suffix == "f64" || suffix.empty())
    {
      return compile_typed.operator()<double>();
    }
    emit_error(
      std::format("unknown float literal suffix '{}'", suffix),
      token
    );
  }

} // namespace basedhlir
