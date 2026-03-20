#ifndef BASEDIR_COMPILER_H
#define BASEDIR_COMPILER_H

#include <format>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "basedparse/ast.h"
#include "ir.h"
#include "type.h"

namespace basedir
{

  class Compiler
  {
  public:
    class Compile_error: public std::exception
    {
    public:
      explicit Compile_error(int line, int column, std::string message)
          : _message{std::format("line {} col {}: {}", line, column, message)}
      {
      }

      char const *what() const noexcept override
      {
        return _message.c_str();
      }

    private:
      std::string _message;
    };

    /// Convert a TU parse tree into a representation of a valid program, or
    /// throw `Compile_error`
    Program compile(basedparse::Translation_unit const &unit);

  private:
    struct Symbol_handle
    {
      struct Named_type
      {
        Type *type;
      };

      struct Named_function
      {
        std::int64_t function_index;
      };

      struct Named_value
      {
        std::int64_t register_index;
      };

      std::variant<
        Named_type,
        Named_function,
        Named_value
      >
        value;
    };

    using Scope = std::unordered_map<std::string, Symbol_handle>;

    class Scope_guard
    {
    public:
      explicit Scope_guard(Compiler *compiler);

      ~Scope_guard();

    private:
      Compiler *_compiler;
    };

    class Function_guard
    {
    public:
      explicit Function_guard(Compiler *compiler);

      ~Function_guard();
      
    private:
      Compiler *_compiler;
    };

    enum class Value_category
    {
      lvalue,
      rvalue,
    };

    /// Stores the current `Program` being compiled. Mostly non-null.
    Program *_program{};

    /// Stores the current `Function_body` being compiled. Can be null.
    Function_body *_function_body{};

    /// The currently-visible stack of scopes in the single pass of the
    /// compiler. The global scope (which contains only functions) is always at
    /// index 0.
    ///
    /// Going into a nested function body hides everything outside other than
    /// the global scope.
    std::vector<Scope> _scopes;

    void compile_top_level_let_statement(
      basedparse::Let_statement const &statement
    );

    /// Sets a body of previously declared function
    ///
    /// \return the index of the function
    void define_function(
      std::int64_t function_index,
      std::vector<Basic_block> blocks
    );

    /// Converts a type expression to a `Type *` in the current context
    Type *compile_type_expression(basedparse::Type_expression const &type_expr);

    /// Converts an expression to a `Type *` in the current context
    Type *compile_expression_type(basedparse::Expression const &expr);

    /// Converts a type expression to a `Value_category` in the current context
    Value_category compile_expression_value_category();

    /// Add a stack value of a given type to the current function and a symbol
    /// mapping to that local to the current scope. The local is in an
    /// uninitialized state.
    ///
    /// \return the index of the register holding the address of the local
    std::int64_t declare_named_stack_value(std::string name, Type *type);

    /// Add a function to the global function list and a symbol mapping to that
    /// function to the current scope. The function has no body, only a
    /// prototype.
    ///
    /// \return the index of the function
    std::int64_t declare_named_function(std::string name, Type *type);

    /// Add a named type to the type pool and a symbol mapping to that type to
    /// the current scope.
    ///
    /// \return the type
    Type *declare_named_type(std::string name);

    /// Declare a symbol in the current scope
    void bind_identifier(std::string identifier, Symbol_handle symbol);

    /// Find the symbol referred to by an identifier in the current scope
    Symbol_handle resolve_identifier(basedlex::Lexeme const &identifier) const;

    Scope_guard scope_guard()
    {
      return Scope_guard{this};
    }

    void push_scope();

    void pop_scope() noexcept;

    std::int64_t reserve_register(Type *type);

    Type_pool &types() const noexcept;

    // static bool types_compatible(Type *expected, Type *actual);
  };

} // namespace basedir

#endif // BASEDIR_COMPILER_H
