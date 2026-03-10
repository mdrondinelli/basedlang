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
#include "type.h"

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
      Type *type;
    };

    struct Global_binding
    {
      std::size_t index;
    };

    struct Type_symbol
    {
      Type *type;
    };

    using Symbol = std::variant<Local_binding, Global_binding, Type_symbol>;
    using Scope = std::unordered_map<std::string, Symbol>;

    Program *_program = nullptr;
    std::vector<Scope> _scopes;
    std::vector<std::string> _local_names;
    std::vector<Type *> _return_type_stack;

    void push_scope();

    void pop_scope();

    void define(std::string const &name, Symbol symbol);

    Symbol const *lookup(std::string const &name) const;

    std::size_t alloc_local(std::string const &name);

    Type *resolve_type(basedparse::Type_expression const &type_expr);

    Type *compile_function_type(basedparse::Fn_expression const &fn);

    Function_body compile_function_body(basedparse::Fn_expression const &fn);

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

    static Type *strip_reference(Type *type);

    static bool types_compatible(Type *expected, Type *actual);

    Expression
    compile_int_literal(basedparse::Int_literal_expression const &expression);

    Expression compile_identifier_expression(
      basedparse::Identifier_expression const &expression
    );

    Expression compile_fn(basedparse::Fn_expression const &expression);

    Expression compile_paren(basedparse::Paren_expression const &expression);

    Expression compile_unary(basedparse::Unary_expression const &expression);

    Expression compile_binary(basedparse::Binary_expression const &expression);

    Expression compile_call(basedparse::Call_expression const &expression);

    Expression compile_index(basedparse::Index_expression const &expression);

    Expression
    compile_constructor(basedparse::Constructor_expression const &expression);

    Expression compile_block(basedparse::Block_expression const &expression);

    Expression compile_if(basedparse::If_expression const &expression);
  };

} // namespace basedir

#endif // BASEDIR_COMPILER_H
