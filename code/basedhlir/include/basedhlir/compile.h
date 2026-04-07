#ifndef BASEDHLIR_COMPILE_H
#define BASEDHLIR_COMPILE_H

#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <basedast/ast.h>
#include <basedast/operator.h>

#include "constant_value.h"
#include "hlir.h"
#include "operator_overload.h"
#include "source_span.h"
#include "symbol_table.h"
#include "type.h"

namespace basedhlir
{
  struct Diagnostic_note
  {
    std::string message;
    Source_span location;
  };

  struct Diagnostic
  {
    std::string message;
    Source_span location;
    std::vector<Diagnostic_note> notes;
  };

  class Compilation_failure: public std::exception
  {
  public:
    explicit Compilation_failure(std::vector<Diagnostic> diagnostics)
        : _diagnostics{std::move(diagnostics)}
    {
      auto os = std::ostringstream{};
      for (auto const &diag : _diagnostics)
      {
        os << diag.location.start.line << ':' << diag.location.start.column
           << ": " << diag.message << '\n';
        for (auto const &note : diag.notes)
        {
          os << note.location.start.line << ':' << note.location.start.column
             << ": note: " << note.message << '\n';
        }
      }
      _what = os.str();
    }

    std::vector<Diagnostic> const &diagnostics() const
    {
      return _diagnostics;
    }

    char const *what() const noexcept override
    {
      return _what.c_str();
    }

  private:
    std::vector<Diagnostic> _diagnostics;
    std::string _what;
  };

  struct Compilation_error
  {
  };

  struct Not_a_constant_error
  {
    std::optional<Source_span> expression_location;
  };

  class Compilation_context
  {
  public:
    explicit Compilation_context(Type_pool *type_pool);

    Translation_unit compile(basedast::Translation_unit const &ast);

    Type *type_of_constant(Constant_value const &value);

    [[noreturn]] void emit_error(std::string message, Source_span location);

    template <typename T>
    [[noreturn]] void emit_error(std::string message, T const &node)
    {
      emit_error(std::move(message), basedast::span_of(node));
    }

    Symbol *try_lookup_identifier(basedlex::Lexeme const &identifier);

    Symbol *lookup_identifier(basedlex::Lexeme const &identifier);

    Unary_operator_overload *
    find_unary_overload(basedast::Operator op, Type *operand_type);

    Binary_operator_overload *
    find_binary_overload(basedast::Operator op, Type *lhs_type, Type *rhs_type);

    bool is_type_compatible(Type *parameter_type, Type *argument_type);

    Type *compile_type_expression(basedast::Expression const &expr);

    Constant_value
    evaluate_constant_expression(basedast::Expression const &expr);

    Basic_block *new_block();

    void set_current_block(Basic_block *block);

    void emit(Terminator terminator);

    Register allocate_register(Type *type);

    Type *type_of_register(Register r) const;

    Type *type_of_operand(Operand const &operand);

    void emit(Instruction instruction);

    Operand compile_expression(basedast::Expression const &expr);

    Operand compile_expression(basedast::Int_literal_expression const &expr);

    Operand compile_expression(basedast::Float_literal_expression const &expr);

    Operand compile_expression(basedast::Identifier_expression const &expr);

    Operand compile_expression(basedast::Recurse_expression const &expr);

    Operand compile_expression(basedast::Fn_expression const &expr);

    Operand compile_expression(basedast::Paren_expression const &expr);

    Operand compile_expression(basedast::Prefix_expression const &expr);

    Operand compile_expression(basedast::Postfix_expression const &expr);

    Operand compile_expression(basedast::Binary_expression const &expr);

    Operand compile_expression(basedast::Call_expression const &expr);

    Operand compile_expression(basedast::Index_expression const &expr);

    Operand compile_expression(basedast::Prefix_bracket_expression const &expr);

    Operand compile_expression(basedast::Block_expression const &expr);

    Operand compile_expression(basedast::If_expression const &expr);

    Operand compile_int_literal(
      std::string_view text,
      bool negate,
      basedlex::Lexeme const &token
    );

    Operand compile_float_literal(
      std::string_view text,
      bool negate,
      basedlex::Lexeme const &token
    );

    void compile_statement(basedast::Statement const &stmt);

    void compile_statement(basedast::Let_statement const &stmt);

    void compile_statement(basedast::While_statement const &stmt);

    void compile_statement(basedast::Return_statement const &stmt);

    void compile_statement(basedast::Expression_statement const &stmt);

    Function *compile_function(basedast::Fn_expression const &expr);

    bool is_top_level() const;

  private:
    Type_pool *_type_pool;
    Translation_unit _translation_unit;
    Symbol_table _symbol_table;
    Function *_current_function{};
    Basic_block *_current_block{};
    std::vector<Type *> _register_types;
    std::vector<Diagnostic> _diagnostics;
    std::unordered_map<
      basedast::Operator,
      std::vector<std::unique_ptr<Unary_operator_overload>>
    >
      _unary_overloads;
    std::unordered_map<
      basedast::Operator,
      std::vector<std::unique_ptr<Binary_operator_overload>>
    >
      _binary_overloads;
  };

  Translation_unit
  compile(basedast::Translation_unit const &ast, Type_pool *type_pool);

  std::optional<std::uint64_t>
  validate_int_literal(std::string_view digits, std::uint64_t max_value);

} // namespace basedhlir

#endif
