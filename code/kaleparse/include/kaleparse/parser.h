#ifndef BASEDPARSE_PARSER_H
#define BASEDPARSE_PARSER_H

#include <memory>

#include "kalelex/lexeme_stream_reader.h"

#include <kaleast/ast.h>
#include <kaleast/operator.h>

namespace kaleparse
{

  class Parser
  {
  public:
    explicit Parser(kalelex::Lexeme_stream_reader *reader) noexcept;

    kaleast::Translation_unit parse_translation_unit();

    kaleast::Statement parse_statement();

    kaleast::Let_statement parse_let_statement();

    kaleast::Return_statement parse_return_statement();

    kaleast::Expression_statement parse_expression_statement();

    std::unique_ptr<kaleast::Expression> parse_expression();

    std::unique_ptr<kaleast::Expression> parse_primary_expression();

    kaleast::Int_literal_expression parse_int_literal_expression();

    kaleast::Float_literal_expression parse_float_literal_expression();

    kaleast::Identifier_expression parse_identifier_expression();

    kaleast::Paren_expression parse_paren_expression();

    kaleast::Block_expression parse_block_expression();

    kaleast::If_expression parse_if_expression();

    kaleast::While_statement parse_while_statement();

    kaleast::Fn_expression parse_fn_expression();

    kaleast::Fn_expression::Return_type_specifier
    parse_return_type_specifier();

  private:
    std::unique_ptr<kaleast::Expression>
    parse_expression(int current_precedence);

    kalelex::Lexeme expect(kalelex::Token token);

    kalelex::Lexeme_stream_reader *_reader;
  };

} // namespace kaleparse

#endif // BASEDPARSE_PARSER_H
