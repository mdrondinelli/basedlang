#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "bytecode/module_builder.h"

TEST_CASE("Module_builder patches label-based jmp_i instructions")
{
  using benson::bytecode::Module_builder;
  using benson::bytecode::Opcode;

  auto builder = Module_builder{};
  auto const start = builder.make_label();
  auto const end = builder.make_label();

  builder.place_label(start);
  builder.emit_jmp_i(end);
  builder.emit_jmp_i(start);
  builder.place_label(end);
  builder.emit_exit();

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

TEST_CASE("Module_builder interns and deduplicates inline constants")
{
  using benson::bytecode::Module_builder;
  using benson::bytecode::Opcode;
  using benson::bytecode::Register;

  auto builder = Module_builder{};

  builder.emit_add_f32_k(Register::gpr_1, Register::gpr_2, 0.3F);
  builder.emit_mul_f32_k(Register::gpr_3, Register::gpr_4, 0.3F);
  builder.emit_add_i32_k(Register::gpr_5, Register::gpr_6, 7);
  builder.emit_sub_i32_k(Register::gpr_7, Register::gpr_8, 7);
  builder.emit_exit();

  auto const module = builder.build();

  CHECK(
    module.code == std::vector<std::byte>{
                     static_cast<std::byte>(Opcode::add_f32_k),
                     static_cast<std::byte>(Register::gpr_1),
                     static_cast<std::byte>(Register::gpr_2),
                     std::byte{0x00},
                     static_cast<std::byte>(Opcode::mul_f32_k),
                     static_cast<std::byte>(Register::gpr_3),
                     static_cast<std::byte>(Register::gpr_4),
                     std::byte{0x00},
                     static_cast<std::byte>(Opcode::add_i32_k),
                     static_cast<std::byte>(Register::gpr_5),
                     static_cast<std::byte>(Register::gpr_6),
                     std::byte{0x01},
                     static_cast<std::byte>(Opcode::sub_i32_k),
                     static_cast<std::byte>(Register::gpr_7),
                     static_cast<std::byte>(Register::gpr_8),
                     std::byte{0x01},
                     static_cast<std::byte>(Opcode::exit),
                   }
  );
  CHECK(module.constant_table == std::vector<std::ptrdiff_t>{0, 4});
  CHECK(module.constant_data.size() == 8);
}
