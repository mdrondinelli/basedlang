#ifndef BASEDHLIR_COMPILE_H
#define BASEDHLIR_COMPILE_H

#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include <basedparse/ast.h>
#include <basedparse/operator.h>

#include "constant_value.h"
#include "hlir.h"
#include "source_location.h"
#include "symbol_table.h"
#include "type.h"

namespace basedhlir
{

  struct Diagnostic
  {
    std::string message;
    Source_location location;
  };

  class Compilation_failure: public std::exception
  {
  public:
    explicit Compilation_failure(std::vector<Diagnostic> diagnostics)
        : _diagnostics{std::move(diagnostics)}
    {
    }

    std::vector<Diagnostic> const &diagnostics() const
    {
      return _diagnostics;
    }

    char const *what() const noexcept override
    {
      return "compilation failed";
    }

  private:
    std::vector<Diagnostic> _diagnostics;
  };

  struct Compilation_error
  {
  };

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

    virtual Constant_value
    evaluate(Constant_value lhs, Constant_value rhs) const = 0;
  };

  class Compilation_context
  {
  public:
    explicit Compilation_context(Type_pool *type_pool);

    Translation_unit compile(basedparse::Translation_unit const &ast);

    Type *type_of_constant(Constant_value const &value);

    [[noreturn]] void
    emit_error(std::string message, basedlex::Lexeme const &lexeme);

    [[noreturn]] void emit_error(std::string message, Source_location location);

    Symbol *try_lookup_identifier(basedlex::Lexeme const &identifier);

    Symbol *lookup_identifier(basedlex::Lexeme const &identifier);

    Unary_operator_overload *
    find_unary_overload(basedparse::Operator op, Type *operand_type);

    Binary_operator_overload *find_binary_overload(
      basedparse::Operator op,
      Type *lhs_type,
      Type *rhs_type
    );

    bool is_type_compatible(Type *parameter_type, Type *argument_type);

    Type *compile_type_expression(basedparse::Expression const &expr);

    Type *type_of_expression(basedparse::Expression const &expr);

    Type *type_of_expression(basedparse::Int_literal_expression const &);

    Type *type_of_expression(basedparse::Identifier_expression const &expr);

    Type *type_of_expression(basedparse::Recurse_expression const &expr);

    Type *type_of_expression(basedparse::Fn_expression const &expr);

    Type *type_of_expression(basedparse::Paren_expression const &expr);

    Type *type_of_expression(basedparse::Unary_expression const &expr);

    Type *type_of_expression(basedparse::Binary_expression const &expr);

    Type *type_of_expression(basedparse::Call_expression const &expr);

    Type *type_of_expression(basedparse::Prefix_bracket_expression const &expr);

    Type *type_of_expression(basedparse::Index_expression const &expr);

    Type *type_of_expression(basedparse::Block_expression const &expr);

    Type *type_of_expression(basedparse::If_expression const &expr);

    Constant_value
    evaluate_constant_expression(basedparse::Expression const &expr);

    Constant_value evaluate_constant_expression(
      basedparse::Int_literal_expression const &expr
    );

    Constant_value
    evaluate_constant_expression(basedparse::Paren_expression const &expr);

    Constant_value
    evaluate_constant_expression(basedparse::Unary_expression const &expr);

    Constant_value
    evaluate_constant_expression(basedparse::Binary_expression const &expr);

    Constant_value
    evaluate_constant_expression(basedparse::Identifier_expression const &);

    Constant_value
    evaluate_constant_expression(basedparse::Recurse_expression const &);

    Constant_value
    evaluate_constant_expression(basedparse::Fn_expression const &);

    Constant_value
    evaluate_constant_expression(basedparse::Call_expression const &);

    Constant_value
    evaluate_constant_expression(basedparse::Prefix_bracket_expression const &);

    Constant_value
    evaluate_constant_expression(basedparse::Index_expression const &);

    Constant_value
    evaluate_constant_expression(basedparse::Block_expression const &);

    Constant_value
    evaluate_constant_expression(basedparse::If_expression const &);

    bool is_top_level() const;

    void compile_expression(basedparse::Expression const &expr);

    void compile_expression(basedparse::Int_literal_expression const &expr);

    void compile_expression(basedparse::Identifier_expression const &expr);

    void compile_expression(basedparse::Recurse_expression const &expr);

    void compile_expression(basedparse::Fn_expression const &expr);

    void compile_expression(basedparse::Paren_expression const &expr);

    void compile_expression(basedparse::Unary_expression const &expr);

    void compile_expression(basedparse::Binary_expression const &expr);

    void compile_expression(basedparse::Call_expression const &expr);

    void compile_expression(basedparse::Index_expression const &expr);

    void compile_expression(basedparse::Prefix_bracket_expression const &expr);

    void compile_expression(basedparse::Block_expression const &expr);

    void compile_expression(basedparse::If_expression const &expr);

    void compile_statement(basedparse::Statement const &stmt);

    void compile_statement(basedparse::Let_statement const &stmt);

    void compile_statement(basedparse::While_statement const &stmt);

    void compile_statement(basedparse::Return_statement const &stmt);

    void compile_statement(basedparse::Expression_statement const &stmt);

  private:
    Type_pool *_type_pool;
    Translation_unit _translation_unit;
    Symbol_table _symbol_table;
    Function *_current_function{};
    std::vector<Diagnostic> _diagnostics;
    std::unordered_map<
      basedparse::Operator,
      std::vector<std::unique_ptr<Unary_operator_overload>>
    >
      _unary_overloads;
    std::unordered_map<
      basedparse::Operator,
      std::vector<std::unique_ptr<Binary_operator_overload>>
    >
      _binary_overloads;
  };

  Translation_unit
  compile(basedparse::Translation_unit const &ast, Type_pool *type_pool);

} // namespace basedhlir

#endif
