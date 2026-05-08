#include <stdexcept>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "bytecode/module_builder.h"
#include "spelling/spelling.h"

using benson::bytecode::gpr;

auto reg_byte(benson::bytecode::Register reg) -> std::byte
{
  return static_cast<std::byte>(reg.value);
}

TEST_CASE("Module_builder patches label-based jmp_i instructions")
{
  using benson::bytecode::Module_builder;
  using benson::bytecode::Opcode;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  auto const start = builder.make_label();
  auto const end = builder.make_label();

  builder.place_label(start);
  writer.emit_jmp(builder.label_target(end));
  writer.emit_jmp(builder.label_target(start));
  builder.place_label(end);
  writer.emit_exit();

  auto const module = builder.build();

  CHECK(
    module.code == std::vector<std::byte>{
                     static_cast<std::byte>(Opcode::wide),
                     static_cast<std::byte>(Opcode::jmp_i),
                     std::byte{0x02},
                     std::byte{0x00},
                     static_cast<std::byte>(Opcode::jmp_i),
                     std::byte{0xFA},
                     static_cast<std::byte>(Opcode::exit),
                   }
  );
  CHECK(module.constant_data.empty());
  CHECK(module.constant_table.empty());
}

TEST_CASE("Module_builder patches label-based jnz_i instructions")
{
  using benson::bytecode::Module_builder;
  using benson::bytecode::Opcode;
  using benson::bytecode::Register;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  auto const start = builder.make_label();
  auto const end = builder.make_label();

  builder.place_label(start);
  writer.emit_jnz(gpr(1), builder.label_target(end));
  writer.emit_jnz(gpr(2), builder.label_target(start));
  builder.place_label(end);
  writer.emit_exit();

  auto const module = builder.build();

  CHECK(
    module.code == std::vector<std::byte>{
                     static_cast<std::byte>(Opcode::wide),
                     static_cast<std::byte>(Opcode::jnz_i),
                     reg_byte(gpr(1)),
                     std::byte{0x00},
                     std::byte{0x03},
                     std::byte{0x00},
                     static_cast<std::byte>(Opcode::jnz_i),
                     reg_byte(gpr(2)),
                     std::byte{0xF7},
                     static_cast<std::byte>(Opcode::exit),
                   }
  );
  CHECK(module.constant_data.empty());
  CHECK(module.constant_table.empty());
}

TEST_CASE("Module_builder interns and deduplicates inline constants")
{
  using benson::bytecode::Module_builder;
  using enum benson::bytecode::Opcode;
  using benson::bytecode::gpr;

  auto builder = Module_builder{};
  auto &writer = builder.writer();

  writer.emit_add_f32_k(gpr(2), builder.constant(0.3F), gpr(1));
  writer.emit_mul_f32_k(gpr(4), builder.constant(0.3F), gpr(3));
  writer.emit_add_i32_k(gpr(6), builder.constant(7), gpr(5));
  writer.emit_sub_i32_k(gpr(8), builder.constant(7), gpr(7));
  writer.emit_exit();

  auto const module = builder.build();

  CHECK(
    module.code == std::vector<std::byte>{
                     static_cast<std::byte>(add_f32_k),
                     reg_byte(gpr(2)),
                     std::byte{0x00},
                     reg_byte(gpr(1)),
                     static_cast<std::byte>(mul_f32_k),
                     reg_byte(gpr(4)),
                     std::byte{0x00},
                     reg_byte(gpr(3)),
                     static_cast<std::byte>(add_i32_k),
                     reg_byte(gpr(6)),
                     std::byte{0x01},
                     reg_byte(gpr(5)),
                     static_cast<std::byte>(sub_i32_k),
                     reg_byte(gpr(8)),
                     std::byte{0x01},
                     reg_byte(gpr(7)),
                     static_cast<std::byte>(exit),
                   }
  );
  CHECK(module.constant_table == std::vector<std::ptrdiff_t>{0, 4});
  CHECK(module.constant_data.size() == 8);
}

TEST_CASE("Module_builder records indexed functions")
{
  using benson::bytecode::Module_builder;
  using enum benson::bytecode::Scalar_type;

  auto spellings = benson::Spelling_table{};
  auto const foo = spellings.intern("foo");
  auto const bar = spellings.intern("bar");

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  auto const foo_index =
    builder.declare_function(foo, {int32, int32}, int64, 3);
  auto const bar_index = builder.declare_function(bar, {}, void_, 0);

  builder.place_function(foo_index);
  writer.emit_ret(gpr(0));
  builder.place_function(bar_index);
  writer.emit_ret_void();
  writer.emit_exit();

  auto const module = builder.build();

  REQUIRE(module.functions.size() == 2);
  CHECK(foo_index.value == 0);
  CHECK(bar_index.value == 1);

  auto const &foo_entry = module.functions[0];
  CHECK(foo_entry.position == 0);
  CHECK(foo_entry.parameter_types == std::vector{int32, int32});
  CHECK(foo_entry.return_type == int64);
  CHECK(foo_entry.register_count == 3);

  auto const &bar_entry = module.functions[1];
  CHECK(bar_entry.position == 2);
  CHECK(bar_entry.parameter_types.empty());
  CHECK(bar_entry.return_type == void_);
  CHECK(bar_entry.register_count == 0);

  CHECK(module.function_indices.at(foo).value == 0);
  CHECK(module.function_indices.at(bar).value == 1);
}

TEST_CASE("Module_builder declares functions before placing code")
{
  using benson::bytecode::Module_builder;
  using enum benson::bytecode::Scalar_type;

  auto spellings = benson::Spelling_table{};
  auto const recurse = spellings.intern("recurse");

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  auto const recurse_index = builder.declare_function(recurse, {}, void_, 0);
  writer.emit_exit();
  auto const placement_position = writer.position();
  builder.place_function(recurse_index);
  writer.emit_ret_void();

  auto const module = builder.build();

  REQUIRE(module.functions.size() == 1);
  CHECK(recurse_index.value == 0);
  CHECK(module.function_indices.at(recurse).value == 0);
  auto const &entry = module.functions[0];
  CHECK(entry.position == placement_position);
  CHECK(entry.parameter_types.empty());
  CHECK(entry.return_type == void_);
  CHECK(entry.register_count == 0);
}

TEST_CASE("Module_builder rejects declared functions that were never placed")
{
  using benson::bytecode::Module_builder;
  using enum benson::bytecode::Scalar_type;

  auto spellings = benson::Spelling_table{};
  auto const ghost = spellings.intern("ghost");

  auto builder = Module_builder{};
  (void) builder.declare_function(ghost, {}, void_, 0);

  CHECK_THROWS_AS(builder.build(), std::runtime_error);
}
