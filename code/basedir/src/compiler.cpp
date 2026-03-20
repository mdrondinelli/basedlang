#include <cassert>
#include <string>
#include <utility>

#include "basedir/compiler.h"
#include "basedparse/operator.h"

namespace basedir
{

  Compiler::Scope_guard::Scope_guard(Compiler *compiler)
      : _compiler{compiler}
  {
    assert(_compiler);
    _compiler->push_scope();
  }

  Compiler::Scope_guard::~Scope_guard()
  {
    _compiler->pop_scope();
  }

  // Compiles a parsed AST (Translation_unit) into the IR (Program). The
  // compiler maintains a symbol table (a stack of lexical scopes) to resolve
  // identifiers to either local frame slots or global function indices, and
  // tracks types to validate operations like address-of and dereference.
  //
  // Symbols resolve to one of three kinds:
  //   - Local_binding:  a slot index into the current function's frame + type
  //   - Global_binding: a slot index into the program-wide list of functions
  //   - Type_symbol:    a type (e.g. i32)
  //
  // Within a function, parameters and let-bindings each allocate a local slot
  // via alloc_local(). The final slot count becomes the function's frame size
  // (local_names.size()). Block scopes push/pop on the scope stack so that
  // inner let-bindings shadow outer ones, but all locals share the same flat
  // frame — scopes only affect name visibility, not storage.

  Program Compiler::compile(basedparse::Translation_unit const &unit)
  {
    auto program = Program{};
    _program = &program;
    auto const global_scope_guard = scope_guard();
    declare_named_type("void");
    declare_named_type("i32");
    for (auto const &let_statement : unit.let_statements)
    {
      compile_top_level_let_statement(let_statement);
    }
    _program = nullptr;
    return program;
  }

  void Compiler::compile_top_level_let_statement(
    basedparse::Let_statement const &statement
  )
  {
    if (statement.kw_mut.has_value())
    {
      throw Compile_error{
        statement.kw_mut->line,
        statement.kw_mut->column,
        "top level let statement must not have `mut`",
      };
    }
    auto initializer_ast = &statement.initializer;
    while (auto const paren_expr_ast =
             std::get_if<basedparse::Paren_expression>(&initializer_ast->value))
    {
      initializer_ast = paren_expr_ast->inner.get();
    }
    if (!std::holds_alternative<basedparse::Fn_expression>(
          initializer_ast->value
        ))
    {
      throw Compile_error{
        statement.kw_let.line,
        statement.kw_let.column,
        "top level let statement initializer must be a function expression",
      };
    }
    auto const &fn_expr =
      std::get<basedparse::Fn_expression>(initializer_ast->value);
    auto const fn_type = compile_expression_type(*initializer_ast);
    auto const fn_index = declare_named_function(statement.name.text, fn_type);
    for (auto const &param : fn_expr.parameters)
    {
      if (param.kw_mut.has_value())
      {
        assert(false && "`mut` parameters not supported yet");
      }
    }
    auto const prev_fn_body = _function_body;
    auto fn_body = Function_body{};
    _function_body = &fn_body;
    auto tmp_fn_body = std::move(*_function_body);
    /// TODO
    define_function(fn_index, blocks);
  }

  Type *Compiler::compile_type_expression(
    basedparse::Type_expression const &type_expr
  )
  {
    if (auto const ident =
          std::get_if<basedparse::Identifier_type_expression>(&type_expr.value))
    {
      auto const type = types().get_named(ident->identifier.text);
      if (!type)
      {
        throw Compile_error{
          ident->identifier.line,
          ident->identifier.column,
          "unknown type: " + ident->identifier.text,
        };
      }
      return type;
    }
    if (auto const arr =
          std::get_if<basedparse::Array_type_expression>(&type_expr.value))
    {
      auto const element = compile_type_expression(*arr->element_type);
      auto size = std::optional<std::size_t>{};
      if (arr->size)
      {
        auto const size_expr =
          std::get_if<basedparse::Int_literal_expression>(&arr->size->value);
        if (!size_expr)
        {
          // TODO: allow any compile-time constant expression of integral type
          assert(false && "Only int literal array size supported for now");
        }
        size = static_cast<std::size_t>(std::stoi(size_expr->literal.text));
      }
      return types().get_array(element, size);
    }
    if (auto const ptr =
          std::get_if<basedparse::Pointer_type_expression>(&type_expr.value))
    {
      auto const pointee = compile_type_expression(*ptr->pointee_type);
      return types().get_pointer(pointee, ptr->kw_mut.has_value());
    }
    assert(false && "Unreachable");
  }

  Type *Compiler::compile_expression_type(basedparse::Expression const &expr)
  {
    struct Visitor
    {
      Compiler *self;

      Type *operator()(basedparse::Int_literal_expression const &) const
      {
        return self->types().get_named("i32");
      }

      Type *operator()(basedparse::Identifier_expression const &id_expr) const
      {
        auto const &identifier = id_expr.identifier;
        auto const &symbol = self->resolve_identifier(identifier);
        if (auto const function_symbol =
              std::get_if<Function_symbol>(&symbol.value))
        {
          return self->_program->functions[function_symbol->function]
            .prototype.type;
        }
        if (auto const local_symbol = std::get_if<Local_symbol>(&symbol.value))
        {
        }
        if (!std::holds_alternative<Type_symbol>(symbol.value))
        {
          throw Compile_error{
            identifier.line,
            identifier.column,
            std::format("'{}' is not a type", identifier.text),
          };
        }
        return std::get<Type_symbol>(symbol.value).type;
      }

      Type *operator()(basedparse::Fn_expression const &fn_expr) const
      {
        auto param_types = std::vector<Type *>{};
        for (auto const &param : fn_expr.parameters)
        {
          param_types.push_back(
            self->compile_type_expression(param.type_expression)
          );
        }
        if (!fn_expr.return_type_specifier)
        {
          assert(false && "Deduced return type not supported for now");
        }
        auto const return_type = self->compile_type_expression(
          fn_expr.return_type_specifier->type_expression
        );
        return self->types().get_function(param_types, return_type);
      }

      Type *operator()(basedparse::Paren_expression const &paren_expr) const
      {
        return self->compile_expression_type(*paren_expr.inner);
      }

      Type *operator()(basedparse::Unary_expression const &unary_expr) const
      {
        auto const op = *basedparse::get_unary_operator(unary_expr.op.token);
        auto const operand_type =
          self->compile_expression_type(*unary_expr.operand);
        switch (op)
        {
          using enum basedparse::Operator;
        case address_of:
          return self->types().get_pointer(operand_type, false);
        case address_of_mut:
          return self->types().get_pointer(operand_type, true);
        case dereference:
          {
            if (!std::holds_alternative<Pointer_type>(operand_type->value))
            {
              throw Compile_error{
                unary_expr.op.line,
                unary_expr.op.column,
                "operand of dereference expression must be a pointer",
              };
            }
            auto const &pointer_type =
              std::get<Pointer_type>(operand_type->value);
            auto const pointee_type = pointer_type.pointee;
            return self->types().get_reference(
              pointee_type,
              pointer_type.is_mutable
            );
          }
        case unary_plus:
          {
            auto const i32_type = self->types().get_named("i32");
            if (operand_type != i32_type)
            {
              throw Compile_error{
                unary_expr.op.line,
                unary_expr.op.column,
                "operand of unary plus expression must be i32",
              };
            }
            return i32_type;
          }
        case unary_minus:
          {
            auto const i32_type = self->types().get_named("i32");
            if (operand_type != i32_type)
            {
              throw Compile_error{
                unary_expr.op.line,
                unary_expr.op.column,
                "operand of unary minus expression must be i32",
              };
            }
            return i32_type;
          }
        default:
          assert(false && "Unreachable");
          break;
        }
      }

      Type *operator()(basedparse::Binary_expression const &binary_expr) const
      {
        auto const op = *basedparse::get_binary_operator(binary_expr.op.token);
        auto const lhs_type = self->compile_expression_type(*binary_expr.left);
        auto const rhs_type = self->compile_expression_type(*binary_expr.right);
        switch (op)
        {
          using enum basedparse::Operator;
        case multiply:
          {
            auto const i32_type = self->types().get_named("i32");
            if (lhs_type != i32_type || rhs_type != i32_type)
            {
              throw Compile_error{
                binary_expr.op.line,
                binary_expr.op.column,
                "operands of multiply expression must be i32",
              };
            }
            return i32_type;
          }
        case divide:
          {
            auto const i32_type = self->types().get_named("i32");
            if (lhs_type != i32_type || rhs_type != i32_type)
            {
              throw Compile_error{
                binary_expr.op.line,
                binary_expr.op.column,
                "operands of divide expression must be i32",
              };
            }
            return i32_type;
          }
        case modulo:
          {
            auto const i32_type = self->types().get_named("i32");
            if (lhs_type != i32_type || rhs_type != i32_type)
            {
              throw Compile_error{
                binary_expr.op.line,
                binary_expr.op.column,
                "operands of modulo expression must be i32",
              };
            }
            return i32_type;
          }
        case add:
          {
            auto const i32_type = self->types().get_named("i32");
            if (lhs_type != i32_type || rhs_type != i32_type)
            {
              throw Compile_error{
                binary_expr.op.line,
                binary_expr.op.column,
                "operands of add expression must be i32",
              };
            }
            return i32_type;
          }
        case subtract:
          {
            auto const i32_type = self->types().get_named("i32");
            if (lhs_type != i32_type || rhs_type != i32_type)
            {
              throw Compile_error{
                binary_expr.op.line,
                binary_expr.op.column,
                "operands of subtract expression must be i32",
              };
            }
            return i32_type;
          }
        case less:
          {
            auto const i32_type = self->types().get_named("i32");
            if (lhs_type != i32_type || rhs_type != i32_type)
            {
              throw Compile_error{
                binary_expr.op.line,
                binary_expr.op.column,
                "operands of less-than expression must be i32",
              };
            }
            return self->types().get_named("bool");
          }
        case less_eq:
          {
            auto const i32_type = self->types().get_named("i32");
            if (lhs_type != i32_type || rhs_type != i32_type)
            {
              throw Compile_error{
                binary_expr.op.line,
                binary_expr.op.column,
                "operands of less-than-or-equal-to expression must be i32",
              };
            }
            return self->types().get_named("bool");
          }
        case greater:
          {
            auto const i32_type = self->types().get_named("i32");
            if (lhs_type != i32_type || rhs_type != i32_type)
            {
              throw Compile_error{
                binary_expr.op.line,
                binary_expr.op.column,
                "operands of greater-than expression must be i32",
              };
            }
            return self->types().get_named("bool");
          }
        case greater_eq:
          {
            auto const i32_type = self->types().get_named("i32");
            if (lhs_type != i32_type || rhs_type != i32_type)
            {
              throw Compile_error{
                binary_expr.op.line,
                binary_expr.op.column,
                "operands of greater-than-or-equal-to expression must be i32",
              };
            }
            return self->types().get_named("bool");
          }
        case equal:
          {
            if (lhs_type == rhs_type)
            {
              throw Compile_error{
                binary_expr.op.line,
                binary_expr.op.column,
                "operands of equal expression must have the same type",
              };
            }
            return self->types().get_named("bool");
          }
        case not_equal:
          {
            if (lhs_type == rhs_type)
            {
              throw Compile_error{
                binary_expr.op.line,
                binary_expr.op.column,
                "operands of not-equal expression must have the same type",
              };
            }
            return self->types().get_named("bool");
          }
        case assign:
          {
            if (auto const lhs_ref_type = lhs_type->is_reference_type();
                !lhs_ref_type || !lhs_ref_type->is_mutable)
            {
              throw Compile_error{
                binary_expr.op.line,
                binary_expr.op.column,
                "left hand side of assignment expression must refer to a "
                "mutable value",
              };
            }
            if (lhs_type->strip_reference() != rhs_type->strip_reference())
            {
              throw Compile_error{
                binary_expr.op.line,
                binary_expr.op.column,
                "left and right hand sides of assignment expression must have "
                "the same type",
              };
            }
            return lhs_type;
          }
        default:
          assert(false && "Unreachable");
          break;
        }
      }

      Type *operator()(basedparse::Call_expression const &call_expr) const
      {
        auto const callee_type =
          self->compile_expression_type(*call_expr.callee);
        auto const callee_fn_type = callee_type->is_function_type();
        if (!callee_fn_type)
        {
          throw Compile_error{
            call_expr.lparen.line,
            call_expr.lparen.column,
            "callee must have function type",
          };
        }
        return callee_fn_type->return_type;
      }

      Type *operator()(basedparse::Index_expression const &index_expr) const
      {
        auto const operand_type =
          self->compile_expression_type(*index_expr.operand);
        auto const operand_array_type = operand_type->is_array_type();
        if (!operand_array_type)
        {
          throw Compile_error{
            index_expr.lbracket.line,
            index_expr.lbracket.column,
            "index expression must operate on an array",
          };
        }
        return operand_array_type->element;
      }
    };

    return std::visit(Visitor{this}, expr.value);
  }

  Type *
  Compiler::compile_fn_expression_type(basedparse::Fn_expression const &fn_expr)
  {
    auto param_types = std::vector<Type *>{};
    for (auto const &param : fn_expr.parameters)
    {
      param_types.push_back(compile_type_expression(param.type_expression));
    }
    if (!fn_expr.return_type_specifier)
    {
      assert(false && "Deduced return type not supported for now");
    }
    auto const return_type =
      compile_type_expression(fn_expr.return_type_specifier->type_expression);
    return types().get_function(param_types, return_type);
  }

  std::int64_t
  Compiler::declare_named_stack_value(std::string name, Type *type, bool mut)
  {
    assert(_program != nullptr);
    assert(_function_body != nullptr);
    return reserve_register(types().get_pointer(type, mut));
  }

  std::int64_t Compiler::declare_named_function(std::string name, Type *type)
  {
    assert(type->is_function_type());
    auto const index = static_cast<std::int64_t>(_program->functions.size());
    _program->functions.emplace_back(
      Function{
        .prototype =
          Function_prototype{
            .name = name,
            .type = type,
          },
        .body = std::nullopt,
      }
    );
    bind_identifier(std::move(name), {Symbol_handle::Named_function{index}});
    return index;
  }

  Type *Compiler::declare_named_type(std::string name)
  {
    auto const type = types().get_named(name);
    bind_identifier(std::move(name), {Symbol_handle::Named_type{type}});
    return type;
  }

  void Compiler::bind_identifier(std::string name, Symbol_handle symbol)
  {
    _scopes.back().emplace(std::move(name), symbol);
  }

  Compiler::Symbol_handle
  Compiler::resolve_identifier(basedlex::Lexeme const &identifier) const
  {
    for (auto scope_it = _scopes.rbegin(); scope_it != _scopes.rend();
         ++scope_it)
    {
      if (auto const symbol_it = scope_it->find(identifier.text);
          symbol_it != scope_it->end())
      {
        return symbol_it->second;
      }
    }
    throw Compile_error{
      identifier.line,
      identifier.column,
      std::format("failed to resolve identifier '{}'", identifier.text),
    };
  }

  void Compiler::push_scope()
  {
    _scopes.emplace_back();
  }

  void Compiler::pop_scope() noexcept
  {
    _scopes.pop_back();
  }

  std::int64_t Compiler::reserve_register(Type *type)
  {
    assert(_program);
    assert(_function_body);
    auto const index =
      static_cast<std::int64_t>(_function_body->register_types.size());
    _function_body->register_types.emplace_back(type);
    return index;
  }

  Type_pool &Compiler::types() const noexcept
  {
    assert(_program);
    return _program->types;
  }

} // namespace basedir
