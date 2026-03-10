#ifndef BASEDIR_COMPILER_H
#define BASEDIR_COMPILER_H

#include <cstddef>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "basedparse/ast.h"
#include "ir.h"

namespace basedir
{

  class Compiler
  {
  public:
    class Compile_error: public std::runtime_error
    {
    public:
      using std::runtime_error::runtime_error;
    };

    Program compile(basedparse::Translation_unit const &unit);

  private:
    struct Local_binding
    {
      std::size_t index;
      bool is_mutable;
    };

    struct Global_binding
    {
      std::size_t index;
    };

    enum class Type_symbol
    {
      i32,
    };

    using Symbol = std::variant<Local_binding, Global_binding, Type_symbol>;
    using Scope = std::unordered_map<std::string, Symbol>;

    std::vector<Scope> _scopes;
    std::vector<std::string> _local_names;

    void push_scope();

    void pop_scope();

    void define(std::string const &name, Symbol symbol);

    Symbol const *lookup(std::string const &name) const;

    std::size_t alloc_local(std::string const &name);

    Function compile_function(
      std::optional<std::string> name,
      basedparse::Fn_expression const &fn
    );

    Statement compile_statement(basedparse::Statement const &statement);

    Statement compile_let_statement(basedparse::Let_statement const &statement);

    Statement
    compile_while_statement(basedparse::While_statement const &statement);

    Statement
    compile_return_statement(basedparse::Return_statement const &statement);

    Statement compile_expression_statement(
      basedparse::Expression_statement const &statement
    );

    Expression compile_expression(basedparse::Expression const &expression);

    Expression
    compile_int_literal(basedparse::Int_literal_expression const &expression);

    Expression
    compile_identifier_expression(basedparse::Identifier_expression const &expression);

    Expression compile_fn(basedparse::Fn_expression const &expression);

    Expression compile_paren(basedparse::Paren_expression const &expression);

    Expression compile_unary(basedparse::Unary_expression const &expression);

    Expression compile_binary(basedparse::Binary_expression const &expression);

    Expression compile_call(basedparse::Call_expression const &expression);

    Block_expression
    compile_block(basedparse::Block_expression const &expression);

    Expression compile_if(basedparse::If_expression const &expression);
  };

} // namespace basedir

#endif // BASEDIR_COMPILER_H
