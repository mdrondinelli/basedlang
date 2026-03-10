#include <string>
#include <utility>

#include "basedir/compiler.h"
#include "basedparse/operator.h"

namespace basedir
{

  // Compiles a parsed AST (Translation_unit) into the IR (Program). The
  // compiler maintains a symbol table (a stack of lexical scopes) to resolve
  // identifiers to either local frame slots or global function indices.
  //
  // Symbols resolve to one of three kinds:
  //   - Local_binding:  a slot index into the current function's frame
  //   - Global_binding: an index into Program::functions
  //   - Type_symbol:    a built-in type (e.g. i32)
  //
  // Top-level compilation has two phases:
  //   1. Register all function names as Global_bindings (so every function is
  //      visible during body compilation, enabling recursion and mutual
  //      recursion).
  //   2. Compile each function body, during which the symbol table resolves
  //      identifiers to Local_expression or Global_expression IR nodes.
  //
  // Within a function, parameters and let-bindings each allocate a local slot
  // via alloc_local(). The final slot count becomes the function's frame size
  // (local_names.size()). Block scopes push/pop on the scope stack so that
  // inner let-bindings shadow outer ones, but all locals share the same flat
  // frame — scopes only affect name visibility, not storage.

  Program Compiler::compile(basedparse::Translation_unit const &unit)
  {
    auto program = Program{};
    push_scope();
    define("i32", Type_symbol::i32);
    // Phase 1: register all top-level function names before compiling any body.
    for (auto const &statement : unit.statements)
    {
      auto const fn_def =
        std::get_if<basedparse::Function_definition>(&statement.value);
      if (!fn_def)
      {
        throw Compile_error{"top-level statement is not a function definition"};
      }
      auto const index = program.functions.size();
      define(fn_def->name.text, Global_binding{index});
      program.functions.emplace_back();
    }
    // Phase 2: compile each function body with all globals in scope.
    for (auto i = std::size_t{}; i < unit.statements.size(); ++i)
    {
      auto const &fn_def =
        std::get<basedparse::Function_definition>(unit.statements[i].value);
      program.functions[i] =
        compile_function(fn_def.name.text, fn_def.function);
    }
    pop_scope();
    return program;
  }

  void Compiler::push_scope()
  {
    _scopes.emplace_back();
  }

  void Compiler::pop_scope()
  {
    _scopes.pop_back();
  }

  // insert_or_assign allows let-shadowing within the same scope:
  // `let x = 1; let x = x + 1;` rebinds x to a new local slot.
  void Compiler::define(std::string const &name, Symbol symbol)
  {
    _scopes.back().insert_or_assign(name, std::move(symbol));
  }

  // Walk scopes innermost-first to find the nearest binding for a name.
  Compiler::Symbol const *Compiler::lookup(std::string const &name) const
  {
    for (auto it = _scopes.rbegin(); it != _scopes.rend(); ++it)
    {
      auto const found = it->find(name);
      if (found != it->end())
      {
        return &found->second;
      }
    }
    return nullptr;
  }

  // Allocate a new local slot in the current function's frame and record its
  // name for debugging/error messages.
  std::size_t Compiler::alloc_local(std::string const &name)
  {
    auto const index = _local_names.size();
    _local_names.push_back(name);
    return index;
  }

  Function Compiler::compile_function(
    std::optional<std::string> name,
    basedparse::Fn_expression const &fn
  )
  {
    _local_names.clear();
    push_scope();
    for (auto const &param : fn.parameters)
    {
      auto const is_mutable = param.kw_mut.has_value();
      auto const index = alloc_local(param.name.text);
      define(param.name.text, Local_binding{index, is_mutable});
    }
    auto body = compile_block(*fn.body);
    pop_scope();
    return Function{
      std::move(name),
      fn.parameters.size(),
      std::move(_local_names),
      std::move(body),
    };
  }

  Statement Compiler::compile_statement(basedparse::Statement const &statement)
  {
    if (auto const s = std::get_if<basedparse::Let_statement>(&statement.value))
    {
      return compile_let_statement(*s);
    }
    if (auto const s =
          std::get_if<basedparse::While_statement>(&statement.value))
    {
      return compile_while_statement(*s);
    }
    if (auto const s =
          std::get_if<basedparse::Return_statement>(&statement.value))
    {
      return compile_return_statement(*s);
    }
    if (auto const s =
          std::get_if<basedparse::Expression_statement>(&statement.value))
    {
      return compile_expression_statement(*s);
    }
    throw Compile_error{"unexpected statement type"};
  }

  Statement
  Compiler::compile_let_statement(basedparse::Let_statement const &statement)
  {
    auto initializer = compile_expression(statement.initializer);
    auto const is_mutable = statement.kw_mut.has_value();
    auto const index = alloc_local(statement.name.text);
    define(statement.name.text, Local_binding{index, is_mutable});
    return Statement{Let_statement{index, std::move(initializer)}};
  }

  Statement Compiler::compile_while_statement(
    basedparse::While_statement const &statement
  )
  {
    auto condition =
      std::make_unique<Expression>(compile_expression(*statement.condition));
    auto body = compile_block(statement.body);
    auto while_stmt = While_statement{};
    while_stmt.condition = std::move(condition);
    while_stmt.body = std::move(body);
    return Statement{std::move(while_stmt)};
  }

  Statement Compiler::compile_return_statement(
    basedparse::Return_statement const &statement
  )
  {
    return Statement{Return_statement{compile_expression(statement.value)}};
  }

  Statement Compiler::compile_expression_statement(
    basedparse::Expression_statement const &statement
  )
  {
    return Statement{
      Expression_statement{compile_expression(statement.expression)}
    };
  }

  Expression
  Compiler::compile_expression(basedparse::Expression const &expression)
  {
    if (auto const e =
          std::get_if<basedparse::Int_literal_expression>(&expression.value))
    {
      return compile_int_literal(*e);
    }
    if (auto const e =
          std::get_if<basedparse::Identifier_expression>(&expression.value))
    {
      return compile_identifier_expression(*e);
    }
    if (auto const e =
          std::get_if<basedparse::Fn_expression>(&expression.value))
    {
      return compile_fn(*e);
    }
    if (auto const e =
          std::get_if<basedparse::Paren_expression>(&expression.value))
    {
      return compile_paren(*e);
    }
    if (auto const e =
          std::get_if<basedparse::Unary_expression>(&expression.value))
    {
      return compile_unary(*e);
    }
    if (auto const e =
          std::get_if<basedparse::Binary_expression>(&expression.value))
    {
      return compile_binary(*e);
    }
    if (auto const e =
          std::get_if<basedparse::Call_expression>(&expression.value))
    {
      return compile_call(*e);
    }
    if (auto const e =
          std::get_if<basedparse::Block_expression>(&expression.value))
    {
      return Expression{compile_block(*e)};
    }
    if (auto const e =
          std::get_if<basedparse::If_expression>(&expression.value))
    {
      return compile_if(*e);
    }
    throw Compile_error{"unexpected expression type"};
  }

  Expression Compiler::compile_int_literal(
    basedparse::Int_literal_expression const &expression
  )
  {
    return Expression{
      Int_literal_expression{std::stoi(expression.literal.text)}
    };
  }

  // Resolve an identifier to a Local_expression or Global_expression by
  // looking it up in the symbol table. Type symbols (e.g. i32) are not values
  // and produce an error if used in expression position.
  Expression Compiler::compile_identifier_expression(
    basedparse::Identifier_expression const &expression
  )
  {
    auto const &name = expression.identifier.text;
    auto const symbol = lookup(name);
    if (!symbol)
    {
      throw Compile_error{"undefined symbol: " + name};
    }
    if (auto const local = std::get_if<Local_binding>(symbol))
    {
      return Expression{Local_expression{local->index}};
    }
    if (auto const global = std::get_if<Global_binding>(symbol))
    {
      return Expression{Global_expression{global->index}};
    }
    throw Compile_error{name + " is not a value"};
  }

  Expression
  Compiler::compile_fn(basedparse::Fn_expression const & /*expression*/)
  {
    throw Compile_error{"nested function expressions are not supported"};
  }

  Expression
  Compiler::compile_paren(basedparse::Paren_expression const &expression)
  {
    return compile_expression(*expression.inner);
  }

  Expression
  Compiler::compile_unary(basedparse::Unary_expression const &expression)
  {
    auto const op = basedparse::get_unary_operator(expression.op.token);
    if (!op)
    {
      throw Compile_error{"unknown unary operator"};
    }
    auto operand =
      std::make_unique<Expression>(compile_expression(*expression.operand));
    auto unary = Unary_expression{};
    unary.op = *op;
    unary.operand = std::move(operand);
    return Expression{std::move(unary)};
  }

  Expression
  Compiler::compile_binary(basedparse::Binary_expression const &expression)
  {
    auto const op = basedparse::get_binary_operator(expression.op.token);
    if (!op)
    {
      throw Compile_error{"unknown binary operator"};
    }
    // Assignment is separated into its own IR node (Assign_expression) rather
    // than being a Binary_expression, since the left-hand side is a target
    // (an lvalue) rather than a value to compute.
    if (*op == basedparse::Operator::assign)
    {
      auto target =
        std::make_unique<Expression>(compile_expression(*expression.left));
      auto value =
        std::make_unique<Expression>(compile_expression(*expression.right));
      auto assign = Assign_expression{};
      assign.target = std::move(target);
      assign.value = std::move(value);
      return Expression{std::move(assign)};
    }
    auto left =
      std::make_unique<Expression>(compile_expression(*expression.left));
    auto right =
      std::make_unique<Expression>(compile_expression(*expression.right));
    auto binary = Binary_expression{};
    binary.op = *op;
    binary.left = std::move(left);
    binary.right = std::move(right);
    return Expression{std::move(binary)};
  }

  Expression
  Compiler::compile_call(basedparse::Call_expression const &expression)
  {
    auto callee =
      std::make_unique<Expression>(compile_expression(*expression.callee));
    auto arguments = std::vector<Expression>{};
    arguments.reserve(expression.arguments.size());
    for (auto const &arg : expression.arguments)
    {
      arguments.push_back(compile_expression(arg));
    }
    auto call = Call_expression{};
    call.callee = std::move(callee);
    call.arguments = std::move(arguments);
    return Expression{std::move(call)};
  }

  Block_expression
  Compiler::compile_block(basedparse::Block_expression const &expression)
  {
    push_scope();
    auto statements = std::vector<Statement>{};
    for (auto const &statement : expression.statements)
    {
      statements.push_back(compile_statement(statement));
    }
    auto tail = std::unique_ptr<Expression>{};
    if (expression.tail)
    {
      tail = std::make_unique<Expression>(compile_expression(*expression.tail));
    }
    pop_scope();
    auto block = Block_expression{};
    block.statements = std::move(statements);
    block.tail = std::move(tail);
    return block;
  }

  Expression Compiler::compile_if(basedparse::If_expression const &expression)
  {
    auto condition =
      std::make_unique<Expression>(compile_expression(*expression.condition));
    auto then_block = compile_block(expression.then_block);
    auto else_body = std::unique_ptr<Expression>{};
    if (expression.else_clause)
    {
      else_body = std::make_unique<Expression>(
        compile_expression(*expression.else_clause->body)
      );
    }
    auto if_expr = If_expression{};
    if_expr.condition = std::move(condition);
    if_expr.then_block = std::move(then_block);
    if_expr.else_body = std::move(else_body);
    return Expression{std::move(if_expr)};
  }

} // namespace basedir
