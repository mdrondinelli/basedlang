#include "hlir2llir/lower.h"

#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <optional>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace benson::hlir2llir
{

  namespace
  {

    template <typename T, template <typename> class Template>
    struct Is_instantiation_of: std::false_type
    {
    };

    template <template <typename> class Template, typename Arg>
    struct Is_instantiation_of<Template<Arg>, Template>: std::true_type
    {
    };

    template <typename T, template <typename> class Template>
    inline constexpr auto is_instantiation_of_v =
      Is_instantiation_of<T, Template>::value;

    template <typename T>
    llir::Immediate immediate(T value)
    {
      auto bytes = std::vector<std::byte>(sizeof(T));
      std::memcpy(bytes.data(), &value, sizeof(T));
      return llir::Immediate{
        .type = llir::bits(static_cast<std::int32_t>(sizeof(T) * 8)),
        .little_endian_bytes = std::move(bytes),
      };
    }

    llir::Immediate bool_immediate(bool value)
    {
      return llir::Immediate{
        .type = llir::bits(1),
        .little_endian_bytes = {value ? std::byte{1} : std::byte{0}},
      };
    }

    llir::Register_type type_of(hlir::Type const *type)
    {
      return std::visit(
        [](auto const &t) -> llir::Register_type
        {
          using T = std::decay_t<decltype(t)>;
          if constexpr (std::is_same_v<T, hlir::Int8_type>)
          {
            return llir::bits(8);
          }
          else if constexpr (std::is_same_v<T, hlir::Int16_type>)
          {
            return llir::bits(16);
          }
          else if constexpr (std::is_same_v<T, hlir::Int32_type> ||
                             std::is_same_v<T, hlir::Float32_type>)
          {
            return llir::bits(32);
          }
          else if constexpr (std::is_same_v<T, hlir::Int64_type> ||
                             std::is_same_v<T, hlir::Float64_type>)
          {
            return llir::bits(64);
          }
          else if constexpr (std::is_same_v<T, hlir::Bool_type>)
          {
            return llir::bits(1);
          }
          else
          {
            throw std::runtime_error{"unsupported HLIR type for LLIR lowering"};
          }
        },
        type->data
      );
    }

    std::optional<llir::Immediate>
    immediate_of(hlir::Constant_value const &value)
    {
      return std::visit(
        [](auto const &v) -> std::optional<llir::Immediate>
        {
          using T = std::decay_t<decltype(v)>;
          if constexpr (std::is_same_v<T, bool>)
          {
            return bool_immediate(v);
          }
          else if constexpr (std::is_same_v<T, std::int8_t> ||
                             std::is_same_v<T, std::int16_t> ||
                             std::is_same_v<T, std::int32_t> ||
                             std::is_same_v<T, std::int64_t> ||
                             std::is_same_v<T, float> ||
                             std::is_same_v<T, double>)
          {
            return immediate(v);
          }
          else
          {
            return std::nullopt;
          }
        },
        value
      );
    }

    llir::Binary_opcode
    binary_opcode_of(hlir::Instruction_payload const &payload)
    {
      return std::visit(
        [](auto const &inst) -> llir::Binary_opcode
        {
          using T = std::decay_t<decltype(inst)>;
          if constexpr (is_instantiation_of_v<T, hlir::Add_instruction>)
          {
            return llir::Binary_opcode::add;
          }
          else if constexpr (is_instantiation_of_v<
                               T,
                               hlir::Subtract_instruction
                             >)
          {
            return llir::Binary_opcode::subtract;
          }
          else if constexpr (is_instantiation_of_v<
                               T,
                               hlir::Multiply_instruction
                             >)
          {
            return llir::Binary_opcode::multiply;
          }
          else if constexpr (is_instantiation_of_v<T, hlir::Divide_instruction>)
          {
            return llir::Binary_opcode::divide;
          }
          else if constexpr (is_instantiation_of_v<T, hlir::Modulo_instruction>)
          {
            return llir::Binary_opcode::modulo;
          }
          else if constexpr (is_instantiation_of_v<T, hlir::Equal_instruction>)
          {
            return llir::Binary_opcode::equal;
          }
          else if constexpr (is_instantiation_of_v<
                               T,
                               hlir::Not_equal_instruction
                             >)
          {
            return llir::Binary_opcode::not_equal;
          }
          else if constexpr (is_instantiation_of_v<T, hlir::Less_instruction>)
          {
            return llir::Binary_opcode::less;
          }
          else if constexpr (is_instantiation_of_v<
                               T,
                               hlir::Less_eq_instruction
                             >)
          {
            return llir::Binary_opcode::less_eq;
          }
          else if constexpr (is_instantiation_of_v<
                               T,
                               hlir::Greater_instruction
                             >)
          {
            return llir::Binary_opcode::greater;
          }
          else if constexpr (is_instantiation_of_v<
                               T,
                               hlir::Greater_eq_instruction
                             >)
          {
            return llir::Binary_opcode::greater_eq;
          }
          else
          {
            std::unreachable();
          }
        },
        payload
      );
    }

    struct Function_context
    {
      hlir::Function const *source;
      llir::Function *target;
      std::vector<std::optional<llir::Register>> registers;
      std::unordered_map<hlir::Basic_block const *, llir::Basic_block *> blocks;
    };

    class Lowerer
    {
    public:
      explicit Lowerer(hlir::Translation_unit const *source)
          : _source{source}
      {
      }

      llir::Translation_unit lower()
      {
        declare_functions();
        for (auto const &function : _source->functions)
        {
          lower_function(*function);
        }
        return std::move(_target);
      }

    private:
      void declare_functions()
      {
        for (auto const &function : _source->functions)
        {
          auto lowered = std::make_unique<llir::Function>();
          auto const lowered_ptr = lowered.get();
          _functions[function.get()] = lowered_ptr;
          _target.functions.push_back(std::move(lowered));
        }
        for (auto const &[name, function] : _source->function_table)
        {
          _target.function_table[name] = _functions.at(function);
        }
      }

      void lower_function(hlir::Function const &function)
      {
        auto ctx = Function_context{
          .source = &function,
          .target = _functions.at(&function),
          .registers =
            std::vector<std::optional<llir::Register>>(function.register_count),
          .blocks = {},
        };
        for (auto const &block : function.blocks)
        {
          ctx.blocks[block.get()] = ctx.target->add_block();
        }
        seed_entry_parameters(&ctx);
        for (auto const &block : function.blocks)
        {
          lower_block(&ctx, *block, ctx.blocks.at(block.get()));
        }
      }

      void seed_entry_parameters(Function_context *ctx)
      {
        auto const &function_type =
          std::get<hlir::Function_type>(ctx->source->type->data);
        auto const entry = ctx->source->blocks.front().get();
        auto const lowered_entry = ctx->blocks.at(entry);
        for (auto i = std::size_t{}; i < entry->parameters.size(); ++i)
        {
          auto const reg = ensure_register(
            ctx,
            entry->parameters[i],
            type_of(function_type.parameter_types[i])
          );
          lowered_entry->parameters.push_back(reg);
        }
      }

      void lower_block(
        Function_context *ctx,
        hlir::Basic_block const &block,
        llir::Basic_block *lowered
      )
      {
        for (auto const &inst : block.instructions)
        {
          lower_instruction(ctx, *lowered, inst.payload);
        }
        assert(block.terminator.has_value());
        lowered->terminator = llir::Terminator{
          .payload = lower_terminator(ctx, block.terminator->payload),
        };
      }

      llir::Register ensure_register(
        Function_context *ctx,
        hlir::Register source,
        llir::Register_type type
      )
      {
        assert(source);
        auto const index = static_cast<std::size_t>(*source);
        if (!ctx->registers[index])
        {
          ctx->registers[index] = ctx->target->add_register(std::move(type));
        }
        return *ctx->registers[index];
      }

      llir::Register
      lookup_register(Function_context const *ctx, hlir::Register source) const
      {
        assert(source);
        auto const index = static_cast<std::size_t>(*source);
        assert(ctx->registers[index].has_value());
        return *ctx->registers[index];
      }

      llir::Register_type
      register_type(Function_context const *ctx, hlir::Register source) const
      {
        return ctx->target->type_of(lookup_register(ctx, source));
      }

      std::optional<llir::Operand> lower_operand(
        Function_context const *ctx,
        hlir::Operand const &operand
      ) const
      {
        return std::visit(
          [&](auto const &value) -> std::optional<llir::Operand>
          {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, hlir::Register>)
            {
              return lookup_register(ctx, value);
            }
            else
            {
              auto imm = immediate_of(value);
              if (!imm)
              {
                return std::nullopt;
              }
              return std::move(*imm);
            }
          },
          operand
        );
      }

      llir::Register_type operand_type(
        Function_context const *ctx,
        hlir::Operand const &operand
      ) const
      {
        return std::visit(
          [&](auto const &value) -> llir::Register_type
          {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, hlir::Register>)
            {
              return register_type(ctx, value);
            }
            else
            {
              auto imm = immediate_of(value);
              if (!imm)
              {
                throw std::runtime_error{
                  "unsupported HLIR constant for LLIR lowering"
                };
              }
              return imm->type;
            }
          },
          operand
        );
      }

      void lower_instruction(
        Function_context *ctx,
        llir::Basic_block &block,
        hlir::Instruction_payload const &payload
      )
      {
        std::visit(
          [&](auto const &inst)
          {
            using T = std::decay_t<decltype(inst)>;
            if constexpr (is_instantiation_of_v<T, hlir::Constant_instruction>)
            {
              auto const result = ensure_register(
                ctx,
                inst.result,
                llir::bits(sizeof(typename T::value_type) * 8)
              );
              block.instructions.push_back(
                llir::Instruction{llir::Copy_instruction{
                  .result = result,
                  .value = immediate(inst.value),
                }}
              );
            }
            else if constexpr (std::
                                 is_same_v<T, hlir::Bool_constant_instruction>)
            {
              auto const result =
                ensure_register(ctx, inst.result, llir::bits(1));
              block.instructions.push_back(
                llir::Instruction{llir::Copy_instruction{
                  .result = result,
                  .value = bool_immediate(inst.value),
                }}
              );
            }
            else if constexpr (std::
                                 is_same_v<T, hlir::Void_constant_instruction>)
            {
              // Void has no LLIR value.
            }
            else if constexpr (is_instantiation_of_v<
                                 T,
                                 hlir::Negate_instruction
                               >)
            {
              auto const result = ensure_register(
                ctx,
                inst.result,
                llir::bits(sizeof(typename T::value_type) * 8)
              );
              auto operand = lower_operand(ctx, inst.operand);
              assert(operand.has_value());
              block.instructions.push_back(
                llir::Instruction{llir::Unary_instruction{
                  .opcode = llir::Unary_opcode::negate,
                  .result = result,
                  .operand = std::move(*operand),
                }}
              );
            }
            else if constexpr (is_instantiation_of_v<
                                 T,
                                 hlir::Add_instruction
                               > ||
                               is_instantiation_of_v<
                                 T,
                                 hlir::Subtract_instruction
                               > ||
                               is_instantiation_of_v<
                                 T,
                                 hlir::Multiply_instruction
                               > ||
                               is_instantiation_of_v<
                                 T,
                                 hlir::Divide_instruction
                               > ||
                               is_instantiation_of_v<
                                 T,
                                 hlir::Modulo_instruction
                               > ||
                               is_instantiation_of_v<
                                 T,
                                 hlir::Equal_instruction
                               > ||
                               is_instantiation_of_v<
                                 T,
                                 hlir::Not_equal_instruction
                               > ||
                               is_instantiation_of_v<
                                 T,
                                 hlir::Less_instruction
                               > ||
                               is_instantiation_of_v<
                                 T,
                                 hlir::Less_eq_instruction
                               > ||
                               is_instantiation_of_v<
                                 T,
                                 hlir::Greater_instruction
                               > ||
                               is_instantiation_of_v<
                                 T,
                                 hlir::Greater_eq_instruction
                               >)
            {
              auto const is_compare =
                is_instantiation_of_v<T, hlir::Equal_instruction> ||
                is_instantiation_of_v<T, hlir::Not_equal_instruction> ||
                is_instantiation_of_v<T, hlir::Less_instruction> ||
                is_instantiation_of_v<T, hlir::Less_eq_instruction> ||
                is_instantiation_of_v<T, hlir::Greater_instruction> ||
                is_instantiation_of_v<T, hlir::Greater_eq_instruction>;
              auto const result = ensure_register(
                ctx,
                inst.result,
                is_compare ? llir::bits(1)
                           : llir::bits(sizeof(typename T::value_type) * 8)
              );
              auto lhs = lower_operand(ctx, inst.lhs);
              auto rhs = lower_operand(ctx, inst.rhs);
              assert(lhs.has_value());
              assert(rhs.has_value());
              block.instructions.push_back(
                llir::Instruction{llir::Binary_instruction{
                  .opcode = binary_opcode_of(payload),
                  .result = result,
                  .lhs = std::move(*lhs),
                  .rhs = std::move(*rhs),
                }}
              );
            }
            else if constexpr (std::is_same_v<T, hlir::Call_instruction>)
            {
              auto arguments = std::vector<llir::Operand>{};
              arguments.reserve(inst.arguments.size());
              for (auto const &arg : inst.arguments)
              {
                auto lowered_arg = lower_operand(ctx, arg);
                assert(lowered_arg.has_value());
                arguments.push_back(std::move(*lowered_arg));
              }
              auto const return_type =
                std::get<hlir::Function_type>(inst.callee->type->data)
                  .return_type;
              auto result = std::optional<llir::Register>{};
              if (!std::holds_alternative<hlir::Void_type>(return_type->data))
              {
                result =
                  ensure_register(ctx, inst.result, type_of(return_type));
              }
              block.instructions.push_back(
                llir::Instruction{llir::Call_instruction{
                  .result = result,
                  .callee = _functions.at(inst.callee),
                  .arguments = std::move(arguments),
                }}
              );
            }
          },
          payload
        );
      }

      std::vector<llir::Operand> lower_block_arguments(
        Function_context *ctx,
        hlir::Basic_block const *target,
        std::span<hlir::Operand const> arguments
      )
      {
        auto lowered_arguments = std::vector<llir::Operand>{};
        auto const lowered_target = ctx->blocks.at(target);
        lowered_arguments.reserve(arguments.size());
        for (auto i = std::size_t{}; i < arguments.size(); ++i)
        {
          auto const param = target->parameters[i];
          auto const type = operand_type(ctx, arguments[i]);
          auto const lowered_param = ensure_register(ctx, param, type);
          if (lowered_target->parameters.size() <= i)
          {
            lowered_target->parameters.push_back(lowered_param);
          }
          auto lowered_argument = lower_operand(ctx, arguments[i]);
          assert(lowered_argument.has_value());
          lowered_arguments.push_back(std::move(*lowered_argument));
        }
        return lowered_arguments;
      }

      llir::Terminator_payload lower_terminator(
        Function_context *ctx,
        hlir::Terminator_payload const &payload
      )
      {
        return std::visit(
          [&](auto const &term) -> llir::Terminator_payload
          {
            using T = std::decay_t<decltype(term)>;
            if constexpr (std::is_same_v<T, hlir::Jump_terminator>)
            {
              return llir::Jump_terminator{
                .target = ctx->blocks.at(term.target),
                .arguments = lower_block_arguments(
                  ctx,
                  term.target,
                  std::span<hlir::Operand const>{term.arguments}
                ),
              };
            }
            else if constexpr (std::is_same_v<T, hlir::Branch_terminator>)
            {
              auto condition = lower_operand(ctx, term.condition);
              assert(condition.has_value());
              return llir::Branch_terminator{
                .condition = std::move(*condition),
                .then_target = ctx->blocks.at(term.then_target),
                .then_arguments = lower_block_arguments(
                  ctx,
                  term.then_target,
                  std::span<hlir::Operand const>{term.then_arguments}
                ),
                .else_target = ctx->blocks.at(term.else_target),
                .else_arguments = lower_block_arguments(
                  ctx,
                  term.else_target,
                  std::span<hlir::Operand const>{term.else_arguments}
                ),
              };
            }
            else
            {
              auto value = lower_operand(ctx, term.value);
              return llir::Return_terminator{.value = std::move(value)};
            }
          },
          payload
        );
      }

      hlir::Translation_unit const *_source;
      llir::Translation_unit _target;
      std::unordered_map<hlir::Function const *, llir::Function *> _functions;
    };

  } // namespace

  llir::Translation_unit lower(hlir::Translation_unit const &tu)
  {
    return Lowerer{&tu}.lower();
  }

} // namespace benson::hlir2llir
