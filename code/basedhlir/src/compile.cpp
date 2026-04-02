#include <cassert>
#include <cstdint>
#include <format>
#include <memory>
#include <utility>
#include <variant>
#include <vector>

#include <basedparse/source_span.h>

#include "basedhlir/compile.h"
#include "basedhlir/interpret.h"

namespace basedhlir
{

  template <typename T>
  class Scoped_assign
  {
  public:
    Scoped_assign(T &target, T value)
        : _target{target}, _previous{target}
    {
      _target = value;
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

  class Int32_unary_plus final: public Unary_operator_overload
  {
  public:
    explicit Int32_unary_plus(Type_pool *type_pool)
        : _type{type_pool->int32_type()}
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

    Constant_value evaluate(Constant_value operand) const override
    {
      return std::get<std::int32_t>(operand);
    }

  private:
    Type *_type;
  };

  class Int32_unary_minus final: public Unary_operator_overload
  {
  public:
    explicit Int32_unary_minus(Type_pool *type_pool)
        : _type{type_pool->int32_type()}
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

    Constant_value evaluate(Constant_value operand) const override
    {
      return -std::get<std::int32_t>(operand);
    }

  private:
    Type *_type;
  };

  class Int32_add final: public Binary_operator_overload
  {
  public:
    explicit Int32_add(Type_pool *type_pool)
        : _type{type_pool->int32_type()}
    {
    }

    Type *lhs_type() const override
    {
      return _type;
    }

    Type *rhs_type() const override
    {
      return _type;
    }

    Type *result_type() const override
    {
      return _type;
    }

    Constant_value
    evaluate(Constant_value lhs, Constant_value rhs) const override
    {
      return std::get<std::int32_t>(lhs) + std::get<std::int32_t>(rhs);
    }

  private:
    Type *_type;
  };

  class Int32_subtract final: public Binary_operator_overload
  {
  public:
    explicit Int32_subtract(Type_pool *type_pool)
        : _type{type_pool->int32_type()}
    {
    }

    Type *lhs_type() const override
    {
      return _type;
    }

    Type *rhs_type() const override
    {
      return _type;
    }

    Type *result_type() const override
    {
      return _type;
    }

    Constant_value
    evaluate(Constant_value lhs, Constant_value rhs) const override
    {
      return std::get<std::int32_t>(lhs) - std::get<std::int32_t>(rhs);
    }

  private:
    Type *_type;
  };

  class Int32_multiply final: public Binary_operator_overload
  {
  public:
    explicit Int32_multiply(Type_pool *type_pool)
        : _type{type_pool->int32_type()}
    {
    }

    Type *lhs_type() const override
    {
      return _type;
    }

    Type *rhs_type() const override
    {
      return _type;
    }

    Type *result_type() const override
    {
      return _type;
    }

    Constant_value
    evaluate(Constant_value lhs, Constant_value rhs) const override
    {
      return std::get<std::int32_t>(lhs) * std::get<std::int32_t>(rhs);
    }

  private:
    Type *_type;
  };

  class Int32_divide final: public Binary_operator_overload
  {
  public:
    explicit Int32_divide(Type_pool *type_pool)
        : _type{type_pool->int32_type()}
    {
    }

    Type *lhs_type() const override
    {
      return _type;
    }

    Type *rhs_type() const override
    {
      return _type;
    }

    Type *result_type() const override
    {
      return _type;
    }

    Constant_value
    evaluate(Constant_value lhs, Constant_value rhs) const override
    {
      return std::get<std::int32_t>(lhs) / std::get<std::int32_t>(rhs);
    }

  private:
    Type *_type;
  };

  class Int32_modulo final: public Binary_operator_overload
  {
  public:
    explicit Int32_modulo(Type_pool *type_pool)
        : _type{type_pool->int32_type()}
    {
    }

    Type *lhs_type() const override
    {
      return _type;
    }

    Type *rhs_type() const override
    {
      return _type;
    }

    Type *result_type() const override
    {
      return _type;
    }

    Constant_value
    evaluate(Constant_value lhs, Constant_value rhs) const override
    {
      return std::get<std::int32_t>(lhs) % std::get<std::int32_t>(rhs);
    }

  private:
    Type *_type;
  };

  class Int32_equal final: public Binary_operator_overload
  {
  public:
    explicit Int32_equal(Type_pool *type_pool)
        : _int32{type_pool->int32_type()}, _bool{type_pool->bool_type()}
    {
    }

    Type *lhs_type() const override
    {
      return _int32;
    }

    Type *rhs_type() const override
    {
      return _int32;
    }

    Type *result_type() const override
    {
      return _bool;
    }

    Constant_value
    evaluate(Constant_value lhs, Constant_value rhs) const override
    {
      return std::get<std::int32_t>(lhs) == std::get<std::int32_t>(rhs);
    }

  private:
    Type *_int32;
    Type *_bool;
  };

  class Int32_not_equal final: public Binary_operator_overload
  {
  public:
    explicit Int32_not_equal(Type_pool *type_pool)
        : _int32{type_pool->int32_type()}, _bool{type_pool->bool_type()}
    {
    }

    Type *lhs_type() const override
    {
      return _int32;
    }

    Type *rhs_type() const override
    {
      return _int32;
    }

    Type *result_type() const override
    {
      return _bool;
    }

    Constant_value
    evaluate(Constant_value lhs, Constant_value rhs) const override
    {
      return std::get<std::int32_t>(lhs) != std::get<std::int32_t>(rhs);
    }

  private:
    Type *_int32;
    Type *_bool;
  };

  class Int32_less final: public Binary_operator_overload
  {
  public:
    explicit Int32_less(Type_pool *type_pool)
        : _int32{type_pool->int32_type()}, _bool{type_pool->bool_type()}
    {
    }

    Type *lhs_type() const override
    {
      return _int32;
    }

    Type *rhs_type() const override
    {
      return _int32;
    }

    Type *result_type() const override
    {
      return _bool;
    }

    Constant_value
    evaluate(Constant_value lhs, Constant_value rhs) const override
    {
      return std::get<std::int32_t>(lhs) < std::get<std::int32_t>(rhs);
    }

  private:
    Type *_int32;
    Type *_bool;
  };

  class Int32_less_eq final: public Binary_operator_overload
  {
  public:
    explicit Int32_less_eq(Type_pool *type_pool)
        : _int32{type_pool->int32_type()}, _bool{type_pool->bool_type()}
    {
    }

    Type *lhs_type() const override
    {
      return _int32;
    }

    Type *rhs_type() const override
    {
      return _int32;
    }

    Type *result_type() const override
    {
      return _bool;
    }

    Constant_value
    evaluate(Constant_value lhs, Constant_value rhs) const override
    {
      return std::get<std::int32_t>(lhs) <= std::get<std::int32_t>(rhs);
    }

  private:
    Type *_int32;
    Type *_bool;
  };

  class Int32_greater final: public Binary_operator_overload
  {
  public:
    explicit Int32_greater(Type_pool *type_pool)
        : _int32{type_pool->int32_type()}, _bool{type_pool->bool_type()}
    {
    }

    Type *lhs_type() const override
    {
      return _int32;
    }

    Type *rhs_type() const override
    {
      return _int32;
    }

    Type *result_type() const override
    {
      return _bool;
    }

    Constant_value
    evaluate(Constant_value lhs, Constant_value rhs) const override
    {
      return std::get<std::int32_t>(lhs) > std::get<std::int32_t>(rhs);
    }

  private:
    Type *_int32;
    Type *_bool;
  };

  class Int32_greater_eq final: public Binary_operator_overload
  {
  public:
    explicit Int32_greater_eq(Type_pool *type_pool)
        : _int32{type_pool->int32_type()}, _bool{type_pool->bool_type()}
    {
    }

    Type *lhs_type() const override
    {
      return _int32;
    }

    Type *rhs_type() const override
    {
      return _int32;
    }

    Type *result_type() const override
    {
      return _bool;
    }

    Constant_value
    evaluate(Constant_value lhs, Constant_value rhs) const override
    {
      return std::get<std::int32_t>(lhs) >= std::get<std::int32_t>(rhs);
    }

  private:
    Type *_int32;
    Type *_bool;
  };

  class Bool_equal final: public Binary_operator_overload
  {
  public:
    explicit Bool_equal(Type_pool *type_pool)
        : _bool{type_pool->bool_type()}
    {
    }

    Type *lhs_type() const override
    {
      return _bool;
    }

    Type *rhs_type() const override
    {
      return _bool;
    }

    Type *result_type() const override
    {
      return _bool;
    }

    Constant_value
    evaluate(Constant_value lhs, Constant_value rhs) const override
    {
      return std::get<bool>(lhs) == std::get<bool>(rhs);
    }

  private:
    Type *_bool;
  };

  class Bool_not_equal final: public Binary_operator_overload
  {
  public:
    explicit Bool_not_equal(Type_pool *type_pool)
        : _bool{type_pool->bool_type()}
    {
    }

    Type *lhs_type() const override
    {
      return _bool;
    }

    Type *rhs_type() const override
    {
      return _bool;
    }

    Type *result_type() const override
    {
      return _bool;
    }

    Constant_value
    evaluate(Constant_value lhs, Constant_value rhs) const override
    {
      return std::get<bool>(lhs) != std::get<bool>(rhs);
    }

  private:
    Type *_bool;
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

    Constant_value evaluate(Constant_value operand) const override
    {
      return Type_value{
        _type_pool->pointer_type(std::get<Type_value>(operand).type, false)
      };
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

    Constant_value evaluate(Constant_value operand) const override
    {
      return Type_value{
        _type_pool->pointer_type(std::get<Type_value>(operand).type, true)
      };
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

    Constant_value evaluate(Constant_value) const override
    {
      // Dereference is never a constant expression; this is handled by
      // evaluate_constant_expression(Postfix_expression) before reaching here.
      std::abort();
    }
  };

  Compilation_context::Compilation_context(Type_pool *type_pool)
      : _type_pool{type_pool}
  {
    assert(_type_pool != nullptr);
    _symbol_table.declare_value("Int32", Type_value{_type_pool->int32_type()});
    _symbol_table.declare_value("Bool", Type_value{_type_pool->bool_type()});
    _symbol_table.declare_value("Void", Type_value{_type_pool->void_type()});
    _unary_overloads[basedparse::Operator::unary_plus].push_back(
      std::make_unique<Int32_unary_plus>(_type_pool)
    );
    _unary_overloads[basedparse::Operator::unary_minus].push_back(
      std::make_unique<Int32_unary_minus>(_type_pool)
    );
    _unary_overloads[basedparse::Operator::pointer_to].push_back(
      std::make_unique<Pointer_to>(_type_pool)
    );
    _unary_overloads[basedparse::Operator::pointer_to_mut].push_back(
      std::make_unique<Pointer_to_mut>(_type_pool)
    );
    _unary_overloads[basedparse::Operator::dereference].push_back(
      std::make_unique<Dereference>()
    );
    _binary_overloads[basedparse::Operator::add].push_back(
      std::make_unique<Int32_add>(_type_pool)
    );
    _binary_overloads[basedparse::Operator::subtract].push_back(
      std::make_unique<Int32_subtract>(_type_pool)
    );
    _binary_overloads[basedparse::Operator::multiply].push_back(
      std::make_unique<Int32_multiply>(_type_pool)
    );
    _binary_overloads[basedparse::Operator::divide].push_back(
      std::make_unique<Int32_divide>(_type_pool)
    );
    _binary_overloads[basedparse::Operator::modulo].push_back(
      std::make_unique<Int32_modulo>(_type_pool)
    );
    _binary_overloads[basedparse::Operator::equal].push_back(
      std::make_unique<Int32_equal>(_type_pool)
    );
    _binary_overloads[basedparse::Operator::not_equal].push_back(
      std::make_unique<Int32_not_equal>(_type_pool)
    );
    _binary_overloads[basedparse::Operator::less].push_back(
      std::make_unique<Int32_less>(_type_pool)
    );
    _binary_overloads[basedparse::Operator::less_eq].push_back(
      std::make_unique<Int32_less_eq>(_type_pool)
    );
    _binary_overloads[basedparse::Operator::greater].push_back(
      std::make_unique<Int32_greater>(_type_pool)
    );
    _binary_overloads[basedparse::Operator::greater_eq].push_back(
      std::make_unique<Int32_greater_eq>(_type_pool)
    );
    _binary_overloads[basedparse::Operator::equal].push_back(
      std::make_unique<Bool_equal>(_type_pool)
    );
    _binary_overloads[basedparse::Operator::not_equal].push_back(
      std::make_unique<Bool_not_equal>(_type_pool)
    );
  }

  Translation_unit
  Compilation_context::compile(basedparse::Translation_unit const &ast)
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
    for (auto &[op, overloads] : _unary_overloads)
    {
      for (auto &overload : overloads)
      {
        _translation_unit.unary_overloads.push_back(std::move(overload));
      }
    }
    for (auto &[op, overloads] : _binary_overloads)
    {
      for (auto &overload : overloads)
      {
        _translation_unit.binary_overloads.push_back(std::move(overload));
      }
    }
    return std::move(_translation_unit);
  }

  Type *Compilation_context::type_of_constant(Constant_value const &value)
  {
    return std::visit(
      [this](auto const &v) -> Type *
      {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, std::int32_t>)
        {
          return _type_pool->int32_type();
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

  [[noreturn]] void Compilation_context::emit_error(
    std::string message,
    basedlex::Lexeme const &lexeme
  )
  {
    emit_error(std::move(message), lexeme.location);
  }

  [[noreturn]] void
  Compilation_context::emit_error(std::string message, Source_location location)
  {
    _diagnostics.push_back(
      Diagnostic{
        .message = std::move(message),
        .location = location,
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
    basedparse::Operator op,
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
    basedparse::Operator op,
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

  Type *Compilation_context::compile_type_expression(
    basedparse::Expression const &expr
  )
  {
    auto const type = type_of_expression(expr);
    if (type != _type_pool->type_type())
    {
      auto const span = basedparse::span_of(expr);
      emit_error("expected a type expression", span.start);
    }
    return std::get<Type_value>(evaluate_constant_expression(expr)).type;
  }

  Type *
  Compilation_context::type_of_expression(basedparse::Expression const &expr)
  {
    return std::visit(
      [&](auto const &e) -> Type *
      {
        return type_of_expression(e);
      },
      expr.value
    );
  }

  Type *Compilation_context::type_of_expression(
    basedparse::Int_literal_expression const &
  )
  {
    return _type_pool->int32_type();
  }

  Type *Compilation_context::type_of_expression(
    basedparse::Identifier_expression const &expr
  )
  {
    auto const sym = lookup_identifier(expr.identifier);
    auto const ob = std::get_if<Object_binding>(&sym->data);
    if (ob != nullptr)
    {
      return ob->type;
    }
    auto const cv = std::get_if<Constant_value>(&sym->data);
    if (cv != nullptr)
    {
      return type_of_constant(*cv);
    }
    emit_error(
      std::format("'{}' is not a value", expr.identifier.text),
      expr.identifier
    );
  }

  Type *Compilation_context::type_of_expression(
    basedparse::Recurse_expression const &expr
  )
  {
    if (is_top_level())
    {
      emit_error("'recurse' used outside of a function body", expr.kw_recurse);
    }
    return type_of_constant(Function_value{.function = _current_function});
  }

  Type *
  Compilation_context::type_of_expression(basedparse::Fn_expression const &expr)
  {
    auto parameter_types = std::vector<Type *>{};
    for (auto const &param : expr.parameters)
    {
      parameter_types.push_back(compile_type_expression(*param.type));
    }
    if (!expr.return_type_specifier.has_value())
    {
      throw std::runtime_error{"return type deduction not yet supported"};
    }
    auto const return_type =
      compile_type_expression(*expr.return_type_specifier->type);
    return _type_pool->function_type(std::move(parameter_types), return_type);
  }

  Type *Compilation_context::type_of_expression(
    basedparse::Paren_expression const &expr
  )
  {
    return type_of_expression(*expr.inner);
  }

  Type *Compilation_context::type_of_expression(
    basedparse::Prefix_expression const &expr
  )
  {
    auto const op = basedparse::get_prefix_operator(expr.op.token);
    assert(op.has_value());
    auto const operand_type = type_of_expression(*expr.operand);
    auto const overload = find_unary_overload(*op, operand_type);
    if (overload != nullptr)
    {
      return overload->result_type(operand_type);
    }
    emit_error(
      std::format(
        "no matching overload for prefix operator '{}'",
        expr.op.text
      ),
      expr.op
    );
  }

  Type *Compilation_context::type_of_expression(
    basedparse::Postfix_expression const &expr
  )
  {
    auto const op = basedparse::get_postfix_operator(expr.op.token);
    assert(op.has_value());
    auto const operand_type = type_of_expression(*expr.operand);
    auto const overload = find_unary_overload(*op, operand_type);
    if (overload != nullptr)
    {
      return overload->result_type(operand_type);
    }
    emit_error(
      std::format(
        "no matching overload for postfix operator '{}'",
        expr.op.text
      ),
      expr.op
    );
  }

  Type *Compilation_context::type_of_expression(
    basedparse::Binary_expression const &expr
  )
  {
    auto const op = basedparse::get_binary_operator(expr.op.token);
    assert(op.has_value());
    auto const lhs_type = type_of_expression(*expr.left);
    auto const rhs_type = type_of_expression(*expr.right);
    auto const overload = find_binary_overload(*op, lhs_type, rhs_type);
    if (overload != nullptr)
    {
      return overload->result_type();
    }
    emit_error(
      std::format(
        "no matching overload for binary operator '{}'",
        expr.op.text
      ),
      expr.op
    );
  }

  Type *Compilation_context::type_of_expression(
    basedparse::Call_expression const &expr
  )
  {
    auto const callee_type = type_of_expression(*expr.callee);
    auto const ft = std::get_if<Function_type>(&callee_type->data);
    if (ft == nullptr)
    {
      emit_error("expression is not callable", expr.lparen);
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
    for (auto i = std::size_t{}; i < expr.arguments.size(); ++i)
    {
      auto const arg_type = type_of_expression(expr.arguments[i]);
      if (!is_type_compatible(ft->parameter_types[i], arg_type))
      {
        auto const span = basedparse::span_of(expr.arguments[i]);
        emit_error(
          std::format(
            "argument {} is not compatible with parameter type",
            i + 1
          ),
          span.start
        );
      }
    }
    return ft->return_type;
  }

  Type *Compilation_context::type_of_expression(
    basedparse::Prefix_bracket_expression const &
  )
  {
    return _type_pool->type_type();
  }

  Type *Compilation_context::type_of_expression(
    basedparse::Index_expression const &expr
  )
  {
    auto const operand_type = type_of_expression(*expr.operand);
    auto const sa = std::get_if<Sized_array_type>(&operand_type->data);
    if (sa != nullptr)
    {
      return sa->element;
    }
    auto const ua = std::get_if<Unsized_array_type>(&operand_type->data);
    if (ua != nullptr)
    {
      return ua->element;
    }
    emit_error("expression is not indexable", expr.lbracket);
  }

  Type *Compilation_context::type_of_expression(
    basedparse::Block_expression const &expr
  )
  {
    if (!expr.tail)
    {
      return _type_pool->void_type();
    }
    _symbol_table.push_scope();
    // TODO: handle inner function definitions
    for (auto const &stmt : expr.statements)
    {
      auto const let = std::get_if<basedparse::Let_statement>(&stmt.value);
      if (let != nullptr)
      {
        auto const type = type_of_expression(let->initializer);
        _symbol_table
          .declare_object(let->name.text, type, let->kw_mut.has_value());
      }
    }
    auto const result = type_of_expression(*expr.tail);
    _symbol_table.pop_scope();
    return result;
  }

  // TODO: consider what to do when branch types differ only in mutability
  Type *
  Compilation_context::type_of_expression(basedparse::If_expression const &expr)
  {
    if (!expr.else_part.has_value())
    {
      return _type_pool->void_type();
    }
    auto const then_type = type_of_expression(expr.then_block);
    for (auto const &else_if : expr.else_if_parts)
    {
      auto const else_if_type = type_of_expression(else_if.body);
      if (else_if_type != then_type)
      {
        emit_error(
          "else-if branch type does not match then branch type",
          else_if.body.lbrace
        );
      }
    }
    auto const else_type = type_of_expression(expr.else_part->body);
    if (else_type != then_type)
    {
      emit_error(
        "else branch type does not match then branch type",
        expr.else_part->body.lbrace
      );
    }
    return then_type;
  }

  Constant_value Compilation_context::evaluate_constant_expression(
    basedparse::Expression const &expr
  )
  {
    return std::visit(
      [&](auto const &e) -> Constant_value
      {
        return evaluate_constant_expression(e);
      },
      expr.value
    );
  }

  Constant_value Compilation_context::evaluate_constant_expression(
    basedparse::Int_literal_expression const &expr
  )
  {
    return static_cast<std::int32_t>(std::stoi(expr.literal.text));
  }

  Constant_value Compilation_context::evaluate_constant_expression(
    basedparse::Paren_expression const &expr
  )
  {
    return evaluate_constant_expression(*expr.inner);
  }

  Constant_value Compilation_context::evaluate_constant_expression(
    basedparse::Prefix_expression const &expr
  )
  {
    auto const op = basedparse::get_prefix_operator(expr.op.token);
    assert(op.has_value());
    auto const operand_type = type_of_expression(*expr.operand);
    auto const overload = find_unary_overload(*op, operand_type);
    assert(overload != nullptr);
    return overload->evaluate(evaluate_constant_expression(*expr.operand));
  }

  Constant_value Compilation_context::evaluate_constant_expression(
    basedparse::Postfix_expression const &expr
  )
  {
    emit_error("dereference is not a constant expression", expr.op);
  }

  Constant_value Compilation_context::evaluate_constant_expression(
    basedparse::Binary_expression const &expr
  )
  {
    auto const op = basedparse::get_binary_operator(expr.op.token);
    assert(op.has_value());
    auto const lhs_type = type_of_expression(*expr.left);
    auto const rhs_type = type_of_expression(*expr.right);
    auto const overload = find_binary_overload(*op, lhs_type, rhs_type);
    assert(overload != nullptr);
    return overload->evaluate(
      evaluate_constant_expression(*expr.left),
      evaluate_constant_expression(*expr.right)
    );
  }

  Constant_value Compilation_context::evaluate_constant_expression(
    basedparse::Identifier_expression const &expr
  )
  {
    auto const sym = lookup_identifier(expr.identifier);
    auto const cv = std::get_if<Constant_value>(&sym->data);
    if (cv == nullptr)
    {
      emit_error("expression is not a compile-time constant", expr.identifier);
    }
    return *cv;
  }

  Constant_value Compilation_context::evaluate_constant_expression(
    basedparse::Recurse_expression const &expr
  )
  {
    if (is_top_level())
    {
      emit_error("'recurse' used outside of a function body", expr.kw_recurse);
    }
    return Function_value{.function = _current_function};
  }

  Constant_value Compilation_context::evaluate_constant_expression(
    basedparse::Fn_expression const &expr
  )
  {
    return Function_value{.function = compile_function(expr)};
  }

  Constant_value Compilation_context::evaluate_constant_expression(
    basedparse::Call_expression const &expr
  )
  {
    auto const callee = evaluate_constant_expression(*expr.callee);
    auto const fv = std::get_if<Function_value>(&callee);
    if (fv == nullptr)
    {
      auto const span = basedparse::span_of(*expr.callee);
      emit_error("expression is not callable at compile time", span.start);
    }
    auto args = std::vector<Constant_value>{};
    for (auto const &arg : expr.arguments)
    {
      args.push_back(evaluate_constant_expression(arg));
    }
    return interpret(*fv->function, args);
  }

  Constant_value Compilation_context::evaluate_constant_expression(
    basedparse::Prefix_bracket_expression const &expr
  )
  {
    auto const element =
      std::get<Type_value>(evaluate_constant_expression(*expr.operand)).type;
    if (expr.size == nullptr)
    {
      return Type_value{_type_pool->unsized_array_type(element)};
    }
    auto const size_type = type_of_expression(*expr.size);
    if (size_type != _type_pool->int32_type())
    {
      auto const span = basedparse::span_of(*expr.size);
      emit_error("array size must be an integer", span.start);
    }
    auto const size =
      std::get<std::int32_t>(evaluate_constant_expression(*expr.size));
    if (size <= 0)
    {
      auto const span = basedparse::span_of(*expr.size);
      emit_error("array size must be positive", span.start);
    }
    return Type_value{_type_pool->sized_array_type(element, size)};
  }

  Constant_value Compilation_context::evaluate_constant_expression(
    basedparse::Index_expression const &expr
  )
  {
    auto const span = basedparse::span_of(expr);
    emit_error("expression is not a compile-time constant", span.start);
  }

  Constant_value Compilation_context::evaluate_constant_expression(
    basedparse::Block_expression const &expr
  )
  {
    if (!expr.tail)
    {
      return Void_value{};
    }
    return evaluate_constant_expression(*expr.tail);
  }

  Constant_value Compilation_context::evaluate_constant_expression(
    basedparse::If_expression const &expr
  )
  {
    if (!expr.else_part.has_value())
    {
      return Void_value{};
    }
    auto const condition = evaluate_constant_expression(*expr.condition);
    if (std::get<bool>(condition))
    {
      return evaluate_constant_expression(expr.then_block);
    }
    for (auto const &else_if : expr.else_if_parts)
    {
      auto const else_if_condition =
        evaluate_constant_expression(*else_if.condition);
      if (std::get<bool>(else_if_condition))
      {
        return evaluate_constant_expression(else_if.body);
      }
    }
    return evaluate_constant_expression(expr.else_part->body);
  }

  bool Compilation_context::is_top_level() const
  {
    return _current_function == nullptr;
  }

  Register Compilation_context::allocate_register()
  {
    assert(!is_top_level());
    return Register{_next_register++};
  }

  void Compilation_context::emit(Instruction instruction)
  {
    assert(_current_body != nullptr);
    _current_body->push_back(std::move(instruction));
  }

  Register
  Compilation_context::compile_expression(basedparse::Expression const &expr)
  {
    return std::visit(
      [this](auto const &e) -> Register
      {
        return compile_expression(e);
      },
      expr.value
    );
  }

  Register Compilation_context::compile_expression(
    basedparse::Int_literal_expression const &expr
  )
  {
    auto const result = allocate_register();
    auto const value = std::stoi(expr.literal.text);
    emit(
      Instruction{
        .data = Int32_constant_instruction{.result = result, .value = value}
      }
    );
    return result;
  }

  Register Compilation_context::compile_expression(
    basedparse::Identifier_expression const &expr
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
    auto const result = allocate_register();
    std::visit(
      [&](auto const &v)
      {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, std::int32_t>)
        {
          emit(
            Instruction{
              .data = Int32_constant_instruction{.result = result, .value = v}
            }
          );
        }
        else if constexpr (std::is_same_v<T, bool>)
        {
          emit(
            Instruction{
              .data = Bool_constant_instruction{.result = result, .value = v}
            }
          );
        }
        else if constexpr (std::is_same_v<T, Void_value>)
        {
          emit(
            Instruction{.data = Void_constant_instruction{.result = result}}
          );
        }
        else
        {
          emit_error(
            "cannot compile this constant as a runtime value",
            expr.identifier
          );
        }
      },
      *cv
    );
    return result;
  }

  Register Compilation_context::compile_expression(
    basedparse::Recurse_expression const &expr
  )
  {
    if (is_top_level())
    {
      emit_error("'recurse' used outside of a function body", expr.kw_recurse);
    }
    // recurse is not a runtime value — it's resolved at call sites
    emit_error(
      "'recurse' cannot be used as a value; use it in a call expression",
      expr.kw_recurse
    );
  }

  Register
  Compilation_context::compile_expression(basedparse::Fn_expression const &expr)
  {
    emit_error(
      "function expressions cannot appear in this position",
      expr.kw_fn
    );
  }

  Register Compilation_context::compile_expression(
    basedparse::Paren_expression const &expr
  )
  {
    return compile_expression(*expr.inner);
  }

  Register Compilation_context::compile_expression(
    basedparse::Prefix_expression const &expr
  )
  {
    auto const operand_type = type_of_expression(*expr.operand);
    auto const op = basedparse::get_prefix_operator(expr.op.token);
    auto const overload = find_unary_overload(*op, operand_type);
    auto const operand = compile_expression(*expr.operand);
    auto const result = allocate_register();
    emit(
      Instruction{
        .data = Unary_instruction{
          .result = result,
          .overload = overload,
          .operand = operand,
        }
      }
    );
    return result;
  }

  Register Compilation_context::compile_expression(
    basedparse::Postfix_expression const &expr
  )
  {
    emit_error("dereference is not supported in this context", expr.op);
  }

  Register Compilation_context::compile_expression(
    basedparse::Binary_expression const &expr
  )
  {
    auto const lhs_type = type_of_expression(*expr.left);
    auto const rhs_type = type_of_expression(*expr.right);
    auto const op = basedparse::get_binary_operator(expr.op.token);
    auto const overload = find_binary_overload(*op, lhs_type, rhs_type);
    auto const lhs = compile_expression(*expr.left);
    auto const rhs = compile_expression(*expr.right);
    auto const result = allocate_register();
    emit(
      Instruction{
        .data = Binary_instruction{
          .result = result,
          .overload = overload,
          .lhs = lhs,
          .rhs = rhs,
        }
      }
    );
    return result;
  }

  Register Compilation_context::compile_expression(
    basedparse::Call_expression const &expr
  )
  {
    // Resolve callee to a Function* — must be a compile-time constant
    auto const callee_type = type_of_expression(*expr.callee);
    auto const ft = std::get_if<Function_type>(&callee_type->data);
    if (ft == nullptr)
    {
      emit_error("expression is not callable", expr.lparen);
    }
    Function *callee = nullptr;
    if (auto const ident =
          std::get_if<basedparse::Identifier_expression>(&expr.callee->value))
    {
      auto const sym = lookup_identifier(ident->identifier);
      auto const cv = std::get_if<Constant_value>(&sym->data);
      if (cv != nullptr)
      {
        auto const fv = std::get_if<Function_value>(cv);
        if (fv != nullptr)
        {
          callee = fv->function;
        }
      }
    }
    else if (std::holds_alternative<basedparse::Recurse_expression>(
               expr.callee->value
             ))
    {
      callee = _current_function;
    }
    if (callee == nullptr)
    {
      auto const span = basedparse::span_of(*expr.callee);
      emit_error("callee must be a known function", span.start);
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
    for (auto i = std::size_t{}; i < expr.arguments.size(); ++i)
    {
      auto const arg_type = type_of_expression(expr.arguments[i]);
      if (!is_type_compatible(ft->parameter_types[i], arg_type))
      {
        auto const span = basedparse::span_of(expr.arguments[i]);
        emit_error(
          std::format(
            "argument {} is not compatible with parameter type",
            i + 1
          ),
          span.start
        );
      }
    }
    auto args = std::vector<Register>{};
    for (auto const &arg : expr.arguments)
    {
      args.push_back(compile_expression(arg));
    }
    auto const result = allocate_register();
    emit(
      Instruction{
        .data = Call_instruction{
          .result = result,
          .callee = callee,
          .arguments = std::move(args),
        }
      }
    );
    return result;
  }

  Register Compilation_context::compile_expression(
    basedparse::Index_expression const &
  )
  {
    throw std::runtime_error{"index expressions are not implemented"};
  }

  Register Compilation_context::compile_expression(
    basedparse::Prefix_bracket_expression const &
  )
  {
    std::unreachable();
  }

  Register Compilation_context::compile_expression(
    basedparse::Block_expression const &expr
  )
  {
    _symbol_table.push_scope();
    for (auto const &stmt : expr.statements)
    {
      compile_statement(stmt);
    }
    auto const result = expr.tail ? compile_expression(*expr.tail) : [this]
    {
      auto const r = allocate_register();
      emit(Instruction{.data = Void_constant_instruction{.result = r}});
      return r;
    }();
    _symbol_table.pop_scope();
    return result;
  }

  Register
  Compilation_context::compile_expression(basedparse::If_expression const &expr)
  {
    auto const result = allocate_register();
    auto const compile_block = [this](auto compile_fn) -> std::unique_ptr<Block>
    {
      auto instructions = std::vector<Instruction>{};
      auto const saved_body = Scoped_assign{_current_body, &instructions};
      auto const block_result = compile_fn();
      return std::make_unique<Block>(
        Block{.result = block_result, .instructions = std::move(instructions)}
      );
    };
    auto condition = compile_block(
      [&]
      {
        return compile_expression(*expr.condition);
      }
    );
    auto then_body = compile_block(
      [&]
      {
        return compile_expression(expr.then_block);
      }
    );
    auto else_ifs = std::vector<If_instruction::Else_if>{};
    for (auto const &part : expr.else_if_parts)
    {
      auto ei_condition = compile_block(
        [&]
        {
          return compile_expression(*part.condition);
        }
      );
      auto ei_body = compile_block(
        [&]
        {
          return compile_expression(part.body);
        }
      );
      else_ifs.push_back(
        If_instruction::Else_if{
          .condition = std::move(ei_condition),
          .body = std::move(ei_body),
        }
      );
    }
    auto else_body = std::unique_ptr<Block>{};
    if (expr.else_part.has_value())
    {
      else_body = compile_block(
        [&]
        {
          return compile_expression(expr.else_part->body);
        }
      );
    }
    emit(
      Instruction{
        .data = If_instruction{
          .result = result,
          .condition = std::move(condition),
          .then_body = std::move(then_body),
          .else_ifs = std::move(else_ifs),
          .else_body = std::move(else_body),
        }
      }
    );
    return result;
  }

  void Compilation_context::compile_statement(basedparse::Statement const &stmt)
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
    basedparse::Let_statement const &stmt
  )
  {
    auto const is_mutable = stmt.kw_mut.has_value();
    auto const type = type_of_expression(stmt.initializer);
    auto const is_object = is_object_type(type);
    if (is_mutable && !is_object)
    {
      emit_error("mutable binding requires an object type", *stmt.kw_mut);
    }
    if (is_mutable && is_top_level())
    {
      emit_error("top-level bindings cannot be mutable", *stmt.kw_mut);
    }
    auto const requires_const_eval = is_top_level() || !is_object;
    if (requires_const_eval)
    {
      auto const value = evaluate_constant_expression(stmt.initializer);
      _symbol_table.declare_value(stmt.name.text, value);
    }
    else
    {
      auto const reg = compile_expression(stmt.initializer);
      _symbol_table.declare_object(stmt.name.text, type, is_mutable, reg);
    }
  }

  void Compilation_context::compile_statement(
    basedparse::While_statement const &stmt
  )
  {
    emit_error(
      "while statements are not supported without mutation",
      stmt.kw_while
    );
  }

  void Compilation_context::compile_statement(
    basedparse::Return_statement const &stmt
  )
  {
    auto const value = compile_expression(stmt.value);
    emit(Instruction{.data = Return_instruction{.value = value}});
  }

  void Compilation_context::compile_statement(
    basedparse::Expression_statement const &stmt
  )
  {
    compile_expression(stmt.expression);
  }

  Function *Compilation_context::compile_function(
    basedparse::Fn_expression const &expr
  )
  {
    auto const fn_type = type_of_expression(expr);
    auto const ft = std::get<Function_type>(fn_type->data);
    auto params = std::vector<Parameter>{};
    for (auto const &param : expr.parameters)
    {
      params.push_back(
        Parameter{.type = compile_type_expression(*param.type)}
      );
    }
    auto func = std::make_unique<Function>(Function{
      .type = fn_type,
      .parameters = std::move(params),
      .body = {},
      .register_count = 0,
    });
    auto const func_ptr = func.get();
    _translation_unit.functions.push_back(std::move(func));
    auto const saved_function = Scoped_assign{_current_function, func_ptr};
    auto const saved_next_register =
      Scoped_assign{_next_register, std::int32_t{}};
    auto const saved_body = Scoped_assign{_current_body, &func_ptr->body};
    _symbol_table.push_scope(true);
    for (auto i = std::size_t{}; i < func_ptr->parameters.size(); ++i)
    {
      auto const reg = allocate_register();
      _symbol_table.declare_object(
        expr.parameters[i].name.text,
        ft.parameter_types[i],
        false,
        reg
      );
    }
    // Compile the body
    if (auto const block =
          std::get_if<basedparse::Block_expression>(&expr.body->value))
    {
      for (auto const &stmt : block->statements)
      {
        compile_statement(stmt);
      }
      if (block->tail)
      {
        auto const tail_reg = compile_expression(*block->tail);
        emit(Instruction{.data = Return_instruction{.value = tail_reg}});
      }
    }
    else
    {
      auto const body_reg = compile_expression(*expr.body);
      emit(Instruction{.data = Return_instruction{.value = body_reg}});
    }
    _symbol_table.pop_scope();
    func_ptr->register_count = _next_register;
    return func_ptr;
  }

  Translation_unit
  compile(basedparse::Translation_unit const &ast, Type_pool *type_pool)
  {
    auto ctx = Compilation_context{type_pool};
    return ctx.compile(ast);
  }

} // namespace basedhlir
