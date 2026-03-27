#include <cassert>
#include <cstdint>
#include <format>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include <basedparse/operator.h>
#include <basedparse/source_span.h>

#include "basedhlir/compile.h"
#include "basedhlir/symbol_table.h"

namespace basedhlir
{

  namespace
  {

    struct Compilation_error
    {
    };

    using Constant_value = std::variant<std::int32_t, bool>;

    class Unary_operator_overload
    {
    public:
      virtual ~Unary_operator_overload() = default;

      virtual Type *operand_type() const = 0;

      virtual Type *result_type() const = 0;

      virtual Constant_value evaluate(Constant_value operand) const = 0;
    };

    class Binary_operator_overload
    {
    public:
      virtual ~Binary_operator_overload() = default;

      virtual Type *lhs_type() const = 0;

      virtual Type *rhs_type() const = 0;

      virtual Type *result_type() const = 0;

      virtual Constant_value evaluate(Constant_value lhs, Constant_value rhs) const = 0;
    };

    class I32_unary_plus final : public Unary_operator_overload
    {
    public:
      explicit I32_unary_plus(Type_pool *type_pool) : _type{type_pool->i32_type()} {}
      Type *operand_type() const override { return _type; }
      Type *result_type() const override { return _type; }
      Constant_value evaluate(Constant_value operand) const override { return std::get<std::int32_t>(operand); }
    private:
      Type *_type;
    };

    class I32_unary_minus final : public Unary_operator_overload
    {
    public:
      explicit I32_unary_minus(Type_pool *type_pool) : _type{type_pool->i32_type()} {}
      Type *operand_type() const override { return _type; }
      Type *result_type() const override { return _type; }
      Constant_value evaluate(Constant_value operand) const override { return -std::get<std::int32_t>(operand); }
    private:
      Type *_type;
    };

    class I32_add final : public Binary_operator_overload
    {
    public:
      explicit I32_add(Type_pool *type_pool) : _type{type_pool->i32_type()} {}
      Type *lhs_type() const override { return _type; }
      Type *rhs_type() const override { return _type; }
      Type *result_type() const override { return _type; }
      Constant_value evaluate(Constant_value lhs, Constant_value rhs) const override { return std::get<std::int32_t>(lhs) + std::get<std::int32_t>(rhs); }
    private:
      Type *_type;
    };

    class I32_subtract final : public Binary_operator_overload
    {
    public:
      explicit I32_subtract(Type_pool *type_pool) : _type{type_pool->i32_type()} {}
      Type *lhs_type() const override { return _type; }
      Type *rhs_type() const override { return _type; }
      Type *result_type() const override { return _type; }
      Constant_value evaluate(Constant_value lhs, Constant_value rhs) const override { return std::get<std::int32_t>(lhs) - std::get<std::int32_t>(rhs); }
    private:
      Type *_type;
    };

    class I32_multiply final : public Binary_operator_overload
    {
    public:
      explicit I32_multiply(Type_pool *type_pool) : _type{type_pool->i32_type()} {}
      Type *lhs_type() const override { return _type; }
      Type *rhs_type() const override { return _type; }
      Type *result_type() const override { return _type; }
      Constant_value evaluate(Constant_value lhs, Constant_value rhs) const override { return std::get<std::int32_t>(lhs) * std::get<std::int32_t>(rhs); }
    private:
      Type *_type;
    };

    class I32_divide final : public Binary_operator_overload
    {
    public:
      explicit I32_divide(Type_pool *type_pool) : _type{type_pool->i32_type()} {}
      Type *lhs_type() const override { return _type; }
      Type *rhs_type() const override { return _type; }
      Type *result_type() const override { return _type; }
      Constant_value evaluate(Constant_value lhs, Constant_value rhs) const override { return std::get<std::int32_t>(lhs) / std::get<std::int32_t>(rhs); }
    private:
      Type *_type;
    };

    class I32_modulo final : public Binary_operator_overload
    {
    public:
      explicit I32_modulo(Type_pool *type_pool) : _type{type_pool->i32_type()} {}
      Type *lhs_type() const override { return _type; }
      Type *rhs_type() const override { return _type; }
      Type *result_type() const override { return _type; }
      Constant_value evaluate(Constant_value lhs, Constant_value rhs) const override { return std::get<std::int32_t>(lhs) % std::get<std::int32_t>(rhs); }
    private:
      Type *_type;
    };

    class I32_equal final : public Binary_operator_overload
    {
    public:
      explicit I32_equal(Type_pool *type_pool) : _i32{type_pool->i32_type()}, _bool{type_pool->bool_type()} {}
      Type *lhs_type() const override { return _i32; }
      Type *rhs_type() const override { return _i32; }
      Type *result_type() const override { return _bool; }
      Constant_value evaluate(Constant_value lhs, Constant_value rhs) const override { return std::get<std::int32_t>(lhs) == std::get<std::int32_t>(rhs); }
    private:
      Type *_i32;
      Type *_bool;
    };

    class I32_not_equal final : public Binary_operator_overload
    {
    public:
      explicit I32_not_equal(Type_pool *type_pool) : _i32{type_pool->i32_type()}, _bool{type_pool->bool_type()} {}
      Type *lhs_type() const override { return _i32; }
      Type *rhs_type() const override { return _i32; }
      Type *result_type() const override { return _bool; }
      Constant_value evaluate(Constant_value lhs, Constant_value rhs) const override { return std::get<std::int32_t>(lhs) != std::get<std::int32_t>(rhs); }
    private:
      Type *_i32;
      Type *_bool;
    };

    class I32_less final : public Binary_operator_overload
    {
    public:
      explicit I32_less(Type_pool *type_pool) : _i32{type_pool->i32_type()}, _bool{type_pool->bool_type()} {}
      Type *lhs_type() const override { return _i32; }
      Type *rhs_type() const override { return _i32; }
      Type *result_type() const override { return _bool; }
      Constant_value evaluate(Constant_value lhs, Constant_value rhs) const override { return std::get<std::int32_t>(lhs) < std::get<std::int32_t>(rhs); }
    private:
      Type *_i32;
      Type *_bool;
    };

    class I32_less_eq final : public Binary_operator_overload
    {
    public:
      explicit I32_less_eq(Type_pool *type_pool) : _i32{type_pool->i32_type()}, _bool{type_pool->bool_type()} {}
      Type *lhs_type() const override { return _i32; }
      Type *rhs_type() const override { return _i32; }
      Type *result_type() const override { return _bool; }
      Constant_value evaluate(Constant_value lhs, Constant_value rhs) const override { return std::get<std::int32_t>(lhs) <= std::get<std::int32_t>(rhs); }
    private:
      Type *_i32;
      Type *_bool;
    };

    class I32_greater final : public Binary_operator_overload
    {
    public:
      explicit I32_greater(Type_pool *type_pool) : _i32{type_pool->i32_type()}, _bool{type_pool->bool_type()} {}
      Type *lhs_type() const override { return _i32; }
      Type *rhs_type() const override { return _i32; }
      Type *result_type() const override { return _bool; }
      Constant_value evaluate(Constant_value lhs, Constant_value rhs) const override { return std::get<std::int32_t>(lhs) > std::get<std::int32_t>(rhs); }
    private:
      Type *_i32;
      Type *_bool;
    };

    class I32_greater_eq final : public Binary_operator_overload
    {
    public:
      explicit I32_greater_eq(Type_pool *type_pool) : _i32{type_pool->i32_type()}, _bool{type_pool->bool_type()} {}
      Type *lhs_type() const override { return _i32; }
      Type *rhs_type() const override { return _i32; }
      Type *result_type() const override { return _bool; }
      Constant_value evaluate(Constant_value lhs, Constant_value rhs) const override { return std::get<std::int32_t>(lhs) >= std::get<std::int32_t>(rhs); }
    private:
      Type *_i32;
      Type *_bool;
    };

    class Bool_equal final : public Binary_operator_overload
    {
    public:
      explicit Bool_equal(Type_pool *type_pool) : _bool{type_pool->bool_type()} {}
      Type *lhs_type() const override { return _bool; }
      Type *rhs_type() const override { return _bool; }
      Type *result_type() const override { return _bool; }
      Constant_value evaluate(Constant_value lhs, Constant_value rhs) const override { return std::get<bool>(lhs) == std::get<bool>(rhs); }
    private:
      Type *_bool;
    };

    class Bool_not_equal final : public Binary_operator_overload
    {
    public:
      explicit Bool_not_equal(Type_pool *type_pool) : _bool{type_pool->bool_type()} {}
      Type *lhs_type() const override { return _bool; }
      Type *rhs_type() const override { return _bool; }
      Type *result_type() const override { return _bool; }
      Constant_value evaluate(Constant_value lhs, Constant_value rhs) const override { return std::get<bool>(lhs) != std::get<bool>(rhs); }
    private:
      Type *_bool;
    };

    class Compilation_context
    {
    public:
      explicit Compilation_context(Type_pool *type_pool)
        : _type_pool{type_pool}
      {
        assert(_type_pool != nullptr);
        _symbol_table.declare_type("i32", _type_pool->i32_type());
        _symbol_table.declare_type("bool", _type_pool->bool_type());
        _symbol_table.declare_type("void", _type_pool->void_type());
        _unary_overloads[basedparse::Operator::unary_plus].push_back(std::make_unique<I32_unary_plus>(_type_pool));
        _unary_overloads[basedparse::Operator::unary_minus].push_back(std::make_unique<I32_unary_minus>(_type_pool));
        _binary_overloads[basedparse::Operator::add].push_back(std::make_unique<I32_add>(_type_pool));
        _binary_overloads[basedparse::Operator::subtract].push_back(std::make_unique<I32_subtract>(_type_pool));
        _binary_overloads[basedparse::Operator::multiply].push_back(std::make_unique<I32_multiply>(_type_pool));
        _binary_overloads[basedparse::Operator::divide].push_back(std::make_unique<I32_divide>(_type_pool));
        _binary_overloads[basedparse::Operator::modulo].push_back(std::make_unique<I32_modulo>(_type_pool));
        _binary_overloads[basedparse::Operator::equal].push_back(std::make_unique<I32_equal>(_type_pool));
        _binary_overloads[basedparse::Operator::not_equal].push_back(std::make_unique<I32_not_equal>(_type_pool));
        _binary_overloads[basedparse::Operator::less].push_back(std::make_unique<I32_less>(_type_pool));
        _binary_overloads[basedparse::Operator::less_eq].push_back(std::make_unique<I32_less_eq>(_type_pool));
        _binary_overloads[basedparse::Operator::greater].push_back(std::make_unique<I32_greater>(_type_pool));
        _binary_overloads[basedparse::Operator::greater_eq].push_back(std::make_unique<I32_greater_eq>(_type_pool));
        _binary_overloads[basedparse::Operator::equal].push_back(std::make_unique<Bool_equal>(_type_pool));
        _binary_overloads[basedparse::Operator::not_equal].push_back(std::make_unique<Bool_not_equal>(_type_pool));
      }

      Translation_unit compile(basedparse::Translation_unit const &ast)
      {
        for (auto const &func_def : ast.function_definitions)
        {
          try
          {
            compile_top_level_let_statement(func_def);
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

    private:
      Type_pool *_type_pool;
      Translation_unit _translation_unit;
      Symbol_table _symbol_table;
      std::vector<Diagnostic> _diagnostics;
      std::unordered_map<basedparse::Operator, std::vector<std::unique_ptr<Unary_operator_overload>>> _unary_overloads;
      std::unordered_map<basedparse::Operator, std::vector<std::unique_ptr<Binary_operator_overload>>> _binary_overloads;

      Function *get_function(std::uint64_t function_handle)
      {
        return _translation_unit.functions[function_handle].get();
      }

      [[noreturn]] void emit_error(std::string message, basedlex::Lexeme const &lexeme)
      {
        emit_error(std::move(message), lexeme.location);
      }

      [[noreturn]] void emit_error(std::string message, Source_location location)
      {
        _diagnostics.push_back(Diagnostic{
          .message = std::move(message),
          .location = location,
        });
        throw Compilation_error{};
      }

      Symbol *try_lookup_identifier(basedlex::Lexeme const &identifier)
      {
        return _symbol_table.lookup(identifier.text);
      }

      Symbol *lookup_identifier(basedlex::Lexeme const &identifier)
      {
        auto const sym = try_lookup_identifier(identifier);
        if (sym == nullptr)
        {
          emit_error(std::format("undefined identifier: {}", identifier.text), identifier);
        }
        return sym;
      }

      bool is_type_compatible(Type *parameter_type, Type *argument_type)
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
          auto const param_unsized = std::get_if<Unsized_array_type>(&param_ptr->pointee->data);
          auto const arg_sized = std::get_if<Sized_array_type>(&argument_type->data);
          if (param_unsized != nullptr && arg_sized != nullptr)
          {
            return is_type_compatible(param_unsized->element, arg_sized->element);
          }
        }
        return false;
      }

      Type *compile_type_expression(basedparse::Expression const &expr)
      {
        auto const ident = std::get_if<basedparse::Identifier_expression>(&expr.value);
        if (ident != nullptr)
        {
          auto const sym = lookup_identifier(ident->identifier);
          auto const ts = std::get_if<Type_symbol>(&sym->data);
          if (ts == nullptr)
          {
            emit_error(std::format("'{}' is not a type", ident->identifier.text), ident->identifier);
          }
          return ts->type;
        }
        auto const prefix = std::get_if<basedparse::Prefix_bracket_expression>(&expr.value);
        if (prefix != nullptr)
        {
          auto const element = compile_type_expression(*prefix->operand);
          if (prefix->size == nullptr)
          {
            return _type_pool->unsized_array_type(element);
          }
          throw std::runtime_error{"sized array types not yet supported"};
        }
        auto const unary = std::get_if<basedparse::Unary_expression>(&expr.value);
        if (unary != nullptr)
        {
          if (unary->op.token == basedlex::Token::star)
          {
            return _type_pool->pointer_type(compile_type_expression(*unary->operand), false);
          }
          if (unary->op.token == basedlex::Token::star_mut)
          {
            return _type_pool->pointer_type(compile_type_expression(*unary->operand), true);
          }
        }
        auto const span = basedparse::span_of(expr);
        emit_error("expected a type expression", span.start);
      }

      Type *type_of_expression(basedparse::Expression const &expr)
      {
        return std::visit(
          [&](auto const &e) -> Type *
          {
            return type_of_expression(e);
          },
          expr.value
        );
      }

      Type *type_of_expression(basedparse::Int_literal_expression const &)
      {
        return _type_pool->i32_type();
      }

      Type *type_of_expression(basedparse::Identifier_expression const &expr)
      {
        auto const sym = lookup_identifier(expr.identifier);
        auto const vs = std::get_if<Value_symbol>(&sym->data);
        if (vs != nullptr)
        {
          return vs->type;
        }
        auto const fs = std::get_if<Function_symbol>(&sym->data);
        if (fs != nullptr)
        {
          return get_function(fs->function_handle)->type;
        }
        emit_error(std::format("'{}' is not a value", expr.identifier.text), expr.identifier);
      }

      Type *type_of_expression(basedparse::Fn_expression const &expr)
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
        auto const return_type = compile_type_expression(*expr.return_type_specifier->type);
        return _type_pool->function_type(std::move(parameter_types), return_type);
      }

      Type *type_of_expression(basedparse::Paren_expression const &expr)
      {
        return type_of_expression(*expr.inner);
      }

      Type *type_of_expression(basedparse::Unary_expression const &expr)
      {
        auto const op = basedparse::get_unary_operator(expr.op.token);
        assert(op.has_value());
        auto const operand_type = type_of_expression(*expr.operand);
        auto const it = _unary_overloads.find(*op);
        if (it != _unary_overloads.end())
        {
          for (auto const &overload : it->second)
          {
            if (overload->operand_type() == operand_type)
            {
              return overload->result_type();
            }
          }
        }
        emit_error(std::format("no matching overload for unary operator '{}'", expr.op.text), expr.op);
      }

      Type *type_of_expression(basedparse::Binary_expression const &expr)
      {
        auto const op = basedparse::get_binary_operator(expr.op.token);
        assert(op.has_value());
        auto const lhs_type = type_of_expression(*expr.left);
        auto const rhs_type = type_of_expression(*expr.right);
        auto const it = _binary_overloads.find(*op);
        if (it != _binary_overloads.end())
        {
          for (auto const &overload : it->second)
          {
            if (overload->lhs_type() == lhs_type && overload->rhs_type() == rhs_type)
            {
              return overload->result_type();
            }
          }
        }
        emit_error(std::format("no matching overload for binary operator '{}'", expr.op.text), expr.op);
      }

      Type *type_of_expression(basedparse::Call_expression const &expr)
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
            std::format("expected {} arguments, got {}", ft->parameter_types.size(), expr.arguments.size()),
            expr.lparen
          );
        }
        for (auto i = std::size_t{}; i < expr.arguments.size(); ++i)
        {
          auto const arg_type = type_of_expression(expr.arguments[i]);
          if (!is_type_compatible(ft->parameter_types[i], arg_type))
          {
            auto const span = basedparse::span_of(expr.arguments[i]);
            emit_error(std::format("argument {} is not compatible with parameter type", i + 1), span.start);
          }
        }
        return ft->return_type;
      }

      Type *type_of_expression(basedparse::Prefix_bracket_expression const &expr)
      {
        emit_error("prefix bracket expression is not a value", expr.lbracket);
      }

      Type *type_of_expression(basedparse::Index_expression const &expr)
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

      Type *type_of_expression(basedparse::Block_expression const &expr)
      {
        if (!expr.tail)
        {
          return _type_pool->void_type();
        }
        _symbol_table.push_scope();
        for (auto const &stmt : expr.statements)
        {
          auto const let = std::get_if<basedparse::Let_statement>(&stmt.value);
          if (let != nullptr)
          {
            auto const type = type_of_expression(let->initializer);
            _symbol_table.declare_value(let->name.text, type, let->kw_mut.has_value());
          }
        }
        auto const result = type_of_expression(*expr.tail);
        _symbol_table.pop_scope();
        return result;
      }

      // TODO: consider what to do when branch types differ only in mutability
      Type *type_of_expression(basedparse::If_expression const &expr)
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
            emit_error("else-if branch type does not match then branch type", else_if.body.lbrace);
          }
        }
        auto const else_type = type_of_expression(expr.else_part->body);
        if (else_type != then_type)
        {
          emit_error("else branch type does not match then branch type", expr.else_part->body.lbrace);
        }
        return then_type;
      }

      bool is_constant_expression(basedparse::Expression const &expr)
      {
        return std::visit(
          [&](auto const &e) -> bool
          {
            return is_constant_expression(e);
          },
          expr.value
        );
      }

      bool is_constant_expression(basedparse::Int_literal_expression const &)
      {
        return true;
      }

      bool is_constant_expression(basedparse::Identifier_expression const &)
      {
        return false;
      }

      bool is_constant_expression(basedparse::Fn_expression const &)
      {
        return false;
      }

      bool is_constant_expression(basedparse::Paren_expression const &expr)
      {
        return is_constant_expression(*expr.inner);
      }

      bool is_constant_expression(basedparse::Unary_expression const &expr)
      {
        return is_constant_expression(*expr.operand);
      }

      bool is_constant_expression(basedparse::Binary_expression const &expr)
      {
        return is_constant_expression(*expr.left) && is_constant_expression(*expr.right);
      }

      bool is_constant_expression(basedparse::Call_expression const &)
      {
        return false;
      }

      bool is_constant_expression(basedparse::Prefix_bracket_expression const &)
      {
        return false;
      }

      bool is_constant_expression(basedparse::Index_expression const &expr)
      {
        return is_constant_expression(*expr.operand) && is_constant_expression(*expr.index);
      }

      bool is_constant_expression(basedparse::Block_expression const &)
      {
        return false;
      }

      bool is_constant_expression(basedparse::If_expression const &)
      {
        return false;
      }

      void compile_top_level_let_statement(basedparse::Function_definition const &func_def)
      {
        auto const &fn_expr = func_def.function;
        if (!fn_expr.return_type_specifier.has_value())
        {
          throw std::runtime_error{"return type deduction not yet supported"};
        }
        auto parameter_types = std::vector<Type *>{};
        auto parameters = std::vector<Parameter>{};
        for (auto const &param : fn_expr.parameters)
        {
          parameter_types.push_back(compile_type_expression(*param.type));
          parameters.push_back(Parameter{
            .name = param.name.text,
            .is_mutable = param.kw_mut.has_value(),
          });
        }
        auto const return_type = compile_type_expression(*fn_expr.return_type_specifier->type);
        auto const function_type = _type_pool->function_type(std::move(parameter_types), return_type);
        auto const function_handle = static_cast<std::uint64_t>(_translation_unit.functions.size());
        _translation_unit.functions.push_back(std::make_unique<Function>(Function{
          .type = function_type,
          .parameters = std::move(parameters),
        }));
        _symbol_table.declare_function(func_def.name.text, function_handle);
      }
    };

  } // namespace

  Translation_unit compile(basedparse::Translation_unit const &ast, Type_pool *type_pool)
  {
    auto ctx = Compilation_context{type_pool};
    return ctx.compile(ast);
  }

} // namespace basedhlir
