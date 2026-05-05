#include <array>
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

#include "bytecode/module_builder.h"
#include "spelling/spelling.h"
#include "vm/pointer.h"
#include "vm/vm.h"

namespace
{
  using benson::bytecode::gpr;

  template <typename T>
  void store_constant(
    std::vector<std::byte> *constant_memory,
    std::vector<std::ptrdiff_t> *constant_table,
    benson::bytecode::Constant index,
    T value
  )
  {
    if (constant_table->size() <= index.value)
    {
      constant_table->resize(index.value + 1);
    }
    (*constant_table)[index.value] =
      static_cast<std::ptrdiff_t>(constant_memory->size());
    auto const start = constant_memory->size();
    constant_memory->resize(start + sizeof(T));
    std::memcpy(constant_memory->data() + start, &value, sizeof(T));
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
  vm.registers.resize(4);
  vm.set_register_value(gpr(0), value);

  CHECK(vm.get_register_value<TestType>(gpr(0)) == value);
}

TEST_CASE("Virtual_machine keeps exact floating-point register bit pattern")
{
  auto vm = benson::vm::Virtual_machine{};
  vm.registers.resize(1);
  vm.set_register_value(gpr(0), -0.0F);

  CHECK(
    std::bit_cast<std::uint32_t>(vm.get_register_value<float>(gpr(0))) ==
    std::bit_cast<std::uint32_t>(-0.0F)
  );
}

TEST_CASE("Virtual_machine runs arithmetic and comparison instructions")
{
  using benson::bytecode::Immediate;
  using benson::bytecode::Module_builder;
  using enum benson::bytecode::Scalar_type;

  auto spellings = benson::Spelling_table{};
  auto const entry = spellings.intern("entry");

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  auto const entry_label = builder.make_label();
  auto const entry_index =
    builder.place_function(entry_label, entry, {}, void_, 3);
  (void) entry_index;
  writer.emit_mov_i(gpr(0), Immediate{40});
  writer.emit_add_i32_i(gpr(1), gpr(0), Immediate{2});
  writer.emit_cmp_eq_i32_i(gpr(2), gpr(1), Immediate{42});
  writer.emit_ret_void();
  auto const module = builder.build();

  auto vm = benson::vm::Virtual_machine{};
  vm.load(module);

  auto const result = vm.call(entry, {});

  CHECK(result.type() == void_);
  CHECK(vm.get_register_value<std::int32_t>(gpr(1)) == 42);
  CHECK(vm.get_register_value<bool>(gpr(2)));
}

TEST_CASE("Virtual_machine uses constant and stack address spaces")
{
  using benson::bytecode::Constant;
  using benson::bytecode::Immediate;
  using benson::bytecode::Module_builder;
  using benson::vm::Address_space;
  using benson::vm::Pointer;
  using enum benson::bytecode::Scalar_type;

  auto spellings = benson::Spelling_table{};
  auto const entry = spellings.intern("entry");

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  auto const entry_label = builder.make_label();
  auto const entry_index =
    builder.place_function(entry_label, entry, {}, void_, 5);
  (void) entry_index;
  writer.emit_lookup_k(gpr(0), Constant{0});
  writer.emit_load_32(gpr(1), gpr(0), Immediate{0});
  writer.emit_mov_i(gpr(4), Immediate{8});
  writer.emit_push_sp_i(Immediate{4});
  writer.emit_push_sp(gpr(4));
  writer.emit_mov_sp_i(gpr(2), Immediate{0});
  writer.emit_store_32(gpr(1), gpr(2), Immediate{0});
  writer.emit_load_sp_32(gpr(3), Immediate{0});
  writer.emit_ret_void();
  auto module = builder.build();
  store_constant(
    &module.constant_data,
    &module.constant_table,
    Constant{0},
    std::int32_t{42}
  );

  auto vm = benson::vm::Virtual_machine{};
  vm.load(module);

  auto const result = vm.call(entry, {});

  CHECK(result.type() == void_);
  auto const stack_pointer =
    Pointer{static_cast<std::uint64_t>(vm.get_register_value<Pointer>(gpr(2)))}
      .decode();
  CHECK(vm.get_register_value<std::int32_t>(gpr(3)) == 42);
  CHECK(stack_pointer.space == Address_space::stack);
  CHECK(
    stack_pointer.offset == static_cast<std::uint64_t>(vm.stack->size() - 12)
  );
}

TEST_CASE("Virtual_machine indexed call slides register window")
{
  using benson::bytecode::Immediate;
  using benson::bytecode::Module_builder;
  using enum benson::bytecode::Scalar_type;

  auto spellings = benson::Spelling_table{};
  auto const add = spellings.intern("add");
  auto const entry = spellings.intern("entry");

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  auto const add_label = builder.make_label();
  auto const add_index =
    builder.declare_function(add, {int32, int32}, int32, 3);
  auto const entry_label = builder.make_label();
  auto const entry_index =
    builder.place_function(entry_label, entry, {}, void_, 12);
  (void) entry_index;

  writer.emit_mov_i(gpr(10), Immediate{40});
  writer.emit_mov_i(gpr(11), Immediate{2});
  writer.emit_call_i(add_index, gpr(10), gpr(0));
  writer.emit_ret_void();

  builder.place_function(add_index, add_label);
  writer.emit_add_i32(gpr(2), gpr(0), gpr(1));
  writer.emit_ret(gpr(2));

  auto const module = builder.build();

  auto vm = benson::vm::Virtual_machine{};
  vm.load(module);

  auto const result = vm.call(entry, {});

  CHECK(result.type() == void_);
  CHECK(vm.get_register_value<std::int32_t>(gpr(0)) == 42);
  CHECK(vm.get_register_value<std::int32_t>(gpr(10)) == 40);
  CHECK(vm.get_register_value<std::int32_t>(gpr(11)) == 2);
}

TEST_CASE("Virtual_machine void call restores caller frame and stack pointer")
{
  using benson::bytecode::Immediate;
  using benson::bytecode::Module_builder;
  using enum benson::bytecode::Scalar_type;

  auto spellings = benson::Spelling_table{};
  auto const clobber = spellings.intern("clobber");
  auto const entry = spellings.intern("entry");

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  auto const clobber_label = builder.make_label();
  auto const clobber_index =
    builder.declare_function(clobber, {int32}, void_, 2);
  auto const entry_label = builder.make_label();
  auto const entry_index =
    builder.place_function(entry_label, entry, {}, void_, 11);
  (void) entry_index;

  writer.emit_mov_i(gpr(5), Immediate{123});
  writer.emit_mov_i(gpr(10), Immediate{456});
  writer.emit_call_void_i(clobber_index, gpr(10));
  writer.emit_ret_void();

  builder.place_function(clobber_index, clobber_label);
  writer.emit_push_sp_i(Immediate{16});
  writer.emit_mov_i(gpr(0), Immediate{99});
  writer.emit_ret_void();

  auto const module = builder.build();

  auto vm = benson::vm::Virtual_machine{};
  vm.load(module);
  auto const initial_sp = vm.stack_pointer;

  auto const result = vm.call(entry, {});

  CHECK(result.type() == void_);
  CHECK(vm.get_register_value<std::int32_t>(gpr(5)) == 123);
  CHECK(vm.stack_pointer == initial_sp);
}

TEST_CASE("Virtual_machine::call invokes an i32 add function")
{
  using benson::bytecode::Module_builder;
  using enum benson::bytecode::Scalar_type;

  auto spellings = benson::Spelling_table{};
  auto const add = spellings.intern("add");

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  auto const add_label = builder.make_label();
  auto const add_index =
    builder.place_function(add_label, add, {int32, int32}, int32, 3);
  (void) add_index;
  writer.emit_add_i32(gpr(2), gpr(0), gpr(1));
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

TEST_CASE("Virtual_machine::call returns void for void functions")
{
  using benson::bytecode::Module_builder;
  using enum benson::bytecode::Scalar_type;

  auto spellings = benson::Spelling_table{};
  auto const noop = spellings.intern("noop");

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  auto const noop_label = builder.make_label();
  auto const noop_index =
    builder.place_function(noop_label, noop, {}, void_, 0);
  (void) noop_index;
  writer.emit_ret_void();
  auto const module = builder.build();

  auto vm = benson::vm::Virtual_machine{};
  vm.load(module);

  auto const result = vm.call(noop, {});

  CHECK(result.type() == void_);
}

TEST_CASE(
  "Virtual_machine::call restores execution state after bytecode throws"
)
{
  using benson::bytecode::Constant;
  using benson::bytecode::Immediate;
  using benson::bytecode::Module_builder;
  using enum benson::bytecode::Scalar_type;

  auto spellings = benson::Spelling_table{};
  auto const fail = spellings.intern("fail");
  auto const ok = spellings.intern("ok");

  auto builder = Module_builder{};
  auto &writer = builder.writer();
  auto const fail_label = builder.make_label();
  auto const fail_index =
    builder.place_function(fail_label, fail, {}, void_, 2);
  (void) fail_index;
  writer.emit_lookup_k(gpr(0), Constant{0});
  writer.emit_mov_i(gpr(1), Immediate{42});
  writer.emit_push_sp_i(Immediate{16});
  writer.emit_store_32(gpr(1), gpr(0), Immediate{0});
  writer.emit_ret_void();

  auto const ok_label = builder.make_label();
  auto const ok_index = builder.place_function(ok_label, ok, {}, int32, 1);
  (void) ok_index;
  writer.emit_mov_i(gpr(0), Immediate{7});
  writer.emit_ret(gpr(0));

  auto module = builder.build();
  store_constant(
    &module.constant_data,
    &module.constant_table,
    Constant{0},
    std::int32_t{0}
  );

  auto vm = benson::vm::Virtual_machine{};
  vm.load(module);
  auto const initial_ip = vm.instruction_pointer;
  auto const initial_sp = vm.stack_pointer;
  auto const initial_call_stack_size = vm.call_stack.size();
  vm.base_register = 3;

  CHECK_THROWS_AS(vm.call(fail, {}), std::runtime_error);

  CHECK(vm.instruction_pointer == initial_ip);
  CHECK(vm.base_register == 3);
  CHECK(vm.stack_pointer == initial_sp);
  CHECK(vm.call_stack.size() == initial_call_stack_size);

  vm.base_register = 0;
  auto const result = vm.call(ok, {});

  REQUIRE(result.type() == int32);
  CHECK(result.as<std::int32_t>() == 7);
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
  auto const known_index =
    builder.place_function(known_label, known, {int32}, int32, 1);
  (void) known_index;
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
