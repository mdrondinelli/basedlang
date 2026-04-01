#ifndef BASEDHLIR_HLIR_H
#define BASEDHLIR_HLIR_H

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "operator_overload.h"
#include "type.h"

namespace basedhlir
{

  class Unary_operator_overload;
  class Binary_operator_overload;

  struct Register
  {
    std::int32_t id;

    bool operator==(Register const &) const = default;
  };

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
    Register source;
  };

  struct Unary_instruction
  {
    Register result;
    Unary_operator_overload *overload;
    Register operand;
  };

  struct Binary_instruction
  {
    Register result;
    Binary_operator_overload *overload;
    Register lhs;
    Register rhs;
  };

  struct Function;

  struct Call_instruction
  {
    Register result;
    Function *callee;
    std::vector<Register> arguments;
  };

  struct Block;
  struct Instruction;

  struct If_instruction
  {
    struct Else_if
    {
      std::unique_ptr<Block> condition;
      std::unique_ptr<Block> body;
    };

    Register result;
    std::unique_ptr<Block> condition;
    std::unique_ptr<Block> then_body;
    std::vector<Else_if> else_ifs;
    std::unique_ptr<Block> else_body;
  };

  struct Block
  {
    Register result;
    std::vector<Instruction> instructions;
  };

  struct Return_instruction
  {
    Register value;
  };

  struct Instruction
  {
    std::variant<
      Int32_constant_instruction,
      Bool_constant_instruction,
      Void_constant_instruction,
      Copy_instruction,
      Unary_instruction,
      Binary_instruction,
      Call_instruction,
      If_instruction,
      Return_instruction
    >
      data;
  };

  struct Parameter
  {
    std::string name;
    Type *type;
  };

  struct Function
  {
    std::string name;
    Type *type;
    std::vector<Parameter> parameters;
    std::vector<Instruction> body;
    std::int32_t register_count;
  };

  struct Translation_unit
  {
    std::vector<std::unique_ptr<Function>> functions;
    std::vector<std::unique_ptr<Unary_operator_overload>> unary_overloads;
    std::vector<std::unique_ptr<Binary_operator_overload>> binary_overloads;
  };

} // namespace basedhlir

#endif
