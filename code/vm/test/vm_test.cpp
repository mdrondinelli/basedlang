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
  using benson::bytecode::sp;

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

  auto vm = benson::Virtual_machine{};
  vm.set_register_value(gpr(255), value);

  CHECK(vm.get_register_value<TestType>(gpr(255)) == value);
}

TEST_CASE("Virtual_machine keeps exact floating-point register bit pattern")
{
  auto vm = benson::Virtual_machine{};
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
  using benson::Address_space;
  using benson::Pointer;
  using benson::bytecode::gpr;
  using benson::bytecode::Module_builder;
  using benson::bytecode::sp;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  writer.emit_lookup_k(gpr(1), builder.constant(5));
  writer.emit_lookup_k(gpr(2), builder.constant(2.5));
  writer.emit_exit();
  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);

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
  using benson::bytecode::sp;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  writer.emit_neg_i32(gpr(1), gpr(2));
  writer.emit_exit();
  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);
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
  using benson::bytecode::sp;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  writer.emit_mov(gpr(1), gpr(2));
  writer.emit_mov_i(gpr(3), Immediate{-4});
  writer.emit_mov_i(gpr(4), Immediate{0x0102});
  writer.emit_mov_i(gpr(5), Immediate{-0x0102});
  writer.emit_exit();
  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);
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
  writer.emit_mov_i(gpr(300), Immediate{5});
  writer.emit_mov_i(gpr(301), Immediate{7});
  writer.emit_add_i32(gpr(302), gpr(300), gpr(301));
  writer.emit_sub_i64_i(sp, sp, Immediate{8});
  writer.emit_store_32(gpr(302), sp, Immediate{0});
  writer.emit_load_32(gpr(303), sp, Immediate{0});
  writer.emit_jnz(gpr(303), builder.label_target(done));
  writer.emit_mov_i(gpr(304), Immediate{1});
  builder.place_label(done);
  writer.emit_mov(gpr(305), gpr(303));
  writer.emit_neg_i32(gpr(306), gpr(305));
  writer.emit_exit();
  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);

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
  using benson::bytecode::sp;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  writer.emit_add_i32(gpr(1), gpr(2), gpr(3));
  writer.emit_sub_i32(gpr(4), gpr(1), gpr(5));
  writer.emit_mul_i32(gpr(6), gpr(4), gpr(7));
  writer.emit_div_i32(gpr(8), gpr(6), gpr(9));
  writer.emit_mod_i32(gpr(10), gpr(8), gpr(11));
  writer.emit_exit();
  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);
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
  using benson::bytecode::sp;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  writer.emit_add_i32_i(gpr(1), gpr(2), Immediate{5});
  writer.emit_sub_i32_i(gpr(3), gpr(1), Immediate{-3});
  writer.emit_mul_i32_i(gpr(4), gpr(3), Immediate{0x0102});
  writer.emit_div_i32_i(gpr(5), gpr(4), Immediate{6});
  writer.emit_mod_i32_i(gpr(6), gpr(5), Immediate{7});
  writer.emit_exit();
  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);
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
  using benson::bytecode::sp;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  writer.emit_add_f64(gpr(1), gpr(2), gpr(3));
  writer.emit_sub_f64(gpr(4), gpr(1), gpr(5));
  writer.emit_mul_f64(gpr(6), gpr(4), gpr(7));
  writer.emit_div_f64(gpr(8), gpr(6), gpr(9));
  writer.emit_exit();
  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);
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
  using benson::bytecode::sp;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  writer.emit_add_i32_k(gpr(1), gpr(2), builder.constant(5));
  writer.emit_sub_i32_k(gpr(4), gpr(1), builder.constant(3));
  writer.emit_mul_i32_k(gpr(6), gpr(4), builder.constant(4));
  writer.emit_div_i32_k(gpr(8), gpr(6), builder.constant(6));
  writer.emit_mod_i32_k(gpr(10), gpr(8), builder.constant(7));
  writer.emit_add_f64_k(gpr(12), gpr(14), builder.constant(2.5));
  writer.emit_sub_f64_k(gpr(16), gpr(12), builder.constant(1.5));
  writer.emit_mul_f64_k(gpr(18), gpr(16), builder.constant(4.0));
  writer.emit_div_f64_k(gpr(20), gpr(18), builder.constant(2.0));
  writer.emit_exit();
  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);
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
  writer.emit_add_i32_k(gpr(1), gpr(2), Constant{0x0304});
  writer.emit_sub_i32_k(gpr(4), gpr(1), Constant{0x0305});
  writer.emit_mul_i32_k(gpr(6), gpr(4), Constant{0x0306});
  writer.emit_div_i32_k(gpr(8), gpr(6), Constant{0x0307});
  writer.emit_mod_i32_k(gpr(10), gpr(8), Constant{0x0308});
  writer.emit_add_f64_k(gpr(12), gpr(14), Constant{0x0309});
  writer.emit_sub_f64_k(gpr(16), gpr(12), Constant{0x030A});
  writer.emit_mul_f64_k(gpr(18), gpr(16), Constant{0x030B});
  writer.emit_div_f64_k(gpr(20), gpr(18), Constant{0x030C});
  writer.emit_exit();
  writer.flush();

  auto module = benson::bytecode::Module{};
  module.constant_data = std::move(constant_memory);
  module.constant_table = std::move(constant_table);

  auto vm = benson::Virtual_machine{};
  vm.module = &module;
  vm.instruction_pointer = stream.bytes().data();
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
  using benson::Address_space;
  using benson::Pointer;
  using benson::bytecode::Immediate;
  using benson::bytecode::Module_builder;
  using benson::bytecode::Register;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  writer.emit_store_8(gpr(1), gpr(2), Immediate{0});
  writer.emit_exit();
  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);
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

TEST_CASE("Virtual_machine load_8 reads one byte from stack")
{
  using benson::Address_space;
  using benson::Pointer;
  using benson::bytecode::Immediate;
  using benson::bytecode::Module_builder;
  using benson::bytecode::Register;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  writer.emit_load_8(gpr(1), gpr(2), Immediate{0});
  writer.emit_exit();
  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);
  (*vm.stack)[0] = static_cast<std::byte>(std::uint8_t{0xAB});
  vm.set_register_value<std::uint64_t>(
    gpr(2),
    static_cast<std::uint64_t>(Pointer{Address_space::stack, 0})
  );

  vm.run();

  CHECK(vm.get_register_value<std::uint8_t>(gpr(1)) == 0xAB);
}

TEST_CASE("Virtual_machine store_64 and load_64 round-trip a 64-bit value")
{
  using benson::Address_space;
  using benson::Pointer;
  using benson::bytecode::Immediate;
  using benson::bytecode::Module_builder;
  using benson::bytecode::Register;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  writer.emit_store_64(gpr(1), gpr(3), Immediate{0});
  writer.emit_load_64(gpr(2), gpr(3), Immediate{0});
  writer.emit_exit();
  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);
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
  using benson::Address_space;
  using benson::Pointer;
  using benson::bytecode::Immediate;
  using benson::bytecode::Module_builder;
  using benson::bytecode::Register;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  writer.emit_store_32(gpr(1), gpr(3), Immediate{8});
  writer.emit_load_32(gpr(2), gpr(3), Immediate{8});
  writer.emit_exit();
  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);
  vm.set_register_value<std::int32_t>(gpr(1), std::int32_t{0xDEAD});
  vm.set_register_value<std::uint64_t>(
    gpr(3),
    static_cast<std::uint64_t>(Pointer{Address_space::stack, 0})
  );

  vm.run();

  CHECK(vm.get_register_value<std::int32_t>(gpr(2)) == std::int32_t{0xDEAD});
}

TEST_CASE("Virtual_machine load_32 reads from constant memory via pointer")
{
  using benson::bytecode::gpr;
  using benson::bytecode::Immediate;
  using benson::bytecode::Module_builder;
  using benson::bytecode::sp;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  writer.emit_lookup_k(gpr(2), builder.constant(std::int32_t{0x1234ABCD}));
  writer.emit_load_32(gpr(1), gpr(2), Immediate{0});
  writer.emit_exit();
  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);

  vm.run();

  CHECK(
    vm.get_register_value<std::int32_t>(gpr(1)) == std::int32_t{0x1234ABCD}
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
  writer.emit_add_i32_i(gpr(1), gpr(1), Immediate{1});
  builder.place_label(done);
  writer.emit_add_i32_i(gpr(1), gpr(1), Immediate{2});
  writer.emit_exit();

  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);
  vm.set_register_value<std::int32_t>(gpr(1), 40);

  vm.run();

  CHECK(vm.get_register_value<std::int32_t>(gpr(1)) == 42);
}

TEST_CASE("Virtual_machine takes narrow call_i and ret")
{
  using benson::bytecode::Immediate;
  using benson::bytecode::Register;

  auto stream = Recording_output_stream{};
  auto writer = benson::bytecode::Bytecode_writer{&stream};
  writer.emit_call(std::ptrdiff_t{7});
  writer.emit_add_i32_i(gpr(1), gpr(1), Immediate{100});
  writer.emit_exit();
  writer.emit_add_i32_i(gpr(1), gpr(1), Immediate{2});
  writer.emit_ret();
  writer.flush();

  auto vm = benson::Virtual_machine{};
  vm.instruction_pointer = stream.bytes().data();
  vm.set_register_value<std::int32_t>(gpr(1), 40);

  vm.run();

  CHECK(vm.get_register_value<std::int32_t>(gpr(1)) == 142);
  CHECK(
    vm.get_register_value<benson::Pointer>(sp).decode().offset ==
    vm.stack->size()
  );
}

TEST_CASE("Virtual_machine takes wide call_i and ret")
{
  using benson::bytecode::Immediate;
  using benson::bytecode::Register;

  auto stream = Recording_output_stream{};
  auto writer = benson::bytecode::Bytecode_writer{&stream};
  writer.emit_call(std::ptrdiff_t{165});
  writer.emit_add_i32_i(gpr(1), gpr(1), Immediate{100});
  writer.emit_exit();
  for (auto i = 0; i < 39; ++i)
  {
    writer.emit_add_i32_i(gpr(2), gpr(2), Immediate{1});
  }
  writer.emit_add_i32_i(gpr(1), gpr(1), Immediate{2});
  writer.emit_ret();
  writer.flush();

  auto vm = benson::Virtual_machine{};
  vm.instruction_pointer = stream.bytes().data();
  vm.set_register_value<std::int32_t>(gpr(1), 40);
  vm.set_register_value<std::int32_t>(gpr(2), 0);

  vm.run();

  CHECK(vm.get_register_value<std::int32_t>(gpr(1)) == 142);
  CHECK(vm.get_register_value<std::int32_t>(gpr(2)) == 0);
  CHECK(
    vm.get_register_value<benson::Pointer>(sp).decode().offset ==
    vm.stack->size()
  );
}

TEST_CASE("Virtual_machine does not take narrow jnz_i when condition is zero")
{
  using benson::bytecode::Immediate;
  using benson::bytecode::Register;

  auto stream = Recording_output_stream{};
  auto writer = benson::bytecode::Bytecode_writer{&stream};
  writer.emit_jnz(gpr(2), std::ptrdiff_t{7});
  writer.emit_add_i32_i(gpr(1), gpr(1), Immediate{1});
  writer.emit_exit();
  writer.flush();

  auto vm = benson::Virtual_machine{};
  vm.instruction_pointer = stream.bytes().data();
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
  writer.emit_add_i32_i(gpr(1), gpr(1), Immediate{1});
  writer.emit_exit();
  writer.flush();

  auto vm = benson::Virtual_machine{};
  vm.instruction_pointer = stream.bytes().data();
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
    writer.emit_add_i32_i(gpr(1), gpr(1), Immediate{1});
  }
  writer.emit_exit();
  writer.flush();

  auto vm = benson::Virtual_machine{};
  vm.instruction_pointer = stream.bytes().data();
  vm.set_register_value<std::int32_t>(gpr(1), 40);
  vm.set_register_value<std::int32_t>(gpr(2), 1);

  vm.run();

  CHECK(vm.get_register_value<std::int32_t>(gpr(1)) == 40);
}

TEST_CASE("Virtual_machine runs countdown sum program with jnz_i loop")
{
  using benson::bytecode::gpr;
  using benson::bytecode::Immediate;
  using benson::bytecode::Module_builder;
  using benson::bytecode::sp;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  auto const loop = builder.make_label();

  builder.place_label(loop);
  writer.emit_add_i32(gpr(2), gpr(2), gpr(1));
  writer.emit_sub_i32_i(gpr(1), gpr(1), Immediate{1});
  writer.emit_cmp_gt_i32_i(gpr(3), gpr(1), Immediate{0});
  writer.emit_jnz(gpr(3), builder.label_target(loop));
  writer.emit_exit();

  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);
  vm.set_register_value<std::int32_t>(gpr(1), 5);
  vm.set_register_value<std::int32_t>(gpr(2), 0);

  vm.run();

  CHECK(vm.get_register_value<std::int32_t>(gpr(1)) == 0);
  CHECK(vm.get_register_value<std::int32_t>(gpr(2)) == 15);
  CHECK(!vm.get_register_value<bool>(gpr(3)));
}

TEST_CASE(
  "Virtual_machine runs recursive quicksort program with call_i and ret"
)
{
  using benson::Address_space;
  using benson::Pointer;
  using benson::bytecode::Constant;
  using benson::bytecode::gpr;
  using benson::bytecode::Immediate;
  using benson::bytecode::Module_builder;
  using benson::bytecode::sp;

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

  // Copy the input array from constant memory into stack memory so the program
  // can sort it in place.
  writer.emit_lookup_k(gpr(21), Constant{1});
  writer.emit_lookup_k(gpr(22), builder.constant(std::int32_t{5}));
  writer.emit_load_32(gpr(4), gpr(22), Immediate{0});
  writer.emit_sub_i32(gpr(3), gpr(4), gpr(4));
  writer.emit_sub_i32_i(gpr(4), gpr(4), Immediate{1});
  writer.emit_sub_i64_i(sp, sp, Immediate{20});
  writer.emit_add_i64_i(gpr(2), sp, Immediate{0});

  writer.emit_load_32(gpr(10), gpr(21), Immediate{0});
  writer.emit_store_32(gpr(10), gpr(2), Immediate{0});
  writer.emit_load_32(gpr(10), gpr(21), Immediate{4});
  writer.emit_store_32(gpr(10), gpr(2), Immediate{4});
  writer.emit_load_32(gpr(10), gpr(21), Immediate{8});
  writer.emit_store_32(gpr(10), gpr(2), Immediate{8});
  writer.emit_load_32(gpr(10), gpr(21), Immediate{12});
  writer.emit_store_32(gpr(10), gpr(2), Immediate{12});
  writer.emit_load_32(gpr(10), gpr(21), Immediate{16});
  writer.emit_store_32(gpr(10), gpr(2), Immediate{16});

  writer.emit_sub_i64_i(sp, sp, Immediate{8});
  writer.emit_store_64(gpr(2), sp, Immediate{0});
  writer.emit_sub_i64_i(sp, sp, Immediate{8});
  writer.emit_store_64(gpr(3), sp, Immediate{0});
  writer.emit_sub_i64_i(sp, sp, Immediate{8});
  writer.emit_store_64(gpr(4), sp, Immediate{0});
  writer.emit_call(builder.label_target(quicksort));
  writer.emit_add_i64_i(sp, sp, Immediate{24});
  writer.emit_exit();

  builder.place_label(quicksort);
  // Stack arguments are hi, lo, base above the pushed return address.
  writer.emit_load_64(gpr(4), sp, Immediate{8});
  writer.emit_load_64(gpr(3), sp, Immediate{16});
  writer.emit_load_64(gpr(2), sp, Immediate{24});
  writer.emit_cmp_ge_i32(gpr(20), gpr(3), gpr(4));
  writer.emit_jnz(gpr(20), builder.label_target(return_base));

  // Partition around the pivot stored at hi.
  writer.emit_mul_i32_i(gpr(8), gpr(4), Immediate{4});
  writer.emit_add_i64(gpr(9), gpr(2), gpr(8));
  writer.emit_load_32(gpr(5), gpr(9), Immediate{0});
  writer.emit_sub_i32_i(gpr(6), gpr(3), Immediate{1});
  writer.emit_add_i32_i(gpr(7), gpr(3), Immediate{0});

  builder.place_label(partition_loop);
  writer.emit_cmp_ge_i32(gpr(20), gpr(7), gpr(4));
  writer.emit_jnz(gpr(20), builder.label_target(after_partition));

  writer.emit_mul_i32_i(gpr(8), gpr(7), Immediate{4});
  writer.emit_add_i64(gpr(11), gpr(2), gpr(8));
  writer.emit_load_32(gpr(10), gpr(11), Immediate{0});
  writer.emit_cmp_gt_i32(gpr(20), gpr(10), gpr(5));
  writer.emit_jnz(gpr(20), builder.label_target(skip_swap));

  writer.emit_add_i32_i(gpr(6), gpr(6), Immediate{1});
  writer.emit_mul_i32_i(gpr(8), gpr(6), Immediate{4});
  writer.emit_add_i64(gpr(13), gpr(2), gpr(8));
  writer.emit_load_32(gpr(12), gpr(13), Immediate{0});
  writer.emit_store_32(gpr(10), gpr(13), Immediate{0});
  writer.emit_store_32(gpr(12), gpr(11), Immediate{0});

  builder.place_label(skip_swap);
  writer.emit_add_i32_i(gpr(7), gpr(7), Immediate{1});
  writer.emit_jmp(builder.label_target(partition_loop));

  builder.place_label(after_partition);
  // Move the pivot into its final slot and recurse on each side.
  writer.emit_add_i32_i(gpr(14), gpr(6), Immediate{1});
  writer.emit_mul_i32_i(gpr(8), gpr(14), Immediate{4});
  writer.emit_add_i64(gpr(13), gpr(2), gpr(8));
  writer.emit_load_32(gpr(12), gpr(13), Immediate{0});
  writer.emit_mul_i32_i(gpr(8), gpr(4), Immediate{4});
  writer.emit_add_i64(gpr(11), gpr(2), gpr(8));
  writer.emit_load_32(gpr(15), gpr(11), Immediate{0});
  writer.emit_store_32(gpr(15), gpr(13), Immediate{0});
  writer.emit_store_32(gpr(12), gpr(11), Immediate{0});
  writer.emit_store_64(gpr(14), sp, Immediate{16});

  writer.emit_sub_i32_i(gpr(8), gpr(14), Immediate{1});
  writer.emit_cmp_lt_i32(gpr(20), gpr(3), gpr(8));
  writer.emit_jnz(gpr(20), builder.label_target(do_left));
  writer.emit_jmp(builder.label_target(after_left));

  builder.place_label(do_left);
  // Caller cleanup: push base, lo, hi for the left partition.
  writer.emit_sub_i64_i(sp, sp, Immediate{8});
  writer.emit_store_64(gpr(2), sp, Immediate{0});
  writer.emit_sub_i64_i(sp, sp, Immediate{8});
  writer.emit_store_64(gpr(3), sp, Immediate{0});
  writer.emit_sub_i64_i(sp, sp, Immediate{8});
  writer.emit_store_64(gpr(8), sp, Immediate{0});
  writer.emit_call(builder.label_target(quicksort));
  writer.emit_add_i64_i(sp, sp, Immediate{24});

  builder.place_label(after_left);
  writer.emit_load_64(gpr(14), sp, Immediate{16});
  writer.emit_add_i32_i(gpr(8), gpr(14), Immediate{1});
  writer.emit_load_64(gpr(4), sp, Immediate{8});
  writer.emit_cmp_lt_i32(gpr(20), gpr(8), gpr(4));
  writer.emit_jnz(gpr(20), builder.label_target(do_right));
  writer.emit_jmp(builder.label_target(return_base));

  builder.place_label(do_right);
  // Caller cleanup again for the right partition.
  writer.emit_load_64(gpr(2), sp, Immediate{24});
  writer.emit_sub_i64_i(sp, sp, Immediate{8});
  writer.emit_store_64(gpr(2), sp, Immediate{0});
  writer.emit_sub_i64_i(sp, sp, Immediate{8});
  writer.emit_store_64(gpr(8), sp, Immediate{0});
  writer.emit_sub_i64_i(sp, sp, Immediate{8});
  writer.emit_store_64(gpr(4), sp, Immediate{0});
  writer.emit_call(builder.label_target(quicksort));
  writer.emit_add_i64_i(sp, sp, Immediate{24});

  builder.place_label(return_base);
  // Return the base pointer in gpr(1).
  writer.emit_load_64(gpr(1), sp, Immediate{24});
  writer.emit_ret();

  auto module = builder.build();
  store_constant(
    module.constant_data,
    module.constant_table,
    Constant{1},
    input
  );

  auto vm = benson::Virtual_machine{};
  vm.load(module);

  vm.run();

  auto const array_pointer = vm.get_register_value<Pointer>(gpr(1)).decode();
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
  using benson::bytecode::gpr;
  using benson::bytecode::Immediate;
  using benson::bytecode::Module_builder;
  using benson::bytecode::sp;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  auto const loop = builder.make_label();

  writer.emit_lookup_k(gpr(4), builder.constant(std::int32_t{5}));
  writer.emit_lookup_k(gpr(5), builder.constant(std::int32_t{1}));
  writer.emit_load_32(gpr(1), gpr(4), Immediate{0});
  writer.emit_load_32(gpr(2), gpr(5), Immediate{0});
  builder.place_label(loop);
  writer.emit_mul_i32(gpr(2), gpr(2), gpr(1));
  writer.emit_sub_i32_i(gpr(1), gpr(1), Immediate{1});
  writer.emit_cmp_gt_i32_i(gpr(3), gpr(1), Immediate{1});
  writer.emit_jnz(gpr(3), builder.label_target(loop));
  writer.emit_exit();

  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);

  vm.run();

  CHECK(vm.get_register_value<std::int32_t>(gpr(1)) == 1);
  CHECK(vm.get_register_value<std::int32_t>(gpr(2)) == 120);
  CHECK(!vm.get_register_value<bool>(gpr(3)));
  CHECK(module.constant_table.size() == 2);
}

TEST_CASE("Virtual_machine runs Newton square root program with jnz_i loop")
{
  using benson::bytecode::gpr;
  using benson::bytecode::Immediate;
  using benson::bytecode::Module_builder;
  using benson::bytecode::sp;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  auto const loop = builder.make_label();

  writer.emit_lookup_k(gpr(10), builder.constant(9.0F));
  writer.emit_lookup_k(gpr(11), builder.constant(1.0F));
  writer.emit_load_32(gpr(1), gpr(10), Immediate{0});
  writer.emit_load_32(gpr(2), gpr(11), Immediate{0});
  writer.emit_add_i32_k(gpr(3), gpr(3), builder.constant(5));
  builder.place_label(loop);
  writer.emit_div_f32(gpr(4), gpr(1), gpr(2));
  writer.emit_add_f32(gpr(5), gpr(2), gpr(4));
  writer.emit_mul_f32_k(gpr(2), gpr(5), builder.constant(0.5F));
  writer.emit_sub_i32_i(gpr(3), gpr(3), Immediate{1});
  writer.emit_cmp_gt_i32_i(gpr(6), gpr(3), Immediate{0});
  writer.emit_jnz(gpr(6), builder.label_target(loop));
  writer.emit_exit();

  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);

  vm.run();

  auto const estimate = vm.get_register_value<float>(gpr(2));
  CHECK(estimate > 2.999F);
  CHECK(estimate < 3.001F);
  CHECK(vm.get_register_value<std::int32_t>(gpr(3)) == 0);
  CHECK(!vm.get_register_value<bool>(gpr(6)));
  CHECK(module.constant_table.size() == 4);
}

TEST_CASE("Virtual_machine runs stack RPN program with jnz_i dispatch loop")
{
  using benson::Address_space;
  using benson::Pointer;
  using benson::bytecode::Constant;
  using benson::bytecode::gpr;
  using benson::bytecode::Immediate;
  using benson::bytecode::Module_builder;
  using benson::bytecode::sp;

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
  writer.emit_lookup_k(gpr(10), expression_k);
  // load the length of the RPN expression
  writer.emit_load_32(gpr(1), gpr(10), Immediate{0});
  // calculate the pointer to the start of the RPN expression
  writer.emit_add_i64_i(gpr(11), gpr(10), Immediate{4});

  builder.place_label(loop);

  // compare the "length" of the RPN expression with zero
  writer.emit_cmp_eq_i32_i(gpr(2), gpr(1), Immediate{0});
  // if the length is zero, we're done
  writer.emit_jnz(gpr(2), builder.label_target(done));

  // load the operand/operator tag
  writer.emit_load_32(gpr(3), gpr(11), Immediate{0});
  // load the operand/operator value
  writer.emit_load_32(gpr(4), gpr(11), Immediate{4});

  // if tag == 0, it's an operand
  writer.emit_cmp_eq_i32_i(gpr(2), gpr(3), Immediate{0});
  writer.emit_jnz(gpr(2), builder.label_target(operand));

  // else if value == 1, it's an addition
  writer.emit_cmp_eq_i32_i(gpr(2), gpr(4), Immediate{1});
  writer.emit_jnz(gpr(2), builder.label_target(add));

  // else if value == 2, it's a subtraction
  writer.emit_cmp_eq_i32_i(gpr(2), gpr(4), Immediate{2});
  writer.emit_jnz(gpr(2), builder.label_target(sub));

  // else if value == 3, it's a division
  writer.emit_cmp_eq_i32_i(gpr(2), gpr(4), Immediate{3});
  writer.emit_jnz(gpr(2), builder.label_target(mul));

  // else it's a "done" instruction
  writer.emit_jmp(builder.label_target(done));

  // push the operand value onto the stack
  builder.place_label(operand);
  writer.emit_sub_i64_i(sp, sp, Immediate{8});
  writer.emit_store_64(gpr(4), sp, Immediate{0});
  writer.emit_jmp(builder.label_target(advance));

  builder.place_label(add);
  writer.emit_load_64(gpr(5), sp, Immediate{0});
  writer.emit_load_64(gpr(4), sp, Immediate{8});
  writer.emit_add_i64_i(sp, sp, Immediate{8});
  writer.emit_add_i32(gpr(6), gpr(4), gpr(5));
  writer.emit_store_64(gpr(6), sp, Immediate{0});
  writer.emit_jmp(builder.label_target(advance));

  builder.place_label(sub);
  writer.emit_load_64(gpr(5), sp, Immediate{0});
  writer.emit_load_64(gpr(4), sp, Immediate{8});
  writer.emit_add_i64_i(sp, sp, Immediate{8});
  writer.emit_sub_i32(gpr(6), gpr(4), gpr(5));
  writer.emit_store_64(gpr(6), sp, Immediate{0});
  writer.emit_jmp(builder.label_target(advance));

  builder.place_label(mul);
  writer.emit_load_64(gpr(5), sp, Immediate{0});
  writer.emit_load_64(gpr(4), sp, Immediate{8});
  writer.emit_add_i64_i(sp, sp, Immediate{8});
  writer.emit_mul_i32(gpr(6), gpr(4), gpr(5));
  writer.emit_store_64(gpr(6), sp, Immediate{0});
  writer.emit_jmp(builder.label_target(advance));

  // advance the RPN expression pointer and decrement the length
  builder.place_label(advance);
  writer.emit_add_i64_i(gpr(11), gpr(11), Immediate{8});
  writer.emit_sub_i32_i(gpr(1), gpr(1), Immediate{1});
  writer.emit_jmp(builder.label_target(loop));

  // pop top of stack to gpr(9) and exit
  builder.place_label(done);
  writer.emit_load_64(gpr(9), sp, Immediate{0});
  writer.emit_add_i64_i(sp, sp, Immediate{8});
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
  CHECK(vm.get_register_value<std::int32_t>(gpr(9)) == 7);
  CHECK(vm.get_register_value<std::int32_t>(gpr(1)) == 0);
  CHECK(stack_pointer.space == Address_space::stack);
  CHECK(stack_pointer.offset == vm.stack->size());
  CHECK(module.constant_table.size() == 1);
}

TEST_CASE("Virtual_machine runs Module_builder program with inline constants")
{
  using benson::bytecode::gpr;
  using benson::bytecode::Module_builder;
  using benson::bytecode::sp;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  writer.emit_add_i32_k(gpr(1), gpr(1), builder.constant(5));
  writer.emit_mul_f32_k(gpr(2), gpr(2), builder.constant(0.5F));
  writer.emit_add_f32_k(gpr(2), gpr(2), builder.constant(0.5F));
  writer.emit_exit();

  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);
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
  using benson::Address_space;
  using benson::Pointer;
  using benson::bytecode::Immediate;
  using benson::bytecode::Register;

  auto stream = Recording_output_stream{};
  auto writer = benson::bytecode::Bytecode_writer{&stream};
  // offset 0x0101 = 257, which exceeds Immediate max of 127 → wide encoding
  writer.emit_store_16(gpr(1), gpr(3), Immediate{0x0101});
  writer.emit_load_16(gpr(2), gpr(3), Immediate{0x0101});
  writer.emit_exit();
  writer.flush();

  auto vm = benson::Virtual_machine{};
  vm.instruction_pointer = stream.bytes().data();
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
  using benson::bytecode::sp;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  writer.emit_sx_8(gpr(1), gpr(2));
  writer.emit_exit();
  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);
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
  using benson::bytecode::sp;

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  writer.emit_cmp_eq_i32(gpr(1), gpr(2), gpr(3));
  writer.emit_cmp_eq_i64_i(gpr(4), gpr(5), Immediate{9});
  writer.emit_cmp_ne_f32(gpr(6), gpr(7), gpr(8));
  writer.emit_cmp_ne_f64_k(gpr(9), gpr(10), builder.constant(4.0));
  writer.emit_cmp_lt_i32_i(gpr(11), gpr(12), Immediate{8});
  writer.emit_cmp_lt_i64_k(gpr(13), gpr(14), builder.constant(std::int64_t{7}));
  writer.emit_cmp_le_f32_k(gpr(15), gpr(16), builder.constant(3.5F));
  writer.emit_cmp_le_f64(gpr(17), gpr(18), gpr(19));
  writer.emit_cmp_gt_i32_k(gpr(20), gpr(21), builder.constant(1));
  writer.emit_cmp_gt_i64(gpr(22), gpr(23), gpr(24));
  writer.emit_cmp_ge_f32(gpr(25), gpr(26), gpr(27));
  writer.emit_cmp_ge_f64_k(gpr(28), gpr(29), builder.constant(6.0));
  writer.emit_exit();
  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);
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
  writer.emit_cmp_eq_i32_i(gpr(1), gpr(2), Immediate{0x0101});
  writer.emit_cmp_ne_i64_k(gpr(3), gpr(4), Constant{0x0201});
  writer.emit_cmp_lt_f32_k(gpr(5), gpr(6), Constant{0x0203});
  writer.emit_cmp_le_f64_k(gpr(7), gpr(8), Constant{0x0204});
  writer.emit_cmp_gt_i32_k(gpr(9), gpr(10), Constant{0x0201});
  writer.emit_cmp_ge_i64_i(gpr(11), gpr(12), Immediate{0x0100});
  writer.emit_cmp_ge_f32_k(gpr(13), gpr(14), Constant{0x0205});
  writer.emit_cmp_eq_f64_k(gpr(15), gpr(16), Constant{0x0206});
  writer.emit_exit();
  writer.flush();

  auto module = benson::bytecode::Module{};
  module.constant_data = std::move(constant_memory);
  module.constant_table = std::move(constant_table);

  auto vm = benson::Virtual_machine{};
  vm.module = &module;
  vm.instruction_pointer = stream.bytes().data();
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

TEST_CASE("Virtual_machine::call invokes an i32 add function")
{
  using benson::bytecode::Immediate;
  using benson::bytecode::Module_builder;
  using enum benson::bytecode::Scalar_type;

  auto spellings = benson::Spelling_table{};
  auto const add = spellings.intern("add");

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  auto const add_label = builder.make_label();
  builder.place_function(add_label, add, {int32, int32}, int32);
  writer.emit_load_32(gpr(2), sp, Immediate{8});
  writer.emit_load_32(gpr(3), sp, Immediate{12});
  writer.emit_add_i32(gpr(1), gpr(2), gpr(3));
  writer.emit_ret();
  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);

  auto const args = std::array<benson::Virtual_machine::Scalar, 2>{
    std::int32_t{40},
    std::int32_t{2},
  };
  auto const result = vm.call(add, args);
  REQUIRE(result);
  REQUIRE(std::holds_alternative<std::int32_t>(*result));
  CHECK(std::get<std::int32_t>(*result) == 42);
}

TEST_CASE("Virtual_machine::call round-trips a float through gpr(1)")
{
  using benson::bytecode::Immediate;
  using benson::bytecode::Module_builder;
  using enum benson::bytecode::Scalar_type;

  auto spellings = benson::Spelling_table{};
  auto const identity = spellings.intern("identity");

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  auto const identity_label = builder.make_label();
  builder.place_function(identity_label, identity, {float_}, float_);
  writer.emit_load_32(gpr(1), sp, Immediate{8});
  writer.emit_ret();
  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);

  auto const args =
    std::array<benson::Virtual_machine::Scalar, 1>{float{3.14159F}};
  auto const result = vm.call(identity, args);
  REQUIRE(result);
  REQUIRE(std::holds_alternative<float>(*result));
  CHECK(std::get<float>(*result) == 3.14159F);
}

TEST_CASE("Virtual_machine::call returns empty optional for void functions")
{
  using benson::bytecode::Module_builder;
  using enum benson::bytecode::Scalar_type;

  auto spellings = benson::Spelling_table{};
  auto const noop = spellings.intern("noop");

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  auto const noop_label = builder.make_label();
  builder.place_function(noop_label, noop, {}, void_);
  writer.emit_ret();
  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);

  auto const result = vm.call(noop, {});
  CHECK(!result);
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
  auto const inc_label = builder.make_label();
  builder.place_function(inc_label, inc, {int32}, int32);
  writer.emit_load_32(gpr(1), sp, Immediate{8});
  writer.emit_add_i32_i(gpr(1), gpr(1), Immediate{1});
  writer.emit_ret();
  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);

  auto const args1 =
    std::array<benson::Virtual_machine::Scalar, 1>{std::int32_t{10}};
  auto const r1 = vm.call(inc, args1);
  REQUIRE(r1);
  CHECK(std::get<std::int32_t>(*r1) == 11);

  auto const args2 =
    std::array<benson::Virtual_machine::Scalar, 1>{std::int32_t{99}};
  auto const r2 = vm.call(inc, args2);
  REQUIRE(r2);
  CHECK(std::get<std::int32_t>(*r2) == 100);
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
  auto const known_label = builder.make_label();
  builder.place_function(known_label, known, {int32}, int32);
  writer.emit_ret();
  auto const module = builder.build();

  auto vm = benson::Virtual_machine{};
  vm.load(module);

  CHECK_THROWS_AS(
    vm.call(unknown, {}),
    benson::Virtual_machine::Unknown_function_error
  );
  CHECK_THROWS_AS(
    vm.call(known, {}),
    benson::Virtual_machine::Argument_count_error
  );
  auto const wrong_type =
    std::array<benson::Virtual_machine::Scalar, 1>{std::int64_t{0}};
  CHECK_THROWS_AS(
    vm.call(known, wrong_type),
    benson::Virtual_machine::Argument_type_error
  );
}
