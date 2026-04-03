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

  [[noreturn]] void
  Compilation_context::emit_error(std::string message, Source_span location)
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
      emit_error("expected a type expression", expr);
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
        emit_error(
          std::format(
            "argument {} is not compatible with parameter type",
            i + 1
          ),
          expr.arguments[i]
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
    auto const result = compile_expression(expr);
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

  Register Compilation_context::allocate_register()
  {
    assert(_current_function != nullptr);
    return Register{_current_function->register_count++};
  }

  void Compilation_context::emit(Instruction instruction)
  {
    assert(_current_block != nullptr);
    assert(!_current_block->has_terminator());
    _current_block->instructions.push_back(std::move(instruction));
  }

  void Compilation_context::emit(Terminator terminator)
  {
    assert(_current_block != nullptr);
    assert(!_current_block->has_terminator());
    _current_block->terminator = std::move(terminator);
  }

  Operand to_operand(Expression_compile_result const &result)
  {
    return std::visit(
      [](auto const &r) -> Operand
      {
        using T = std::decay_t<decltype(r)>;
        if constexpr (std::is_same_v<T, Typed_register>)
        {
          return r.reg;
        }
        else
        {
          return r;
        }
      },
      result
    );
  }

  Expression_compile_result
  Compilation_context::compile_expression(basedparse::Expression const &expr)
  {
    auto const result = std::visit(
      [this](auto const &e) -> Expression_compile_result
      {
        return compile_expression(e);
      },
      expr.value
    );
    if (_current_block != nullptr)
    {
      assert(!_current_block->has_terminator());
    }
    return result;
  }

  Expression_compile_result Compilation_context::compile_expression(
    basedparse::Int_literal_expression const &expr
  )
  {
    return Constant_value{
      static_cast<std::int32_t>(std::stoi(expr.literal.text))
    };
  }

  Expression_compile_result Compilation_context::compile_expression(
    basedparse::Identifier_expression const &expr
  )
  {
    auto const sym = lookup_identifier(expr.identifier);
    auto const ob = std::get_if<Object_binding>(&sym->data);
    if (ob != nullptr)
    {
      return Typed_register{ob->reg, ob->type};
    }
    auto const cv = std::get_if<Constant_value>(&sym->data);
    assert(cv != nullptr);
    return *cv;
  }

  Expression_compile_result Compilation_context::compile_expression(
    basedparse::Recurse_expression const &expr
  )
  {
    if (is_top_level())
    {
      emit_error("'recurse' used outside of a function body", expr.kw_recurse);
    }
    return Constant_value{Function_value{.function = _current_function}};
  }

  Expression_compile_result
  Compilation_context::compile_expression(basedparse::Fn_expression const &expr)
  {
    return Constant_value{Function_value{.function = compile_function(expr)}};
  }

  Expression_compile_result Compilation_context::compile_expression(
    basedparse::Paren_expression const &expr
  )
  {
    return compile_expression(*expr.inner);
  }

  Expression_compile_result Compilation_context::compile_expression(
    basedparse::Prefix_expression const &expr
  )
  {
    auto const operand_result = compile_expression(*expr.operand);
    auto const op = basedparse::get_prefix_operator(expr.op.token);
    auto const operand_type = type_of_expression(*expr.operand);
    auto const overload = find_unary_overload(*op, operand_type);
    if (auto const cv = std::get_if<Constant_value>(&operand_result))
    {
      return overload->evaluate(*cv);
    }
    auto const result = allocate_register();
    emit(
      Instruction{Unary_instruction{
        .result = result,
        .overload = overload,
        .operand = to_operand(operand_result),
      }}
    );
    return Typed_register{result, overload->result_type(operand_type)};
  }

  Expression_compile_result Compilation_context::compile_expression(
    basedparse::Postfix_expression const &expr
  )
  {
    emit_error("dereference is not supported in this context", expr.op);
  }

  Expression_compile_result Compilation_context::compile_expression(
    basedparse::Binary_expression const &expr
  )
  {
    auto const lhs_result = compile_expression(*expr.left);
    auto const rhs_result = compile_expression(*expr.right);
    auto const op = basedparse::get_binary_operator(expr.op.token);
    auto const lhs_type = type_of_expression(*expr.left);
    auto const rhs_type = type_of_expression(*expr.right);
    auto const overload = find_binary_overload(*op, lhs_type, rhs_type);
    auto const lhs_cv = std::get_if<Constant_value>(&lhs_result);
    auto const rhs_cv = std::get_if<Constant_value>(&rhs_result);
    if (lhs_cv != nullptr && rhs_cv != nullptr)
    {
      return overload->evaluate(*lhs_cv, *rhs_cv);
    }
    auto const result = allocate_register();
    emit(
      Instruction{Binary_instruction{
        .result = result,
        .overload = overload,
        .lhs = to_operand(lhs_result),
        .rhs = to_operand(rhs_result),
      }}
    );
    return Typed_register{result, overload->result_type()};
  }

  Expression_compile_result Compilation_context::compile_expression(
    basedparse::Call_expression const &expr
  )
  {
    auto const callee_type = type_of_expression(*expr.callee);
    auto const ft = std::get_if<Function_type>(&callee_type->data);
    if (ft == nullptr)
    {
      emit_error("expression is not callable", expr.lparen);
    }
    auto const callee_result = compile_expression(*expr.callee);
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
    auto arg_results = std::vector<Expression_compile_result>{};
    arg_results.reserve(expr.arguments.size());
    auto all_constant = bool{true};
    for (auto i = std::size_t{}; i < expr.arguments.size(); ++i)
    {
      auto const arg_result = compile_expression(expr.arguments[i]);
      auto const arg_type = type_of_expression(expr.arguments[i]);
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
    auto runtime_args = std::vector<Operand>{};
    runtime_args.reserve(arg_results.size());
    for (auto const &r : arg_results)
    {
      runtime_args.push_back(to_operand(r));
    }
    auto const result = allocate_register();
    emit(
      Instruction{Call_instruction{
        .result = result,
        .callee = fv->function,
        .arguments = std::move(runtime_args),
      }}
    );
    return Typed_register{result, ft->return_type};
  }

  Expression_compile_result
  Compilation_context::compile_expression(basedparse::Index_expression const &)
  {
    throw std::runtime_error{"index expressions are not implemented"};
  }

  Expression_compile_result Compilation_context::compile_expression(
    basedparse::Prefix_bracket_expression const &expr
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
    auto const size_type = type_of_expression(*expr.size);
    if (size_type != _type_pool->int32_type())
    {
      emit_error("array size must be an integer", *expr.size);
    }
    auto const size_result = compile_expression(*expr.size);
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

  Expression_compile_result Compilation_context::compile_expression(
    basedparse::Block_expression const &expr
  )
  {
    _symbol_table.push_scope();
    for (auto const &stmt : expr.statements)
    {
      compile_statement(stmt);
    }
    auto const result =
      expr.tail ? compile_expression(*expr.tail)
                : Expression_compile_result{Constant_value{Void_value{}}};
    _symbol_table.pop_scope();
    return result;
  }

  Expression_compile_result
  Compilation_context::compile_expression(basedparse::If_expression const &expr)
  {
    auto const cond_result = compile_expression(*expr.condition);
    // Constant condition folding
    if (auto const cond_cv = std::get_if<Constant_value>(&cond_result))
    {
      if (!expr.else_part.has_value())
      {
        return Constant_value{Void_value{}};
      }
      if (std::get<bool>(*cond_cv))
      {
        return compile_expression(expr.then_block);
      }
      if (expr.else_if_parts.empty())
      {
        return compile_expression(expr.else_part->body);
      }
      // Constant false with else-if parts: fold at top level (where emitting
      // instructions isn't possible), fall through to runtime path otherwise
      if (is_top_level())
      {
        for (auto const &part : expr.else_if_parts)
        {
          auto const ei_result = compile_expression(*part.condition);
          auto const ei_cv = std::get_if<Constant_value>(&ei_result);
          if (ei_cv == nullptr)
          {
            emit_error(
              "else-if condition is not a compile-time constant",
              *part.condition
            );
          }
          if (std::get<bool>(*ei_cv))
          {
            return compile_expression(part.body);
          }
        }
        return compile_expression(expr.else_part->body);
      }
    }
    // Runtime condition path
    auto const merge_block = new_block();
    auto const then_block = new_block();
    auto const first_else_target = [&]() -> Basic_block *
    {
      if (!expr.else_if_parts.empty())
      {
        return new_block();
      }
      if (expr.else_part.has_value())
      {
        return new_block();
      }
      return merge_block;
    }();
    emit(
      Terminator{Branch_terminator{
        .condition = to_operand(cond_result),
        .then_target = then_block,
        .then_arguments = {},
        .else_target = first_else_target,
        .else_arguments = {},
      }}
    );
    // Compile then block
    set_current_block(then_block);
    auto const then_result = compile_expression(expr.then_block);
    auto const then_type = type_of_expression(expr.then_block);
    auto const merge_param = [&]() -> Register
    {
      if (then_type == _type_pool->void_type())
      {
        return Register{};
      }
      auto const r = allocate_register();
      merge_block->parameters.push_back(r);
      return r;
    }();
    auto const jump_to_merge = [&](Expression_compile_result const &result)
    {
      emit(
        Terminator{Jump_terminator{
          .target = merge_block,
          .arguments = merge_param ? std::vector<Operand>{to_operand(result)}
                                   : std::vector<Operand>{},
        }}
      );
    };
    jump_to_merge(then_result);
    // Compile else-if chain
    auto current_else_block = first_else_target;
    for (auto i = std::size_t{}; i < expr.else_if_parts.size(); ++i)
    {
      auto const &part = expr.else_if_parts[i];
      set_current_block(current_else_block);
      auto const ei_cond_result = compile_expression(*part.condition);
      auto const ei_then = new_block();
      auto const ei_else = [&]() -> Basic_block *
      {
        if (i + 1 < expr.else_if_parts.size())
        {
          return new_block();
        }
        if (expr.else_part.has_value())
        {
          return new_block();
        }
        return merge_block;
      }();
      emit(
        Terminator{Branch_terminator{
          .condition = to_operand(ei_cond_result),
          .then_target = ei_then,
          .then_arguments = {},
          .else_target = ei_else,
          .else_arguments = {},
        }}
      );
      set_current_block(ei_then);
      auto const ei_result = compile_expression(part.body);
      auto const ei_type = type_of_expression(part.body);
      if (ei_type != then_type)
      {
        emit_error(
          "else-if branch type does not match then branch type",
          part.body.lbrace
        );
      }
      jump_to_merge(ei_result);
      current_else_block = ei_else;
    }
    // Compile else block
    if (expr.else_part.has_value())
    {
      set_current_block(current_else_block);
      auto const else_result = compile_expression(expr.else_part->body);
      auto const else_type = type_of_expression(expr.else_part->body);
      if (else_type != then_type)
      {
        emit_error(
          "else branch type does not match then branch type",
          expr.else_part->body.lbrace
        );
      }
      jump_to_merge(else_result);
    }
    set_current_block(merge_block);
    if (merge_param)
    {
      return Typed_register{merge_param, then_type};
    }
    return Constant_value{Void_value{}};
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
    auto const result = compile_expression(stmt.initializer);
    if (is_top_level() || !is_object)
    {
      auto const cv = std::get_if<Constant_value>(&result);
      if (cv == nullptr)
      {
        emit_error(
          "expression is not a compile-time constant",
          stmt.initializer
        );
      }
      _symbol_table.declare_value(stmt.name.text, *cv);
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
        auto const tr = std::get<Typed_register>(result);
        _symbol_table.declare_object(stmt.name.text, tr.type, false, tr.reg);
      }
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
    auto const result = compile_expression(stmt.value);
    emit(Terminator{Return_terminator{.value = to_operand(result)}});
    // Start a new (dead) block for any subsequent code
    set_current_block(new_block());
  }

  void Compilation_context::compile_statement(
    basedparse::Expression_statement const &stmt
  )
  {
    compile_expression(stmt.expression);
  }

  Function *
  Compilation_context::compile_function(basedparse::Fn_expression const &expr)
  {
    auto const fn_type = type_of_expression(expr);
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
    auto const entry = new_block();
    set_current_block(entry);
    _symbol_table.push_scope(true);
    for (auto i = std::size_t{}; i < expr.parameters.size(); ++i)
    {
      auto const reg = allocate_register();
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
    emit(Terminator{Return_terminator{.value = to_operand(body_result)}});
    _symbol_table.pop_scope();
    for (auto const &block : func_ptr->blocks)
    {
      assert(block->has_terminator());
    }
    return func_ptr;
  }

  Translation_unit
  compile(basedparse::Translation_unit const &ast, Type_pool *type_pool)
  {
    auto ctx = Compilation_context{type_pool};
    return ctx.compile(ast);
  }

} // namespace basedhlir
