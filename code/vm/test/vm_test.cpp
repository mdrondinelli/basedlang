#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

#include "vm/pointer.h"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

#include "bytecode/bytecode_writer.h"
#include "bytecode/module_builder.h"
#include "vm/vm.h"

namespace
{

  class Recording_binary_output_stream: public benson::Binary_output_stream
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
    benson::bytecode::Wide_constant index,
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

  auto vm = benson::Virtual_machine{};
  vm.set_register_value(Register::gpr_255, value);

  CHECK(vm.get_register_value<TestType>(Register::gpr_255) == value);
}

TEST_CASE("Virtual_machine keeps exact floating-point register bit pattern")
{
  auto vm = benson::Virtual_machine{};
  vm.set_register_value(benson::bytecode::Register::gpr_1, -0.0F);

  CHECK(
    std::bit_cast<std::uint32_t>(
      vm.get_register_value<float>(benson::bytecode::Register::gpr_1)
    ) == std::bit_cast<std::uint32_t>(-0.0F)
  );
}

TEST_CASE(
  "Virtual_machine runs lookup constant program emitted by Module_builder"
)
{
  using benson::Address_space;
  using benson::Pointer;
  using benson::bytecode::Module_builder;
  using benson::bytecode::Register;

  auto builder = Module_builder{};
  builder.emit_lookup_k(Register::gpr_1, std::int32_t{5});
  builder.emit_lookup_k(Register::gpr_2, 2.5);
  builder.emit_exit();
  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);

  vm.run();

  auto const p1 =
    Pointer{vm.get_register_value<std::uint64_t>(Register::gpr_1)}.decode();
  auto const p2 =
    Pointer{vm.get_register_value<std::uint64_t>(Register::gpr_2)}.decode();

  CHECK(p1.space == Address_space::constant);
  CHECK(p1.offset == static_cast<std::uint64_t>(module.constant_table[0]));
  CHECK(p2.space == Address_space::constant);
  CHECK(p2.offset == static_cast<std::uint64_t>(module.constant_table[1]));
  CHECK(vm.instruction_pointer == module.code.data() + 7);
}

TEST_CASE("Virtual_machine runs negation program emitted by Module_builder")
{
  using benson::bytecode::Module_builder;
  using benson::bytecode::Register;

  auto builder = Module_builder{};
  builder.emit_neg_i32(Register::gpr_1, Register::gpr_2);
  builder.emit_exit();
  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);
  vm.set_register_value<std::int32_t>(Register::gpr_2, 123);

  vm.run();

  CHECK(vm.get_register_value<std::int32_t>(Register::gpr_1) == -123);
  CHECK(vm.get_register_value<std::int32_t>(Register::gpr_2) == 123);
  CHECK(vm.instruction_pointer == module.code.data() + 4);
}

TEST_CASE(
  "Virtual_machine runs integer arithmetic program emitted by Module_builder"
)
{
  using benson::bytecode::Module_builder;
  using benson::bytecode::Register;

  auto builder = Module_builder{};
  builder.emit_add_i32(Register::gpr_1, Register::gpr_2, Register::gpr_3);
  builder.emit_sub_i32(Register::gpr_4, Register::gpr_1, Register::gpr_5);
  builder.emit_mul_i32(Register::gpr_6, Register::gpr_4, Register::gpr_7);
  builder.emit_div_i32(Register::gpr_8, Register::gpr_6, Register::gpr_9);
  builder.emit_mod_i32(Register::gpr_10, Register::gpr_8, Register::gpr_11);
  builder.emit_exit();
  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);
  vm.set_register_value<std::int32_t>(Register::gpr_2, 10);
  vm.set_register_value<std::int32_t>(Register::gpr_3, 5);
  vm.set_register_value<std::int32_t>(Register::gpr_5, 3);
  vm.set_register_value<std::int32_t>(Register::gpr_7, 4);
  vm.set_register_value<std::int32_t>(Register::gpr_9, 6);
  vm.set_register_value<std::int32_t>(Register::gpr_11, 7);

  vm.run();

  CHECK(vm.get_register_value<std::int32_t>(Register::gpr_1) == 15);
  CHECK(vm.get_register_value<std::int32_t>(Register::gpr_4) == 12);
  CHECK(vm.get_register_value<std::int32_t>(Register::gpr_6) == 48);
  CHECK(vm.get_register_value<std::int32_t>(Register::gpr_8) == 8);
  CHECK(vm.get_register_value<std::int32_t>(Register::gpr_10) == 1);
  CHECK(vm.instruction_pointer == module.code.data() + 21);
}

TEST_CASE(
  "Virtual_machine runs integer immediate arithmetic program emitted by "
  "Module_builder"
)
{
  using benson::bytecode::Module_builder;
  using benson::bytecode::Register;
  using benson::bytecode::Wide_immediate;

  auto builder = Module_builder{};
  builder.emit_add_i32_i(Register::gpr_1, Register::gpr_2, Wide_immediate{5});
  builder.emit_sub_i32_i(Register::gpr_3, Register::gpr_1, Wide_immediate{-3});
  builder.emit_mul_i32_i(Register::gpr_4, Register::gpr_3, Wide_immediate{0x0102});
  builder.emit_div_i32_i(Register::gpr_5, Register::gpr_4, Wide_immediate{6});
  builder.emit_mod_i32_i(Register::gpr_6, Register::gpr_5, Wide_immediate{7});
  builder.emit_exit();
  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);
  vm.set_register_value<std::int32_t>(Register::gpr_2, 10);

  vm.run();

  CHECK(vm.get_register_value<std::int32_t>(Register::gpr_1) == 15);
  CHECK(vm.get_register_value<std::int32_t>(Register::gpr_3) == 18);
  CHECK(vm.get_register_value<std::int32_t>(Register::gpr_4) == 4644);
  CHECK(vm.get_register_value<std::int32_t>(Register::gpr_5) == 774);
  CHECK(vm.get_register_value<std::int32_t>(Register::gpr_6) == 4);
  CHECK(vm.instruction_pointer == module.code.data() + 23);
}

TEST_CASE(
  "Virtual_machine runs floating-point arithmetic program emitted by "
  "Module_builder"
)
{
  using benson::bytecode::Module_builder;
  using benson::bytecode::Register;

  auto builder = Module_builder{};
  builder.emit_add_f64(Register::gpr_1, Register::gpr_2, Register::gpr_3);
  builder.emit_sub_f64(Register::gpr_4, Register::gpr_1, Register::gpr_5);
  builder.emit_mul_f64(Register::gpr_6, Register::gpr_4, Register::gpr_7);
  builder.emit_div_f64(Register::gpr_8, Register::gpr_6, Register::gpr_9);
  builder.emit_exit();
  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);
  vm.set_register_value<double>(Register::gpr_2, 10.0);
  vm.set_register_value<double>(Register::gpr_3, 2.5);
  vm.set_register_value<double>(Register::gpr_5, 1.5);
  vm.set_register_value<double>(Register::gpr_7, 4.0);
  vm.set_register_value<double>(Register::gpr_9, 2.0);

  vm.run();

  CHECK(vm.get_register_value<double>(Register::gpr_1) == 12.5);
  CHECK(vm.get_register_value<double>(Register::gpr_4) == 11.0);
  CHECK(vm.get_register_value<double>(Register::gpr_6) == 44.0);
  CHECK(vm.get_register_value<double>(Register::gpr_8) == 22.0);
  CHECK(vm.instruction_pointer == module.code.data() + 17);
}

TEST_CASE(
  "Virtual_machine runs narrow constant arithmetic program emitted by "
  "Module_builder"
)
{
  using benson::bytecode::Module_builder;
  using benson::bytecode::Register;

  auto builder = Module_builder{};
  builder.emit_add_i32_k(Register::gpr_1, Register::gpr_2, 5);
  builder.emit_sub_i32_k(Register::gpr_4, Register::gpr_1, 3);
  builder.emit_mul_i32_k(Register::gpr_6, Register::gpr_4, 4);
  builder.emit_div_i32_k(Register::gpr_8, Register::gpr_6, 6);
  builder.emit_mod_i32_k(Register::gpr_10, Register::gpr_8, 7);
  builder.emit_add_f64_k(Register::gpr_12, Register::gpr_14, 2.5);
  builder.emit_sub_f64_k(Register::gpr_16, Register::gpr_12, 1.5);
  builder.emit_mul_f64_k(Register::gpr_18, Register::gpr_16, 4.0);
  builder.emit_div_f64_k(Register::gpr_20, Register::gpr_18, 2.0);
  builder.emit_exit();
  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);
  vm.set_register_value<std::int32_t>(Register::gpr_2, 10);
  vm.set_register_value<double>(Register::gpr_14, 10.0);

  vm.run();

  CHECK(vm.get_register_value<std::int32_t>(Register::gpr_1) == 15);
  CHECK(vm.get_register_value<std::int32_t>(Register::gpr_4) == 12);
  CHECK(vm.get_register_value<std::int32_t>(Register::gpr_6) == 48);
  CHECK(vm.get_register_value<std::int32_t>(Register::gpr_8) == 8);
  CHECK(vm.get_register_value<std::int32_t>(Register::gpr_10) == 1);
  CHECK(vm.get_register_value<double>(Register::gpr_12) == 12.5);
  CHECK(vm.get_register_value<double>(Register::gpr_16) == 11.0);
  CHECK(vm.get_register_value<double>(Register::gpr_18) == 44.0);
  CHECK(vm.get_register_value<double>(Register::gpr_20) == 22.0);
  CHECK(vm.instruction_pointer == module.code.data() + 37);
}

TEST_CASE(
  "Virtual_machine runs wide constant arithmetic program emitted by "
  "Bytecode_writer"
)
{
  using benson::bytecode::Register;
  using benson::bytecode::Wide_constant;

  auto constant_memory = std::vector<std::byte>{};
  auto constant_table = std::vector<std::ptrdiff_t>{};
  store_constant<std::int32_t>(constant_memory, constant_table, Wide_constant{0x0304}, 5);
  store_constant<std::int32_t>(constant_memory, constant_table, Wide_constant{0x0305}, 3);
  store_constant<std::int32_t>(constant_memory, constant_table, Wide_constant{0x0306}, 4);
  store_constant<std::int32_t>(constant_memory, constant_table, Wide_constant{0x0307}, 6);
  store_constant<std::int32_t>(constant_memory, constant_table, Wide_constant{0x0308}, 7);
  store_constant<double>(constant_memory, constant_table, Wide_constant{0x0309}, 2.5);
  store_constant<double>(constant_memory, constant_table, Wide_constant{0x030A}, 1.5);
  store_constant<double>(constant_memory, constant_table, Wide_constant{0x030B}, 4.0);
  store_constant<double>(constant_memory, constant_table, Wide_constant{0x030C}, 2.0);

  auto stream = Recording_binary_output_stream{};
  auto writer = benson::bytecode::Bytecode_writer{&stream};
  writer.emit_add_i32_k(Register::gpr_1, Register::gpr_2, Wide_constant{0x0304});
  writer.emit_sub_i32_k(Register::gpr_4, Register::gpr_1, Wide_constant{0x0305});
  writer.emit_mul_i32_k(Register::gpr_6, Register::gpr_4, Wide_constant{0x0306});
  writer.emit_div_i32_k(Register::gpr_8, Register::gpr_6, Wide_constant{0x0307});
  writer.emit_mod_i32_k(Register::gpr_10, Register::gpr_8, Wide_constant{0x0308});
  writer.emit_add_f64_k(Register::gpr_12, Register::gpr_14, Wide_constant{0x0309});
  writer.emit_sub_f64_k(Register::gpr_16, Register::gpr_12, Wide_constant{0x030A});
  writer.emit_mul_f64_k(Register::gpr_18, Register::gpr_16, Wide_constant{0x030B});
  writer.emit_div_f64_k(Register::gpr_20, Register::gpr_18, Wide_constant{0x030C});
  writer.emit_exit();
  writer.flush();

  auto vm = benson::Virtual_machine{};
  vm.instruction_pointer = stream.bytes().data();
  vm.constant_memory = constant_memory.data();
  vm.constant_table = constant_table.data();
  vm.set_register_value<std::int32_t>(Register::gpr_2, 10);
  vm.set_register_value<double>(Register::gpr_14, 10.0);

  vm.run();

  CHECK(vm.get_register_value<std::int32_t>(Register::gpr_1) == 15);
  CHECK(vm.get_register_value<std::int32_t>(Register::gpr_4) == 12);
  CHECK(vm.get_register_value<std::int32_t>(Register::gpr_6) == 48);
  CHECK(vm.get_register_value<std::int32_t>(Register::gpr_8) == 8);
  CHECK(vm.get_register_value<std::int32_t>(Register::gpr_10) == 1);
  CHECK(vm.get_register_value<double>(Register::gpr_12) == 12.5);
  CHECK(vm.get_register_value<double>(Register::gpr_16) == 11.0);
  CHECK(vm.get_register_value<double>(Register::gpr_18) == 44.0);
  CHECK(vm.get_register_value<double>(Register::gpr_20) == 22.0);
  CHECK(vm.instruction_pointer == stream.bytes().data() + 55);
}

TEST_CASE("Virtual_machine store_8 writes one byte to stack")
{
  using benson::Address_space;
  using benson::Pointer;
  using benson::bytecode::Module_builder;
  using benson::bytecode::Register;
  using benson::bytecode::Wide_immediate;

  auto builder = Module_builder{};
  builder.emit_store_8(Register::gpr_1, Register::gpr_2, Wide_immediate{0});
  builder.emit_exit();
  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);
  vm.set_register_value<std::uint64_t>(
    Register::gpr_1,
    static_cast<std::uint64_t>(std::int8_t{-42})
  );
  vm.set_register_value<std::uint64_t>(
    Register::gpr_2,
    static_cast<std::uint64_t>(Pointer{Address_space::stack, 0})
  );

  vm.run();

  CHECK(
    (*vm.stack)[0] == static_cast<std::byte>(static_cast<std::uint8_t>(-42))
  );
}

TEST_CASE("Virtual_machine load_8 reads one byte from stack")
{
  using benson::Address_space;
  using benson::Pointer;
  using benson::bytecode::Module_builder;
  using benson::bytecode::Register;
  using benson::bytecode::Wide_immediate;

  auto builder = Module_builder{};
  builder.emit_load_8(Register::gpr_1, Register::gpr_2, Wide_immediate{0});
  builder.emit_exit();
  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);
  (*vm.stack)[0] = static_cast<std::byte>(std::uint8_t{0xAB});
  vm.set_register_value<std::uint64_t>(
    Register::gpr_2,
    static_cast<std::uint64_t>(Pointer{Address_space::stack, 0})
  );

  vm.run();

  CHECK(vm.get_register_value<std::uint8_t>(Register::gpr_1) == 0xAB);
}

TEST_CASE("Virtual_machine store_64 and load_64 round-trip a 64-bit value")
{
  using benson::Address_space;
  using benson::Pointer;
  using benson::bytecode::Module_builder;
  using benson::bytecode::Register;
  using benson::bytecode::Wide_immediate;

  auto builder = Module_builder{};
  builder.emit_store_64(Register::gpr_1, Register::gpr_3, Wide_immediate{0});
  builder.emit_load_64(Register::gpr_2, Register::gpr_3, Wide_immediate{0});
  builder.emit_exit();
  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);
  vm.set_register_value<std::int64_t>(
    Register::gpr_1,
    std::int64_t{-1234567890123LL}
  );
  vm.set_register_value<std::uint64_t>(
    Register::gpr_3,
    static_cast<std::uint64_t>(Pointer{Address_space::stack, 0})
  );

  vm.run();

  CHECK(
    vm.get_register_value<std::int64_t>(Register::gpr_2) ==
    std::int64_t{-1234567890123LL}
  );
}

TEST_CASE("Virtual_machine load_32 and store_32 respect immediate offset")
{
  using benson::Address_space;
  using benson::Pointer;
  using benson::bytecode::Module_builder;
  using benson::bytecode::Register;
  using benson::bytecode::Wide_immediate;

  auto builder = Module_builder{};
  builder.emit_store_32(Register::gpr_1, Register::gpr_3, Wide_immediate{8});
  builder.emit_load_32(Register::gpr_2, Register::gpr_3, Wide_immediate{8});
  builder.emit_exit();
  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);
  vm.set_register_value<std::int32_t>(Register::gpr_1, std::int32_t{0xDEAD});
  vm.set_register_value<std::uint64_t>(
    Register::gpr_3,
    static_cast<std::uint64_t>(Pointer{Address_space::stack, 0})
  );

  vm.run();

  CHECK(
    vm.get_register_value<std::int32_t>(Register::gpr_2) == std::int32_t{0xDEAD}
  );
}

TEST_CASE("Virtual_machine load_32 reads from constant memory via pointer")
{
  using benson::bytecode::Module_builder;
  using benson::bytecode::Register;
  using benson::bytecode::Wide_immediate;

  auto builder = Module_builder{};
  builder.emit_lookup_k(Register::gpr_2, std::int32_t{0x1234ABCD});
  builder.emit_load_32(Register::gpr_1, Register::gpr_2, Wide_immediate{0});
  builder.emit_exit();
  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);

  vm.run();

  CHECK(
    vm.get_register_value<std::int32_t>(Register::gpr_1) ==
    std::int32_t{0x1234ABCD}
  );
}

TEST_CASE("Virtual_machine runs Module_builder program with forward jmp_i")
{
  using benson::bytecode::Module_builder;
  using benson::bytecode::Register;
  using benson::bytecode::Wide_immediate;

  auto builder = Module_builder{};
  auto const done = builder.make_label();
  builder.emit_jmp_i(done);
  builder.emit_add_i32_i(Register::gpr_1, Register::gpr_1, Wide_immediate{1});
  builder.place_label(done);
  builder.emit_add_i32_i(Register::gpr_1, Register::gpr_1, Wide_immediate{2});
  builder.emit_exit();

  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);
  vm.set_register_value<std::int32_t>(Register::gpr_1, 40);

  vm.run();

  CHECK(vm.get_register_value<std::int32_t>(Register::gpr_1) == 42);
}

TEST_CASE("Virtual_machine runs Module_builder program with inline constants")
{
  using benson::bytecode::Module_builder;
  using benson::bytecode::Register;

  auto builder = Module_builder{};
  builder.emit_add_i32_k(Register::gpr_1, Register::gpr_1, 5);
  builder.emit_mul_f32_k(Register::gpr_2, Register::gpr_2, 0.5F);
  builder.emit_add_f32_k(Register::gpr_2, Register::gpr_2, 0.5F);
  builder.emit_exit();

  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);
  vm.set_register_value<std::int32_t>(Register::gpr_1, 37);
  vm.set_register_value<float>(Register::gpr_2, 3.0F);

  vm.run();

  CHECK(vm.get_register_value<std::int32_t>(Register::gpr_1) == 42);
  CHECK(vm.get_register_value<float>(Register::gpr_2) == 2.0F);
  CHECK(module.constant_table.size() == 2);
}

TEST_CASE(
  "Virtual_machine load_16 and store_16 use wide offset when offset > 255"
)
{
  using benson::Address_space;
  using benson::Pointer;
  using benson::bytecode::Register;
  using benson::bytecode::Wide_immediate;

  auto stream = Recording_binary_output_stream{};
  auto writer = benson::bytecode::Bytecode_writer{&stream};
  // offset 0x0101 = 257, which exceeds Immediate max of 127 → wide encoding
  writer.emit_store_16(Register::gpr_1, Register::gpr_3, Wide_immediate{0x0101});
  writer.emit_load_16(Register::gpr_2, Register::gpr_3, Wide_immediate{0x0101});
  writer.emit_exit();
  writer.flush();

  auto vm = benson::Virtual_machine{};
  vm.instruction_pointer = stream.bytes().data();
  vm.set_register_value<std::int16_t>(Register::gpr_1, std::int16_t{0x7FFF});
  vm.set_register_value<std::uint64_t>(
    Register::gpr_3,
    static_cast<std::uint64_t>(Pointer{Address_space::stack, 0})
  );

  vm.run();

  CHECK(
    vm.get_register_value<std::int16_t>(Register::gpr_2) == std::int16_t{0x7FFF}
  );
}
