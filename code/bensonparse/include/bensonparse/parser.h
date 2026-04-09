#ifndef BASEDPARSE_PARSER_H
#define BASEDPARSE_PARSER_H

#include <memory>

#include "bensonlex/lexeme_stream_reader.h"

#include <bensonast/ast.h>
#include <bensonast/operator.h>

namespace benson
{

  class Parser
  {
  public:
    explicit Parser(benson::Lexeme_stream_reader *reader) noexcept;

    benson::ast::Translation_unit parse_translation_unit();

    benson::ast::Statement parse_statement();

    benson::ast::Let_statement parse_let_statement();

    benson::ast::Return_statement parse_return_statement();

    benson::ast::Expression_statement parse_expression_statement();

    std::unique_ptr<benson::ast::Expression> parse_expression();

    std::unique_ptr<benson::ast::Expression> parse_primary_expression();

    benson::ast::Int_literal_expression parse_int_literal_expression();

    benson::ast::Float_literal_expression parse_float_literal_expression();

    benson::ast::Identifier_expression parse_identifier_expression();

    benson::ast::Paren_expression parse_paren_expression();

    benson::ast::Block_expression parse_block_expression();

    benson::ast::If_expression parse_if_expression();

    benson::ast::While_statement parse_while_statement();

    benson::ast::Fn_expression parse_fn_expression();

    benson::ast::Fn_expression::Return_type_specifier
    parse_return_type_specifier();

  private:
    std::unique_ptr<benson::ast::Expression>
    parse_expression(int current_precedence);

    benson::Lexeme expect(benson::Token token);

    benson::Lexeme_stream_reader *_reader;
  };

} // namespace benson

#endif // BASEDPARSE_PARSER_H
