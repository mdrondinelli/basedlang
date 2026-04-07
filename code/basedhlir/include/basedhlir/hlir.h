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
  struct Constant_instruction
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

  template <typename T>
  struct Negate_instruction
  {
    using value_type = T;
    Register result;
    Operand operand;
  };

  template <typename T>
  struct Add_instruction
  {
    using value_type = T;
    Register result;
    Operand lhs;
    Operand rhs;
  };

  template <typename T>
  struct Subtract_instruction
  {
    using value_type = T;
    Register result;
    Operand lhs;
    Operand rhs;
  };

  template <typename T>
  struct Multiply_instruction
  {
    using value_type = T;
    Register result;
    Operand lhs;
    Operand rhs;
  };

  template <typename T>
  struct Divide_instruction
  {
    using value_type = T;
    Register result;
    Operand lhs;
    Operand rhs;
  };

  template <typename T>
  struct Modulo_instruction
  {
    using value_type = T;
    Register result;
    Operand lhs;
    Operand rhs;
  };

  template <typename T>
  struct Equal_instruction
  {
    using value_type = T;
    Register result;
    Operand lhs;
    Operand rhs;
  };

  template <typename T>
  struct Not_equal_instruction
  {
    using value_type = T;
    Register result;
    Operand lhs;
    Operand rhs;
  };

  template <typename T>
  struct Less_instruction
  {
    using value_type = T;
    Register result;
    Operand lhs;
    Operand rhs;
  };

  template <typename T>
  struct Less_eq_instruction
  {
    using value_type = T;
    Register result;
    Operand lhs;
    Operand rhs;
  };

  template <typename T>
  struct Greater_instruction
  {
    using value_type = T;
    Register result;
    Operand lhs;
    Operand rhs;
  };

  template <typename T>
  struct Greater_eq_instruction
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
    Constant_instruction<std::int8_t>,
    Constant_instruction<std::int16_t>,
    Constant_instruction<std::int32_t>,
    Constant_instruction<std::int64_t>,
    Constant_instruction<float>,
    Constant_instruction<double>,
    Bool_constant_instruction,
    Void_constant_instruction,
    Negate_instruction<std::int8_t>,
    Negate_instruction<std::int16_t>,
    Negate_instruction<std::int32_t>,
    Negate_instruction<std::int64_t>,
    Negate_instruction<float>,
    Negate_instruction<double>,
    Add_instruction<std::int8_t>,
    Add_instruction<std::int16_t>,
    Add_instruction<std::int32_t>,
    Add_instruction<std::int64_t>,
    Add_instruction<float>,
    Add_instruction<double>,
    Subtract_instruction<std::int8_t>,
    Subtract_instruction<std::int16_t>,
    Subtract_instruction<std::int32_t>,
    Subtract_instruction<std::int64_t>,
    Subtract_instruction<float>,
    Subtract_instruction<double>,
    Multiply_instruction<std::int8_t>,
    Multiply_instruction<std::int16_t>,
    Multiply_instruction<std::int32_t>,
    Multiply_instruction<std::int64_t>,
    Multiply_instruction<float>,
    Multiply_instruction<double>,
    Divide_instruction<std::int8_t>,
    Divide_instruction<std::int16_t>,
    Divide_instruction<std::int32_t>,
    Divide_instruction<std::int64_t>,
    Divide_instruction<float>,
    Divide_instruction<double>,
    Modulo_instruction<std::int8_t>,
    Modulo_instruction<std::int16_t>,
    Modulo_instruction<std::int32_t>,
    Modulo_instruction<std::int64_t>,
    Equal_instruction<std::int8_t>,
    Equal_instruction<std::int16_t>,
    Equal_instruction<std::int32_t>,
    Equal_instruction<std::int64_t>,
    Equal_instruction<float>,
    Equal_instruction<double>,
    Not_equal_instruction<std::int8_t>,
    Not_equal_instruction<std::int16_t>,
    Not_equal_instruction<std::int32_t>,
    Not_equal_instruction<std::int64_t>,
    Not_equal_instruction<float>,
    Not_equal_instruction<double>,
    Less_instruction<std::int8_t>,
    Less_instruction<std::int16_t>,
    Less_instruction<std::int32_t>,
    Less_instruction<std::int64_t>,
    Less_instruction<float>,
    Less_instruction<double>,
    Less_eq_instruction<std::int8_t>,
    Less_eq_instruction<std::int16_t>,
    Less_eq_instruction<std::int32_t>,
    Less_eq_instruction<std::int64_t>,
    Less_eq_instruction<float>,
    Less_eq_instruction<double>,
    Greater_instruction<std::int8_t>,
    Greater_instruction<std::int16_t>,
    Greater_instruction<std::int32_t>,
    Greater_instruction<std::int64_t>,
    Greater_instruction<float>,
    Greater_instruction<double>,
    Greater_eq_instruction<std::int8_t>,
    Greater_eq_instruction<std::int16_t>,
    Greater_eq_instruction<std::int32_t>,
    Greater_eq_instruction<std::int64_t>,
    Greater_eq_instruction<float>,
    Greater_eq_instruction<double>,
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
