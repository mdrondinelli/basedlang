#include <cstddef>
#include <memory>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "llir/llir.h"

TEST_CASE("LLIR registers default to invalid and allocated registers are typed")
{
  auto function = benson::llir::Function{};
  auto const invalid = benson::llir::Register{};
  auto const i32 = function.add_register(benson::llir::bits(32));
  auto const i1 = function.add_register(benson::llir::bits(1));

  CHECK_FALSE(invalid);
  REQUIRE(i32);
  REQUIRE(i1);
  CHECK(*i32 == 0);
  CHECK(*i1 == 1);
  CHECK(
    std::get<benson::llir::Bit_register_type>(function.type_of(i32))
      .bit_width == 32
  );
  CHECK(
    std::get<benson::llir::Bit_register_type>(function.type_of(i1)).bit_width ==
    1
  );
}

TEST_CASE("LLIR register types support arbitrary bit widths")
{
  auto function = benson::llir::Function{};
  auto const wide = function.add_register(benson::llir::bits(257));
  auto immediate = benson::llir::Immediate{
    .type = benson::llir::bits(257),
    .little_endian_bytes = std::vector<std::byte>(33),
  };
  immediate.little_endian_bytes.front() = std::byte{0x2A};

  CHECK(
    std::get<benson::llir::Bit_register_type>(function.type_of(wide))
      .bit_width == 257
  );
  CHECK(immediate.little_endian_bytes.size() == 33);
  CHECK(immediate.little_endian_bytes.front() == std::byte{0x2A});
}

TEST_CASE("LLIR blocks carry parameters, instructions, and terminators")
{
  auto function = benson::llir::Function{};
  auto const condition = function.add_register(benson::llir::bits(1));
  auto const value = function.add_register(benson::llir::bits(64));
  auto const entry = function.add_block();
  auto const then_block = function.add_block();
  auto const else_block = function.add_block();

  then_block->parameters.push_back(value);
  entry->instructions.push_back(
    benson::llir::Instruction{benson::llir::Copy_instruction{
      .result = condition,
      .value = benson::llir::Immediate{
        .type = benson::llir::bits(1),
        .little_endian_bytes = {std::byte{1}},
      },
    }}
  );
  entry->terminator = benson::llir::Terminator{benson::llir::Branch_terminator{
    .condition = condition,
    .then_target = then_block,
    .then_arguments = {value},
    .else_target = else_block,
    .else_arguments = {},
  }};
  else_block->terminator =
    benson::llir::Terminator{benson::llir::Return_terminator{}};

  CHECK(entry->instructions.size() == 1);
  CHECK(entry->has_terminator());
  CHECK(then_block->parameters == std::vector{value});
  CHECK(else_block->has_terminator());
}

TEST_CASE("LLIR translation unit owns functions and preserves name lookup")
{
  auto spellings = benson::Spelling_table{};
  auto const name = spellings.intern("main");
  auto tu = benson::llir::Translation_unit{};
  auto function = std::make_unique<benson::llir::Function>();
  auto const function_ptr = function.get();

  tu.function_table[name] = function_ptr;
  tu.functions.push_back(std::move(function));

  REQUIRE(tu.functions.size() == 1);
  CHECK(tu.function_table.at(name) == tu.functions.front().get());
}
