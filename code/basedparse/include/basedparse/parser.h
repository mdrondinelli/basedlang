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

    Translation_unit parse_translation_unit();

  private:
    std::unique_ptr<Statement> parse_statement();
    std::unique_ptr<Expression> parse_expression();
    std::unique_ptr<Type_expression> parse_type_expression();
    Fn_expression parse_fn_expression();
    std::unique_ptr<Block_statement> parse_block_statement();

    basedlex::Lexeme expect(basedlex::Token token);

    basedlex::Lexeme_stream_reader *_reader;
  };

} // namespace basedparse

#endif // BASEDPARSE_PARSER_H
