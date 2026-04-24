#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "bytecode/module_builder.h"

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
  writer.emit_jnz(Register::gpr_1, builder.label_target(end));
  writer.emit_jnz(Register::gpr_2, builder.label_target(start));
  builder.place_label(end);
  writer.emit_exit();

  auto const module = builder.build();

  CHECK(
    module.code == std::vector<std::byte>{
                     static_cast<std::byte>(Opcode::wide),
                     static_cast<std::byte>(Opcode::jnz_i),
                     static_cast<std::byte>(Register::gpr_1),
                     std::byte{0x03},
                     std::byte{0x00},
                     static_cast<std::byte>(Opcode::jnz_i),
                     static_cast<std::byte>(Register::gpr_2),
                     std::byte{0xF8},
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
  using enum benson::bytecode::Register;

  auto builder = Module_builder{};
  auto &writer = builder.writer();

  writer.emit_add_f32_k(gpr_1, gpr_2, builder.constant(0.3F));
  writer.emit_mul_f32_k(gpr_3, gpr_4, builder.constant(0.3F));
  writer.emit_add_i32_k(gpr_5, gpr_6, builder.constant(7));
  writer.emit_sub_i32_k(gpr_7, gpr_8, builder.constant(7));
  writer.emit_exit();

  auto const module = builder.build();

  CHECK(
    module.code == std::vector<std::byte>{
                     static_cast<std::byte>(add_f32_k),
                     static_cast<std::byte>(gpr_1),
                     static_cast<std::byte>(gpr_2),
                     std::byte{0x00},
                     static_cast<std::byte>(mul_f32_k),
                     static_cast<std::byte>(gpr_3),
                     static_cast<std::byte>(gpr_4),
                     std::byte{0x00},
                     static_cast<std::byte>(add_i32_k),
                     static_cast<std::byte>(gpr_5),
                     static_cast<std::byte>(gpr_6),
                     std::byte{0x01},
                     static_cast<std::byte>(sub_i32_k),
                     static_cast<std::byte>(gpr_7),
                     static_cast<std::byte>(gpr_8),
                     std::byte{0x01},
                     static_cast<std::byte>(exit),
                   }
  );
  CHECK(module.constant_table == std::vector<std::ptrdiff_t>{0, 4});
  CHECK(module.constant_data.size() == 8);
}
