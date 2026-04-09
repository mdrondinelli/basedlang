#ifndef BASEDPARSE_PARSER_H
#define BASEDPARSE_PARSER_H

#include <memory>

#include "bensonlex/lexeme_stream_reader.h"

#include <bensonast/ast.h>
#include <bensonast/operator.h>

namespace bensonparse
{

  class Parser
  {
  public:
    explicit Parser(bensonlex::Lexeme_stream_reader *reader) noexcept;

    bensonast::Translation_unit parse_translation_unit();

    bensonast::Statement parse_statement();

    bensonast::Let_statement parse_let_statement();

    bensonast::Return_statement parse_return_statement();

    bensonast::Expression_statement parse_expression_statement();

    std::unique_ptr<bensonast::Expression> parse_expression();

    std::unique_ptr<bensonast::Expression> parse_primary_expression();

    bensonast::Int_literal_expression parse_int_literal_expression();

    bensonast::Float_literal_expression parse_float_literal_expression();

    bensonast::Identifier_expression parse_identifier_expression();

    bensonast::Paren_expression parse_paren_expression();

    bensonast::Block_expression parse_block_expression();

    bensonast::If_expression parse_if_expression();

    bensonast::While_statement parse_while_statement();

    bensonast::Fn_expression parse_fn_expression();

    bensonast::Fn_expression::Return_type_specifier
    parse_return_type_specifier();

  private:
    std::unique_ptr<bensonast::Expression>
    parse_expression(int current_precedence);

    bensonlex::Lexeme expect(bensonlex::Token token);

    bensonlex::Lexeme_stream_reader *_reader;
  };

} // namespace bensonparse

#endif // BASEDPARSE_PARSER_H
