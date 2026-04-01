#include <stdexcept>
#include <variant>
#include <vector>

#include "basedhlir/interpret.h"
#include "basedhlir/operator_overload.h"

namespace basedhlir
{

  struct Return_value
  {
    Constant_value value;
  };

  struct Fuel_exhausted: std::runtime_error
  {
    Fuel_exhausted()
        : std::runtime_error{"compile-time evaluation ran out of fuel"}
    {
    }
  };

  static void execute_instructions(
    std::vector<Constant_value> &registers,
    std::vector<Instruction> const &instructions,
    int &fuel
  );

  static void execute_block(
    std::vector<Constant_value> &registers,
    Block const &block,
    int &fuel
  )
  {
    execute_instructions(registers, block.instructions, fuel);
  }

  static void execute_instruction(
    std::vector<Constant_value> &registers,
    Instruction const &instruction,
    int &fuel
  )
  {
    if (fuel <= 0)
    {
      throw Fuel_exhausted{};
    }
    --fuel;
    std::visit(
      [&](auto const &inst)
      {
        using T = std::decay_t<decltype(inst)>;
        if constexpr (std::is_same_v<T, Int32_constant_instruction>)
        {
          registers[inst.result.id] = inst.value;
        }
        else if constexpr (std::is_same_v<T, Bool_constant_instruction>)
        {
          registers[inst.result.id] = inst.value;
        }
        else if constexpr (std::is_same_v<T, Void_constant_instruction>)
        {
          registers[inst.result.id] = Void_value{};
        }
        else if constexpr (std::is_same_v<T, Copy_instruction>)
        {
          registers[inst.result.id] = registers[inst.source.id];
        }
        else if constexpr (std::is_same_v<T, Unary_instruction>)
        {
          auto const operand = registers[inst.operand.id];
          registers[inst.result.id] = inst.overload->evaluate(operand);
        }
        else if constexpr (std::is_same_v<T, Binary_instruction>)
        {
          auto const lhs = registers[inst.lhs.id];
          auto const rhs = registers[inst.rhs.id];
          registers[inst.result.id] = inst.overload->evaluate(lhs, rhs);
        }
        else if constexpr (std::is_same_v<T, Call_instruction>)
        {
          auto args = std::vector<Constant_value>{};
          args.reserve(inst.arguments.size());
          for (auto const &arg : inst.arguments)
          {
            args.push_back(registers[arg.id]);
          }
          registers[inst.result.id] = interpret(*inst.callee, args, fuel);
        }
        else if constexpr (std::is_same_v<T, If_instruction>)
        {
          execute_block(registers, *inst.condition, fuel);
          if (std::get<bool>(registers[inst.condition->result.id]))
          {
            execute_block(registers, *inst.then_body, fuel);
            registers[inst.result.id] = registers[inst.then_body->result.id];
          }
          else
          {
            auto matched = false;
            for (auto const &ei : inst.else_ifs)
            {
              execute_block(registers, *ei.condition, fuel);
              if (std::get<bool>(registers[ei.condition->result.id]))
              {
                execute_block(registers, *ei.body, fuel);
                registers[inst.result.id] = registers[ei.body->result.id];
                matched = true;
                break;
              }
            }
            if (!matched)
            {
              if (inst.else_body)
              {
                execute_block(registers, *inst.else_body, fuel);
                registers[inst.result.id] =
                  registers[inst.else_body->result.id];
              }
              else
              {
                registers[inst.result.id] = Void_value{};
              }
            }
          }
        }
        else if constexpr (std::is_same_v<T, Return_instruction>)
        {
          throw Return_value{.value = registers[inst.value.id]};
        }
      },
      instruction.data
    );
  }

  static void execute_instructions(
    std::vector<Constant_value> &registers,
    std::vector<Instruction> const &instructions,
    int &fuel
  )
  {
    for (auto const &inst : instructions)
    {
      execute_instruction(registers, inst, fuel);
    }
  }

  Constant_value interpret(
    Function const &function,
    std::vector<Constant_value> const &arguments,
    int fuel
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
