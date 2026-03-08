#ifndef BASEDPARSE_PARSER_H
#define BASEDPARSE_PARSER_H

#include <memory>

#include "basedlex/lexeme_stream_reader.h"

#include "expression.h"
#include "statement.h"
#include "translation_unit.h"
#include "type_expression.h"

namespace basedparse
{

  class Parser
  {
  public:
    explicit Parser(basedlex::Lexeme_stream_reader *reader) noexcept;

    std::unique_ptr<Translation_unit> parse_translation_unit();

    std::unique_ptr<Function_definition> parse_function_definition();

    std::unique_ptr<Statement> parse_statement();

    std::unique_ptr<Let_statement> parse_let_statement();

    std::unique_ptr<Return_statement> parse_return_statement();

    std::unique_ptr<Expression_statement> parse_expression_statement();

    std::unique_ptr<Block_statement> parse_block_statement();

    std::unique_ptr<Expression> parse_expression();

    std::unique_ptr<Expression> parse_primary_expression();

    std::unique_ptr<Fn_expression> parse_fn_expression();

    std::unique_ptr<Int_literal_expression> parse_int_literal_expression();

    std::unique_ptr<Identifier_expression> parse_identifier_expression();

    std::unique_ptr<Paren_expression> parse_paren_expression();

    Fn_expression::Return_type_specifier parse_return_type_specifier();

    std::unique_ptr<Type_expression> parse_type_expression();

    std::unique_ptr<Identifier_type_expression>
    parse_identifier_type_expression();

  private:
    basedlex::Lexeme expect(basedlex::Token token);

    basedlex::Lexeme_stream_reader *_reader;
  };

} // namespace basedparse

#endif // BASEDPARSE_PARSER_H
