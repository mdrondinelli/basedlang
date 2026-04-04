#ifndef BASEDHLIR_HLIR_H
#define BASEDHLIR_HLIR_H

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "constant_value.h"
#include "operator_overload.h"
#include "type.h"

namespace basedhlir
{

  class Unary_operator_overload;
  class Binary_operator_overload;

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

  struct Int32_constant_instruction
  {
    Register result;
    std::int32_t value;
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

  struct Unary_instruction
  {
    Register result;
    Unary_operator_overload *overload;
    Operand operand;
  };

  struct Binary_instruction
  {
    Register result;
    Binary_operator_overload *overload;
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
    Int32_constant_instruction,
    Bool_constant_instruction,
    Void_constant_instruction,
    Copy_instruction,
    Unary_instruction,
    Binary_instruction,
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
    std::vector<std::unique_ptr<Unary_operator_overload>> unary_overloads;
    std::vector<std::unique_ptr<Binary_operator_overload>> binary_overloads;
  };

} // namespace basedhlir

#endif
