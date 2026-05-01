#ifndef BASED_FRONTEND_COMPILE_H
#define BASED_FRONTEND_COMPILE_H

#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <ast/ast.h>
#include <ast/operator.h>
#include <ast/source_span.h>
#include <ir/constant_value.h>
#include <ir/hlir.h>
#include <ir/type.h>
#include <source/source_span.h>
#include <spelling/spelling.h>

#include "operator_overload.h"
#include "symbol_table.h"

namespace benson
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
    Compilation_context(ir::Type_pool *type_pool, Spelling_table *spellings);

    ir::Translation_unit compile(ast::Translation_unit const &ast);

    ir::Type *type_of_constant(ir::Constant_value const &value);

    [[noreturn]] void emit_error(std::string message, Source_span location);

    template <typename T>
    [[noreturn]] void emit_error(std::string message, T const &node)
    {
      emit_error(std::move(message), span_of(node));
    }

    Symbol *try_lookup_identifier(Lexeme const &identifier);

    Symbol *lookup_identifier(Lexeme const &identifier);

    Unary_operator_overload *
    find_unary_overload(ast::Operator op, ir::Type *operand_type);

    Binary_operator_overload *find_binary_overload(
      ast::Operator op,
      ir::Type *lhs_type,
      ir::Type *rhs_type
    );

    bool is_type_compatible(ir::Type *parameter_type, ir::Type *argument_type);

    ir::Type *compile_type_expression(ast::Expression const &expr);

    ir::Constant_value
    evaluate_constant_expression(ast::Expression const &expr);

    ir::Basic_block *new_block();

    void set_current_block(ir::Basic_block *block);

    void emit(ir::Terminator terminator, Source_span location);

    ir::Register allocate_register(ir::Type *type);

    ir::Type *type_of_register(ir::Register r) const;

    ir::Type *type_of_operand(ir::Operand const &operand);

    void emit(ir::Instruction instruction, Source_span location);

    ir::Operand compile_expression(ast::Expression const &expr);

    ir::Operand compile_expression(ast::Int_literal_expression const &expr);

    ir::Operand compile_expression(ast::Float_literal_expression const &expr);

    ir::Operand compile_expression(ast::Identifier_expression const &expr);

    ir::Operand compile_expression(ast::Recurse_expression const &expr);

    ir::Operand compile_expression(ast::Fn_expression const &expr);

    ir::Operand compile_expression(ast::Paren_expression const &expr);

    ir::Operand compile_expression(ast::Prefix_expression const &expr);

    ir::Operand compile_expression(ast::Postfix_expression const &expr);

    ir::Operand compile_expression(ast::Binary_expression const &expr);

    ir::Operand compile_expression(ast::Call_expression const &expr);

    ir::Operand compile_expression(ast::Index_expression const &expr);

    ir::Operand compile_expression(ast::Prefix_bracket_expression const &expr);

    ir::Operand compile_expression(ast::Block_expression const &expr);

    ir::Operand compile_expression(ast::If_expression const &expr);

    ir::Operand compile_int_literal(
      std::string_view text,
      bool negate,
      Lexeme const &token
    );

    ir::Operand
    compile_float_literal(std::string_view text, Lexeme const &token);

    void compile_statement(ast::Statement const &stmt);

    void compile_statement(ast::Let_statement const &stmt);

    void compile_statement(ast::While_statement const &stmt);

    void compile_statement(ast::Return_statement const &stmt);

    void compile_statement(ast::Expression_statement const &stmt);

    ir::Function *compile_function(ast::Fn_expression const &expr);

    bool is_top_level() const;

  private:
    ir::Type_pool *_type_pool;
    Spelling_table *_spellings;
    ir::Translation_unit _translation_unit;
    Symbol_table _symbol_table;
    ir::Function *_current_function{};
    ir::Basic_block *_current_block{};
    std::vector<ir::Type *> _register_types;
    std::vector<Diagnostic> _diagnostics;
    std::unordered_map<
      ast::Operator,
      std::vector<std::unique_ptr<Unary_operator_overload>>
    >
      _unary_overloads;
    std::unordered_map<
      ast::Operator,
      std::vector<std::unique_ptr<Binary_operator_overload>>
    >
      _binary_overloads;
  };

  ir::Translation_unit compile(
    ast::Translation_unit const &ast,
    Spelling_table *spellings,
    ir::Type_pool *type_pool
  );

  std::optional<std::uint64_t>
  validate_int_literal(std::string_view digits, std::uint64_t max_value);

} // namespace benson

#endif
