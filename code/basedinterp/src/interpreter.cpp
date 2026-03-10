#include <cassert>
#include <cstdint>
#include <string>
#include <utility>
#include <variant>

#include "basedinterp/interpreter.h"
#include "basedir/type.h"
#include "basedparse/operator.h"

namespace basedinterp
{

  namespace
  {

    Value strip_reference(Value const &v)
    {
      if (auto const ref = std::get_if<Reference_value>(&v.data))
      {
        return *ref->target;
      }
      return v;
    }

  } // namespace

  struct Return_signal
  {
    Value value;
  };

  Interpreter::Interpreter(basedir::Program const *program)
      : _program{program}
  {
    for (auto i = std::size_t{}; i < _program->functions.size(); ++i)
    {
      _globals.push_back(std::make_shared<Value>(Value{Function_value{i}}));
    }
  }

  Value Interpreter::call(std::string const &name, std::vector<Value> arguments)
  {
    for (auto i = std::size_t{}; i < _program->functions.size(); ++i)
    {
      if (_program->functions[i].declaration.name == name)
      {
        return call(i, std::move(arguments));
      }
    }
    throw Runtime_error{"undefined function: " + name};
  }

  Value Interpreter::call(
    std::size_t function_index,
    std::vector<Value> arguments
  )
  {
    return execute_function(function_index, arguments);
  }

  void Interpreter::push_frame(std::size_t frame_size)
  {
    _call_stack.emplace_back();
    auto &locals = _call_stack.back().locals;
    locals.reserve(frame_size);
    for (auto i = std::size_t{}; i < frame_size; ++i)
    {
      locals.push_back(std::make_shared<Value>());
    }
  }

  void Interpreter::pop_frame()
  {
    _call_stack.pop_back();
  }

  std::shared_ptr<Value> &Interpreter::local_ptr(std::size_t index)
  {
    return _call_stack.back().locals[index];
  }

  Value &Interpreter::local(std::size_t index)
  {
    return *_call_stack.back().locals[index];
  }

  Value Interpreter::execute_function(
    std::size_t function_index,
    std::vector<Value> const &arguments
  )
  {
    auto const &fn = _program->functions[function_index];
    auto const &fn_type =
      std::get<basedir::Function_type>(fn.declaration.type->value);
    assert(fn.definition);
    auto const &body = *fn.definition;
    push_frame(body.local_names.size());
    for (auto i = std::size_t{}; i < fn_type.parameter_types.size(); ++i)
    {
      local(i) = arguments[i];
    }
    auto result = Value{};
    try
    {
      result = evaluate_block(body.body);
    }
    catch (Return_signal &signal)
    {
      result = std::move(signal.value);
    }
    result = strip_reference(result);
    pop_frame();
    return result;
  }

  void Interpreter::execute_statement(basedir::Statement const &statement)
  {
    if (auto const s = std::get_if<basedir::Let_statement>(&statement.value))
    {
      execute_let_statement(*s);
    }
    else if (auto const s =
               std::get_if<basedir::While_statement>(&statement.value))
    {
      execute_while_statement(*s);
    }
    else if (auto const s =
               std::get_if<basedir::Return_statement>(&statement.value))
    {
      throw Return_signal{evaluate_expression(s->value)};
    }
    else if (auto const s =
               std::get_if<basedir::Expression_statement>(&statement.value))
    {
      execute_expression_statement(*s);
    }
    else
    {
      throw Runtime_error{"unexpected statement type"};
    }
  }

  void Interpreter::execute_let_statement(
    basedir::Let_statement const &statement
  )
  {
    local(statement.index) =
      strip_reference(evaluate_expression(statement.initializer));
  }

  void Interpreter::execute_while_statement(
    basedir::While_statement const &statement
  )
  {
    for (;;)
    {
      auto const condition =
        strip_reference(evaluate_expression(*statement.condition));
      auto const int_val = std::get_if<std::int32_t>(&condition.data);
      if (!int_val)
      {
        throw Runtime_error{"while condition must be an integer"};
      }
      if (*int_val == 0)
      {
        break;
      }
      for (auto const &s : statement.body.statements)
      {
        execute_statement(s);
      }
    }
  }

  void Interpreter::execute_expression_statement(
    basedir::Expression_statement const &statement
  )
  {
    evaluate_expression(statement.expression);
  }

  Value Interpreter::evaluate_expression(basedir::Expression const &expression)
  {
    if (auto const e =
          std::get_if<basedir::Int_literal_expression>(&expression.value))
    {
      return Value{static_cast<std::int32_t>(e->value)};
    }
    if (auto const e =
          std::get_if<basedir::Binding_expression>(&expression.value))
    {
      if (auto const local = std::get_if<basedir::Local_binding>(&e->value))
      {
        return Value{Reference_value{local_ptr(local->index)}};
      }
      auto const &global = std::get<basedir::Global_binding>(e->value);
      return Value{Reference_value{_globals[global.index]}};
    }
    if (auto const e =
          std::get_if<basedir::Unary_expression>(&expression.value))
    {
      return evaluate_unary(*e);
    }
    if (auto const e =
          std::get_if<basedir::Binary_expression>(&expression.value))
    {
      return evaluate_binary(*e);
    }
    if (auto const e =
          std::get_if<basedir::Assign_expression>(&expression.value))
    {
      return evaluate_assign(*e);
    }
    if (auto const e = std::get_if<basedir::Call_expression>(&expression.value))
    {
      return evaluate_call(*e);
    }
    if (auto const e =
          std::get_if<basedir::Index_expression>(&expression.value))
    {
      return evaluate_index(*e);
    }
    if (auto const e =
          std::get_if<basedir::Constructor_expression>(&expression.value))
    {
      return evaluate_constructor(*e);
    }
    if (auto const e =
          std::get_if<basedir::Block_expression>(&expression.value))
    {
      return evaluate_block(*e);
    }
    if (auto const e = std::get_if<basedir::If_expression>(&expression.value))
    {
      return evaluate_if(*e);
    }
    throw Runtime_error{"unexpected expression type"};
  }

  Value Interpreter::evaluate_unary(basedir::Unary_expression const &expression)
  {
    if (expression.op == basedparse::Operator::address_of)
    {
      auto const operand = evaluate_expression(*expression.operand);
      auto const ref = std::get_if<Reference_value>(&operand.data);
      if (!ref)
      {
        throw Runtime_error{"address-of requires an lvalue operand"};
      }
      return Value{Pointer_value{ref->target}};
    }
    if (expression.op == basedparse::Operator::dereference)
    {
      auto const operand =
        strip_reference(evaluate_expression(*expression.operand));
      auto const ptr = std::get_if<Pointer_value>(&operand.data);
      if (!ptr)
      {
        throw Runtime_error{"dereference requires a pointer operand"};
      }
      auto const locked = ptr->target.lock();
      if (!locked)
      {
        throw Runtime_error{"dangling pointer"};
      }
      return Value{Reference_value{locked}};
    }
    auto const operand =
      strip_reference(evaluate_expression(*expression.operand));
    auto const int_val = std::get_if<std::int32_t>(&operand.data);
    if (!int_val)
    {
      throw Runtime_error{"unary operator requires an integer operand"};
    }
    switch (expression.op)
    {
    case basedparse::Operator::unary_plus:
      return Value{*int_val};
    case basedparse::Operator::unary_minus:
      return Value{static_cast<std::int32_t>(-*int_val)};
    default:
      throw Runtime_error{"unsupported unary operator"};
    }
  }

  Value Interpreter::evaluate_binary(
    basedir::Binary_expression const &expression
  )
  {
    auto const left = strip_reference(evaluate_expression(*expression.left));
    auto const right = strip_reference(evaluate_expression(*expression.right));
    auto const lhs = std::get_if<std::int32_t>(&left.data);
    auto const rhs = std::get_if<std::int32_t>(&right.data);
    if (!lhs || !rhs)
    {
      throw Runtime_error{"binary operator requires integer operands"};
    }
    switch (expression.op)
    {
    case basedparse::Operator::add:
      return Value{static_cast<std::int32_t>(*lhs + *rhs)};
    case basedparse::Operator::subtract:
      return Value{static_cast<std::int32_t>(*lhs - *rhs)};
    case basedparse::Operator::multiply:
      return Value{static_cast<std::int32_t>(*lhs * *rhs)};
    case basedparse::Operator::divide:
      if (*rhs == 0)
      {
        throw Runtime_error{"division by zero"};
      }
      return Value{static_cast<std::int32_t>(*lhs / *rhs)};
    case basedparse::Operator::modulo:
      if (*rhs == 0)
      {
        throw Runtime_error{"modulo by zero"};
      }
      return Value{static_cast<std::int32_t>(*lhs % *rhs)};
    case basedparse::Operator::less:
      return Value{static_cast<std::int32_t>(*lhs < *rhs)};
    case basedparse::Operator::less_eq:
      return Value{static_cast<std::int32_t>(*lhs <= *rhs)};
    case basedparse::Operator::greater:
      return Value{static_cast<std::int32_t>(*lhs > *rhs)};
    case basedparse::Operator::greater_eq:
      return Value{static_cast<std::int32_t>(*lhs >= *rhs)};
    case basedparse::Operator::equal:
      return Value{static_cast<std::int32_t>(*lhs == *rhs)};
    case basedparse::Operator::not_equal:
      return Value{static_cast<std::int32_t>(*lhs != *rhs)};
    default:
      throw Runtime_error{"unsupported binary operator"};
    }
  }

  Value Interpreter::evaluate_assign(
    basedir::Assign_expression const &expression
  )
  {
    auto const target = evaluate_expression(*expression.target);
    auto const ref = std::get_if<Reference_value>(&target.data);
    if (!ref)
    {
      throw Runtime_error{"assignment target must be an lvalue"};
    }
    auto const rhs = strip_reference(evaluate_expression(*expression.value));
    *ref->target = rhs;
    return target;
  }

  Value Interpreter::evaluate_call(basedir::Call_expression const &expression)
  {
    auto const callee =
      strip_reference(evaluate_expression(*expression.callee));
    auto const fn = std::get_if<Function_value>(&callee.data);
    if (!fn)
    {
      throw Runtime_error{"callee is not a function"};
    }
    auto arguments = std::vector<Value>{};
    arguments.reserve(expression.arguments.size());
    for (auto const &arg : expression.arguments)
    {
      arguments.push_back(strip_reference(evaluate_expression(arg)));
    }
    return execute_function(fn->index, arguments);
  }

  Value Interpreter::evaluate_index(basedir::Index_expression const &expression)
  {
    auto const operand = evaluate_expression(*expression.operand);
    auto const index_val =
      strip_reference(evaluate_expression(*expression.index));
    auto const idx = std::get_if<std::int32_t>(&index_val.data);
    // The operand may be a reference to an array, or a direct array value.
    auto const resolve_array =
      [&](Value const &v) -> Array_value const *
    {
      if (auto const ref = std::get_if<Reference_value>(&v.data))
      {
        return std::get_if<Array_value>(&ref->target->data);
      }
      return std::get_if<Array_value>(&v.data);
    };
    auto const arr = resolve_array(operand);
    if (!arr)
    {
      throw Runtime_error{"index requires an array operand"};
    }
    auto const i = static_cast<std::size_t>(*idx);
    if (i >= arr->elements.size())
    {
      throw Runtime_error{
        "array index out of bounds: " + std::to_string(*idx) + " >= " +
        std::to_string(arr->elements.size())
      };
    }
    return Value{Reference_value{arr->elements[i]}};
  }

  Value Interpreter::evaluate_constructor(
    basedir::Constructor_expression const &expression
  )
  {
    auto elements = std::vector<std::shared_ptr<Value>>{};
    elements.reserve(expression.arguments.size());
    for (auto const &arg : expression.arguments)
    {
      elements.push_back(
        std::make_shared<Value>(strip_reference(evaluate_expression(arg)))
      );
    }
    return Value{Array_value{std::move(elements)}};
  }

  Value Interpreter::evaluate_block(basedir::Block_expression const &expression)
  {
    for (auto const &statement : expression.statements)
    {
      execute_statement(statement);
    }
    if (expression.tail)
    {
      return evaluate_expression(*expression.tail);
    }
    return Value{std::int32_t{0}};
  }

  Value Interpreter::evaluate_if(basedir::If_expression const &expression)
  {
    auto const condition =
      strip_reference(evaluate_expression(*expression.condition));
    auto const int_val = std::get_if<std::int32_t>(&condition.data);
    if (!int_val)
    {
      throw Runtime_error{"if condition must be an integer"};
    }
    if (*int_val != 0)
    {
      return evaluate_block(expression.then_block);
    }
    if (expression.else_body)
    {
      return evaluate_expression(*expression.else_body);
    }
    return Value{std::int32_t{0}};
  }

} // namespace basedinterp
