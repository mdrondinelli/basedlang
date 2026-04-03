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
#include "operator_overload.h"
#include "source_span.h"
#include "symbol_table.h"
#include "type.h"

namespace basedhlir
{

  struct Diagnostic
  {
    std::string message;
    Source_span location;
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

  class Compilation_context
  {
  public:
    explicit Compilation_context(Type_pool *type_pool);

    Translation_unit compile(basedparse::Translation_unit const &ast);

    Type *type_of_constant(Constant_value const &value);

    [[noreturn]] void emit_error(std::string message, Source_span location);

    template <typename T>
    [[noreturn]] void emit_error(std::string message, T const &node)
    {
      emit_error(std::move(message), basedparse::span_of(node));
    }

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

    Type *type_of_expression(basedparse::Prefix_expression const &expr);

    Type *type_of_expression(basedparse::Postfix_expression const &expr);

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
    evaluate_constant_expression(basedparse::Prefix_expression const &expr);

    Constant_value
    evaluate_constant_expression(basedparse::Postfix_expression const &expr);

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

    Basic_block *new_block();

    void set_current_block(Basic_block *block);

    void emit(Terminator terminator);

    Register allocate_register();

    void emit(Instruction instruction);

    Typed_register compile_expression(basedparse::Expression const &expr);

    Typed_register
    compile_expression(basedparse::Int_literal_expression const &expr);

    Typed_register
    compile_expression(basedparse::Identifier_expression const &expr);

    Typed_register
    compile_expression(basedparse::Recurse_expression const &expr);

    Typed_register compile_expression(basedparse::Fn_expression const &expr);

    Typed_register compile_expression(basedparse::Paren_expression const &expr);

    Typed_register
    compile_expression(basedparse::Prefix_expression const &expr);

    Typed_register
    compile_expression(basedparse::Postfix_expression const &expr);

    Typed_register
    compile_expression(basedparse::Binary_expression const &expr);

    Typed_register compile_expression(basedparse::Call_expression const &expr);

    Typed_register compile_expression(basedparse::Index_expression const &expr);

    Typed_register
    compile_expression(basedparse::Prefix_bracket_expression const &expr);

    Typed_register compile_expression(basedparse::Block_expression const &expr);

    Typed_register compile_expression(basedparse::If_expression const &expr);

    void compile_statement(basedparse::Statement const &stmt);

    void compile_statement(basedparse::Let_statement const &stmt);

    void compile_statement(basedparse::While_statement const &stmt);

    void compile_statement(basedparse::Return_statement const &stmt);

    void compile_statement(basedparse::Expression_statement const &stmt);

    Function *compile_function(basedparse::Fn_expression const &expr);

    bool is_top_level() const;

  private:
    Type_pool *_type_pool;
    Translation_unit _translation_unit;
    Symbol_table _symbol_table;
    Function *_current_function{};
    std::int32_t _next_register{};
    Basic_block *_current_block{};
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
