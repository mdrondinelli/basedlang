#ifndef BASEDPARSE_PARSER_H
#define BASEDPARSE_PARSER_H

#include <memory>

#include "basedlex/lexeme_stream_reader.h"

#include "ast.h"
#include "operator.h"

namespace basedparse
{

  class Parser
  {
  public:
    explicit Parser(basedlex::Lexeme_stream_reader *reader) noexcept;

    std::unique_ptr<Translation_unit> parse_translation_unit();

    Function_definition parse_function_definition();

    Statement parse_statement();

    Let_statement parse_let_statement();

    Return_statement parse_return_statement();

    Expression_statement parse_expression_statement();

    Block_expression parse_block_expression();

    std::unique_ptr<Expression> parse_expression();

    std::unique_ptr<Expression> parse_primary_expression();

    Fn_expression parse_fn_expression();

    Int_literal_expression parse_int_literal_expression();

    Identifier_expression parse_identifier_expression();

    Paren_expression parse_paren_expression();

    Constructor_expression parse_constructor_expression();

    Fn_expression::Return_type_specifier parse_return_type_specifier();

    std::unique_ptr<Type_expression> parse_type_expression();

  private:
    std::unique_ptr<Expression> parse_expression(int current_precedence);

    basedlex::Lexeme expect(basedlex::Token token);

    basedlex::Lexeme_stream_reader *_reader;
  };

} // namespace basedparse

#endif // BASEDPARSE_PARSER_H
