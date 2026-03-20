#ifndef BASEDIR_IR_H
#define BASEDIR_IR_H

#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "type.h"

namespace basedir
{
  // program

  struct Program;
  struct Function;
  struct Basic_block;
  struct Instruction;

  struct Program
  {
    Type_pool types;
    std::vector<Function> functions;
  };

  struct Function_prototype
  {
    std::string name;
    Type *type;
  };

  struct Function_body
  {
    std::vector<Type *> register_types;
    std::vector<Basic_block> blocks;
  };

  struct Function
  {
    Function_prototype prototype;
    std::optional<Function_body> body;
  };

  struct Basic_block
  {
    std::vector<Instruction> instructions;
  };

  struct Constant_integral_value {
    Type *type;
    std::uint64_t bits;
  };

  struct Constant_function_value {
    Type *type;
    std::int64_t function;
  };

  struct Constant_value {
    std::variant<Constant_integral_value, Constant_function_value> value;
  };

  struct Load_constant_instruction {
    Constant_value value;
    std::int64_t dst_register;
  };

  struct Alloca_instruction {
    Type *type;
    std::int64_t dst_register;
  };

  struct Load_instruction { 
    std::int64_t dst_register;
    std::int64_t pointer_register;
  };

  struct Store_instruction {
    std::int64_t src_register;
    std::int64_t pointer_register;
  };

  struct Add_instruction {
    std::int64_t dst_register;
    std::array<std::int64_t, 2> src_register;
  };

  struct Sub_instruction {
    std::int64_t dst_register;
    std::array<std::int64_t, 2> src_register;
  };

  struct Compare_instruction {
    std::int64_t dst_register;
    std::array<std::int64_t, 2> src_register;
  };

  struct Jump_instruction{
    std::int64_t dst_block;
  };

  struct Branch_instruction {
    std::int64_t condition_register;
    std::int64_t then_block;
    std::int64_t else_block;
  };

  struct Instruction
  {
    std::variant<
      Load_constant_instruction,
      Alloca_instruction,
      Load_instruction,
      Store_instruction,
      Add_instruction,
      Sub_instruction,
      Compare_instruction,
      Branch_instruction
    >
      value;
  };
} // namespace basedir

#endif // BASEDIR_IR_H
