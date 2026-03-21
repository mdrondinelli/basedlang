#include <cassert>
#include <cstdint>
#include <format>
#include <stdexcept>
#include <utility>
#include <variant>

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

      Function *get_function(std::uint64_t function_handle)
      {
        return _translation_unit.functions[function_handle].get();
      }

      [[noreturn]] void emit_error(std::string message, basedlex::Lexeme const &lexeme)
      {
        _diagnostics.push_back(Diagnostic{
          .message = std::move(message),
          .line = lexeme.line,
          .column = lexeme.column,
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

      Type *compile_type_expression(basedparse::Type_expression const &type_expr)
      {
        return std::visit(
          [&](auto const &expr) -> Type *
          {
            return compile_type_expression(expr);
          },
          type_expr.value
        );
      }

      Type *compile_type_expression(basedparse::Identifier_type_expression const &expr)
      {
        auto const sym = lookup_identifier(expr.identifier);
        auto const ts = std::get_if<Type_symbol>(&sym->data);
        if (ts == nullptr)
        {
          emit_error(std::format("'{}' is not a type", expr.identifier.text), expr.identifier);
        }
        return ts->type;
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

      Type *type_of_expression(basedparse::Fn_expression const &)
      {
        throw std::runtime_error{"type_of_expression not yet implemented for fn expressions"};
      }

      Type *type_of_expression(basedparse::Paren_expression const &expr)
      {
        return type_of_expression(*expr.inner);
      }

      Type *type_of_expression(basedparse::Unary_expression const &)
      {
        throw std::runtime_error{"type_of_expression not yet implemented for unary expressions"};
      }

      Type *type_of_expression(basedparse::Binary_expression const &)
      {
        throw std::runtime_error{"type_of_expression not yet implemented for binary expressions"};
      }

      Type *type_of_expression(basedparse::Call_expression const &)
      {
        throw std::runtime_error{"type_of_expression not yet implemented for call expressions"};
      }

      Type *type_of_expression(basedparse::Index_expression const &)
      {
        throw std::runtime_error{"type_of_expression not yet implemented for index expressions"};
      }

      Type *type_of_expression(basedparse::Block_expression const &)
      {
        throw std::runtime_error{"type_of_expression not yet implemented for block expressions"};
      }

      Type *type_of_expression(basedparse::If_expression const &)
      {
        throw std::runtime_error{"type_of_expression not yet implemented for if expressions"};
      }

      Type *type_of_expression(basedparse::Constructor_expression const &)
      {
        throw std::runtime_error{"type_of_expression not yet implemented for constructor expressions"};
      }

      Type *compile_type_expression(basedparse::Array_type_expression const &expr)
      {
        auto const element = compile_type_expression(*expr.element_type);
        if (expr.size == nullptr)
        {
          return _type_pool->unsized_array_type(element);
        }
        throw std::runtime_error{"sized array types not yet supported"};
      }

      Type *compile_type_expression(basedparse::Pointer_type_expression const &expr)
      {
        return _type_pool->pointer_type(compile_type_expression(*expr.pointee_type));
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

      bool is_constant_expression(basedparse::Constructor_expression const &)
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
          parameter_types.push_back(compile_type_expression(param.type_expression));
          parameters.push_back(Parameter{
            .name = param.name.text,
            .is_mutable = param.kw_mut.has_value(),
          });
        }
        auto const return_type = compile_type_expression(fn_expr.return_type_specifier->type_expression);
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
