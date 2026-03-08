#ifndef BASEDPARSE_STATEMENT_H
#define BASEDPARSE_STATEMENT_H

#include <memory>
#include <vector>

#include "basedlex/lexeme.h"

#include "expression.h"

namespace basedparse
{

  class Statement
  {
  public:
    virtual ~Statement() = default;
  };

  class Let_statement: public Statement
  {
  public:
    basedlex::Lexeme kw_let;
    basedlex::Lexeme name;
    basedlex::Lexeme eq;
    std::unique_ptr<Expression> initializer;
    basedlex::Lexeme semicolon;
  };

  class Return_statement: public Statement
  {
  public:
    basedlex::Lexeme kw_return;
    std::unique_ptr<Expression> value;
    basedlex::Lexeme semicolon;
  };

  class Expression_statement: public Statement
  {
  public:
    std::unique_ptr<Expression> expression;
    basedlex::Lexeme semicolon;
  };

  class Block_statement: public Statement
  {
  public:
    basedlex::Lexeme lbrace;
    std::vector<std::unique_ptr<Statement>> statements;
    basedlex::Lexeme rbrace;
  };

  class Function_definition: public Statement
  {
  public:
    basedlex::Lexeme kw_let;
    basedlex::Lexeme name;
    basedlex::Lexeme eq;
    Fn_expression function;
    basedlex::Lexeme semicolon;
  };

} // namespace basedparse

#endif // BASEDPARSE_STATEMENT_H
