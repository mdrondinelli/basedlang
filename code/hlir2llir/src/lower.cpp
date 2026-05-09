#include "hlir2llir/lower.h"

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
    inline constexpr auto is_unary_instruction_v =
      is_instantiation_of_v<T, hlir::Negate_instruction>;

    template <typename T>
    inline constexpr auto is_binary_instruction_v =
      is_instantiation_of_v<T, hlir::Add_instruction> ||
      is_instantiation_of_v<T, hlir::Subtract_instruction> ||
      is_instantiation_of_v<T, hlir::Multiply_instruction> ||
      is_instantiation_of_v<T, hlir::Divide_instruction> ||
      is_instantiation_of_v<T, hlir::Modulo_instruction> ||
      is_instantiation_of_v<T, hlir::Equal_instruction> ||
      is_instantiation_of_v<T, hlir::Not_equal_instruction> ||
      is_instantiation_of_v<T, hlir::Less_instruction> ||
      is_instantiation_of_v<T, hlir::Less_eq_instruction> ||
      is_instantiation_of_v<T, hlir::Greater_instruction> ||
      is_instantiation_of_v<T, hlir::Greater_eq_instruction>;

    template <typename T>
    inline constexpr auto is_compare_instruction_v =
      is_instantiation_of_v<T, hlir::Equal_instruction> ||
      is_instantiation_of_v<T, hlir::Not_equal_instruction> ||
      is_instantiation_of_v<T, hlir::Less_instruction> ||
      is_instantiation_of_v<T, hlir::Less_eq_instruction> ||
      is_instantiation_of_v<T, hlir::Greater_instruction> ||
      is_instantiation_of_v<T, hlir::Greater_eq_instruction>;

    template <typename T>
    llir::Register_type bits_for_value()
    {
      return llir::bits(static_cast<std::int32_t>(sizeof(T) * 8));
    }

    template <typename T>
    llir::Immediate immediate(T value)
    {
      auto bytes = std::vector<std::byte>(sizeof(T));
      std::memcpy(bytes.data(), &value, sizeof(T));
      return llir::Immediate{
        .type = bits_for_value<T>(),
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

    template <typename InstructionT>
    llir::Binary_opcode binary_opcode()
    {
      if constexpr (is_instantiation_of_v<InstructionT, hlir::Add_instruction>)
      {
        return llir::Binary_opcode::add;
      }
      else if constexpr (is_instantiation_of_v<
                           InstructionT,
                           hlir::Subtract_instruction
                         >)
      {
        return llir::Binary_opcode::subtract;
      }
      else if constexpr (is_instantiation_of_v<
                           InstructionT,
                           hlir::Multiply_instruction
                         >)
      {
        return llir::Binary_opcode::multiply;
      }
      else if constexpr (is_instantiation_of_v<
                           InstructionT,
                           hlir::Divide_instruction
                         >)
      {
        return llir::Binary_opcode::divide;
      }
      else if constexpr (is_instantiation_of_v<
                           InstructionT,
                           hlir::Modulo_instruction
                         >)
      {
        return llir::Binary_opcode::modulo;
      }
      else if constexpr (is_instantiation_of_v<
                           InstructionT,
                           hlir::Equal_instruction
                         >)
      {
        return llir::Binary_opcode::equal;
      }
      else if constexpr (is_instantiation_of_v<
                           InstructionT,
                           hlir::Not_equal_instruction
                         >)
      {
        return llir::Binary_opcode::not_equal;
      }
      else if constexpr (is_instantiation_of_v<
                           InstructionT,
                           hlir::Less_instruction
                         >)
      {
        return llir::Binary_opcode::less;
      }
      else if constexpr (is_instantiation_of_v<
                           InstructionT,
                           hlir::Less_eq_instruction
                         >)
      {
        return llir::Binary_opcode::less_eq;
      }
      else if constexpr (is_instantiation_of_v<
                           InstructionT,
                           hlir::Greater_instruction
                         >)
      {
        return llir::Binary_opcode::greater;
      }
      else if constexpr (is_instantiation_of_v<
                           InstructionT,
                           hlir::Greater_eq_instruction
                         >)
      {
        return llir::Binary_opcode::greater_eq;
      }
      else
      {
        std::unreachable();
      }
    }

    struct Lowered_function
    {
      hlir::Function const *source;
      llir::Function *target;
      std::vector<std::optional<llir::Register>> registers;
      std::unordered_map<hlir::Basic_block const *, llir::Basic_block *> blocks;
    };

    class Function_lowerer
    {
    public:
      Function_lowerer(
        hlir::Function const *source,
        llir::Function *target,
        std::unordered_map<hlir::Function const *, llir::Function *> const
          *functions
      )
          : _function{
              .source = source,
              .target = target,
              .registers = std::vector<std::optional<llir::Register>>(
                source->register_count
              ),
              .blocks = {},
            },
            _functions{functions}
      {
      }

      void lower()
      {
        create_blocks();
        seed_entry_parameters();
        lower_blocks();
      }

    private:
      void create_blocks()
      {
        for (auto const &block : _function.source->blocks)
        {
          _function.blocks[block.get()] = _function.target->add_block();
        }
      }

      void seed_entry_parameters()
      {
        auto const &function_type =
          std::get<hlir::Function_type>(_function.source->type->data);
        auto const entry = _function.source->blocks.front().get();
        auto const lowered_entry = lowered_block(entry);
        for (auto i = std::size_t{}; i < entry->parameters.size(); ++i)
        {
          lowered_entry->parameters.push_back(ensure_register(
            entry->parameters[i],
            type_of(function_type.parameter_types[i])
          ));
        }
      }

      void lower_blocks()
      {
        for (auto const &block : _function.source->blocks)
        {
          lower_block(*block, lowered_block(block.get()));
        }
      }

      void lower_block(
        hlir::Basic_block const &source,
        llir::Basic_block *target
      )
      {
        for (auto const &inst : source.instructions)
        {
          lower_instruction(*target, inst.payload);
        }
        assert(source.terminator.has_value());
        target->terminator = llir::Terminator{
          .payload = lower_terminator(source.terminator->payload),
        };
      }

      llir::Basic_block *lowered_block(hlir::Basic_block const *source) const
      {
        return _function.blocks.at(source);
      }

      llir::Function *lowered_function(hlir::Function const *source) const
      {
        return _functions->at(source);
      }

      llir::Register
      ensure_register(hlir::Register source, llir::Register_type type)
      {
        assert(source);
        auto const index = static_cast<std::size_t>(*source);
        if (!_function.registers[index])
        {
          _function.registers[index] =
            _function.target->add_register(std::move(type));
        }
        return *_function.registers[index];
      }

      llir::Register lookup_register(hlir::Register source) const
      {
        assert(source);
        auto const index = static_cast<std::size_t>(*source);
        assert(_function.registers[index].has_value());
        return *_function.registers[index];
      }

      llir::Register_type register_type(hlir::Register source) const
      {
        return _function.target->type_of(lookup_register(source));
      }

      std::optional<llir::Operand>
      lower_operand(hlir::Operand const &operand) const
      {
        return std::visit(
          [&](auto const &value) -> std::optional<llir::Operand>
          {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, hlir::Register>)
            {
              return lookup_register(value);
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

      llir::Register_type operand_type(hlir::Operand const &operand) const
      {
        return std::visit(
          [&](auto const &value) -> llir::Register_type
          {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, hlir::Register>)
            {
              return register_type(value);
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
        llir::Basic_block &block,
        hlir::Instruction_payload const &payload
      )
      {
        std::visit(
          [&](auto const &inst)
          {
            lower_instruction_variant(block, inst);
          },
          payload
        );
      }

      template <typename InstructionT>
      void lower_instruction_variant(
        llir::Basic_block &block,
        InstructionT const &inst
      )
      {
        if constexpr (is_instantiation_of_v<
                        InstructionT,
                        hlir::Constant_instruction
                      >)
        {
          lower_constant_instruction(block, inst);
        }
        else if constexpr (std::is_same_v<
                             InstructionT,
                             hlir::Bool_constant_instruction
                           >)
        {
          lower_bool_constant_instruction(block, inst);
        }
        else if constexpr (std::is_same_v<
                             InstructionT,
                             hlir::Void_constant_instruction
                           >)
        {
          lower_void_constant_instruction(inst);
        }
        else if constexpr (is_unary_instruction_v<InstructionT>)
        {
          lower_unary_instruction(block, inst);
        }
        else if constexpr (is_binary_instruction_v<InstructionT>)
        {
          lower_binary_instruction(block, inst);
        }
        else if constexpr (std::is_same_v<InstructionT, hlir::Call_instruction>)
        {
          lower_call_instruction(block, inst);
        }
      }

      template <typename InstructionT>
      void lower_constant_instruction(
        llir::Basic_block &block,
        InstructionT const &inst
      )
      {
        auto const result = ensure_register(
          inst.result,
          bits_for_value<typename InstructionT::value_type>()
        );
        block.instructions.push_back(
          llir::Instruction{llir::Copy_instruction{
            .result = result,
            .value = immediate(inst.value),
          }}
        );
      }

      void lower_bool_constant_instruction(
        llir::Basic_block &block,
        hlir::Bool_constant_instruction const &inst
      )
      {
        auto const result = ensure_register(inst.result, llir::bits(1));
        block.instructions.push_back(
          llir::Instruction{llir::Copy_instruction{
            .result = result,
            .value = bool_immediate(inst.value),
          }}
        );
      }

      void lower_void_constant_instruction(
        hlir::Void_constant_instruction const &
      )
      {
        // Void has no LLIR value.
      }

      template <typename InstructionT>
      void lower_unary_instruction(
        llir::Basic_block &block,
        InstructionT const &inst
      )
      {
        auto const result = ensure_register(
          inst.result,
          bits_for_value<typename InstructionT::value_type>()
        );
        auto operand = lower_operand(inst.operand);
        assert(operand.has_value());
        block.instructions.push_back(
          llir::Instruction{llir::Unary_instruction{
            .opcode = llir::Unary_opcode::negate,
            .result = result,
            .operand = std::move(*operand),
          }}
        );
      }

      template <typename InstructionT>
      void lower_binary_instruction(
        llir::Basic_block &block,
        InstructionT const &inst
      )
      {
        auto const result = ensure_register(
          inst.result,
          is_compare_instruction_v<InstructionT>
            ? llir::bits(1)
            : bits_for_value<typename InstructionT::value_type>()
        );
        auto lhs = lower_operand(inst.lhs);
        auto rhs = lower_operand(inst.rhs);
        assert(lhs.has_value());
        assert(rhs.has_value());
        block.instructions.push_back(
          llir::Instruction{llir::Binary_instruction{
            .opcode = binary_opcode<InstructionT>(),
            .result = result,
            .lhs = std::move(*lhs),
            .rhs = std::move(*rhs),
          }}
        );
      }

      void lower_call_instruction(
        llir::Basic_block &block,
        hlir::Call_instruction const &inst
      )
      {
        auto const return_type =
          std::get<hlir::Function_type>(inst.callee->type->data).return_type;
        auto result = std::optional<llir::Register>{};
        if (!std::holds_alternative<hlir::Void_type>(return_type->data))
        {
          result = ensure_register(inst.result, type_of(return_type));
        }
        // TODO: HLIR still gives void calls a result register. If that register
        // is later returned or passed as a block argument, this lowering leaves
        // it unmapped and lookup_register will assert. Either model void
        // registers explicitly or teach operand lowering to erase them
        // consistently.
        block.instructions.push_back(
          llir::Instruction{llir::Call_instruction{
            .result = result,
            .callee = lowered_function(inst.callee),
            .arguments = lower_operands(
              std::span<hlir::Operand const>{
                inst.arguments,
              }
            ),
          }}
        );
      }

      std::vector<llir::Operand>
      lower_operands(std::span<hlir::Operand const> operands) const
      {
        auto lowered = std::vector<llir::Operand>{};
        lowered.reserve(operands.size());
        for (auto const &operand : operands)
        {
          auto value = lower_operand(operand);
          assert(value.has_value());
          lowered.push_back(std::move(*value));
        }
        return lowered;
      }

      std::vector<llir::Operand> lower_block_arguments(
        hlir::Basic_block const *target,
        std::span<hlir::Operand const> arguments
      )
      {
        auto lowered_arguments = std::vector<llir::Operand>{};
        auto const target_block = lowered_block(target);
        lowered_arguments.reserve(arguments.size());
        for (auto i = std::size_t{}; i < arguments.size(); ++i)
        {
          auto const lowered_param =
            ensure_register(target->parameters[i], operand_type(arguments[i]));
          if (target_block->parameters.size() <= i)
          {
            target_block->parameters.push_back(lowered_param);
          }
          auto lowered_argument = lower_operand(arguments[i]);
          assert(lowered_argument.has_value());
          lowered_arguments.push_back(std::move(*lowered_argument));
        }
        return lowered_arguments;
      }

      llir::Terminator_payload
      lower_terminator(hlir::Terminator_payload const &payload)
      {
        return std::visit(
          [&](auto const &term) -> llir::Terminator_payload
          {
            return lower_terminator_variant(term);
          },
          payload
        );
      }

      llir::Terminator_payload
      lower_terminator_variant(hlir::Jump_terminator const &term)
      {
        return llir::Jump_terminator{
          .target = lowered_block(term.target),
          .arguments = lower_block_arguments(
            term.target,
            std::span<hlir::Operand const>{term.arguments}
          ),
        };
      }

      llir::Terminator_payload
      lower_terminator_variant(hlir::Branch_terminator const &term)
      {
        auto condition = lower_operand(term.condition);
        assert(condition.has_value());
        return llir::Branch_terminator{
          .condition = std::move(*condition),
          .then_target = lowered_block(term.then_target),
          .then_arguments = lower_block_arguments(
            term.then_target,
            std::span<hlir::Operand const>{term.then_arguments}
          ),
          .else_target = lowered_block(term.else_target),
          .else_arguments = lower_block_arguments(
            term.else_target,
            std::span<hlir::Operand const>{term.else_arguments}
          ),
        };
      }

      llir::Terminator_payload
      lower_terminator_variant(hlir::Return_terminator const &term)
      {
        return llir::Return_terminator{.value = lower_operand(term.value)};
      }

      Lowered_function _function;
      std::unordered_map<hlir::Function const *, llir::Function *> const
        *_functions;
    };

    class Translation_unit_lowerer
    {
    public:
      explicit Translation_unit_lowerer(hlir::Translation_unit const *source)
          : _source{source}
      {
      }

      llir::Translation_unit lower()
      {
        declare_functions();
        lower_function_bodies();
        return std::move(_target);
      }

    private:
      void declare_functions()
      {
        for (auto const &function : _source->functions)
        {
          auto lowered = std::make_unique<llir::Function>();
          _functions[function.get()] = lowered.get();
          _target.functions.push_back(std::move(lowered));
        }
        for (auto const &[name, function] : _source->function_table)
        {
          _target.function_table[name] = _functions.at(function);
        }
      }

      void lower_function_bodies()
      {
        for (auto const &function : _source->functions)
        {
          Function_lowerer{
            function.get(),
            _functions.at(function.get()),
            &_functions,
          }
            .lower();
        }
      }

      hlir::Translation_unit const *_source;
      llir::Translation_unit _target;
      std::unordered_map<hlir::Function const *, llir::Function *> _functions;
    };

  } // namespace

  llir::Translation_unit lower(hlir::Translation_unit const &tu)
  {
    return Translation_unit_lowerer{&tu}.lower();
  }

} // namespace benson::hlir2llir
