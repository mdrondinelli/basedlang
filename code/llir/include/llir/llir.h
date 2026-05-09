#ifndef BENSON_LLIR_LLIR_H
#define BENSON_LLIR_LLIR_H

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include <spelling/spelling.h>

namespace benson::llir
{

  class Register
  {
  public:
    Register() = default;

    explicit Register(std::int32_t id)
        : _id{id}
    {
      assert(id >= 0);
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
      assert(has_value());
      return _id;
    }

    bool operator==(Register const &) const = default;

  private:
    std::int32_t _id{-1};
  };

  struct Bit_register_type
  {
    std::int32_t bit_width;

    bool operator==(Bit_register_type const &) const = default;
  };

  using Register_type = std::variant<Bit_register_type>;

  inline Register_type bits(std::int32_t bit_width)
  {
    assert(bit_width > 0);
    return Bit_register_type{.bit_width = bit_width};
  }

  struct Immediate
  {
    Register_type type;
    std::vector<std::byte> little_endian_bytes;
  };

  using Operand = std::variant<Register, Immediate>;

  enum class Unary_opcode
  {
    negate,
  };

  enum class Binary_opcode
  {
    add,
    subtract,
    multiply,
    divide,
    modulo,
    equal,
    not_equal,
    less,
    less_eq,
    greater,
    greater_eq,
  };

  struct Copy_instruction
  {
    Register result;
    Operand value;
  };

  struct Unary_instruction
  {
    Unary_opcode opcode;
    Register result;
    Operand operand;
  };

  struct Binary_instruction
  {
    Binary_opcode opcode;
    Register result;
    Operand lhs;
    Operand rhs;
  };

  using Instruction_payload =
    std::variant<Copy_instruction, Unary_instruction, Binary_instruction>;

  struct Instruction
  {
    Instruction_payload payload;
  };

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
    std::optional<Operand> value;
  };

  using Terminator_payload =
    std::variant<Jump_terminator, Branch_terminator, Return_terminator>;

  struct Terminator
  {
    Terminator_payload payload;
  };

  struct Basic_block
  {
    std::vector<Register> parameters;
    std::vector<Instruction> instructions;
    std::optional<Terminator> terminator;

    bool has_terminator() const
    {
      return terminator.has_value();
    }
  };

  struct Function
  {
    std::vector<Register_type> register_types;
    std::vector<std::unique_ptr<Basic_block>> blocks;

    Register add_register(Register_type type)
    {
      auto const reg =
        Register{static_cast<std::int32_t>(register_types.size())};
      register_types.push_back(std::move(type));
      return reg;
    }

    Register_type const &type_of(Register reg) const
    {
      assert(reg);
      assert(*reg < static_cast<std::int32_t>(register_types.size()));
      return register_types[static_cast<std::size_t>(*reg)];
    }

    Basic_block *add_block()
    {
      blocks.push_back(std::make_unique<Basic_block>());
      return blocks.back().get();
    }
  };

  struct Translation_unit
  {
    std::vector<std::unique_ptr<Function>> functions;
    std::unordered_map<Spelling, Function *> function_table;
  };

} // namespace benson::llir

#endif // BENSON_LLIR_LLIR_H
