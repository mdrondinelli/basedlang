#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <utility>
#include <variant>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "hlir2llir/lower.h"
#include "spelling/spelling.h"

namespace
{

  auto bit_width(benson::llir::Register_type const &type) -> std::int32_t
  {
    return std::get<benson::llir::Bit_register_type>(type).bit_width;
  }

  auto immediate_int32(benson::llir::Operand const &operand) -> std::int32_t
  {
    auto const &immediate = std::get<benson::llir::Immediate>(operand);
    REQUIRE(immediate.little_endian_bytes.size() == 4);
    auto value = std::int32_t{};
    std::memcpy(&value, immediate.little_endian_bytes.data(), sizeof(value));
    return value;
  }

  auto make_function(
    benson::hlir::Type_pool *types,
    std::vector<benson::hlir::Type *> parameter_types,
    benson::hlir::Type *return_type,
    std::int32_t register_count
  ) -> std::unique_ptr<benson::hlir::Function>
  {
    return std::make_unique<benson::hlir::Function>(benson::hlir::Function{
      .type = types->function_type(std::move(parameter_types), return_type),
      .blocks = {},
      .register_count = register_count,
    });
  }

  auto add_block(benson::hlir::Function *function)
    -> benson::hlir::Basic_block *
  {
    function->blocks.push_back(std::make_unique<benson::hlir::Basic_block>());
    return function->blocks.back().get();
  }

} // namespace

TEST_CASE("hlir2llir lowers arithmetic function")
{
  auto types = benson::hlir::Type_pool{};
  auto tu = benson::hlir::Translation_unit{};
  auto function =
    make_function(&types, {types.int32_type()}, types.int32_type(), 3);
  auto const entry = add_block(function.get());
  auto const arg = benson::hlir::Register{0};
  auto const one = benson::hlir::Register{1};
  auto const result = benson::hlir::Register{2};
  entry->parameters.push_back(arg);
  entry->instructions.push_back(
    benson::hlir::Instruction{benson::hlir::Constant_instruction<std::int32_t>{
      .result = one,
      .value = 1,
    }}
  );
  entry->instructions.push_back(
    benson::hlir::Instruction{benson::hlir::Add_instruction<std::int32_t>{
      .result = result,
      .lhs = arg,
      .rhs = one,
    }}
  );
  entry->terminator = benson::hlir::Terminator{
    benson::hlir::Return_terminator{.value = result},
  };
  tu.functions.push_back(std::move(function));

  auto lowered = benson::hlir2llir::lower(tu);

  REQUIRE(lowered.functions.size() == 1);
  auto const &lf = *lowered.functions[0];
  REQUIRE(lf.blocks.size() == 1);
  CHECK(lf.blocks[0]->parameters.size() == 1);
  REQUIRE(lf.register_types.size() == 3);
  CHECK(bit_width(lf.type_of(benson::llir::Register{0})) == 32);
  CHECK(bit_width(lf.type_of(benson::llir::Register{1})) == 32);
  CHECK(bit_width(lf.type_of(benson::llir::Register{2})) == 32);
  REQUIRE(lf.blocks[0]->instructions.size() == 2);
  auto const &copy = std::get<benson::llir::Copy_instruction>(
    lf.blocks[0]->instructions[0].payload
  );
  CHECK(copy.result == benson::llir::Register{1});
  CHECK(immediate_int32(copy.value) == 1);
  auto const &add = std::get<benson::llir::Binary_instruction>(
    lf.blocks[0]->instructions[1].payload
  );
  CHECK(add.opcode == benson::llir::Binary_opcode::add);
  CHECK(add.result == benson::llir::Register{2});
  auto const &ret = std::get<benson::llir::Return_terminator>(
    lf.blocks[0]->terminator->payload
  );
  REQUIRE(ret.value.has_value());
  CHECK(
    std::get<benson::llir::Register>(*ret.value) == benson::llir::Register{2}
  );
}

TEST_CASE("hlir2llir reuses destination registers for block arguments")
{
  auto types = benson::hlir::Type_pool{};
  auto tu = benson::hlir::Translation_unit{};
  auto function =
    make_function(&types, {types.int32_type()}, types.int32_type(), 3);
  auto const entry = add_block(function.get());
  auto const then_block = add_block(function.get());
  auto const else_block = add_block(function.get());
  auto const merge_block = add_block(function.get());
  auto const arg = benson::hlir::Register{0};
  auto const cond = benson::hlir::Register{1};
  auto const merge_param = benson::hlir::Register{2};
  entry->parameters.push_back(arg);
  merge_block->parameters.push_back(merge_param);
  entry->instructions.push_back(
    benson::hlir::Instruction{benson::hlir::Less_instruction<std::int32_t>{
      .result = cond,
      .lhs = arg,
      .rhs = benson::hlir::Constant_value{std::int32_t{0}},
    }}
  );
  entry->terminator = benson::hlir::Terminator{benson::hlir::Branch_terminator{
    .condition = cond,
    .then_target = then_block,
    .then_arguments = {},
    .else_target = else_block,
    .else_arguments = {},
  }};
  then_block->terminator =
    benson::hlir::Terminator{benson::hlir::Jump_terminator{
      .target = merge_block,
      .arguments = {benson::hlir::Constant_value{std::int32_t{0}}},
    }};
  else_block->terminator =
    benson::hlir::Terminator{benson::hlir::Jump_terminator{
      .target = merge_block,
      .arguments = {arg},
    }};
  merge_block->terminator = benson::hlir::Terminator{
    benson::hlir::Return_terminator{.value = merge_param},
  };
  tu.functions.push_back(std::move(function));

  auto lowered = benson::hlir2llir::lower(tu);

  auto const &lf = *lowered.functions[0];
  REQUIRE(lf.blocks.size() == 4);
  REQUIRE(lf.blocks[3]->parameters.size() == 1);
  auto const lowered_param = lf.blocks[3]->parameters[0];
  auto const &then_jump =
    std::get<benson::llir::Jump_terminator>(lf.blocks[1]->terminator->payload);
  auto const &else_jump =
    std::get<benson::llir::Jump_terminator>(lf.blocks[2]->terminator->payload);
  CHECK(then_jump.target == lf.blocks[3].get());
  CHECK(else_jump.target == lf.blocks[3].get());
  CHECK(lf.blocks[3]->parameters[0] == lowered_param);
  auto const &ret = std::get<benson::llir::Return_terminator>(
    lf.blocks[3]->terminator->payload
  );
  REQUIRE(ret.value.has_value());
  CHECK(std::get<benson::llir::Register>(*ret.value) == lowered_param);
}

TEST_CASE("hlir2llir preserves function table and lowers calls")
{
  auto spellings = benson::Spelling_table{};
  auto types = benson::hlir::Type_pool{};
  auto tu = benson::hlir::Translation_unit{};
  auto callee =
    make_function(&types, {types.int32_type()}, types.int32_type(), 1);
  auto const callee_entry = add_block(callee.get());
  auto const callee_arg = benson::hlir::Register{0};
  callee_entry->parameters.push_back(callee_arg);
  callee_entry->terminator = benson::hlir::Terminator{
    benson::hlir::Return_terminator{.value = callee_arg},
  };
  auto const callee_ptr = callee.get();
  auto caller = make_function(&types, {}, types.int32_type(), 1);
  auto const caller_entry = add_block(caller.get());
  auto const call_result = benson::hlir::Register{0};
  caller_entry->instructions.push_back(
    benson::hlir::Instruction{benson::hlir::Call_instruction{
      .result = call_result,
      .callee = callee_ptr,
      .arguments = {benson::hlir::Constant_value{std::int32_t{42}}},
    }}
  );
  caller_entry->terminator = benson::hlir::Terminator{
    benson::hlir::Return_terminator{.value = call_result},
  };
  auto const callee_name = spellings.intern("id");
  auto const caller_name = spellings.intern("main");
  tu.function_table[callee_name] = callee_ptr;
  tu.functions.push_back(std::move(callee));
  tu.function_table[caller_name] = caller.get();
  tu.functions.push_back(std::move(caller));

  auto lowered = benson::hlir2llir::lower(tu);

  REQUIRE(lowered.functions.size() == 2);
  CHECK(lowered.function_table.at(callee_name) == lowered.functions[0].get());
  CHECK(lowered.function_table.at(caller_name) == lowered.functions[1].get());
  auto const &call = std::get<benson::llir::Call_instruction>(
    lowered.functions[1]->blocks[0]->instructions[0].payload
  );
  REQUIRE(call.result.has_value());
  CHECK(*call.result == benson::llir::Register{0});
  CHECK(call.callee == lowered.functions[0].get());
  REQUIRE(call.arguments.size() == 1);
  CHECK(immediate_int32(call.arguments[0]) == 42);
}
