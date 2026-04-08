#include <cstdint>
#include <span>
#include <utility>
#include <variant>
#include <vector>

#include "kalehlir/interpret.h"

namespace kalehlir
{

  namespace
  {

    Constant_value eval_operand(
      Operand const &operand,
      std::vector<Constant_value> const &registers
    )
    {
      return std::visit(
        [&](auto const &op) -> Constant_value
        {
          using T = std::decay_t<decltype(op)>;
          if constexpr (std::is_same_v<T, Register>)
          {
            return registers[*op];
          }
          else
          {
            return op;
          }
        },
        operand
      );
    }

    template <typename T, template <typename> class Template>
    struct Is_instantiation_of : std::false_type
    {
    };

    template <template <typename> class Template, typename Arg>
    struct Is_instantiation_of<Template<Arg>, Template> : std::true_type
    {
    };

    template <typename T, template <typename> class Template>
    inline constexpr auto is_instantiation_of_v =
      Is_instantiation_of<T, Template>::value;

    template <typename InstructionT, typename Fn>
    void execute_unary_instruction(
      std::vector<Constant_value> &register_values,
      InstructionT const &inst,
      Fn &&fn
    )
    {
      auto const operand = eval_operand(inst.operand, register_values);
      register_values[*inst.result] = fn(operand);
    }

    template <typename InstructionT, typename Fn>
    void execute_binary_instruction(
      std::vector<Constant_value> &register_values,
      InstructionT const &inst,
      Fn &&fn
    )
    {
      auto const lhs = eval_operand(inst.lhs, register_values);
      auto const rhs = eval_operand(inst.rhs, register_values);
      register_values[*inst.result] = fn(lhs, rhs);
    }

    void execute_instruction(
      std::vector<Constant_value> &register_values,
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
          if constexpr (is_instantiation_of_v<T, Constant_instruction>)
          {
            register_values[*inst.result] = inst.value;
          }
          else if constexpr (std::is_same_v<T, Bool_constant_instruction>)
          {
            register_values[*inst.result] = inst.value;
          }
          else if constexpr (std::is_same_v<T, Void_constant_instruction>)
          {
            register_values[*inst.result] = Void_value{};
          }
          else if constexpr (is_instantiation_of_v<T, Negate_instruction>)
          {
            using V = typename T::value_type;
            execute_unary_instruction(
              register_values,
              inst,
              [](Constant_value operand) -> Constant_value
              {
                return static_cast<V>(-std::get<V>(operand));
              }
            );
          }
          else if constexpr (is_instantiation_of_v<T, Add_instruction>)
          {
            using V = typename T::value_type;
            execute_binary_instruction(
              register_values,
              inst,
              [](Constant_value lhs, Constant_value rhs) -> Constant_value
              {
                return static_cast<V>(std::get<V>(lhs) + std::get<V>(rhs));
              }
            );
          }
          else if constexpr (is_instantiation_of_v<T, Subtract_instruction>)
          {
            using V = typename T::value_type;
            execute_binary_instruction(
              register_values,
              inst,
              [](Constant_value lhs, Constant_value rhs) -> Constant_value
              {
                return static_cast<V>(std::get<V>(lhs) - std::get<V>(rhs));
              }
            );
          }
          else if constexpr (is_instantiation_of_v<T, Multiply_instruction>)
          {
            using V = typename T::value_type;
            execute_binary_instruction(
              register_values,
              inst,
              [](Constant_value lhs, Constant_value rhs) -> Constant_value
              {
                return static_cast<V>(std::get<V>(lhs) * std::get<V>(rhs));
              }
            );
          }
          else if constexpr (is_instantiation_of_v<T, Divide_instruction>)
          {
            using V = typename T::value_type;
            execute_binary_instruction(
              register_values,
              inst,
              [](Constant_value lhs, Constant_value rhs) -> Constant_value
              {
                return static_cast<V>(std::get<V>(lhs) / std::get<V>(rhs));
              }
            );
          }
          else if constexpr (is_instantiation_of_v<T, Modulo_instruction>)
          {
            using V = typename T::value_type;
            execute_binary_instruction(
              register_values,
              inst,
              [](Constant_value lhs, Constant_value rhs) -> Constant_value
              {
                return static_cast<V>(std::get<V>(lhs) % std::get<V>(rhs));
              }
            );
          }
          else if constexpr (is_instantiation_of_v<T, Equal_instruction>)
          {
            using V = typename T::value_type;
            execute_binary_instruction(
              register_values,
              inst,
              [](Constant_value lhs, Constant_value rhs) -> Constant_value
              {
                return std::get<V>(lhs) == std::get<V>(rhs);
              }
            );
          }
          else if constexpr (is_instantiation_of_v<T, Not_equal_instruction>)
          {
            using V = typename T::value_type;
            execute_binary_instruction(
              register_values,
              inst,
              [](Constant_value lhs, Constant_value rhs) -> Constant_value
              {
                return std::get<V>(lhs) != std::get<V>(rhs);
              }
            );
          }
          else if constexpr (is_instantiation_of_v<T, Less_instruction>)
          {
            using V = typename T::value_type;
            execute_binary_instruction(
              register_values,
              inst,
              [](Constant_value lhs, Constant_value rhs) -> Constant_value
              {
                return std::get<V>(lhs) < std::get<V>(rhs);
              }
            );
          }
          else if constexpr (is_instantiation_of_v<T, Less_eq_instruction>)
          {
            using V = typename T::value_type;
            execute_binary_instruction(
              register_values,
              inst,
              [](Constant_value lhs, Constant_value rhs) -> Constant_value
              {
                return std::get<V>(lhs) <= std::get<V>(rhs);
              }
            );
          }
          else if constexpr (is_instantiation_of_v<T, Greater_instruction>)
          {
            using V = typename T::value_type;
            execute_binary_instruction(
              register_values,
              inst,
              [](Constant_value lhs, Constant_value rhs) -> Constant_value
              {
                return std::get<V>(lhs) > std::get<V>(rhs);
              }
            );
          }
          else if constexpr (is_instantiation_of_v<T, Greater_eq_instruction>)
          {
            using V = typename T::value_type;
            execute_binary_instruction(
              register_values,
              inst,
              [](Constant_value lhs, Constant_value rhs) -> Constant_value
              {
                return std::get<V>(lhs) >= std::get<V>(rhs);
              }
            );
          }
          else if constexpr (std::is_same_v<T, Call_instruction>)
          {
            auto args = std::vector<Constant_value>{};
            args.reserve(inst.arguments.size());
            for (auto const &arg : inst.arguments)
            {
              args.push_back(eval_operand(arg, register_values));
            }
            register_values[*inst.result] = interpret(*inst.callee, args, fuel);
          }
        },
        instruction
      );
    }

  } // namespace

  Constant_value interpret(
    Function const &function,
    std::span<Constant_value const> arguments,
    std::int32_t &fuel
  )
  {
    auto registers =
      std::vector<Constant_value>(function.register_count, Void_value{});
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
          -> std::pair<Basic_block const *, std::vector<Operand> const *>
        {
          using T = std::decay_t<decltype(t)>;
          if constexpr (std::is_same_v<T, std::monostate>)
          {
            std::unreachable();
          }
          else if constexpr (std::is_same_v<T, Jump_terminator>)
          {
            return {t.target, &t.arguments};
          }
          else if constexpr (std::is_same_v<T, Branch_terminator>)
          {
            return std::get<bool>(eval_operand(t.condition, registers))
                     ? std::pair{t.then_target, &t.then_arguments}
                     : std::pair{t.else_target, &t.else_arguments};
          }
          else
          {
            return_value = eval_operand(t.value, registers);
            return {nullptr, nullptr};
          }
        },
        block->terminator
      );
      if (result.first == nullptr)
      {
        break;
      }
      for (auto i = std::size_t{}; i < result.second->size(); ++i)
      {
        registers[*result.first->parameters[i]] =
          eval_operand((*result.second)[i], registers);
      }
      block = result.first;
    }
    return return_value;
  }

  Constant_value
  interpret(Function const &function, std::span<Constant_value const> arguments)
  {
    auto fuel = std::int32_t{100000};
    return interpret(function, arguments, fuel);
  }

} // namespace kalehlir
