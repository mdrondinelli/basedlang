#include <cstdint>
#include <span>
#include <stdexcept>
#include <variant>
#include <vector>

#include "basedhlir/interpret.h"
#include "basedhlir/operator_overload.h"

namespace basedhlir
{

  namespace
  {

    struct Return_value
    {
      Constant_value value;
    };

    void execute_instructions(
      std::vector<Constant_value> &registers,
      std::vector<Instruction> const &instructions,
      std::int32_t &fuel
    );

    void execute_block(
      std::vector<Constant_value> &registers,
      Block const &block,
      std::int32_t &fuel
    )
    {
      execute_instructions(registers, block.instructions, fuel);
    }

    void execute_instruction(
      std::vector<Constant_value> &registers,
      Instruction const &instruction,
      std::int32_t &fuel
    )
    {
      if (fuel <= 0)
      {
        throw Fuel_exhausted_error{};
      }
      --fuel;
      std::visit(
        [&](auto const &inst)
        {
          using T = std::decay_t<decltype(inst)>;
          if constexpr (std::is_same_v<T, Int32_constant_instruction>)
          {
            registers[*inst.result] = inst.value;
          }
          else if constexpr (std::is_same_v<T, Bool_constant_instruction>)
          {
            registers[*inst.result] = inst.value;
          }
          else if constexpr (std::is_same_v<T, Void_constant_instruction>)
          {
            registers[*inst.result] = Void_value{};
          }
          else if constexpr (std::is_same_v<T, Copy_instruction>)
          {
            registers[*inst.result] = registers[*inst.source];
          }
          else if constexpr (std::is_same_v<T, Unary_instruction>)
          {
            auto const operand = registers[*inst.operand];
            registers[*inst.result] = inst.overload->evaluate(operand);
          }
          else if constexpr (std::is_same_v<T, Binary_instruction>)
          {
            auto const lhs = registers[*inst.lhs];
            auto const rhs = registers[*inst.rhs];
            registers[*inst.result] = inst.overload->evaluate(lhs, rhs);
          }
          else if constexpr (std::is_same_v<T, Call_instruction>)
          {
            auto args = std::vector<Constant_value>{};
            args.reserve(inst.arguments.size());
            for (auto const &arg : inst.arguments)
            {
              args.push_back(registers[*arg]);
            }
            registers[*inst.result] = interpret(*inst.callee, args, fuel);
          }
          else if constexpr (std::is_same_v<T, If_instruction>)
          {
            execute_block(registers, *inst.condition, fuel);
            if (std::get<bool>(registers[*inst.condition->result]))
            {
              execute_block(registers, *inst.then_body, fuel);
              registers[*inst.result] = registers[*inst.then_body->result];
            }
            else
            {
              auto matched = false;
              for (auto const &else_if : inst.else_ifs)
              {
                execute_block(registers, *else_if.condition, fuel);
                if (std::get<bool>(registers[*else_if.condition->result]))
                {
                  execute_block(registers, *else_if.body, fuel);
                  registers[*inst.result] = registers[*else_if.body->result];
                  matched = true;
                  break;
                }
              }
              if (!matched)
              {
                if (inst.else_body)
                {
                  execute_block(registers, *inst.else_body, fuel);
                  registers[*inst.result] = registers[*inst.else_body->result];
                }
                else
                {
                  registers[*inst.result] = Void_value{};
                }
              }
            }
          }
          else if constexpr (std::is_same_v<T, Return_instruction>)
          {
            throw Return_value{.value = registers[*inst.value]};
          }
        },
        instruction.data
      );
    }

    void execute_instructions(
      std::vector<Constant_value> &registers,
      std::vector<Instruction> const &instructions,
      std::int32_t &fuel
    )
    {
      for (auto const &inst : instructions)
      {
        execute_instruction(registers, inst, fuel);
      }
    }

  } // namespace

  Constant_value interpret(
    Function const &function,
    std::span<Constant_value const> arguments,
    std::int32_t fuel
  )
  {
    auto registers =
      std::vector<Constant_value>(function.register_count, Void_value{});
    for (auto i = std::size_t{}; i < arguments.size(); ++i)
    {
      registers[i] = arguments[i];
    }
    try
    {
      execute_instructions(registers, function.body, fuel);
      return Void_value{};
    }
    catch (Return_value &rv)
    {
      return std::move(rv.value);
    }
  }

} // namespace basedhlir
