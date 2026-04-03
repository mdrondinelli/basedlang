#include <cstdint>
#include <span>
#include <variant>
#include <vector>

#include "basedhlir/interpret.h"
#include "basedhlir/operator_overload.h"

namespace basedhlir
{

  namespace
  {

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
        },
        instruction.data
      );
    }

  } // namespace

  Constant_value interpret(
    Function const &function,
    std::span<Constant_value const> arguments,
    std::int32_t fuel
  )
  {
    auto registers = std::vector<Constant_value>(function.register_count, Void_value{});
    auto const &entry = *function.blocks[0];
    for (auto i = std::size_t{}; i < arguments.size(); ++i)
    {
      registers[*entry.parameters[i]] = arguments[i];
    }
    auto const *block = &entry;
    auto return_value = Constant_value{Void_value{}};
    for (;;)
    {
      for (auto const &inst : block->instructions)
      {
        execute_instruction(registers, inst, fuel);
      }
      auto const result = std::visit(
        [&](auto const &t)
          -> std::pair<Basic_block const *, std::vector<Register> const *>
        {
          using T = std::decay_t<decltype(t)>;
          if constexpr (std::is_same_v<T, Jump_terminator>)
          {
            return {t.target, &t.arguments};
          }
          else if constexpr (std::is_same_v<T, Branch_terminator>)
          {
            if (std::get<bool>(registers[*t.condition]))
            {
              return {t.then_target, &t.then_arguments};
            }
            return {t.else_target, &t.else_arguments};
          }
          else
          {
            if (t.value.has_value())
            {
              return_value = registers[*t.value];
            }
            return {nullptr, nullptr};
          }
        },
        block->terminator.data
      );
      if (result.first == nullptr)
      {
        break;
      }
      for (auto i = std::size_t{}; i < result.second->size(); ++i)
      {
        registers[*result.first->parameters[i]] =
          registers[*(*result.second)[i]];
      }
      block = result.first;
    }
    return return_value;
  }

} // namespace basedhlir
