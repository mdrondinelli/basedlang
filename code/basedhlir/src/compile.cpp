#include <cassert>
#include <format>
#include <stdexcept>
#include <utility>

#include "basedhlir/compile.h"
#include "basedhlir/symbol_table.h"

namespace basedhlir
{

  namespace
  {

    struct Compilation_error
    {
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
      }

      Translation_unit compile(basedparse::Translation_unit const &ast)
      {
        auto result = Translation_unit{};
        for (auto const &func_def : ast.function_definitions)
        {
          try
          {
            compile_top_level_let_statement(func_def, &result);
          }
          catch (Compilation_error const &)
          {
          }
        }
        if (!_diagnostics.empty())
        {
          throw Compilation_failure{std::move(_diagnostics)};
        }
        return result;
      }

    private:
      Type_pool *_type_pool;
      Symbol_table _symbol_table;
      std::vector<Diagnostic> _diagnostics;

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

      void compile_top_level_let_statement(
        basedparse::Function_definition const &func_def,
        Translation_unit *result
      )
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
        auto const function_handle = static_cast<std::uint64_t>(result->functions.size());
        result->functions.push_back(Function{
          .type = function_type,
          .parameters = std::move(parameters),
        });
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
