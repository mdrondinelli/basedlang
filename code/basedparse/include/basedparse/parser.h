#ifndef BASEDPARSE_PARSER_H
#define BASEDPARSE_PARSER_H

#include <memory>

#include "basedlex/lexeme_stream_reader.h"

#include <basedast/ast.h>
#include <basedast/operator.h>

namespace basedparse
{

  class Parser
  {
  public:
    explicit Parser(basedlex::Lexeme_stream_reader *reader) noexcept;

    basedast::Translation_unit parse_translation_unit();

    basedast::Statement parse_statement();

    basedast::Let_statement parse_let_statement();

    basedast::Return_statement parse_return_statement();

    basedast::Expression_statement parse_expression_statement();

    std::unique_ptr<basedast::Expression> parse_expression();

    std::unique_ptr<basedast::Expression> parse_primary_expression();

    basedast::Int_literal_expression parse_int_literal_expression();

    basedast::Identifier_expression parse_identifier_expression();

    basedast::Paren_expression parse_paren_expression();

    basedast::Block_expression parse_block_expression();

    basedast::If_expression parse_if_expression();

    basedast::While_statement parse_while_statement();

    basedast::Fn_expression parse_fn_expression();

    basedast::Fn_expression::Return_type_specifier
    parse_return_type_specifier();

  private:
    std::unique_ptr<basedast::Expression>
    parse_expression(int current_precedence);

    basedlex::Lexeme expect(basedlex::Token token);

    basedlex::Lexeme_stream_reader *_reader;
  };

} // namespace basedparse

#endif // BASEDPARSE_PARSER_H
