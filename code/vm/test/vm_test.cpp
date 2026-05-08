#include <array>
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>
#include <vector>

#include "vm/pointer.h"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

#include "bytecode/bytecode_writer.h"
#include "bytecode/module_builder.h"
#include "spelling/spelling.h"
#include "vm/vm.h"

namespace
{
  using benson::bytecode::gpr;

  class Recording_output_stream: public benson::Output_stream
  {
  public:
    void write_bytes(std::span<std::byte const> buffer) override
    {
      _bytes.insert(_bytes.end(), buffer.begin(), buffer.end());
    }

    [[nodiscard]] auto bytes() const -> std::vector<std::byte> const &
    {
      return _bytes;
    }

  private:
    std::vector<std::byte> _bytes{};
  };

} // namespace

namespace
{

  template <typename T>
  void store_constant(
    std::vector<std::byte> &constant_memory,
    std::vector<std::ptrdiff_t> &constant_table,
    benson::bytecode::Constant index,
    T value
  )
  {
    if (constant_table.size() <= index.value)
    {
      constant_table.resize(index.value + 1);
    }
    constant_table[index.value] =
      static_cast<std::ptrdiff_t>(constant_memory.size());
    auto const start = constant_memory.size();
    constant_memory.resize(start + sizeof(T));
    std::memcpy(constant_memory.data() + start, &value, sizeof(T));
  }

} // namespace

TEMPLATE_TEST_CASE(
  "Virtual_machine typed register access round-trips values",
  "[vm]",
  std::int8_t,
  std::int16_t,
  std::int32_t,
  std::int64_t,
  float,
  double
)
{
  using benson::bytecode::Register;

  auto const value = [] -> TestType
  {
    if constexpr (std::same_as<TestType, std::int8_t>)
    {
      return std::int8_t{42};
    }
    else if constexpr (std::same_as<TestType, std::int16_t>)
    {
      return std::int16_t{1234};
    }
    else if constexpr (std::same_as<TestType, std::int32_t>)
    {
      return std::int32_t{12345678};
    }
    else if constexpr (std::same_as<TestType, std::int64_t>)
    {
      return std::int64_t{1234567890123456789LL};
    }
    else if constexpr (std::same_as<TestType, float>)
    {
      return 42.5F;
    }
    else
    {
      return 42.5;
    }
  }();

  auto vm = benson::vm::Virtual_machine{};
  vm.registers.resize(256);
  vm.set_register_value(gpr(255), value);

  CHECK(vm.get_register_value<TestType>(gpr(255)) == value);
}

TEST_CASE("Virtual_machine keeps exact floating-point register bit pattern")
{
  auto vm = benson::vm::Virtual_machine{};
  vm.registers.resize(2);
  vm.set_register_value(benson::bytecode::gpr(1), -0.0F);

  CHECK(
    std::bit_cast<std::uint32_t>(
      vm.get_register_value<float>(benson::bytecode::gpr(1))
    ) == std::bit_cast<std::uint32_t>(-0.0F)
  );
}

TEST_CASE(
  "Virtual_machine runs lookup constant program emitted by Module_builder"
)
{
  using benson::bytecode::gpr;
  using benson::bytecode::Module_builder;
  using benson::vm::Address_space;
  using benson::vm::Pointer;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  writer.emit_lookup_k(builder.constant(5), gpr(1));
  writer.emit_lookup_k(builder.constant(2.5), gpr(2));
  writer.emit_exit();
  auto const module = builder.build();

  auto vm = benson::vm::Virtual_machine{};
  vm.load(module);
  vm.registers.resize(3);

  vm.run();

  auto const p1 = vm.get_register_value<Pointer>(gpr(1)).decode();
  auto const p2 = vm.get_register_value<Pointer>(gpr(2)).decode();

  CHECK(p1.space == Address_space::constant);
  CHECK(p1.offset == static_cast<std::uint64_t>(module.constant_table[0]));
  CHECK(p2.space == Address_space::constant);
  CHECK(p2.offset == static_cast<std::uint64_t>(module.constant_table[1]));
  CHECK(vm.instruction_pointer == module.code.data() + 7);
}

TEST_CASE("Virtual_machine runs negation program emitted by Module_builder")
{
  using benson::bytecode::gpr;
  using benson::bytecode::Module_builder;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  writer.emit_neg_i32(gpr(2), gpr(1));
  writer.emit_exit();
  auto const module = builder.build();

  auto vm = benson::vm::Virtual_machine{};
  vm.load(module);
  vm.registers.resize(3);
  vm.set_register_value<std::int32_t>(gpr(2), 123);

  vm.run();

  CHECK(vm.get_register_value<std::int32_t>(gpr(1)) == -123);
  CHECK(vm.get_register_value<std::int32_t>(gpr(2)) == 123);
  CHECK(vm.instruction_pointer == module.code.data() + 4);
}

TEST_CASE("Virtual_machine runs move program emitted by Module_builder")
{
  using benson::bytecode::gpr;
  using benson::bytecode::Immediate;
  using benson::bytecode::Module_builder;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  writer.emit_mov(gpr(2), gpr(1));
  writer.emit_mov_i(Immediate{-4}, gpr(3));
  writer.emit_mov_i(Immediate{0x0102}, gpr(4));
  writer.emit_mov_i(Immediate{-0x0102}, gpr(5));
  writer.emit_exit();
  auto const module = builder.build();

  auto vm = benson::vm::Virtual_machine{};
  vm.load(module);
  vm.registers.resize(6);
  vm.set_register_value<double>(gpr(2), -0.0);

  vm.run();

  CHECK(
    std::bit_cast<std::uint64_t>(vm.get_register_value<double>(gpr(1))) ==
    std::bit_cast<std::uint64_t>(-0.0)
  );
  CHECK(vm.get_register_value<std::int32_t>(gpr(3)) == -4);
  CHECK(vm.get_register_value<std::int64_t>(gpr(3)) == -4);
  CHECK(vm.get_register_value<std::int32_t>(gpr(4)) == 0x0102);
  CHECK(vm.get_register_value<std::int64_t>(gpr(5)) == -0x0102);
  CHECK(vm.instruction_pointer == module.code.data() + 19);
}

TEST_CASE("Virtual_machine runs program with wide registers")
{
  using benson::bytecode::Immediate;
  using benson::bytecode::Module_builder;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  auto const done = builder.make_label();
  writer.emit_mov_i(Immediate{5}, gpr(300));
  writer.emit_mov_i(Immediate{7}, gpr(301));
  writer.emit_add_i32(gpr(300), gpr(301), gpr(302));
  writer.emit_alloca_i(Immediate{8});
  writer.emit_store_sp_32(Immediate{0}, gpr(302));
  writer.emit_load_sp_32(Immediate{0}, gpr(303));
  writer.emit_jnz(gpr(303), builder.label_target(done));
  writer.emit_mov_i(Immediate{1}, gpr(304));
  builder.place_label(done);
  writer.emit_mov(gpr(303), gpr(305));
  writer.emit_neg_i32(gpr(305), gpr(306));
  writer.emit_exit();
  auto const module = builder.build();

  auto vm = benson::vm::Virtual_machine{};
  vm.load(module);
  vm.registers.resize(307);

  vm.run();

  CHECK(vm.get_register_value<std::int32_t>(gpr(303)) == 12);
  CHECK(vm.get_register_value<std::int32_t>(gpr(304)) == 0);
  CHECK(vm.get_register_value<std::int32_t>(gpr(305)) == 12);
  CHECK(vm.get_register_value<std::int32_t>(gpr(306)) == -12);
}

TEST_CASE(
  "Virtual_machine runs integer arithmetic program emitted by Module_builder"
)
{
  using benson::bytecode::gpr;
  using benson::bytecode::Module_builder;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  writer.emit_add_i32(gpr(2), gpr(3), gpr(1));
  writer.emit_sub_i32(gpr(1), gpr(5), gpr(4));
  writer.emit_mul_i32(gpr(4), gpr(7), gpr(6));
  writer.emit_div_i32(gpr(6), gpr(9), gpr(8));
  writer.emit_mod_i32(gpr(8), gpr(11), gpr(10));
  writer.emit_exit();
  auto const module = builder.build();

  auto vm = benson::vm::Virtual_machine{};
  vm.load(module);
  vm.registers.resize(12);
  vm.set_register_value<std::int32_t>(gpr(2), 10);
  vm.set_register_value<std::int32_t>(gpr(3), 5);
  vm.set_register_value<std::int32_t>(gpr(5), 3);
  vm.set_register_value<std::int32_t>(gpr(7), 4);
  vm.set_register_value<std::int32_t>(gpr(9), 6);
  vm.set_register_value<std::int32_t>(gpr(11), 7);

  vm.run();

  CHECK(vm.get_register_value<std::int32_t>(gpr(1)) == 15);
  CHECK(vm.get_register_value<std::int32_t>(gpr(4)) == 12);
  CHECK(vm.get_register_value<std::int32_t>(gpr(6)) == 48);
  CHECK(vm.get_register_value<std::int32_t>(gpr(8)) == 8);
  CHECK(vm.get_register_value<std::int32_t>(gpr(10)) == 1);
  CHECK(vm.instruction_pointer == module.code.data() + 21);
}

TEST_CASE(
  "Virtual_machine runs integer immediate arithmetic program emitted by "
  "Module_builder"
)
{
  using benson::bytecode::gpr;
  using benson::bytecode::Immediate;
  using benson::bytecode::Module_builder;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  writer.emit_add_i32_i(gpr(2), Immediate{5}, gpr(1));
  writer.emit_sub_i32_i(gpr(1), Immediate{-3}, gpr(3));
  writer.emit_mul_i32_i(gpr(3), Immediate{0x0102}, gpr(4));
  writer.emit_div_i32_i(gpr(4), Immediate{6}, gpr(5));
  writer.emit_mod_i32_i(gpr(5), Immediate{7}, gpr(6));
  writer.emit_exit();
  auto const module = builder.build();

  auto vm = benson::vm::Virtual_machine{};
  vm.load(module);
  vm.registers.resize(7);
  vm.set_register_value<std::int32_t>(gpr(2), 10);

  vm.run();

  CHECK(vm.get_register_value<std::int32_t>(gpr(1)) == 15);
  CHECK(vm.get_register_value<std::int32_t>(gpr(3)) == 18);
  CHECK(vm.get_register_value<std::int32_t>(gpr(4)) == 4644);
  CHECK(vm.get_register_value<std::int32_t>(gpr(5)) == 774);
  CHECK(vm.get_register_value<std::int32_t>(gpr(6)) == 4);
  CHECK(vm.instruction_pointer == module.code.data() + 25);
}

TEST_CASE(
  "Virtual_machine runs floating-point arithmetic program emitted by "
  "Module_builder"
)
{
  using benson::bytecode::gpr;
  using benson::bytecode::Module_builder;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  writer.emit_add_f64(gpr(2), gpr(3), gpr(1));
  writer.emit_sub_f64(gpr(1), gpr(5), gpr(4));
  writer.emit_mul_f64(gpr(4), gpr(7), gpr(6));
  writer.emit_div_f64(gpr(6), gpr(9), gpr(8));
  writer.emit_exit();
  auto const module = builder.build();

  auto vm = benson::vm::Virtual_machine{};
  vm.load(module);
  vm.registers.resize(10);
  vm.set_register_value<double>(gpr(2), 10.0);
  vm.set_register_value<double>(gpr(3), 2.5);
  vm.set_register_value<double>(gpr(5), 1.5);
  vm.set_register_value<double>(gpr(7), 4.0);
  vm.set_register_value<double>(gpr(9), 2.0);

  vm.run();

  CHECK(vm.get_register_value<double>(gpr(1)) == 12.5);
  CHECK(vm.get_register_value<double>(gpr(4)) == 11.0);
  CHECK(vm.get_register_value<double>(gpr(6)) == 44.0);
  CHECK(vm.get_register_value<double>(gpr(8)) == 22.0);
  CHECK(vm.instruction_pointer == module.code.data() + 17);
}

TEST_CASE(
  "Virtual_machine runs narrow constant arithmetic program emitted by "
  "Module_builder"
)
{
  using benson::bytecode::gpr;
  using benson::bytecode::Module_builder;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  writer.emit_add_i32_k(gpr(2), builder.constant(5), gpr(1));
  writer.emit_sub_i32_k(gpr(1), builder.constant(3), gpr(4));
  writer.emit_mul_i32_k(gpr(4), builder.constant(4), gpr(6));
  writer.emit_div_i32_k(gpr(6), builder.constant(6), gpr(8));
  writer.emit_mod_i32_k(gpr(8), builder.constant(7), gpr(10));
  writer.emit_add_f64_k(gpr(14), builder.constant(2.5), gpr(12));
  writer.emit_sub_f64_k(gpr(12), builder.constant(1.5), gpr(16));
  writer.emit_mul_f64_k(gpr(16), builder.constant(4.0), gpr(18));
  writer.emit_div_f64_k(gpr(18), builder.constant(2.0), gpr(20));
  writer.emit_exit();
  auto const module = builder.build();

  auto vm = benson::vm::Virtual_machine{};
  vm.load(module);
  vm.registers.resize(21);
  vm.set_register_value<std::int32_t>(gpr(2), 10);
  vm.set_register_value<double>(gpr(14), 10.0);

  vm.run();

  CHECK(vm.get_register_value<std::int32_t>(gpr(1)) == 15);
  CHECK(vm.get_register_value<std::int32_t>(gpr(4)) == 12);
  CHECK(vm.get_register_value<std::int32_t>(gpr(6)) == 48);
  CHECK(vm.get_register_value<std::int32_t>(gpr(8)) == 8);
  CHECK(vm.get_register_value<std::int32_t>(gpr(10)) == 1);
  CHECK(vm.get_register_value<double>(gpr(12)) == 12.5);
  CHECK(vm.get_register_value<double>(gpr(16)) == 11.0);
  CHECK(vm.get_register_value<double>(gpr(18)) == 44.0);
  CHECK(vm.get_register_value<double>(gpr(20)) == 22.0);
  CHECK(vm.instruction_pointer == module.code.data() + 37);
}

TEST_CASE(
  "Virtual_machine runs wide constant arithmetic program emitted by "
  "Bytecode_writer"
)
{
  using benson::bytecode::Constant;
  using benson::bytecode::Register;

  auto constant_memory = std::vector<std::byte>{};
  auto constant_table = std::vector<std::ptrdiff_t>{};
  store_constant<std::int32_t>(
    constant_memory,
    constant_table,
    Constant{0x0304},
    5
  );
  store_constant<std::int32_t>(
    constant_memory,
    constant_table,
    Constant{0x0305},
    3
  );
  store_constant<std::int32_t>(
    constant_memory,
    constant_table,
    Constant{0x0306},
    4
  );
  store_constant<std::int32_t>(
    constant_memory,
    constant_table,
    Constant{0x0307},
    6
  );
  store_constant<std::int32_t>(
    constant_memory,
    constant_table,
    Constant{0x0308},
    7
  );
  store_constant<double>(
    constant_memory,
    constant_table,
    Constant{0x0309},
    2.5
  );
  store_constant<double>(
    constant_memory,
    constant_table,
    Constant{0x030A},
    1.5
  );
  store_constant<double>(
    constant_memory,
    constant_table,
    Constant{0x030B},
    4.0
  );
  store_constant<double>(
    constant_memory,
    constant_table,
    Constant{0x030C},
    2.0
  );

  auto stream = Recording_output_stream{};
  auto writer = benson::bytecode::Bytecode_writer{&stream};
  writer.emit_add_i32_k(gpr(2), Constant{0x0304}, gpr(1));
  writer.emit_sub_i32_k(gpr(1), Constant{0x0305}, gpr(4));
  writer.emit_mul_i32_k(gpr(4), Constant{0x0306}, gpr(6));
  writer.emit_div_i32_k(gpr(6), Constant{0x0307}, gpr(8));
  writer.emit_mod_i32_k(gpr(8), Constant{0x0308}, gpr(10));
  writer.emit_add_f64_k(gpr(14), Constant{0x0309}, gpr(12));
  writer.emit_sub_f64_k(gpr(12), Constant{0x030A}, gpr(16));
  writer.emit_mul_f64_k(gpr(16), Constant{0x030B}, gpr(18));
  writer.emit_div_f64_k(gpr(18), Constant{0x030C}, gpr(20));
  writer.emit_exit();
  writer.flush();

  auto module = benson::bytecode::Module{};
  module.constant_data = std::move(constant_memory);
  module.constant_table = std::move(constant_table);

  auto vm = benson::vm::Virtual_machine{};
  vm.module = &module;
  vm.instruction_pointer = stream.bytes().data();
  vm.registers.resize(21);
  vm.set_register_value<std::int32_t>(gpr(2), 10);
  vm.set_register_value<double>(gpr(14), 10.0);

  vm.run();

  CHECK(vm.get_register_value<std::int32_t>(gpr(1)) == 15);
  CHECK(vm.get_register_value<std::int32_t>(gpr(4)) == 12);
  CHECK(vm.get_register_value<std::int32_t>(gpr(6)) == 48);
  CHECK(vm.get_register_value<std::int32_t>(gpr(8)) == 8);
  CHECK(vm.get_register_value<std::int32_t>(gpr(10)) == 1);
  CHECK(vm.get_register_value<double>(gpr(12)) == 12.5);
  CHECK(vm.get_register_value<double>(gpr(16)) == 11.0);
  CHECK(vm.get_register_value<double>(gpr(18)) == 44.0);
  CHECK(vm.get_register_value<double>(gpr(20)) == 22.0);
  CHECK(vm.instruction_pointer == stream.bytes().data() + 73);
}

TEST_CASE("Virtual_machine store_8 writes one byte to stack")
{
  using benson::bytecode::Immediate;
  using benson::bytecode::Module_builder;
  using benson::bytecode::Register;
  using benson::vm::Address_space;
  using benson::vm::Pointer;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  writer.emit_store_8(gpr(2), Immediate{0}, gpr(1));
  writer.emit_exit();
  auto const module = builder.build();

  auto vm = benson::vm::Virtual_machine{};
  vm.load(module);
  vm.registers.resize(3);
  vm.set_register_value<std::uint64_t>(
    gpr(1),
    static_cast<std::uint64_t>(std::int8_t{-42})
  );
  vm.set_register_value<std::uint64_t>(
    gpr(2),
    static_cast<std::uint64_t>(Pointer{Address_space::stack, 0})
  );

  vm.run();

  CHECK(
    (*vm.stack)[0] == static_cast<std::byte>(static_cast<std::uint8_t>(-42))
  );
}

TEST_CASE("Virtual_machine load_8 sign-extends one byte from stack")
{
  using benson::bytecode::Immediate;
  using benson::bytecode::Module_builder;
  using benson::bytecode::Register;
  using benson::vm::Address_space;
  using benson::vm::Pointer;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  writer.emit_load_8(gpr(2), Immediate{0}, gpr(1));
  writer.emit_exit();
  auto const module = builder.build();

  auto vm = benson::vm::Virtual_machine{};
  vm.load(module);
  vm.registers.resize(3);
  (*vm.stack)[0] = static_cast<std::byte>(std::uint8_t{0xAB});
  vm.set_register_value<std::uint64_t>(
    gpr(2),
    static_cast<std::uint64_t>(Pointer{Address_space::stack, 0})
  );

  vm.run();

  CHECK(vm.get_register_value<std::int64_t>(gpr(1)) == -85);
}

TEST_CASE("Virtual_machine load_16 sign-extends two bytes from stack")
{
  using benson::bytecode::Immediate;
  using benson::bytecode::Module_builder;
  using benson::bytecode::Register;
  using benson::vm::Address_space;
  using benson::vm::Pointer;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  writer.emit_load_16(gpr(2), Immediate{0}, gpr(1));
  writer.emit_exit();
  auto const module = builder.build();

  auto vm = benson::vm::Virtual_machine{};
  vm.load(module);
  vm.registers.resize(3);
  auto const value = std::int16_t{-0x1234};
  std::memcpy(vm.stack->data(), &value, sizeof(value));
  vm.set_register_value<std::uint64_t>(
    gpr(2),
    static_cast<std::uint64_t>(Pointer{Address_space::stack, 0})
  );

  vm.run();

  CHECK(vm.get_register_value<std::int64_t>(gpr(1)) == value);
}

TEST_CASE("Virtual_machine store_64 and load_64 round-trip a 64-bit value")
{
  using benson::bytecode::Immediate;
  using benson::bytecode::Module_builder;
  using benson::bytecode::Register;
  using benson::vm::Address_space;
  using benson::vm::Pointer;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  writer.emit_store_64(gpr(3), Immediate{0}, gpr(1));
  writer.emit_load_64(gpr(3), Immediate{0}, gpr(2));
  writer.emit_exit();
  auto const module = builder.build();

  auto vm = benson::vm::Virtual_machine{};
  vm.load(module);
  vm.registers.resize(4);
  vm.set_register_value<std::int64_t>(gpr(1), std::int64_t{-1234567890123LL});
  vm.set_register_value<std::uint64_t>(
    gpr(3),
    static_cast<std::uint64_t>(Pointer{Address_space::stack, 0})
  );

  vm.run();

  CHECK(
    vm.get_register_value<std::int64_t>(gpr(2)) ==
    std::int64_t{-1234567890123LL}
  );
}

TEST_CASE("Virtual_machine load_32 and store_32 respect immediate offset")
{
  using benson::bytecode::Immediate;
  using benson::bytecode::Module_builder;
  using benson::bytecode::Register;
  using benson::vm::Address_space;
  using benson::vm::Pointer;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  writer.emit_store_32(gpr(3), Immediate{8}, gpr(1));
  writer.emit_load_32(gpr(3), Immediate{8}, gpr(2));
  writer.emit_exit();
  auto const module = builder.build();

  auto vm = benson::vm::Virtual_machine{};
  vm.load(module);
  vm.registers.resize(4);
  vm.set_register_value<std::int32_t>(gpr(1), std::int32_t{0xDEAD});
  vm.set_register_value<std::uint64_t>(
    gpr(3),
    static_cast<std::uint64_t>(Pointer{Address_space::stack, 0})
  );

  vm.run();

  CHECK(vm.get_register_value<std::int64_t>(gpr(2)) == std::int32_t{0xDEAD});
}

TEST_CASE("Virtual_machine load_32 reads from constant memory via pointer")
{
  using benson::bytecode::gpr;
  using benson::bytecode::Immediate;
  using benson::bytecode::Module_builder;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  writer.emit_lookup_k(builder.constant(std::int32_t{-0x1234567}), gpr(2));
  writer.emit_load_32(gpr(2), Immediate{0}, gpr(1));
  writer.emit_exit();
  auto const module = builder.build();

  auto vm = benson::vm::Virtual_machine{};
  vm.load(module);
  vm.registers.resize(3);

  vm.run();

  CHECK(
    vm.get_register_value<std::int64_t>(gpr(1)) == std::int32_t{-0x1234567}
  );
}

TEST_CASE("Virtual_machine runs Module_builder program with forward jmp_i")
{
  using benson::bytecode::Immediate;
  using benson::bytecode::Module_builder;
  using benson::bytecode::Register;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  auto const done = builder.make_label();
  writer.emit_jmp(builder.label_target(done));
  writer.emit_add_i32_i(gpr(1), Immediate{1}, gpr(1));
  builder.place_label(done);
  writer.emit_add_i32_i(gpr(1), Immediate{2}, gpr(1));
  writer.emit_exit();

  auto const module = builder.build();

  auto vm = benson::vm::Virtual_machine{};
  vm.load(module);
  vm.registers.resize(2);
  vm.set_register_value<std::int32_t>(gpr(1), 40);

  vm.run();

  CHECK(vm.get_register_value<std::int32_t>(gpr(1)) == 42);
}

TEST_CASE("Virtual_machine indexed call slides register window")
{
  using benson::bytecode::Immediate;
  using benson::bytecode::Module_builder;
  using enum benson::bytecode::Scalar_type;

  auto spellings = benson::Spelling_table{};
  auto const entry = spellings.intern("entry");
  auto const add = spellings.intern("add");

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  auto const entry_index = builder.declare_function(entry, {}, int32, 12);
  auto const add_index =
    builder.declare_function(add, {int32, int32}, int32, 3);

  builder.place_function(entry_index);
  writer.emit_mov_i(Immediate{40}, gpr(10));
  writer.emit_mov_i(Immediate{2}, gpr(11));
  writer.emit_call_i(add_index, gpr(10), gpr(0));
  writer.emit_ret(gpr(0));

  builder.place_function(add_index);
  writer.emit_add_i32(gpr(0), gpr(1), gpr(2));
  writer.emit_ret(gpr(2));

  auto const module = builder.build();

  auto vm = benson::vm::Virtual_machine{};
  vm.load(module);

  auto const result = vm.call(entry, {});
  REQUIRE(result.type() == int32);
  CHECK(result.as<std::int32_t>() == 42);
}

TEST_CASE("Virtual_machine void call restores caller frame and stack pointer")
{
  using benson::bytecode::Immediate;
  using benson::bytecode::Module_builder;
  using enum benson::bytecode::Scalar_type;

  auto spellings = benson::Spelling_table{};
  auto const entry = spellings.intern("entry");
  auto const clobber = spellings.intern("clobber");

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  auto const entry_index = builder.declare_function(entry, {}, int32, 12);
  auto const clobber_index =
    builder.declare_function(clobber, {int32}, void_, 1);

  builder.place_function(entry_index);
  writer.emit_mov_i(Immediate{123}, gpr(5));
  writer.emit_mov_i(Immediate{456}, gpr(10));
  writer.emit_call_void_i(clobber_index, gpr(10));
  writer.emit_ret(gpr(5));

  builder.place_function(clobber_index);
  writer.emit_alloca_i(Immediate{16});
  writer.emit_mov_i(Immediate{99}, gpr(0));
  writer.emit_ret_void();

  auto const module = builder.build();

  auto vm = benson::vm::Virtual_machine{};
  vm.load(module);
  auto const initial_sp = vm.stack_pointer;

  auto const result = vm.call(entry, {});
  REQUIRE(result.type() == int32);
  CHECK(result.as<std::int32_t>() == 123);
  CHECK(vm.stack_pointer == initial_sp);
}

TEST_CASE("Virtual_machine indexed call uses wide relative registers")
{
  using benson::bytecode::Immediate;
  using benson::bytecode::Module_builder;
  using enum benson::bytecode::Scalar_type;

  auto spellings = benson::Spelling_table{};
  auto const entry = spellings.intern("entry");
  auto const callee = spellings.intern("callee");

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  auto const entry_index = builder.declare_function(entry, {}, int32, 303);
  auto const callee_index =
    builder.declare_function(callee, {int32}, int32, 270);

  builder.place_function(entry_index);
  writer.emit_mov_i(Immediate{40}, gpr(300));
  writer.emit_call_i(callee_index, gpr(300), gpr(302));
  writer.emit_ret(gpr(302));

  builder.place_function(callee_index);
  writer.emit_add_i32_i(gpr(0), Immediate{2}, gpr(269));
  writer.emit_ret(gpr(269));

  auto const module = builder.build();

  auto vm = benson::vm::Virtual_machine{};
  vm.load(module);

  auto const result = vm.call(entry, {});
  REQUIRE(result.type() == int32);
  CHECK(result.as<std::int32_t>() == 42);
}

TEST_CASE("Virtual_machine grows registers vector for wide register window")
{
  using benson::bytecode::Immediate;
  using benson::bytecode::Module_builder;
  using enum benson::bytecode::Scalar_type;
  auto spellings = benson::Spelling_table{};
  auto const wide = spellings.intern("wide");
  auto builder = Module_builder{};
  auto &writer = builder.writer();
  // register_count of 500 forces the VM to grow its registers vector from
  // empty (the constructor leaves it that way) past any small initial size.
  auto const wide_index = builder.declare_function(wide, {}, int32, 500);
  builder.place_function(wide_index);
  writer.emit_mov_i(Immediate{42}, gpr(499));
  writer.emit_ret(gpr(499));
  auto const module = builder.build();
  auto vm = benson::vm::Virtual_machine{};
  vm.load(module);
  REQUIRE(vm.registers.empty());
  auto const result = vm.call(wide, {});
  REQUIRE(result.type() == int32);
  CHECK(result.as<std::int32_t>() == 42);
  // The window plus one return slot must fit in registers.
  CHECK(vm.registers.size() >= 501);
  CHECK(vm.registers.capacity() >= vm.registers.size());
}

TEST_CASE("Virtual_machine does not take narrow jnz_i when condition is zero")
{
  using benson::bytecode::Immediate;
  using benson::bytecode::Register;

  auto stream = Recording_output_stream{};
  auto writer = benson::bytecode::Bytecode_writer{&stream};
  writer.emit_jnz(gpr(2), std::ptrdiff_t{7});
  writer.emit_add_i32_i(gpr(1), Immediate{1}, gpr(1));
  writer.emit_exit();
  writer.flush();

  auto vm = benson::vm::Virtual_machine{};
  vm.instruction_pointer = stream.bytes().data();
  vm.registers.resize(3);
  vm.set_register_value<std::int32_t>(gpr(1), 40);
  vm.set_register_value<std::int32_t>(gpr(2), 0);

  vm.run();

  CHECK(vm.get_register_value<std::int32_t>(gpr(1)) == 41);
}

TEST_CASE("Virtual_machine takes narrow jnz_i when condition is non-zero")
{
  using benson::bytecode::Immediate;
  using benson::bytecode::Register;

  auto stream = Recording_output_stream{};
  auto writer = benson::bytecode::Bytecode_writer{&stream};
  writer.emit_jnz(gpr(2), std::ptrdiff_t{7});
  writer.emit_add_i32_i(gpr(1), Immediate{1}, gpr(1));
  writer.emit_exit();
  writer.flush();

  auto vm = benson::vm::Virtual_machine{};
  vm.instruction_pointer = stream.bytes().data();
  vm.registers.resize(3);
  vm.set_register_value<std::int32_t>(gpr(1), 40);
  vm.set_register_value<std::int32_t>(gpr(2), 1);

  vm.run();

  CHECK(vm.get_register_value<std::int32_t>(gpr(1)) == 40);
}

TEST_CASE("Virtual_machine takes wide jnz_i when target exceeds narrow range")
{
  using benson::bytecode::Immediate;
  using benson::bytecode::Register;

  auto stream = Recording_output_stream{};
  auto writer = benson::bytecode::Bytecode_writer{&stream};
  writer.emit_jnz(gpr(2), std::ptrdiff_t{166});
  for (auto i = 0; i < 40; ++i)
  {
    writer.emit_add_i32_i(gpr(1), Immediate{1}, gpr(1));
  }
  writer.emit_exit();
  writer.flush();

  auto vm = benson::vm::Virtual_machine{};
  vm.instruction_pointer = stream.bytes().data();
  vm.registers.resize(3);
  vm.set_register_value<std::int32_t>(gpr(1), 40);
  vm.set_register_value<std::int32_t>(gpr(2), 1);

  vm.run();

  CHECK(vm.get_register_value<std::int32_t>(gpr(1)) == 40);
}

TEST_CASE("Virtual_machine runs Module_builder program with inline constants")
{
  using benson::bytecode::gpr;
  using benson::bytecode::Module_builder;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  writer.emit_add_i32_k(gpr(1), builder.constant(5), gpr(1));
  writer.emit_mul_f32_k(gpr(2), builder.constant(0.5F), gpr(2));
  writer.emit_add_f32_k(gpr(2), builder.constant(0.5F), gpr(2));
  writer.emit_exit();

  auto const module = builder.build();

  auto vm = benson::vm::Virtual_machine{};
  vm.load(module);
  vm.registers.resize(3);
  vm.set_register_value<std::int32_t>(gpr(1), 37);
  vm.set_register_value<float>(gpr(2), 3.0F);

  vm.run();

  CHECK(vm.get_register_value<std::int32_t>(gpr(1)) == 42);
  CHECK(vm.get_register_value<float>(gpr(2)) == 2.0F);
  CHECK(module.constant_table.size() == 2);
}

TEST_CASE(
  "Virtual_machine load_16 and store_16 use wide offset when offset > 255"
)
{
  using benson::bytecode::Immediate;
  using benson::bytecode::Register;
  using benson::vm::Address_space;
  using benson::vm::Pointer;

  auto stream = Recording_output_stream{};
  auto writer = benson::bytecode::Bytecode_writer{&stream};
  // offset 0x0101 = 257, which exceeds Immediate max of 127 → wide encoding
  writer.emit_store_16(gpr(3), Immediate{0x0101}, gpr(1));
  writer.emit_load_16(gpr(3), Immediate{0x0101}, gpr(2));
  writer.emit_exit();
  writer.flush();

  auto vm = benson::vm::Virtual_machine{};
  vm.instruction_pointer = stream.bytes().data();
  vm.registers.resize(4);
  vm.set_register_value<std::int16_t>(gpr(1), std::int16_t{0x7FFF});
  vm.set_register_value<std::uint64_t>(
    gpr(3),
    static_cast<std::uint64_t>(Pointer{Address_space::stack, 0})
  );

  vm.run();

  CHECK(vm.get_register_value<std::int16_t>(gpr(2)) == std::int16_t{0x7FFF});
}

TEST_CASE("Virtual_machine runs sx_8 program emitted by Module_builder")
{
  using benson::bytecode::gpr;
  using benson::bytecode::Module_builder;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  writer.emit_sx_8(gpr(2), gpr(1));
  writer.emit_exit();
  auto const module = builder.build();

  auto vm = benson::vm::Virtual_machine{};
  vm.load(module);
  vm.registers.resize(3);
  vm.set_register_value<std::int8_t>(gpr(2), std::int8_t{-42});

  vm.run();

  CHECK(vm.get_register_value<std::int8_t>(gpr(1)) == std::int8_t{-42});
}

TEST_CASE(
  "Virtual_machine runs narrow comparison program emitted by Module_builder"
)
{
  using benson::bytecode::gpr;
  using benson::bytecode::Immediate;
  using benson::bytecode::Module_builder;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  writer.emit_cmp_eq_i32(gpr(2), gpr(3), gpr(1));
  writer.emit_cmp_eq_i64_i(gpr(5), Immediate{9}, gpr(4));
  writer.emit_cmp_ne_f32(gpr(7), gpr(8), gpr(6));
  writer.emit_cmp_ne_f64_k(gpr(10), builder.constant(4.0), gpr(9));
  writer.emit_cmp_lt_i32_i(gpr(12), Immediate{8}, gpr(11));
  writer.emit_cmp_lt_i64_k(gpr(14), builder.constant(std::int64_t{7}), gpr(13));
  writer.emit_cmp_le_f32_k(gpr(16), builder.constant(3.5F), gpr(15));
  writer.emit_cmp_le_f64(gpr(18), gpr(19), gpr(17));
  writer.emit_cmp_gt_i32_k(gpr(21), builder.constant(1), gpr(20));
  writer.emit_cmp_gt_i64(gpr(23), gpr(24), gpr(22));
  writer.emit_cmp_ge_f32(gpr(26), gpr(27), gpr(25));
  writer.emit_cmp_ge_f64_k(gpr(29), builder.constant(6.0), gpr(28));
  writer.emit_exit();
  auto const module = builder.build();

  auto vm = benson::vm::Virtual_machine{};
  vm.load(module);
  vm.registers.resize(30);
  vm.set_register_value<std::int32_t>(gpr(2), 42);
  vm.set_register_value<std::int32_t>(gpr(3), 42);
  vm.set_register_value<std::int64_t>(gpr(5), 9);
  vm.set_register_value<float>(gpr(7), 1.5F);
  vm.set_register_value<float>(gpr(8), 2.5F);
  vm.set_register_value<double>(gpr(10), 5.0);
  vm.set_register_value<std::int32_t>(gpr(12), 4);
  vm.set_register_value<std::int64_t>(gpr(14), 6);
  vm.set_register_value<float>(gpr(16), 3.5F);
  vm.set_register_value<double>(gpr(18), 2.0);
  vm.set_register_value<double>(gpr(19), 2.0);
  vm.set_register_value<std::int32_t>(gpr(21), 5);
  vm.set_register_value<std::int64_t>(gpr(23), 7);
  vm.set_register_value<std::int64_t>(gpr(24), 3);
  vm.set_register_value<float>(gpr(26), 2.5F);
  vm.set_register_value<float>(gpr(27), 2.5F);
  vm.set_register_value<double>(gpr(29), 6.0);

  vm.run();

  CHECK(vm.get_register_value<bool>(gpr(1)));
  CHECK(vm.get_register_value<bool>(gpr(4)));
  CHECK(vm.get_register_value<bool>(gpr(6)));
  CHECK(vm.get_register_value<bool>(gpr(9)));
  CHECK(vm.get_register_value<bool>(gpr(11)));
  CHECK(vm.get_register_value<bool>(gpr(13)));
  CHECK(vm.get_register_value<bool>(gpr(15)));
  CHECK(vm.get_register_value<bool>(gpr(17)));
  CHECK(vm.get_register_value<bool>(gpr(20)));
  CHECK(vm.get_register_value<bool>(gpr(22)));
  CHECK(vm.get_register_value<bool>(gpr(25)));
  CHECK(vm.get_register_value<bool>(gpr(28)));
}

TEST_CASE(
  "Virtual_machine runs wide comparison program emitted by Bytecode_writer"
)
{
  using benson::bytecode::Constant;
  using benson::bytecode::Immediate;
  using benson::bytecode::Register;

  auto constant_memory = std::vector<std::byte>{};
  auto constant_table = std::vector<std::ptrdiff_t>{};
  store_constant<std::int32_t>(
    constant_memory,
    constant_table,
    Constant{0x0201},
    9
  );
  store_constant<std::int64_t>(
    constant_memory,
    constant_table,
    Constant{0x0202},
    7
  );
  store_constant<float>(
    constant_memory,
    constant_table,
    Constant{0x0203},
    3.0F
  );
  store_constant<double>(
    constant_memory,
    constant_table,
    Constant{0x0204},
    4.0
  );
  store_constant<float>(
    constant_memory,
    constant_table,
    Constant{0x0205},
    1.5F
  );
  store_constant<double>(
    constant_memory,
    constant_table,
    Constant{0x0206},
    8.0
  );

  auto stream = Recording_output_stream{};
  auto writer = benson::bytecode::Bytecode_writer{&stream};
  writer.emit_cmp_eq_i32_i(gpr(2), Immediate{0x0101}, gpr(1));
  writer.emit_cmp_ne_i64_k(gpr(4), Constant{0x0201}, gpr(3));
  writer.emit_cmp_lt_f32_k(gpr(6), Constant{0x0203}, gpr(5));
  writer.emit_cmp_le_f64_k(gpr(8), Constant{0x0204}, gpr(7));
  writer.emit_cmp_gt_i32_k(gpr(10), Constant{0x0201}, gpr(9));
  writer.emit_cmp_ge_i64_i(gpr(12), Immediate{0x0100}, gpr(11));
  writer.emit_cmp_ge_f32_k(gpr(14), Constant{0x0205}, gpr(13));
  writer.emit_cmp_eq_f64_k(gpr(16), Constant{0x0206}, gpr(15));
  writer.emit_exit();
  writer.flush();

  auto module = benson::bytecode::Module{};
  module.constant_data = std::move(constant_memory);
  module.constant_table = std::move(constant_table);

  auto vm = benson::vm::Virtual_machine{};
  vm.module = &module;
  vm.instruction_pointer = stream.bytes().data();
  vm.registers.resize(17);
  vm.set_register_value<std::int32_t>(gpr(2), 0x0101);
  vm.set_register_value<std::int64_t>(gpr(4), 8);
  vm.set_register_value<float>(gpr(6), 2.5F);
  vm.set_register_value<double>(gpr(8), 3.5);
  vm.set_register_value<std::int32_t>(gpr(10), 12);
  vm.set_register_value<std::int64_t>(gpr(12), 0x0100);
  vm.set_register_value<float>(gpr(14), 1.5F);
  vm.set_register_value<double>(gpr(16), 8.0);

  vm.run();

  CHECK(vm.get_register_value<bool>(gpr(1)));
  CHECK(vm.get_register_value<bool>(gpr(3)));
  CHECK(vm.get_register_value<bool>(gpr(5)));
  CHECK(vm.get_register_value<bool>(gpr(7)));
  CHECK(vm.get_register_value<bool>(gpr(9)));
  CHECK(vm.get_register_value<bool>(gpr(11)));
  CHECK(vm.get_register_value<bool>(gpr(13)));
  CHECK(vm.get_register_value<bool>(gpr(15)));
}

TEST_CASE("Virtual_machine::call invokes an int32 add function")
{
  using benson::bytecode::Immediate;
  using benson::bytecode::Module_builder;
  using enum benson::bytecode::Scalar_type;

  auto spellings = benson::Spelling_table{};
  auto const add = spellings.intern("add");

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  auto const add_index =
    builder.declare_function(add, {int32, int32}, int32, 3);
  builder.place_function(add_index);
  writer.emit_add_i32(gpr(0), gpr(1), gpr(2));
  writer.emit_ret(gpr(2));
  auto const module = builder.build();

  auto vm = benson::vm::Virtual_machine{};
  vm.load(module);

  auto const args = std::array<benson::vm::Scalar, 2>{
    std::int32_t{40},
    std::int32_t{2},
  };
  auto const result = vm.call(add, args);
  REQUIRE(result.type() == int32);
  CHECK(result.as<std::int32_t>() == 42);
}

TEST_CASE(
  "Virtual_machine::call round-trips a float through the register window"
)
{
  using benson::bytecode::Module_builder;
  using enum benson::bytecode::Scalar_type;

  auto spellings = benson::Spelling_table{};
  auto const identity = spellings.intern("identity");

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  auto const identity_index =
    builder.declare_function(identity, {float_}, float_, 1);
  builder.place_function(identity_index);
  writer.emit_ret(gpr(0));
  auto const module = builder.build();

  auto vm = benson::vm::Virtual_machine{};
  vm.load(module);

  auto const args = std::array<benson::vm::Scalar, 1>{float{3.14159F}};
  auto const result = vm.call(identity, args);
  REQUIRE(result.type() == float_);
  CHECK(result.as<float>() == 3.14159F);
}

TEST_CASE("Virtual_machine::call returns void for void functions")
{
  using benson::bytecode::Module_builder;
  using enum benson::bytecode::Scalar_type;

  auto spellings = benson::Spelling_table{};
  auto const noop = spellings.intern("noop");

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  auto const noop_index = builder.declare_function(noop, {}, void_, 0);
  builder.place_function(noop_index);
  writer.emit_ret_void();
  auto const module = builder.build();

  auto vm = benson::vm::Virtual_machine{};
  vm.load(module);

  auto const result = vm.call(noop, {});
  CHECK(result.type() == void_);
}

TEST_CASE("Virtual_machine::call works across consecutive invocations")
{
  using benson::bytecode::Immediate;
  using benson::bytecode::Module_builder;
  using enum benson::bytecode::Scalar_type;

  auto spellings = benson::Spelling_table{};
  auto const inc = spellings.intern("inc");

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  auto const inc_index = builder.declare_function(inc, {int32}, int32, 1);
  builder.place_function(inc_index);
  writer.emit_add_i32_i(gpr(0), Immediate{1}, gpr(0));
  writer.emit_ret(gpr(0));
  auto const module = builder.build();

  auto vm = benson::vm::Virtual_machine{};
  vm.load(module);

  auto const args1 = std::array<benson::vm::Scalar, 1>{std::int32_t{10}};
  auto const r1 = vm.call(inc, args1);
  CHECK(r1.as<std::int32_t>() == 11);

  auto const args2 = std::array<benson::vm::Scalar, 1>{std::int32_t{99}};
  auto const r2 = vm.call(inc, args2);
  CHECK(r2.as<std::int32_t>() == 100);
}

TEST_CASE(
  "Virtual_machine::call restores execution state after bytecode throws"
)
{
  using benson::bytecode::Immediate;
  using benson::bytecode::Module_builder;
  using enum benson::bytecode::Scalar_type;

  auto spellings = benson::Spelling_table{};
  auto const boom = spellings.intern("boom");

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  auto const boom_index = builder.declare_function(boom, {}, int32, 1);
  builder.place_function(boom_index);
  // store_8 to a constant pointer throws "store to constant memory".
  writer.emit_lookup_k(builder.constant(std::int32_t{0}), gpr(0));
  writer.emit_store_8(gpr(0), Immediate{0}, gpr(0));
  writer.emit_ret(gpr(0));
  auto const module = builder.build();

  auto vm = benson::vm::Virtual_machine{};
  vm.load(module);
  auto const saved_ip = vm.instruction_pointer;
  auto const saved_sp = vm.stack_pointer;
  auto const saved_call_stack_size = vm.call_stack.size();
  vm.base_register = 3;

  CHECK_THROWS(vm.call(boom, {}));

  CHECK(vm.instruction_pointer == saved_ip);
  CHECK(vm.base_register == 3);
  CHECK(vm.stack_pointer == saved_sp);
  CHECK(vm.call_stack.size() == saved_call_stack_size);
}

TEST_CASE(
  "Virtual_machine::call decodes return value through per-call return slot"
)
{
  using benson::bytecode::Immediate;
  using benson::bytecode::Module_builder;
  using enum benson::bytecode::Scalar_type;

  auto spellings = benson::Spelling_table{};
  auto const answer = spellings.intern("answer");

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  auto const answer_index = builder.declare_function(answer, {}, int32, 1);
  builder.place_function(answer_index);
  writer.emit_mov_i(Immediate{42}, gpr(0));
  writer.emit_ret(gpr(0));
  auto const module = builder.build();

  auto vm = benson::vm::Virtual_machine{};
  vm.load(module);
  vm.base_register = 3;

  auto const result = vm.call(answer, {});
  REQUIRE(result.type() == int32);
  CHECK(result.as<std::int32_t>() == 42);
}

TEST_CASE("Virtual_machine zero-register-count callee may ret_void")
{
  using benson::bytecode::Module_builder;
  using enum benson::bytecode::Scalar_type;

  auto spellings = benson::Spelling_table{};
  auto const noop = spellings.intern("noop");

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  auto const noop_index = builder.declare_function(noop, {}, void_, 0);
  builder.place_function(noop_index);
  writer.emit_ret_void();
  auto const module = builder.build();

  auto vm = benson::vm::Virtual_machine{};
  vm.load(module);

  auto const result = vm.call(noop, {});
  CHECK(result.type() == void_);
}

TEST_CASE("Virtual_machine::call rejects bad inputs")
{
  using benson::bytecode::Module_builder;
  using enum benson::bytecode::Scalar_type;

  auto spellings = benson::Spelling_table{};
  auto const known = spellings.intern("known");
  auto const unknown = spellings.intern("unknown");

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  auto const known_index = builder.declare_function(known, {int32}, int32, 1);
  builder.place_function(known_index);
  writer.emit_ret(gpr(0));
  auto const module = builder.build();

  auto vm = benson::vm::Virtual_machine{};
  vm.load(module);

  CHECK_THROWS_AS(
    vm.call(unknown, {}),
    benson::vm::Virtual_machine::Unknown_function_error
  );
  CHECK_THROWS_AS(
    vm.call(known, {}),
    benson::vm::Virtual_machine::Argument_count_error
  );
  auto const wrong_type = std::array<benson::vm::Scalar, 1>{std::int64_t{0}};
  CHECK_THROWS_AS(
    vm.call(known, wrong_type),
    benson::vm::Virtual_machine::Argument_type_error
  );
}

TEST_CASE("Virtual_machine alloca_i shrinks stack_pointer by the immediate")
{
  using benson::bytecode::Immediate;
  using benson::bytecode::Module_builder;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  writer.emit_alloca_i(Immediate{16});
  writer.emit_exit();
  auto const module = builder.build();

  auto vm = benson::vm::Virtual_machine{};
  vm.load(module);
  auto const initial = vm.stack_pointer;

  vm.run();

  CHECK(vm.stack_pointer == initial - 16);
}

TEST_CASE(
  "Virtual_machine alloca shrinks stack_pointer by the register's int64 value"
)
{
  using benson::bytecode::Immediate;
  using benson::bytecode::Module_builder;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  writer.emit_alloca(gpr(1));
  writer.emit_exit();
  auto const module = builder.build();

  auto vm = benson::vm::Virtual_machine{};
  vm.load(module);
  vm.registers.resize(2);
  vm.set_register_value<std::int64_t>(gpr(1), 24);
  auto const initial = vm.stack_pointer;

  vm.run();

  CHECK(vm.stack_pointer == initial - 24);
}

TEST_CASE("Virtual_machine alloca accepts negative register values")
{
  using benson::bytecode::Immediate;
  using benson::bytecode::Module_builder;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  writer.emit_alloca_i(Immediate{32});
  writer.emit_alloca(gpr(1));
  writer.emit_exit();
  auto const module = builder.build();

  auto vm = benson::vm::Virtual_machine{};
  vm.load(module);
  vm.registers.resize(2);
  vm.set_register_value<std::int64_t>(gpr(1), std::int64_t{-16});
  auto const initial = vm.stack_pointer;

  vm.run();

  CHECK(vm.stack_pointer == initial - 32 + 16);
}

TEST_CASE("Virtual_machine mov_sp_i produces a stack pointer at sp + offset")
{
  using benson::bytecode::Immediate;
  using benson::bytecode::Module_builder;
  using benson::vm::Address_space;
  using benson::vm::Pointer;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  writer.emit_alloca_i(Immediate{32});
  writer.emit_mov_sp_i(Immediate{0}, gpr(1));
  writer.emit_mov_sp_i(Immediate{8}, gpr(2));
  writer.emit_mov_sp_i(Immediate{-4}, gpr(3));
  writer.emit_exit();
  auto const module = builder.build();

  auto vm = benson::vm::Virtual_machine{};
  vm.load(module);
  vm.registers.resize(4);

  vm.run();

  auto const p1 = vm.get_register_value<Pointer>(gpr(1)).decode();
  auto const p2 = vm.get_register_value<Pointer>(gpr(2)).decode();
  auto const p3 = vm.get_register_value<Pointer>(gpr(3)).decode();
  CHECK(p1.space == Address_space::stack);
  CHECK(p1.offset == static_cast<std::uint64_t>(vm.stack_pointer));
  CHECK(p2.space == Address_space::stack);
  CHECK(p2.offset == static_cast<std::uint64_t>(vm.stack_pointer + 8));
  CHECK(p3.space == Address_space::stack);
  CHECK(p3.offset == static_cast<std::uint64_t>(vm.stack_pointer - 4));
}

TEST_CASE("Virtual_machine store_sp_8 then load_sp_8 round-trips a value")
{
  using benson::bytecode::Immediate;
  using benson::bytecode::Module_builder;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  writer.emit_alloca_i(Immediate{16});
  writer.emit_store_sp_8(Immediate{0}, gpr(1));
  writer.emit_load_sp_8(Immediate{0}, gpr(2));
  writer.emit_exit();
  auto const module = builder.build();

  auto vm = benson::vm::Virtual_machine{};
  vm.load(module);
  vm.registers.resize(3);
  vm.set_register_value<std::int8_t>(gpr(1), std::int8_t{0x42});

  vm.run();

  CHECK(vm.get_register_value<std::int8_t>(gpr(2)) == std::int8_t{0x42});
}

TEST_CASE("Virtual_machine store_sp_16 then load_sp_16 round-trips a value")
{
  using benson::bytecode::Immediate;
  using benson::bytecode::Module_builder;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  writer.emit_alloca_i(Immediate{16});
  writer.emit_store_sp_16(Immediate{0}, gpr(1));
  writer.emit_load_sp_16(Immediate{0}, gpr(2));
  writer.emit_exit();
  auto const module = builder.build();

  auto vm = benson::vm::Virtual_machine{};
  vm.load(module);
  vm.registers.resize(3);
  vm.set_register_value<std::int16_t>(gpr(1), std::int16_t{0x1234});

  vm.run();

  CHECK(vm.get_register_value<std::int16_t>(gpr(2)) == std::int16_t{0x1234});
}

TEST_CASE("Virtual_machine store_sp_32 then load_sp_32 round-trips a value")
{
  using benson::bytecode::Immediate;
  using benson::bytecode::Module_builder;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  writer.emit_alloca_i(Immediate{16});
  writer.emit_store_sp_32(Immediate{0}, gpr(1));
  writer.emit_load_sp_32(Immediate{0}, gpr(2));
  writer.emit_exit();
  auto const module = builder.build();

  auto vm = benson::vm::Virtual_machine{};
  vm.load(module);
  vm.registers.resize(3);
  vm.set_register_value<std::int32_t>(gpr(1), std::int32_t{0x12345678});

  vm.run();

  CHECK(
    vm.get_register_value<std::int32_t>(gpr(2)) == std::int32_t{0x12345678}
  );
}

TEST_CASE("Virtual_machine store_sp_64 then load_sp_64 round-trips a value")
{
  using benson::bytecode::Immediate;
  using benson::bytecode::Module_builder;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  writer.emit_alloca_i(Immediate{16});
  writer.emit_store_sp_64(Immediate{0}, gpr(1));
  writer.emit_load_sp_64(Immediate{0}, gpr(2));
  writer.emit_exit();
  auto const module = builder.build();

  auto vm = benson::vm::Virtual_machine{};
  vm.load(module);
  vm.registers.resize(3);
  auto const value = std::int64_t{0x0102030405060708LL};
  vm.set_register_value<std::int64_t>(gpr(1), value);

  vm.run();

  CHECK(vm.get_register_value<std::int64_t>(gpr(2)) == value);
}

TEST_CASE("Virtual_machine load_sp_N sign-extends negative values")
{
  using benson::bytecode::Immediate;
  using benson::bytecode::Module_builder;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  writer.emit_alloca_i(Immediate{16});
  writer.emit_store_sp_8(Immediate{0}, gpr(1));
  writer.emit_load_sp_8(Immediate{0}, gpr(11));
  writer.emit_store_sp_16(Immediate{0}, gpr(2));
  writer.emit_load_sp_16(Immediate{0}, gpr(12));
  writer.emit_store_sp_32(Immediate{0}, gpr(3));
  writer.emit_load_sp_32(Immediate{0}, gpr(13));
  writer.emit_exit();
  auto const module = builder.build();

  auto vm = benson::vm::Virtual_machine{};
  vm.load(module);
  vm.registers.resize(14);
  vm.set_register_value<std::int8_t>(gpr(1), std::int8_t{-1});
  vm.set_register_value<std::int16_t>(gpr(2), std::int16_t{-2});
  vm.set_register_value<std::int32_t>(gpr(3), std::int32_t{-3});

  vm.run();

  CHECK(
    vm.get_register_value<std::uint64_t>(gpr(11)) ==
    static_cast<std::uint64_t>(std::int64_t{-1})
  );
  CHECK(
    vm.get_register_value<std::uint64_t>(gpr(12)) ==
    static_cast<std::uint64_t>(std::int64_t{-2})
  );
  CHECK(
    vm.get_register_value<std::uint64_t>(gpr(13)) ==
    static_cast<std::uint64_t>(std::int64_t{-3})
  );
}
