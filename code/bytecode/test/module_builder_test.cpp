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
