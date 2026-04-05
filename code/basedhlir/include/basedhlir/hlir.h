#ifndef BASEDHLIR_HLIR_H
#define BASEDHLIR_HLIR_H

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "constant_value.h"
#include "type.h"

namespace basedhlir
{

  class Register
  {
  public:
    Register() = default;

    explicit Register(std::int32_t id)
        : _id{id}
    {
    }

    bool has_value() const
    {
      return _id >= 0;
    }

    explicit operator bool() const
    {
      return has_value();
    }

    std::int32_t operator*() const
    {
      return _id;
    }

    bool operator==(Register const &) const = default;

  private:
    std::int32_t _id{-1};
  };

  using Operand = std::variant<Register, Constant_value>;

  struct Int8_constant_instruction
  {
    Register result;
    std::int8_t value;
  };

  struct Int16_constant_instruction
  {
    Register result;
    std::int16_t value;
  };

  struct Int32_constant_instruction
  {
    Register result;
    std::int32_t value;
  };

  struct Int64_constant_instruction
  {
    Register result;
    std::int64_t value;
  };

  struct Bool_constant_instruction
  {
    Register result;
    bool value;
  };

  struct Void_constant_instruction
  {
    Register result;
  };

  struct Copy_instruction
  {
    Register result;
    Operand source;
  };

  struct Int8_unary_plus_instruction
  {
    Register result;
    Operand operand;
  };

  struct Int8_unary_minus_instruction
  {
    Register result;
    Operand operand;
  };

  struct Int8_add_instruction
  {
    Register result;
    Operand lhs;
    Operand rhs;
  };

  struct Int8_subtract_instruction
  {
    Register result;
    Operand lhs;
    Operand rhs;
  };

  struct Int8_multiply_instruction
  {
    Register result;
    Operand lhs;
    Operand rhs;
  };

  struct Int8_divide_instruction
  {
    Register result;
    Operand lhs;
    Operand rhs;
  };

  struct Int8_modulo_instruction
  {
    Register result;
    Operand lhs;
    Operand rhs;
  };

  struct Int8_equal_instruction
  {
    Register result;
    Operand lhs;
    Operand rhs;
  };

  struct Int8_not_equal_instruction
  {
    Register result;
    Operand lhs;
    Operand rhs;
  };

  struct Int8_less_instruction
  {
    Register result;
    Operand lhs;
    Operand rhs;
  };

  struct Int8_less_eq_instruction
  {
    Register result;
    Operand lhs;
    Operand rhs;
  };

  struct Int8_greater_instruction
  {
    Register result;
    Operand lhs;
    Operand rhs;
  };

  struct Int8_greater_eq_instruction
  {
    Register result;
    Operand lhs;
    Operand rhs;
  };

  struct Int16_unary_plus_instruction
  {
    Register result;
    Operand operand;
  };

  struct Int16_unary_minus_instruction
  {
    Register result;
    Operand operand;
  };

  struct Int16_add_instruction
  {
    Register result;
    Operand lhs;
    Operand rhs;
  };

  struct Int16_subtract_instruction
  {
    Register result;
    Operand lhs;
    Operand rhs;
  };

  struct Int16_multiply_instruction
  {
    Register result;
    Operand lhs;
    Operand rhs;
  };

  struct Int16_divide_instruction
  {
    Register result;
    Operand lhs;
    Operand rhs;
  };

  struct Int16_modulo_instruction
  {
    Register result;
    Operand lhs;
    Operand rhs;
  };

  struct Int16_equal_instruction
  {
    Register result;
    Operand lhs;
    Operand rhs;
  };

  struct Int16_not_equal_instruction
  {
    Register result;
    Operand lhs;
    Operand rhs;
  };

  struct Int16_less_instruction
  {
    Register result;
    Operand lhs;
    Operand rhs;
  };

  struct Int16_less_eq_instruction
  {
    Register result;
    Operand lhs;
    Operand rhs;
  };

  struct Int16_greater_instruction
  {
    Register result;
    Operand lhs;
    Operand rhs;
  };

  struct Int16_greater_eq_instruction
  {
    Register result;
    Operand lhs;
    Operand rhs;
  };

  struct Int32_unary_plus_instruction
  {
    Register result;
    Operand operand;
  };

  struct Int32_unary_minus_instruction
  {
    Register result;
    Operand operand;
  };

  struct Int32_add_instruction
  {
    Register result;
    Operand lhs;
    Operand rhs;
  };

  struct Int32_subtract_instruction
  {
    Register result;
    Operand lhs;
    Operand rhs;
  };

  struct Int32_multiply_instruction
  {
    Register result;
    Operand lhs;
    Operand rhs;
  };

  struct Int32_divide_instruction
  {
    Register result;
    Operand lhs;
    Operand rhs;
  };

  struct Int32_modulo_instruction
  {
    Register result;
    Operand lhs;
    Operand rhs;
  };

  struct Int32_equal_instruction
  {
    Register result;
    Operand lhs;
    Operand rhs;
  };

  struct Int32_not_equal_instruction
  {
    Register result;
    Operand lhs;
    Operand rhs;
  };

  struct Int32_less_instruction
  {
    Register result;
    Operand lhs;
    Operand rhs;
  };

  struct Int32_less_eq_instruction
  {
    Register result;
    Operand lhs;
    Operand rhs;
  };

  struct Int32_greater_instruction
  {
    Register result;
    Operand lhs;
    Operand rhs;
  };

  struct Int32_greater_eq_instruction
  {
    Register result;
    Operand lhs;
    Operand rhs;
  };

  struct Int64_unary_plus_instruction
  {
    Register result;
    Operand operand;
  };

  struct Int64_unary_minus_instruction
  {
    Register result;
    Operand operand;
  };

  struct Int64_add_instruction
  {
    Register result;
    Operand lhs;
    Operand rhs;
  };

  struct Int64_subtract_instruction
  {
    Register result;
    Operand lhs;
    Operand rhs;
  };

  struct Int64_multiply_instruction
  {
    Register result;
    Operand lhs;
    Operand rhs;
  };

  struct Int64_divide_instruction
  {
    Register result;
    Operand lhs;
    Operand rhs;
  };

  struct Int64_modulo_instruction
  {
    Register result;
    Operand lhs;
    Operand rhs;
  };

  struct Int64_equal_instruction
  {
    Register result;
    Operand lhs;
    Operand rhs;
  };

  struct Int64_not_equal_instruction
  {
    Register result;
    Operand lhs;
    Operand rhs;
  };

  struct Int64_less_instruction
  {
    Register result;
    Operand lhs;
    Operand rhs;
  };

  struct Int64_less_eq_instruction
  {
    Register result;
    Operand lhs;
    Operand rhs;
  };

  struct Int64_greater_instruction
  {
    Register result;
    Operand lhs;
    Operand rhs;
  };

  struct Int64_greater_eq_instruction
  {
    Register result;
    Operand lhs;
    Operand rhs;
  };

  struct Bool_equal_instruction
  {
    Register result;
    Operand lhs;
    Operand rhs;
  };

  struct Bool_not_equal_instruction
  {
    Register result;
    Operand lhs;
    Operand rhs;
  };

  struct Function;

  struct Call_instruction
  {
    Register result;
    Function *callee;
    std::vector<Operand> arguments;
  };

  using Instruction = std::variant<
    Int8_constant_instruction,
    Int16_constant_instruction,
    Int32_constant_instruction,
    Int64_constant_instruction,
    Bool_constant_instruction,
    Void_constant_instruction,
    Copy_instruction,
    Int8_unary_plus_instruction,
    Int8_unary_minus_instruction,
    Int8_add_instruction,
    Int8_subtract_instruction,
    Int8_multiply_instruction,
    Int8_divide_instruction,
    Int8_modulo_instruction,
    Int8_equal_instruction,
    Int8_not_equal_instruction,
    Int8_less_instruction,
    Int8_less_eq_instruction,
    Int8_greater_instruction,
    Int8_greater_eq_instruction,
    Int16_unary_plus_instruction,
    Int16_unary_minus_instruction,
    Int16_add_instruction,
    Int16_subtract_instruction,
    Int16_multiply_instruction,
    Int16_divide_instruction,
    Int16_modulo_instruction,
    Int16_equal_instruction,
    Int16_not_equal_instruction,
    Int16_less_instruction,
    Int16_less_eq_instruction,
    Int16_greater_instruction,
    Int16_greater_eq_instruction,
    Int32_unary_plus_instruction,
    Int32_unary_minus_instruction,
    Int32_add_instruction,
    Int32_subtract_instruction,
    Int32_multiply_instruction,
    Int32_divide_instruction,
    Int32_modulo_instruction,
    Int32_equal_instruction,
    Int32_not_equal_instruction,
    Int32_less_instruction,
    Int32_less_eq_instruction,
    Int32_greater_instruction,
    Int32_greater_eq_instruction,
    Int64_unary_plus_instruction,
    Int64_unary_minus_instruction,
    Int64_add_instruction,
    Int64_subtract_instruction,
    Int64_multiply_instruction,
    Int64_divide_instruction,
    Int64_modulo_instruction,
    Int64_equal_instruction,
    Int64_not_equal_instruction,
    Int64_less_instruction,
    Int64_less_eq_instruction,
    Int64_greater_instruction,
    Int64_greater_eq_instruction,
    Bool_equal_instruction,
    Bool_not_equal_instruction,
    Call_instruction
  >;

  struct Basic_block;

  struct Jump_terminator
  {
    Basic_block *target;
    std::vector<Operand> arguments;
  };

  struct Branch_terminator
  {
    Operand condition;
    Basic_block *then_target;
    std::vector<Operand> then_arguments;
    Basic_block *else_target;
    std::vector<Operand> else_arguments;
  };

  struct Return_terminator
  {
    Operand value;
  };

  using Terminator = std::variant<
    std::monostate,
    Jump_terminator,
    Branch_terminator,
    Return_terminator
  >;

  struct Basic_block
  {
    std::vector<Register> parameters;
    std::vector<Instruction> instructions;
    Terminator terminator;

    bool has_terminator() const
    {
      return !std::holds_alternative<std::monostate>(terminator);
    }
  };

  struct Function
  {
    Type *type;
    std::vector<std::unique_ptr<Basic_block>> blocks;
    std::int32_t register_count;
  };

  struct Translation_unit
  {
    std::vector<std::unique_ptr<Function>> functions;
    std::unordered_map<std::string, Function *> function_table;
  };

} // namespace basedhlir

#endif
