#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "bytecode/module_builder.h"

using benson::bytecode::gpr;
using benson::bytecode::sp;

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

TEST_CASE("Module_builder patches label-based call_i instructions")
{
  using benson::bytecode::Module_builder;
  using benson::bytecode::Opcode;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  auto const start = builder.make_label();
  auto const subroutine = builder.make_label();

  builder.place_label(start);
  writer.emit_call(builder.label_target(subroutine));
  writer.emit_exit();
  builder.place_label(subroutine);
  writer.emit_call(builder.label_target(start));
  writer.emit_ret();

  auto const module = builder.build();

  CHECK(
    module.code == std::vector<std::byte>{
                     static_cast<std::byte>(Opcode::wide),
                     static_cast<std::byte>(Opcode::call_i),
                     std::byte{0x01},
                     std::byte{0x00},
                     static_cast<std::byte>(Opcode::exit),
                     static_cast<std::byte>(Opcode::call_i),
                     std::byte{0xF9},
                     static_cast<std::byte>(Opcode::ret),
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
  using benson::bytecode::sp;

  auto builder = Module_builder{};
  auto &writer = builder.writer();

  writer.emit_add_f32_k(gpr(1), gpr(2), builder.constant(0.3F));
  writer.emit_mul_f32_k(gpr(3), gpr(4), builder.constant(0.3F));
  writer.emit_add_i32_k(gpr(5), gpr(6), builder.constant(7));
  writer.emit_sub_i32_k(gpr(7), gpr(8), builder.constant(7));
  writer.emit_exit();

  auto const module = builder.build();

  CHECK(
    module.code == std::vector<std::byte>{
                     static_cast<std::byte>(add_f32_k),
                     reg_byte(gpr(1)),
                     reg_byte(gpr(2)),
                     std::byte{0x00},
                     static_cast<std::byte>(mul_f32_k),
                     reg_byte(gpr(3)),
                     reg_byte(gpr(4)),
                     std::byte{0x00},
                     static_cast<std::byte>(add_i32_k),
                     reg_byte(gpr(5)),
                     reg_byte(gpr(6)),
                     std::byte{0x01},
                     static_cast<std::byte>(sub_i32_k),
                     reg_byte(gpr(7)),
                     reg_byte(gpr(8)),
                     std::byte{0x01},
                     static_cast<std::byte>(exit),
                   }
  );
  CHECK(module.constant_table == std::vector<std::ptrdiff_t>{0, 4});
  CHECK(module.constant_data.size() == 8);
}
