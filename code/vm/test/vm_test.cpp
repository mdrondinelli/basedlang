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
  using enum benson::bytecode::Register;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  writer.emit_lookup_k(gpr_1, builder.constant(5));
  writer.emit_lookup_k(gpr_2, builder.constant(2.5));
  writer.emit_exit();
  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);

  vm.run();

  auto const p1 = vm.get_register_value<Pointer>(gpr_1).decode();
  auto const p2 = vm.get_register_value<Pointer>(gpr_2).decode();

  CHECK(p1.space == Address_space::constant);
  CHECK(p1.offset == static_cast<std::uint64_t>(module.constant_table[0]));
  CHECK(p2.space == Address_space::constant);
  CHECK(p2.offset == static_cast<std::uint64_t>(module.constant_table[1]));
  CHECK(vm.instruction_pointer == module.code.data() + 7);
}

TEST_CASE("Virtual_machine runs negation program emitted by Module_builder")
{
  using benson::bytecode::Module_builder;
  using enum benson::bytecode::Register;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  writer.emit_neg_i32(gpr_1, gpr_2);
  writer.emit_exit();
  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);
  vm.set_register_value<std::int32_t>(gpr_2, 123);

  vm.run();

  CHECK(vm.get_register_value<std::int32_t>(gpr_1) == -123);
  CHECK(vm.get_register_value<std::int32_t>(gpr_2) == 123);
  CHECK(vm.instruction_pointer == module.code.data() + 4);
}

TEST_CASE(
  "Virtual_machine runs integer arithmetic program emitted by Module_builder"
)
{
  using benson::bytecode::Module_builder;
  using enum benson::bytecode::Register;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  writer.emit_add_i32(gpr_1, gpr_2, gpr_3);
  writer.emit_sub_i32(gpr_4, gpr_1, gpr_5);
  writer.emit_mul_i32(gpr_6, gpr_4, gpr_7);
  writer.emit_div_i32(gpr_8, gpr_6, gpr_9);
  writer.emit_mod_i32(gpr_10, gpr_8, gpr_11);
  writer.emit_exit();
  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);
  vm.set_register_value<std::int32_t>(gpr_2, 10);
  vm.set_register_value<std::int32_t>(gpr_3, 5);
  vm.set_register_value<std::int32_t>(gpr_5, 3);
  vm.set_register_value<std::int32_t>(gpr_7, 4);
  vm.set_register_value<std::int32_t>(gpr_9, 6);
  vm.set_register_value<std::int32_t>(gpr_11, 7);

  vm.run();

  CHECK(vm.get_register_value<std::int32_t>(gpr_1) == 15);
  CHECK(vm.get_register_value<std::int32_t>(gpr_4) == 12);
  CHECK(vm.get_register_value<std::int32_t>(gpr_6) == 48);
  CHECK(vm.get_register_value<std::int32_t>(gpr_8) == 8);
  CHECK(vm.get_register_value<std::int32_t>(gpr_10) == 1);
  CHECK(vm.instruction_pointer == module.code.data() + 21);
}

TEST_CASE(
  "Virtual_machine runs integer immediate arithmetic program emitted by "
  "Module_builder"
)
{
  using benson::bytecode::Module_builder;
  using benson::bytecode::Wide_immediate;
  using enum benson::bytecode::Register;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  writer.emit_add_i32_i(gpr_1, gpr_2, Wide_immediate{5});
  writer.emit_sub_i32_i(gpr_3, gpr_1, Wide_immediate{-3});
  writer.emit_mul_i32_i(gpr_4, gpr_3, Wide_immediate{0x0102});
  writer.emit_div_i32_i(gpr_5, gpr_4, Wide_immediate{6});
  writer.emit_mod_i32_i(gpr_6, gpr_5, Wide_immediate{7});
  writer.emit_exit();
  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);
  vm.set_register_value<std::int32_t>(gpr_2, 10);

  vm.run();

  CHECK(vm.get_register_value<std::int32_t>(gpr_1) == 15);
  CHECK(vm.get_register_value<std::int32_t>(gpr_3) == 18);
  CHECK(vm.get_register_value<std::int32_t>(gpr_4) == 4644);
  CHECK(vm.get_register_value<std::int32_t>(gpr_5) == 774);
  CHECK(vm.get_register_value<std::int32_t>(gpr_6) == 4);
  CHECK(vm.instruction_pointer == module.code.data() + 23);
}

TEST_CASE(
  "Virtual_machine runs floating-point arithmetic program emitted by "
  "Module_builder"
)
{
  using benson::bytecode::Module_builder;
  using enum benson::bytecode::Register;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  writer.emit_add_f64(gpr_1, gpr_2, gpr_3);
  writer.emit_sub_f64(gpr_4, gpr_1, gpr_5);
  writer.emit_mul_f64(gpr_6, gpr_4, gpr_7);
  writer.emit_div_f64(gpr_8, gpr_6, gpr_9);
  writer.emit_exit();
  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);
  vm.set_register_value<double>(gpr_2, 10.0);
  vm.set_register_value<double>(gpr_3, 2.5);
  vm.set_register_value<double>(gpr_5, 1.5);
  vm.set_register_value<double>(gpr_7, 4.0);
  vm.set_register_value<double>(gpr_9, 2.0);

  vm.run();

  CHECK(vm.get_register_value<double>(gpr_1) == 12.5);
  CHECK(vm.get_register_value<double>(gpr_4) == 11.0);
  CHECK(vm.get_register_value<double>(gpr_6) == 44.0);
  CHECK(vm.get_register_value<double>(gpr_8) == 22.0);
  CHECK(vm.instruction_pointer == module.code.data() + 17);
}

TEST_CASE(
  "Virtual_machine runs narrow constant arithmetic program emitted by "
  "Module_builder"
)
{
  using benson::bytecode::Module_builder;
  using enum benson::bytecode::Register;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  writer.emit_add_i32_k(gpr_1, gpr_2, builder.constant(5));
  writer.emit_sub_i32_k(gpr_4, gpr_1, builder.constant(3));
  writer.emit_mul_i32_k(gpr_6, gpr_4, builder.constant(4));
  writer.emit_div_i32_k(gpr_8, gpr_6, builder.constant(6));
  writer.emit_mod_i32_k(gpr_10, gpr_8, builder.constant(7));
  writer.emit_add_f64_k(gpr_12, gpr_14, builder.constant(2.5));
  writer.emit_sub_f64_k(gpr_16, gpr_12, builder.constant(1.5));
  writer.emit_mul_f64_k(gpr_18, gpr_16, builder.constant(4.0));
  writer.emit_div_f64_k(gpr_20, gpr_18, builder.constant(2.0));
  writer.emit_exit();
  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);
  vm.set_register_value<std::int32_t>(gpr_2, 10);
  vm.set_register_value<double>(gpr_14, 10.0);

  vm.run();

  CHECK(vm.get_register_value<std::int32_t>(gpr_1) == 15);
  CHECK(vm.get_register_value<std::int32_t>(gpr_4) == 12);
  CHECK(vm.get_register_value<std::int32_t>(gpr_6) == 48);
  CHECK(vm.get_register_value<std::int32_t>(gpr_8) == 8);
  CHECK(vm.get_register_value<std::int32_t>(gpr_10) == 1);
  CHECK(vm.get_register_value<double>(gpr_12) == 12.5);
  CHECK(vm.get_register_value<double>(gpr_16) == 11.0);
  CHECK(vm.get_register_value<double>(gpr_18) == 44.0);
  CHECK(vm.get_register_value<double>(gpr_20) == 22.0);
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
  store_constant<std::int32_t>(
    constant_memory,
    constant_table,
    Wide_constant{0x0304},
    5
  );
  store_constant<std::int32_t>(
    constant_memory,
    constant_table,
    Wide_constant{0x0305},
    3
  );
  store_constant<std::int32_t>(
    constant_memory,
    constant_table,
    Wide_constant{0x0306},
    4
  );
  store_constant<std::int32_t>(
    constant_memory,
    constant_table,
    Wide_constant{0x0307},
    6
  );
  store_constant<std::int32_t>(
    constant_memory,
    constant_table,
    Wide_constant{0x0308},
    7
  );
  store_constant<double>(
    constant_memory,
    constant_table,
    Wide_constant{0x0309},
    2.5
  );
  store_constant<double>(
    constant_memory,
    constant_table,
    Wide_constant{0x030A},
    1.5
  );
  store_constant<double>(
    constant_memory,
    constant_table,
    Wide_constant{0x030B},
    4.0
  );
  store_constant<double>(
    constant_memory,
    constant_table,
    Wide_constant{0x030C},
    2.0
  );

  auto stream = Recording_binary_output_stream{};
  auto writer = benson::bytecode::Bytecode_writer{&stream};
  writer
    .emit_add_i32_k(Register::gpr_1, Register::gpr_2, Wide_constant{0x0304});
  writer
    .emit_sub_i32_k(Register::gpr_4, Register::gpr_1, Wide_constant{0x0305});
  writer
    .emit_mul_i32_k(Register::gpr_6, Register::gpr_4, Wide_constant{0x0306});
  writer
    .emit_div_i32_k(Register::gpr_8, Register::gpr_6, Wide_constant{0x0307});
  writer
    .emit_mod_i32_k(Register::gpr_10, Register::gpr_8, Wide_constant{0x0308});
  writer
    .emit_add_f64_k(Register::gpr_12, Register::gpr_14, Wide_constant{0x0309});
  writer
    .emit_sub_f64_k(Register::gpr_16, Register::gpr_12, Wide_constant{0x030A});
  writer
    .emit_mul_f64_k(Register::gpr_18, Register::gpr_16, Wide_constant{0x030B});
  writer
    .emit_div_f64_k(Register::gpr_20, Register::gpr_18, Wide_constant{0x030C});
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
  auto &writer = builder.writer();
  writer.emit_store_8(Register::gpr_1, Register::gpr_2, Wide_immediate{0});
  writer.emit_exit();
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
  auto &writer = builder.writer();
  writer.emit_load_8(Register::gpr_1, Register::gpr_2, Wide_immediate{0});
  writer.emit_exit();
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
  auto &writer = builder.writer();
  writer.emit_store_64(Register::gpr_1, Register::gpr_3, Wide_immediate{0});
  writer.emit_load_64(Register::gpr_2, Register::gpr_3, Wide_immediate{0});
  writer.emit_exit();
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
  auto &writer = builder.writer();
  writer.emit_store_32(Register::gpr_1, Register::gpr_3, Wide_immediate{8});
  writer.emit_load_32(Register::gpr_2, Register::gpr_3, Wide_immediate{8});
  writer.emit_exit();
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
  using benson::bytecode::Wide_immediate;
  using enum benson::bytecode::Register;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  writer.emit_lookup_k(gpr_2, builder.constant(std::int32_t{0x1234ABCD}));
  writer.emit_load_32(gpr_1, gpr_2, Wide_immediate{0});
  writer.emit_exit();
  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);

  vm.run();

  CHECK(vm.get_register_value<std::int32_t>(gpr_1) == std::int32_t{0x1234ABCD});
}

TEST_CASE("Virtual_machine runs Module_builder program with forward jmp_i")
{
  using benson::bytecode::Module_builder;
  using benson::bytecode::Register;
  using benson::bytecode::Wide_immediate;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  auto const done = builder.make_label();
  writer.emit_jmp(builder.label_target(done));
  writer.emit_add_i32_i(Register::gpr_1, Register::gpr_1, Wide_immediate{1});
  builder.place_label(done);
  writer.emit_add_i32_i(Register::gpr_1, Register::gpr_1, Wide_immediate{2});
  writer.emit_exit();

  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);
  vm.set_register_value<std::int32_t>(Register::gpr_1, 40);

  vm.run();

  CHECK(vm.get_register_value<std::int32_t>(Register::gpr_1) == 42);
}

TEST_CASE("Virtual_machine takes narrow call_i and ret")
{
  using benson::bytecode::Register;
  using benson::bytecode::Wide_immediate;

  auto stream = Recording_binary_output_stream{};
  auto writer = benson::bytecode::Bytecode_writer{&stream};
  writer.emit_call(std::ptrdiff_t{7});
  writer.emit_add_i32_i(Register::gpr_1, Register::gpr_1, Wide_immediate{100});
  writer.emit_exit();
  writer.emit_add_i32_i(Register::gpr_1, Register::gpr_1, Wide_immediate{2});
  writer.emit_ret();
  writer.flush();

  auto vm = benson::Virtual_machine{};
  vm.instruction_pointer = stream.bytes().data();
  vm.set_register_value<std::int32_t>(Register::gpr_1, 40);

  vm.run();

  CHECK(vm.get_register_value<std::int32_t>(Register::gpr_1) == 142);
  CHECK(
    vm.get_register_value<benson::Pointer>(Register::sp).decode().offset ==
    vm.stack->size()
  );
}

TEST_CASE("Virtual_machine takes wide call_i and ret")
{
  using benson::bytecode::Register;
  using benson::bytecode::Wide_immediate;

  auto stream = Recording_binary_output_stream{};
  auto writer = benson::bytecode::Bytecode_writer{&stream};
  writer.emit_call(std::ptrdiff_t{165});
  writer.emit_add_i32_i(Register::gpr_1, Register::gpr_1, Wide_immediate{100});
  writer.emit_exit();
  for (auto i = 0; i < 39; ++i)
  {
    writer.emit_add_i32_i(Register::gpr_2, Register::gpr_2, Wide_immediate{1});
  }
  writer.emit_add_i32_i(Register::gpr_1, Register::gpr_1, Wide_immediate{2});
  writer.emit_ret();
  writer.flush();

  auto vm = benson::Virtual_machine{};
  vm.instruction_pointer = stream.bytes().data();
  vm.set_register_value<std::int32_t>(Register::gpr_1, 40);
  vm.set_register_value<std::int32_t>(Register::gpr_2, 0);

  vm.run();

  CHECK(vm.get_register_value<std::int32_t>(Register::gpr_1) == 142);
  CHECK(vm.get_register_value<std::int32_t>(Register::gpr_2) == 0);
  CHECK(
    vm.get_register_value<benson::Pointer>(Register::sp).decode().offset ==
    vm.stack->size()
  );
}

TEST_CASE("Virtual_machine does not take narrow jnz_i when condition is zero")
{
  using benson::bytecode::Register;
  using benson::bytecode::Wide_immediate;

  auto stream = Recording_binary_output_stream{};
  auto writer = benson::bytecode::Bytecode_writer{&stream};
  writer.emit_jnz(Register::gpr_2, std::ptrdiff_t{7});
  writer.emit_add_i32_i(Register::gpr_1, Register::gpr_1, Wide_immediate{1});
  writer.emit_exit();
  writer.flush();

  auto vm = benson::Virtual_machine{};
  vm.instruction_pointer = stream.bytes().data();
  vm.set_register_value<std::int32_t>(Register::gpr_1, 40);
  vm.set_register_value<std::int32_t>(Register::gpr_2, 0);

  vm.run();

  CHECK(vm.get_register_value<std::int32_t>(Register::gpr_1) == 41);
}

TEST_CASE("Virtual_machine takes narrow jnz_i when condition is non-zero")
{
  using benson::bytecode::Register;
  using benson::bytecode::Wide_immediate;

  auto stream = Recording_binary_output_stream{};
  auto writer = benson::bytecode::Bytecode_writer{&stream};
  writer.emit_jnz(Register::gpr_2, std::ptrdiff_t{7});
  writer.emit_add_i32_i(Register::gpr_1, Register::gpr_1, Wide_immediate{1});
  writer.emit_exit();
  writer.flush();

  auto vm = benson::Virtual_machine{};
  vm.instruction_pointer = stream.bytes().data();
  vm.set_register_value<std::int32_t>(Register::gpr_1, 40);
  vm.set_register_value<std::int32_t>(Register::gpr_2, 1);

  vm.run();

  CHECK(vm.get_register_value<std::int32_t>(Register::gpr_1) == 40);
}

TEST_CASE("Virtual_machine takes wide jnz_i when target exceeds narrow range")
{
  using benson::bytecode::Register;
  using benson::bytecode::Wide_immediate;

  auto stream = Recording_binary_output_stream{};
  auto writer = benson::bytecode::Bytecode_writer{&stream};
  writer.emit_jnz(Register::gpr_2, std::ptrdiff_t{165});
  for (auto i = 0; i < 40; ++i)
  {
    writer.emit_add_i32_i(Register::gpr_1, Register::gpr_1, Wide_immediate{1});
  }
  writer.emit_exit();
  writer.flush();

  auto vm = benson::Virtual_machine{};
  vm.instruction_pointer = stream.bytes().data();
  vm.set_register_value<std::int32_t>(Register::gpr_1, 40);
  vm.set_register_value<std::int32_t>(Register::gpr_2, 1);

  vm.run();

  CHECK(vm.get_register_value<std::int32_t>(Register::gpr_1) == 40);
}

TEST_CASE("Virtual_machine runs countdown sum program with jnz_i loop")
{
  using benson::bytecode::Module_builder;
  using benson::bytecode::Wide_immediate;
  using enum benson::bytecode::Register;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  auto const loop = builder.make_label();

  builder.place_label(loop);
  writer.emit_add_i32(gpr_2, gpr_2, gpr_1);
  writer.emit_sub_i32_i(gpr_1, gpr_1, Wide_immediate{1});
  writer.emit_cmp_gt_i32_i(gpr_3, gpr_1, Wide_immediate{0});
  writer.emit_jnz(gpr_3, builder.label_target(loop));
  writer.emit_exit();

  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);
  vm.set_register_value<std::int32_t>(gpr_1, 5);
  vm.set_register_value<std::int32_t>(gpr_2, 0);

  vm.run();

  CHECK(vm.get_register_value<std::int32_t>(gpr_1) == 0);
  CHECK(vm.get_register_value<std::int32_t>(gpr_2) == 15);
  CHECK(!vm.get_register_value<bool>(gpr_3));
}

TEST_CASE(
  "Virtual_machine runs recursive quicksort program with call_i and ret"
)
{
  using benson::Address_space;
  using benson::Pointer;
  using benson::bytecode::Module_builder;
  using benson::bytecode::Wide_constant;
  using benson::bytecode::Wide_immediate;
  using enum benson::bytecode::Register;

  auto const input = std::array<std::int32_t, 5>{4, 1, 3, 5, 2};
  auto const expected = std::array<std::int32_t, 5>{1, 2, 3, 4, 5};

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  auto const quicksort = builder.make_label();
  auto const partition_loop = builder.make_label();
  auto const skip_swap = builder.make_label();
  auto const after_partition = builder.make_label();
  auto const do_left = builder.make_label();
  auto const after_left = builder.make_label();
  auto const do_right = builder.make_label();
  auto const return_base = builder.make_label();

  writer.emit_lookup_k(gpr_21, Wide_constant{1});
  writer.emit_lookup_k(gpr_22, builder.constant(std::int32_t{5}));
  writer.emit_load_32(gpr_4, gpr_22, Wide_immediate{0});
  writer.emit_sub_i32(gpr_3, gpr_4, gpr_4);
  writer.emit_sub_i32_i(gpr_4, gpr_4, Wide_immediate{1});
  writer.emit_sub_i64_i(sp, sp, Wide_immediate{20});
  writer.emit_add_i64_i(gpr_2, sp, Wide_immediate{0});

  writer.emit_load_32(gpr_10, gpr_21, Wide_immediate{0});
  writer.emit_store_32(gpr_10, gpr_2, Wide_immediate{0});
  writer.emit_load_32(gpr_10, gpr_21, Wide_immediate{4});
  writer.emit_store_32(gpr_10, gpr_2, Wide_immediate{4});
  writer.emit_load_32(gpr_10, gpr_21, Wide_immediate{8});
  writer.emit_store_32(gpr_10, gpr_2, Wide_immediate{8});
  writer.emit_load_32(gpr_10, gpr_21, Wide_immediate{12});
  writer.emit_store_32(gpr_10, gpr_2, Wide_immediate{12});
  writer.emit_load_32(gpr_10, gpr_21, Wide_immediate{16});
  writer.emit_store_32(gpr_10, gpr_2, Wide_immediate{16});

  writer.emit_sub_i64_i(sp, sp, Wide_immediate{8});
  writer.emit_store_64(gpr_2, sp, Wide_immediate{0});
  writer.emit_sub_i64_i(sp, sp, Wide_immediate{8});
  writer.emit_store_64(gpr_3, sp, Wide_immediate{0});
  writer.emit_sub_i64_i(sp, sp, Wide_immediate{8});
  writer.emit_store_64(gpr_4, sp, Wide_immediate{0});
  writer.emit_call(builder.label_target(quicksort));
  writer.emit_add_i64_i(sp, sp, Wide_immediate{24});
  writer.emit_exit();

  builder.place_label(quicksort);
  writer.emit_load_64(gpr_4, sp, Wide_immediate{8});
  writer.emit_load_64(gpr_3, sp, Wide_immediate{16});
  writer.emit_load_64(gpr_2, sp, Wide_immediate{24});
  writer.emit_cmp_ge_i32(gpr_20, gpr_3, gpr_4);
  writer.emit_jnz(gpr_20, builder.label_target(return_base));

  writer.emit_mul_i32_i(gpr_8, gpr_4, Wide_immediate{4});
  writer.emit_add_i64(gpr_9, gpr_2, gpr_8);
  writer.emit_load_32(gpr_5, gpr_9, Wide_immediate{0});
  writer.emit_sub_i32_i(gpr_6, gpr_3, Wide_immediate{1});
  writer.emit_add_i32_i(gpr_7, gpr_3, Wide_immediate{0});

  builder.place_label(partition_loop);
  writer.emit_cmp_ge_i32(gpr_20, gpr_7, gpr_4);
  writer.emit_jnz(gpr_20, builder.label_target(after_partition));

  writer.emit_mul_i32_i(gpr_8, gpr_7, Wide_immediate{4});
  writer.emit_add_i64(gpr_11, gpr_2, gpr_8);
  writer.emit_load_32(gpr_10, gpr_11, Wide_immediate{0});
  writer.emit_cmp_gt_i32(gpr_20, gpr_10, gpr_5);
  writer.emit_jnz(gpr_20, builder.label_target(skip_swap));

  writer.emit_add_i32_i(gpr_6, gpr_6, Wide_immediate{1});
  writer.emit_mul_i32_i(gpr_8, gpr_6, Wide_immediate{4});
  writer.emit_add_i64(gpr_13, gpr_2, gpr_8);
  writer.emit_load_32(gpr_12, gpr_13, Wide_immediate{0});
  writer.emit_store_32(gpr_10, gpr_13, Wide_immediate{0});
  writer.emit_store_32(gpr_12, gpr_11, Wide_immediate{0});

  builder.place_label(skip_swap);
  writer.emit_add_i32_i(gpr_7, gpr_7, Wide_immediate{1});
  writer.emit_jmp(builder.label_target(partition_loop));

  builder.place_label(after_partition);
  writer.emit_add_i32_i(gpr_14, gpr_6, Wide_immediate{1});
  writer.emit_mul_i32_i(gpr_8, gpr_14, Wide_immediate{4});
  writer.emit_add_i64(gpr_13, gpr_2, gpr_8);
  writer.emit_load_32(gpr_12, gpr_13, Wide_immediate{0});
  writer.emit_mul_i32_i(gpr_8, gpr_4, Wide_immediate{4});
  writer.emit_add_i64(gpr_11, gpr_2, gpr_8);
  writer.emit_load_32(gpr_15, gpr_11, Wide_immediate{0});
  writer.emit_store_32(gpr_15, gpr_13, Wide_immediate{0});
  writer.emit_store_32(gpr_12, gpr_11, Wide_immediate{0});
  writer.emit_store_64(gpr_14, sp, Wide_immediate{16});

  writer.emit_sub_i32_i(gpr_8, gpr_14, Wide_immediate{1});
  writer.emit_cmp_lt_i32(gpr_20, gpr_3, gpr_8);
  writer.emit_jnz(gpr_20, builder.label_target(do_left));
  writer.emit_jmp(builder.label_target(after_left));

  builder.place_label(do_left);
  writer.emit_sub_i64_i(sp, sp, Wide_immediate{8});
  writer.emit_store_64(gpr_2, sp, Wide_immediate{0});
  writer.emit_sub_i64_i(sp, sp, Wide_immediate{8});
  writer.emit_store_64(gpr_3, sp, Wide_immediate{0});
  writer.emit_sub_i64_i(sp, sp, Wide_immediate{8});
  writer.emit_store_64(gpr_8, sp, Wide_immediate{0});
  writer.emit_call(builder.label_target(quicksort));
  writer.emit_add_i64_i(sp, sp, Wide_immediate{24});

  builder.place_label(after_left);
  writer.emit_load_64(gpr_14, sp, Wide_immediate{16});
  writer.emit_add_i32_i(gpr_8, gpr_14, Wide_immediate{1});
  writer.emit_load_64(gpr_4, sp, Wide_immediate{8});
  writer.emit_cmp_lt_i32(gpr_20, gpr_8, gpr_4);
  writer.emit_jnz(gpr_20, builder.label_target(do_right));
  writer.emit_jmp(builder.label_target(return_base));

  builder.place_label(do_right);
  writer.emit_load_64(gpr_2, sp, Wide_immediate{24});
  writer.emit_sub_i64_i(sp, sp, Wide_immediate{8});
  writer.emit_store_64(gpr_2, sp, Wide_immediate{0});
  writer.emit_sub_i64_i(sp, sp, Wide_immediate{8});
  writer.emit_store_64(gpr_8, sp, Wide_immediate{0});
  writer.emit_sub_i64_i(sp, sp, Wide_immediate{8});
  writer.emit_store_64(gpr_4, sp, Wide_immediate{0});
  writer.emit_call(builder.label_target(quicksort));
  writer.emit_add_i64_i(sp, sp, Wide_immediate{24});

  builder.place_label(return_base);
  writer.emit_load_64(gpr_1, sp, Wide_immediate{24});
  writer.emit_ret();

  auto module = builder.build();
  store_constant(
    module.constant_data,
    module.constant_table,
    Wide_constant{1},
    input
  );

  auto vm = benson::Virtual_machine{};
  vm.load(module);

  vm.run();

  auto const array_pointer = vm.get_register_value<Pointer>(gpr_1).decode();
  auto const stack_pointer = vm.get_register_value<Pointer>(sp).decode();
  auto result = std::array<std::int32_t, 5>{};
  std::memcpy(
    result.data(),
    vm.stack->data() + array_pointer.offset,
    sizeof(result)
  );

  CHECK(result == expected);
  CHECK(array_pointer.space == Address_space::stack);
  CHECK(array_pointer.offset == vm.stack->size() - sizeof(result));
  CHECK(stack_pointer.space == Address_space::stack);
  CHECK(stack_pointer.offset == array_pointer.offset);
  CHECK(module.constant_table.size() == 2);
}

TEST_CASE("Virtual_machine runs factorial program with jnz_i loop")
{
  using benson::bytecode::Module_builder;
  using benson::bytecode::Wide_immediate;
  using enum benson::bytecode::Register;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  auto const loop = builder.make_label();

  writer.emit_lookup_k(gpr_4, builder.constant(std::int32_t{5}));
  writer.emit_lookup_k(gpr_5, builder.constant(std::int32_t{1}));
  writer.emit_load_32(gpr_1, gpr_4, Wide_immediate{0});
  writer.emit_load_32(gpr_2, gpr_5, Wide_immediate{0});
  builder.place_label(loop);
  writer.emit_mul_i32(gpr_2, gpr_2, gpr_1);
  writer.emit_sub_i32_i(gpr_1, gpr_1, Wide_immediate{1});
  writer.emit_cmp_gt_i32_i(gpr_3, gpr_1, Wide_immediate{1});
  writer.emit_jnz(gpr_3, builder.label_target(loop));
  writer.emit_exit();

  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);

  vm.run();

  CHECK(vm.get_register_value<std::int32_t>(gpr_1) == 1);
  CHECK(vm.get_register_value<std::int32_t>(gpr_2) == 120);
  CHECK(!vm.get_register_value<bool>(gpr_3));
  CHECK(module.constant_table.size() == 2);
}

TEST_CASE("Virtual_machine runs Newton square root program with jnz_i loop")
{
  using benson::bytecode::Module_builder;
  using benson::bytecode::Wide_immediate;
  using enum benson::bytecode::Register;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  auto const loop = builder.make_label();

  writer.emit_lookup_k(gpr_10, builder.constant(9.0F));
  writer.emit_lookup_k(gpr_11, builder.constant(1.0F));
  writer.emit_load_32(gpr_1, gpr_10, Wide_immediate{0});
  writer.emit_load_32(gpr_2, gpr_11, Wide_immediate{0});
  writer.emit_add_i32_k(gpr_3, gpr_3, builder.constant(5));
  builder.place_label(loop);
  writer.emit_div_f32(gpr_4, gpr_1, gpr_2);
  writer.emit_add_f32(gpr_5, gpr_2, gpr_4);
  writer.emit_mul_f32_k(gpr_2, gpr_5, builder.constant(0.5F));
  writer.emit_sub_i32_i(gpr_3, gpr_3, Wide_immediate{1});
  writer.emit_cmp_gt_i32_i(gpr_6, gpr_3, Wide_immediate{0});
  writer.emit_jnz(gpr_6, builder.label_target(loop));
  writer.emit_exit();

  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);

  vm.run();

  auto const estimate = vm.get_register_value<float>(gpr_2);
  CHECK(estimate > 2.999F);
  CHECK(estimate < 3.001F);
  CHECK(vm.get_register_value<std::int32_t>(gpr_3) == 0);
  CHECK(!vm.get_register_value<bool>(gpr_6));
  CHECK(module.constant_table.size() == 4);
}

TEST_CASE("Virtual_machine runs stack RPN program with jnz_i dispatch loop")
{
  using benson::Address_space;
  using benson::Pointer;
  using benson::bytecode::Module_builder;
  using benson::bytecode::Wide_constant;
  using benson::bytecode::Wide_immediate;
  using enum benson::bytecode::Register;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  auto const expression_k = builder.constant(std::int32_t{0});
  auto const loop = builder.make_label();
  auto const operand = builder.make_label();
  auto const add = builder.make_label();
  auto const mul = builder.make_label();
  auto const sub = builder.make_label();
  auto const advance = builder.make_label();
  auto const done = builder.make_label();

  // get the pointer to the start of constant data
  writer.emit_lookup_k(gpr_10, expression_k);
  // load the length of the RPN expression
  writer.emit_load_32(gpr_1, gpr_10, Wide_immediate{0});
  // calculate the pointer to the start of the RPN expression
  writer.emit_add_i64_i(gpr_11, gpr_10, Wide_immediate{4});

  builder.place_label(loop);

  // compare the "length" of the RPN expression with zero
  writer.emit_cmp_eq_i32_i(gpr_2, gpr_1, Wide_immediate{0});
  // if the length is zero, we're done
  writer.emit_jnz(gpr_2, builder.label_target(done));

  // load the operand/operator tag
  writer.emit_load_32(gpr_3, gpr_11, Wide_immediate{0});
  // load the operand/operator value
  writer.emit_load_32(gpr_4, gpr_11, Wide_immediate{4});

  // if tag == 0, it's an operand
  writer.emit_cmp_eq_i32_i(gpr_2, gpr_3, Wide_immediate{0});
  writer.emit_jnz(gpr_2, builder.label_target(operand));

  // else if value == 1, it's an addition
  writer.emit_cmp_eq_i32_i(gpr_2, gpr_4, Wide_immediate{1});
  writer.emit_jnz(gpr_2, builder.label_target(add));

  // else if value == 2, it's a subtraction
  writer.emit_cmp_eq_i32_i(gpr_2, gpr_4, Wide_immediate{2});
  writer.emit_jnz(gpr_2, builder.label_target(sub));

  // else if value == 3, it's a division
  writer.emit_cmp_eq_i32_i(gpr_2, gpr_4, Wide_immediate{3});
  writer.emit_jnz(gpr_2, builder.label_target(mul));

  // else it's a "done" instruction
  writer.emit_jmp(builder.label_target(done));

  // push the operand value onto the stack
  builder.place_label(operand);
  writer.emit_sub_i64_i(sp, sp, Wide_immediate{8});
  writer.emit_store_64(gpr_4, sp, Wide_immediate{0});
  writer.emit_jmp(builder.label_target(advance));

  builder.place_label(add);
  writer.emit_load_64(gpr_5, sp, Wide_immediate{0});
  writer.emit_load_64(gpr_4, sp, Wide_immediate{8});
  writer.emit_add_i64_i(sp, sp, Wide_immediate{8});
  writer.emit_add_i32(gpr_6, gpr_4, gpr_5);
  writer.emit_store_64(gpr_6, sp, Wide_immediate{0});
  writer.emit_jmp(builder.label_target(advance));

  builder.place_label(sub);
  writer.emit_load_64(gpr_5, sp, Wide_immediate{0});
  writer.emit_load_64(gpr_4, sp, Wide_immediate{8});
  writer.emit_add_i64_i(sp, sp, Wide_immediate{8});
  writer.emit_sub_i32(gpr_6, gpr_4, gpr_5);
  writer.emit_store_64(gpr_6, sp, Wide_immediate{0});
  writer.emit_jmp(builder.label_target(advance));

  builder.place_label(mul);
  writer.emit_load_64(gpr_5, sp, Wide_immediate{0});
  writer.emit_load_64(gpr_4, sp, Wide_immediate{8});
  writer.emit_add_i64_i(sp, sp, Wide_immediate{8});
  writer.emit_mul_i32(gpr_6, gpr_4, gpr_5);
  writer.emit_store_64(gpr_6, sp, Wide_immediate{0});
  writer.emit_jmp(builder.label_target(advance));

  // advance the RPN expression pointer and decrement the length
  builder.place_label(advance);
  writer.emit_add_i64_i(gpr_11, gpr_11, Wide_immediate{8});
  writer.emit_sub_i32_i(gpr_1, gpr_1, Wide_immediate{1});
  writer.emit_jmp(builder.label_target(loop));

  // pop top of stack to gpr_9 and exit
  builder.place_label(done);
  writer.emit_load_64(gpr_9, sp, Wide_immediate{0});
  writer.emit_add_i64_i(sp, sp, Wide_immediate{8});
  writer.emit_exit();

  auto module = builder.build();
  auto const expression = std::array<std::int32_t, 15>{
    7,
    0,
    3,
    0,
    4,
    1,
    1,
    0,
    2,
    1,
    3,
    0,
    7,
    1,
    2,
  };
  module.constant_data.clear();
  module.constant_table.clear();
  store_constant(
    module.constant_data,
    module.constant_table,
    expression_k,
    expression
  );

  auto vm = benson::Virtual_machine{};
  vm.load(module);

  vm.run();

  auto const stack_pointer = vm.get_register_value<Pointer>(sp).decode();
  CHECK(vm.get_register_value<std::int32_t>(gpr_9) == 7);
  CHECK(vm.get_register_value<std::int32_t>(gpr_1) == 0);
  CHECK(stack_pointer.space == Address_space::stack);
  CHECK(stack_pointer.offset == vm.stack->size());
  CHECK(module.constant_table.size() == 1);
}

TEST_CASE("Virtual_machine runs Module_builder program with inline constants")
{
  using benson::bytecode::Module_builder;
  using enum benson::bytecode::Register;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  writer.emit_add_i32_k(gpr_1, gpr_1, builder.constant(5));
  writer.emit_mul_f32_k(gpr_2, gpr_2, builder.constant(0.5F));
  writer.emit_add_f32_k(gpr_2, gpr_2, builder.constant(0.5F));
  writer.emit_exit();

  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);
  vm.set_register_value<std::int32_t>(gpr_1, 37);
  vm.set_register_value<float>(gpr_2, 3.0F);

  vm.run();

  CHECK(vm.get_register_value<std::int32_t>(gpr_1) == 42);
  CHECK(vm.get_register_value<float>(gpr_2) == 2.0F);
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
  writer
    .emit_store_16(Register::gpr_1, Register::gpr_3, Wide_immediate{0x0101});
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

TEST_CASE("Virtual_machine runs sx_8 program emitted by Module_builder")
{
  using benson::bytecode::Module_builder;
  using enum benson::bytecode::Register;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  writer.emit_sx_8(gpr_1, gpr_2);
  writer.emit_exit();
  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);
  vm.set_register_value<std::int8_t>(gpr_2, std::int8_t{-42});

  vm.run();

  CHECK(vm.get_register_value<std::int8_t>(gpr_1) == std::int8_t{-42});
}

TEST_CASE(
  "Virtual_machine runs narrow comparison program emitted by Module_builder"
)
{
  using benson::bytecode::Module_builder;
  using benson::bytecode::Wide_immediate;
  using enum benson::bytecode::Register;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  writer.emit_cmp_eq_i32(gpr_1, gpr_2, gpr_3);
  writer.emit_cmp_eq_i64_i(gpr_4, gpr_5, Wide_immediate{9});
  writer.emit_cmp_ne_f32(gpr_6, gpr_7, gpr_8);
  writer.emit_cmp_ne_f64_k(gpr_9, gpr_10, builder.constant(4.0));
  writer.emit_cmp_lt_i32_i(gpr_11, gpr_12, Wide_immediate{8});
  writer.emit_cmp_lt_i64_k(gpr_13, gpr_14, builder.constant(std::int64_t{7}));
  writer.emit_cmp_le_f32_k(gpr_15, gpr_16, builder.constant(3.5F));
  writer.emit_cmp_le_f64(gpr_17, gpr_18, gpr_19);
  writer.emit_cmp_gt_i32_k(gpr_20, gpr_21, builder.constant(1));
  writer.emit_cmp_gt_i64(gpr_22, gpr_23, gpr_24);
  writer.emit_cmp_ge_f32(gpr_25, gpr_26, gpr_27);
  writer.emit_cmp_ge_f64_k(gpr_28, gpr_29, builder.constant(6.0));
  writer.emit_exit();
  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);
  vm.set_register_value<std::int32_t>(gpr_2, 42);
  vm.set_register_value<std::int32_t>(gpr_3, 42);
  vm.set_register_value<std::int64_t>(gpr_5, 9);
  vm.set_register_value<float>(gpr_7, 1.5F);
  vm.set_register_value<float>(gpr_8, 2.5F);
  vm.set_register_value<double>(gpr_10, 5.0);
  vm.set_register_value<std::int32_t>(gpr_12, 4);
  vm.set_register_value<std::int64_t>(gpr_14, 6);
  vm.set_register_value<float>(gpr_16, 3.5F);
  vm.set_register_value<double>(gpr_18, 2.0);
  vm.set_register_value<double>(gpr_19, 2.0);
  vm.set_register_value<std::int32_t>(gpr_21, 5);
  vm.set_register_value<std::int64_t>(gpr_23, 7);
  vm.set_register_value<std::int64_t>(gpr_24, 3);
  vm.set_register_value<float>(gpr_26, 2.5F);
  vm.set_register_value<float>(gpr_27, 2.5F);
  vm.set_register_value<double>(gpr_29, 6.0);

  vm.run();

  CHECK(vm.get_register_value<bool>(gpr_1));
  CHECK(vm.get_register_value<bool>(gpr_4));
  CHECK(vm.get_register_value<bool>(gpr_6));
  CHECK(vm.get_register_value<bool>(gpr_9));
  CHECK(vm.get_register_value<bool>(gpr_11));
  CHECK(vm.get_register_value<bool>(gpr_13));
  CHECK(vm.get_register_value<bool>(gpr_15));
  CHECK(vm.get_register_value<bool>(gpr_17));
  CHECK(vm.get_register_value<bool>(gpr_20));
  CHECK(vm.get_register_value<bool>(gpr_22));
  CHECK(vm.get_register_value<bool>(gpr_25));
  CHECK(vm.get_register_value<bool>(gpr_28));
}

TEST_CASE(
  "Virtual_machine runs wide comparison program emitted by Bytecode_writer"
)
{
  using benson::bytecode::Register;
  using benson::bytecode::Wide_constant;
  using benson::bytecode::Wide_immediate;

  auto constant_memory = std::vector<std::byte>{};
  auto constant_table = std::vector<std::ptrdiff_t>{};
  store_constant<std::int32_t>(
    constant_memory,
    constant_table,
    Wide_constant{0x0201},
    9
  );
  store_constant<std::int64_t>(
    constant_memory,
    constant_table,
    Wide_constant{0x0202},
    7
  );
  store_constant<float>(
    constant_memory,
    constant_table,
    Wide_constant{0x0203},
    3.0F
  );
  store_constant<double>(
    constant_memory,
    constant_table,
    Wide_constant{0x0204},
    4.0
  );
  store_constant<float>(
    constant_memory,
    constant_table,
    Wide_constant{0x0205},
    1.5F
  );
  store_constant<double>(
    constant_memory,
    constant_table,
    Wide_constant{0x0206},
    8.0
  );

  auto stream = Recording_binary_output_stream{};
  auto writer = benson::bytecode::Bytecode_writer{&stream};
  writer.emit_cmp_eq_i32_i(
    Register::gpr_1,
    Register::gpr_2,
    Wide_immediate{0x0101}
  );
  writer
    .emit_cmp_ne_i64_k(Register::gpr_3, Register::gpr_4, Wide_constant{0x0201});
  writer
    .emit_cmp_lt_f32_k(Register::gpr_5, Register::gpr_6, Wide_constant{0x0203});
  writer
    .emit_cmp_le_f64_k(Register::gpr_7, Register::gpr_8, Wide_constant{0x0204});
  writer.emit_cmp_gt_i32_k(
    Register::gpr_9,
    Register::gpr_10,
    Wide_constant{0x0201}
  );
  writer.emit_cmp_ge_i64_i(
    Register::gpr_11,
    Register::gpr_12,
    Wide_immediate{0x0100}
  );
  writer.emit_cmp_ge_f32_k(
    Register::gpr_13,
    Register::gpr_14,
    Wide_constant{0x0205}
  );
  writer.emit_cmp_eq_f64_k(
    Register::gpr_15,
    Register::gpr_16,
    Wide_constant{0x0206}
  );
  writer.emit_exit();
  writer.flush();

  auto vm = benson::Virtual_machine{};
  vm.instruction_pointer = stream.bytes().data();
  vm.constant_memory = constant_memory.data();
  vm.constant_table = constant_table.data();
  vm.set_register_value<std::int32_t>(Register::gpr_2, 0x0101);
  vm.set_register_value<std::int64_t>(Register::gpr_4, 8);
  vm.set_register_value<float>(Register::gpr_6, 2.5F);
  vm.set_register_value<double>(Register::gpr_8, 3.5);
  vm.set_register_value<std::int32_t>(Register::gpr_10, 12);
  vm.set_register_value<std::int64_t>(Register::gpr_12, 0x0100);
  vm.set_register_value<float>(Register::gpr_14, 1.5F);
  vm.set_register_value<double>(Register::gpr_16, 8.0);

  vm.run();

  CHECK(vm.get_register_value<bool>(Register::gpr_1));
  CHECK(vm.get_register_value<bool>(Register::gpr_3));
  CHECK(vm.get_register_value<bool>(Register::gpr_5));
  CHECK(vm.get_register_value<bool>(Register::gpr_7));
  CHECK(vm.get_register_value<bool>(Register::gpr_9));
  CHECK(vm.get_register_value<bool>(Register::gpr_11));
  CHECK(vm.get_register_value<bool>(Register::gpr_13));
  CHECK(vm.get_register_value<bool>(Register::gpr_15));
}
