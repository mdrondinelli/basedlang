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

  template <typename T>
  struct Integer_constant_instruction
  {
    using value_type = T;
    Register result;
    T value;
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

  template <typename T>
  struct Integer_negate_instruction
  {
    using value_type = T;
    Register result;
    Operand operand;
  };

  template <typename T>
  struct Integer_add_instruction
  {
    using value_type = T;
    Register result;
    Operand lhs;
    Operand rhs;
  };

  template <typename T>
  struct Integer_subtract_instruction
  {
    using value_type = T;
    Register result;
    Operand lhs;
    Operand rhs;
  };

  template <typename T>
  struct Integer_multiply_instruction
  {
    using value_type = T;
    Register result;
    Operand lhs;
    Operand rhs;
  };

  template <typename T>
  struct Integer_divide_instruction
  {
    using value_type = T;
    Register result;
    Operand lhs;
    Operand rhs;
  };

  template <typename T>
  struct Integer_modulo_instruction
  {
    using value_type = T;
    Register result;
    Operand lhs;
    Operand rhs;
  };

  template <typename T>
  struct Integer_equal_instruction
  {
    using value_type = T;
    Register result;
    Operand lhs;
    Operand rhs;
  };

  template <typename T>
  struct Integer_not_equal_instruction
  {
    using value_type = T;
    Register result;
    Operand lhs;
    Operand rhs;
  };

  template <typename T>
  struct Integer_less_instruction
  {
    using value_type = T;
    Register result;
    Operand lhs;
    Operand rhs;
  };

  template <typename T>
  struct Integer_less_eq_instruction
  {
    using value_type = T;
    Register result;
    Operand lhs;
    Operand rhs;
  };

  template <typename T>
  struct Integer_greater_instruction
  {
    using value_type = T;
    Register result;
    Operand lhs;
    Operand rhs;
  };

  template <typename T>
  struct Integer_greater_eq_instruction
  {
    using value_type = T;
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
    Integer_constant_instruction<std::int8_t>,
    Integer_constant_instruction<std::int16_t>,
    Integer_constant_instruction<std::int32_t>,
    Integer_constant_instruction<std::int64_t>,
    Bool_constant_instruction,
    Void_constant_instruction,
    Copy_instruction,
    Integer_negate_instruction<std::int8_t>,
    Integer_negate_instruction<std::int16_t>,
    Integer_negate_instruction<std::int32_t>,
    Integer_negate_instruction<std::int64_t>,
    Integer_add_instruction<std::int8_t>,
    Integer_add_instruction<std::int16_t>,
    Integer_add_instruction<std::int32_t>,
    Integer_add_instruction<std::int64_t>,
    Integer_subtract_instruction<std::int8_t>,
    Integer_subtract_instruction<std::int16_t>,
    Integer_subtract_instruction<std::int32_t>,
    Integer_subtract_instruction<std::int64_t>,
    Integer_multiply_instruction<std::int8_t>,
    Integer_multiply_instruction<std::int16_t>,
    Integer_multiply_instruction<std::int32_t>,
    Integer_multiply_instruction<std::int64_t>,
    Integer_divide_instruction<std::int8_t>,
    Integer_divide_instruction<std::int16_t>,
    Integer_divide_instruction<std::int32_t>,
    Integer_divide_instruction<std::int64_t>,
    Integer_modulo_instruction<std::int8_t>,
    Integer_modulo_instruction<std::int16_t>,
    Integer_modulo_instruction<std::int32_t>,
    Integer_modulo_instruction<std::int64_t>,
    Integer_equal_instruction<std::int8_t>,
    Integer_equal_instruction<std::int16_t>,
    Integer_equal_instruction<std::int32_t>,
    Integer_equal_instruction<std::int64_t>,
    Integer_not_equal_instruction<std::int8_t>,
    Integer_not_equal_instruction<std::int16_t>,
    Integer_not_equal_instruction<std::int32_t>,
    Integer_not_equal_instruction<std::int64_t>,
    Integer_less_instruction<std::int8_t>,
    Integer_less_instruction<std::int16_t>,
    Integer_less_instruction<std::int32_t>,
    Integer_less_instruction<std::int64_t>,
    Integer_less_eq_instruction<std::int8_t>,
    Integer_less_eq_instruction<std::int16_t>,
    Integer_less_eq_instruction<std::int32_t>,
    Integer_less_eq_instruction<std::int64_t>,
    Integer_greater_instruction<std::int8_t>,
    Integer_greater_instruction<std::int16_t>,
    Integer_greater_instruction<std::int32_t>,
    Integer_greater_instruction<std::int64_t>,
    Integer_greater_eq_instruction<std::int8_t>,
    Integer_greater_eq_instruction<std::int16_t>,
    Integer_greater_eq_instruction<std::int32_t>,
    Integer_greater_eq_instruction<std::int64_t>,
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
