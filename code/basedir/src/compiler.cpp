#include <string>
#include <utility>

#include "basedir/compiler.h"
#include "basedparse/operator.h"

namespace basedir
{

  // Compiles a parsed AST (Translation_unit) into the IR (Program). The
  // compiler maintains a symbol table (a stack of lexical scopes) to resolve
  // identifiers to either local frame slots or global function indices, and
  // tracks types to validate operations like address-of and dereference.
  //
  // Symbols resolve to one of three kinds:
  //   - Local_binding:  a slot index into the current function's frame + type
  //   - Global_binding: an index into Program::functions
  //   - Type_symbol:    a type (e.g. i32)
  //
  // Top-level compilation has two phases:
  //   1. Register all function names as Global_bindings (so every function is
  //      visible during body compilation, enabling recursion and mutual
  //      recursion).
  //   2. Compile each function body, during which the symbol table resolves
  //      identifiers to Binding_expression IR nodes.
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
    define("i32", Type_symbol{_types.get_named("i32")});
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

  Type *Compiler::resolve_type(basedparse::Type_expression const &type_expr)
  {
    if (auto const ident =
          std::get_if<basedparse::Identifier_type_expression>(&type_expr.value))
    {
      auto const type = _types.get_named(ident->identifier.text);
      if (!type)
      {
        throw Compile_error{"unknown type: " + ident->identifier.text};
      }
      return type;
    }
    if (auto const ptr =
          std::get_if<basedparse::Pointer_type_expression>(&type_expr.value))
    {
      auto const pointee = resolve_type(*ptr->pointee_type);
      return _types.get_pointer(pointee);
    }
    throw Compile_error{"unsupported type expression"};
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
      auto const type = resolve_type(param.type_expression);
      auto const index = alloc_local(param.name.text);
      define(param.name.text, Local_binding{index, is_mutable, type});
    }
    auto body = compile_block(*fn.body);
    pop_scope();
    return Function{
      std::move(name),
      fn.parameters.size(),
      std::move(_local_names),
      std::get<Block_expression>(std::move(body).value),
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
    define(
      statement.name.text,
      Local_binding{index, is_mutable, strip_reference(initializer.type)}
    );
    return Statement{Let_statement{index, std::move(initializer)}};
  }

  Statement Compiler::compile_while_statement(
    basedparse::While_statement const &statement
  )
  {
    auto cond = compile_expression(*statement.condition);
    auto condition = std::make_unique<Expression>(std::move(cond));
    auto body = compile_block(statement.body);
    auto while_stmt = While_statement{};
    while_stmt.condition = std::move(condition);
    while_stmt.body = std::get<Block_expression>(std::move(body).value);
    return Statement{std::move(while_stmt)};
  }

  Statement Compiler::compile_return_statement(
    basedparse::Return_statement const &statement
  )
  {
    auto expr = compile_expression(statement.value);
    return Statement{Return_statement{std::move(expr)}};
  }

  Statement Compiler::compile_expression_statement(
    basedparse::Expression_statement const &statement
  )
  {
    auto expr = compile_expression(statement.expression);
    return Statement{Expression_statement{std::move(expr)}};
  }

  Type *Compiler::strip_reference(Type *type)
  {
    if (!type)
    {
      return nullptr;
    }
    if (auto const ref = std::get_if<Reference_type>(&type->value))
    {
      return ref->referent;
    }
    return type;
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
      return compile_block(*e);
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
    auto result =
      Expression{Int_literal_expression{std::stoi(expression.literal.text)}};
    result.type = _types.get_named("i32");
    return result;
  }

  // Resolve an identifier to a Binding_expression by looking it up in the
  // symbol table. Type symbols (e.g. i32) are not values and produce an error
  // if used in expression position.
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
      auto binding = Binding_expression{};
      binding.value = basedir::Local_binding{local->index};
      auto result = Expression{std::move(binding)};
      result.type = _types.get_reference(local->type);
      return result;
    }
    if (auto const global = std::get_if<Global_binding>(symbol))
    {
      auto binding = Binding_expression{};
      binding.value = basedir::Global_binding{global->index};
      // Functions don't have a type in the type system yet.
      return Expression{std::move(binding)};
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
    if (*op == basedparse::Operator::address_of)
    {
      auto operand = compile_expression(*expression.operand);
      auto const ref = operand.type
                         ? std::get_if<Reference_type>(&operand.type->value)
                         : nullptr;
      if (!ref)
      {
        throw Compile_error{"address-of requires an lvalue operand"};
      }
      auto const result_type = _types.get_pointer(ref->referent);
      auto unary = Unary_expression{};
      unary.op = *op;
      unary.operand = std::make_unique<Expression>(std::move(operand));
      auto result = Expression{std::move(unary)};
      result.type = result_type;
      return result;
    }
    if (*op == basedparse::Operator::dereference)
    {
      auto operand = compile_expression(*expression.operand);
      auto const stripped = strip_reference(operand.type);
      if (!stripped || !std::get_if<Pointer_type>(&stripped->value))
      {
        throw Compile_error{"dereference requires a pointer operand"};
      }
      auto const pointee_type = std::get<Pointer_type>(stripped->value).pointee;
      auto unary = Unary_expression{};
      unary.op = *op;
      unary.operand = std::make_unique<Expression>(std::move(operand));
      auto result = Expression{std::move(unary)};
      result.type = _types.get_reference(pointee_type);
      return result;
    }
    auto operand = compile_expression(*expression.operand);
    auto const result_type = strip_reference(operand.type);
    auto unary = Unary_expression{};
    unary.op = *op;
    unary.operand = std::make_unique<Expression>(std::move(operand));
    auto result = Expression{std::move(unary)};
    result.type = result_type;
    return result;
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
      auto target = compile_expression(*expression.left);
      auto const ref = target.type
                         ? std::get_if<Reference_type>(&target.type->value)
                         : nullptr;
      if (!ref)
      {
        throw Compile_error{"assignment target must be an lvalue"};
      }
      auto const result_type = target.type;
      auto value = compile_expression(*expression.right);
      auto assign = Assign_expression{};
      assign.target = std::make_unique<Expression>(std::move(target));
      assign.value = std::make_unique<Expression>(std::move(value));
      auto result = Expression{std::move(assign)};
      result.type = result_type;
      return result;
    }
    auto left = compile_expression(*expression.left);
    auto const result_type = strip_reference(left.type);
    auto right = compile_expression(*expression.right);
    auto binary = Binary_expression{};
    binary.op = *op;
    binary.left = std::make_unique<Expression>(std::move(left));
    binary.right = std::make_unique<Expression>(std::move(right));
    auto result = Expression{std::move(binary)};
    result.type = result_type;
    return result;
  }

  Expression
  Compiler::compile_call(basedparse::Call_expression const &expression)
  {
    auto callee = compile_expression(*expression.callee);
    auto arguments = std::vector<Expression>{};
    arguments.reserve(expression.arguments.size());
    for (auto const &arg : expression.arguments)
    {
      arguments.push_back(compile_expression(arg));
    }
    auto call = Call_expression{};
    call.callee = std::make_unique<Expression>(std::move(callee));
    call.arguments = std::move(arguments);
    // Without function types, we don't know the return type.
    return Expression{std::move(call)};
  }

  Expression
  Compiler::compile_block(basedparse::Block_expression const &expression)
  {
    push_scope();
    auto statements = std::vector<Statement>{};
    for (auto const &statement : expression.statements)
    {
      statements.push_back(compile_statement(statement));
    }
    auto tail = std::unique_ptr<Expression>{};
    auto tail_type = static_cast<Type *>(nullptr);
    if (expression.tail)
    {
      auto tail_expr = compile_expression(*expression.tail);
      tail_type = tail_expr.type;
      tail = std::make_unique<Expression>(std::move(tail_expr));
    }
    pop_scope();
    auto block = Block_expression{};
    block.statements = std::move(statements);
    block.tail = std::move(tail);
    auto result = Expression{std::move(block)};
    result.type = tail_type;
    return result;
  }

  Expression Compiler::compile_if(basedparse::If_expression const &expression)
  {
    auto cond = compile_expression(*expression.condition);
    auto condition = std::make_unique<Expression>(std::move(cond));
    auto then_expr = compile_block(expression.then_block);
    auto const result_type = then_expr.type;
    auto else_body = std::unique_ptr<Expression>{};
    if (expression.else_clause)
    {
      auto else_expr = compile_expression(*expression.else_clause->body);
      else_body = std::make_unique<Expression>(std::move(else_expr));
    }
    auto if_expr = If_expression{};
    if_expr.condition = std::move(condition);
    if_expr.then_block = std::get<Block_expression>(std::move(then_expr).value);
    if_expr.else_body = std::move(else_body);
    auto result = Expression{std::move(if_expr)};
    result.type = result_type;
    return result;
  }

} // namespace basedir
